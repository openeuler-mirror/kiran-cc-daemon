/**
 * @file          /kiran-cc-daemon/plugins/power/idle/power-idle-xalarm.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "plugins/power/idle/power-idle-xalarm.h"

#include <gdk/gdkx.h>

#include <cinttypes>

namespace Kiran
{
XAlarmInfo::XAlarmInfo(XAlarmType t) : type(t)
{
    this->timeout.hi = 0;
    this->timeout.lo = 0;
    this->xalarm_id = None;
}

PowerIdleXAlarm::PowerIdleXAlarm() : xdisplay_(NULL),
                                     sync_event_base_(0),
                                     sync_error_base_(0),
                                     idle_counter_(None),
                                     added_event_filter_(false)
{
}

PowerIdleXAlarm::~PowerIdleXAlarm()
{
    if (this->added_event_filter_)
    {
        gdk_window_remove_filter(NULL, &PowerIdleXAlarm::on_event_filter_cb, this);
    }

    // 销毁所有报警器
    for (auto &xalarm : this->xalarms_)
    {
        if (xalarm->xalarm_id)
        {
            XSyncDestroyAlarm(this->xdisplay_, xalarm->xalarm_id);
        }
    }
}

void PowerIdleXAlarm::init()
{
    int32_t ncounters;
    auto display = Gdk::Display::get_default();

    this->xdisplay_ = GDK_DISPLAY_XDISPLAY(display->gobj());

    if (!this->xdisplay_)
    {
        LOG_WARNING("Not found xdisplay.");
        return;
    }

    if (!XSyncQueryExtension(this->xdisplay_, &this->sync_event_base_, &this->sync_error_base_))
    {
        LOG_WARNING("No Sync extension.");
        return;
    }

    auto counters = XSyncListSystemCounters(this->xdisplay_, &ncounters);
    for (int32_t i = 0; i < ncounters; i++)
    {
        if (strcmp(counters[i].name, "IDLETIME") == 0)
        {
            this->idle_counter_ = counters[i].counter;
            break;
        }
    }
    XSyncFreeSystemCounterList(counters);

    if (!this->idle_counter_)
    {
        LOG_WARNING("No idle counter.");
        return;
    }

    gdk_window_add_filter(NULL, &PowerIdleXAlarm::on_event_filter_cb, this);
    this->added_event_filter_ = true;

    auto xalarm = std::make_shared<XAlarmInfo>(XAlarmType::XALARM_TYPE_RESET);
    this->add(xalarm);

    return;
}

bool PowerIdleXAlarm::set(XAlarmType type, uint32_t timeout)
{
    SETTINGS_PROFILE("type: %d, timeout: %d.", type, timeout);
    RETURN_VAL_IF_TRUE(type <= 0 || type >= XAlarmType::XALARM_TYPE_LAST || timeout == 0, false);

    auto xalarm = this->find_xalarm_by_type(type);
    if (!xalarm)
    {
        xalarm = std::make_shared<XAlarmInfo>(type);
        this->add(xalarm);
    }

    XSyncIntToValue(&xalarm->timeout, (int32_t)timeout);
    this->register_xalarm_by_xsync(xalarm, XSyncPositiveTransition);

    return true;
}

bool PowerIdleXAlarm::unset(XAlarmType type)
{
    SETTINGS_PROFILE("type: %d.", type);

    auto xalarm = this->find_xalarm_by_type(type);
    RETURN_VAL_IF_FALSE(xalarm, false);

    if (xalarm->xalarm_id)
    {
        XSyncDestroyAlarm(this->xdisplay_, xalarm->xalarm_id);
    }
    return this->remove(xalarm);
}

int64_t PowerIdleXAlarm::get_xidle_time()
{
    RETURN_VAL_IF_TRUE(this->idle_counter_ == None, 0);
    XSyncValue value;
    XSyncQueryCounter(this->xdisplay_, this->idle_counter_, &value);
    return this->xsyncvalue_to_int64(value);
}

bool PowerIdleXAlarm::add(std::shared_ptr<XAlarmInfo> xalarm)
{
    RETURN_VAL_IF_FALSE(xalarm, false);

    if (this->find_xalarm_by_type(xalarm->type))
    {
        LOG_WARNING("The xalarm type %d is already exists.", xalarm->type);
        return false;
    }
    this->xalarms_.push_back(xalarm);
    return true;
}

bool PowerIdleXAlarm::remove(std::shared_ptr<XAlarmInfo> xalarm)
{
    RETURN_VAL_IF_FALSE(xalarm, false);

    for (auto iter = this->xalarms_.begin(); iter != this->xalarms_.end(); ++iter)
    {
        if ((*iter)->xalarm_id == xalarm->xalarm_id)
        {
            this->xalarms_.erase(iter);
            return true;
        }
    }
    return false;
}

void PowerIdleXAlarm::reset_all_xalarm()
{
    SETTINGS_PROFILE("");

    auto reset_xalarm = this->find_xalarm_by_type(XAlarmType::XALARM_TYPE_RESET);
    RETURN_IF_FALSE(reset_xalarm || reset_xalarm->xalarm_id == None);

    for (auto &xalarm : this->xalarms_)
    {
        if (xalarm->type == XAlarmType::XALARM_TYPE_RESET)
        {
            this->unregister_xalarm_by_xsync(xalarm);
        }
        else
        {
            this->register_xalarm_by_xsync(xalarm, XSyncPositiveTransition);
        }
    }

    this->alarm_reset_.emit();
}

void PowerIdleXAlarm::register_xalarm_by_xsync(std::shared_ptr<XAlarmInfo> xalarm, XSyncTestType test_type)
{
    SETTINGS_PROFILE("type: %d, test_type: %d.",
                     xalarm ? xalarm->type : XAlarmType::XALARM_TYPE_LAST,
                     test_type);

    RETURN_IF_TRUE(xalarm == nullptr || this->idle_counter_ == None);

    XSyncAlarmAttributes attr;
    XSyncValue delta;
    unsigned int flags;

    // 这里设置的delta为0，所以报警器只会报警一次，报警完后报警器会切换到Inactive，需要重新调用XSyncChangeAlarm才能激活
    XSyncIntToValue(&delta, 0);

    attr.trigger.counter = this->idle_counter_;
    attr.trigger.value_type = XSyncAbsolute;
    attr.trigger.test_type = test_type;
    attr.trigger.wait_value = xalarm->timeout;
    attr.delta = delta;

    flags = XSyncCACounter | XSyncCAValueType | XSyncCATestType | XSyncCAValue | XSyncCADelta;

    if (xalarm->xalarm_id)
    {
        XSyncChangeAlarm(this->xdisplay_, xalarm->xalarm_id, flags, &attr);
    }
    else
    {
        xalarm->xalarm_id = XSyncCreateAlarm(this->xdisplay_, flags, &attr);
    }

    return;
}

void PowerIdleXAlarm::unregister_xalarm_by_xsync(std::shared_ptr<XAlarmInfo> xalarm)
{
    SETTINGS_PROFILE("type: %d", xalarm ? xalarm->type : XAlarmType::XALARM_TYPE_LAST);

    RETURN_IF_TRUE(this->xdisplay_ == NULL || (!xalarm) || xalarm->xalarm_id == None);

    XSyncDestroyAlarm(this->xdisplay_, xalarm->xalarm_id);
    xalarm->xalarm_id = None;
    return;
}

std::shared_ptr<XAlarmInfo> PowerIdleXAlarm::find_xalarm_by_type(XAlarmType type)
{
    SETTINGS_PROFILE("type: %d", type);

    for (auto &xalarm : this->xalarms_)
    {
        if (xalarm->type == type)
        {
            return xalarm;
        }
    }
    return nullptr;
}

std::shared_ptr<XAlarmInfo> PowerIdleXAlarm::find_xalarm_by_id(XSyncAlarm xalarm_id)
{
    SETTINGS_PROFILE("xalarm_id: %d", (int32_t)xalarm_id);

    for (auto &xalarm : this->xalarms_)
    {
        if (xalarm->xalarm_id == xalarm_id)
        {
            return xalarm;
        }
    }
    return nullptr;
}

int64_t PowerIdleXAlarm::xsyncvalue_to_int64(XSyncValue value)
{
    return ((int64_t)XSyncValueHigh32(value)) << 32 | (int64_t)XSyncValueLow32(value);
}

GdkFilterReturn PowerIdleXAlarm::on_event_filter_cb(GdkXEvent *gdkxevent, GdkEvent *event, gpointer data)
{
    XEvent *xevent = (XEvent *)gdkxevent;
    PowerIdleXAlarm *self = (PowerIdleXAlarm *)data;
    XSyncValue add;
    int overflow = 0;

    RETURN_VAL_IF_TRUE(xevent->type != self->sync_event_base_ + XSyncAlarmNotify, GDK_FILTER_CONTINUE);

    auto alarm_event = (XSyncAlarmNotifyEvent *)xevent;
    auto xalarm = self->find_xalarm_by_id(alarm_event->alarm);
    RETURN_VAL_IF_FALSE(xalarm, GDK_FILTER_CONTINUE);

    /* 当收到非重置报警器的Alarm事件时，说明非重置报警器设置的空闲超时时间已到，这时报警器进入了Inactive状态，
    为了让报警器下次能继续工作（报警），这里向X注册一个重置报警器，当设备进入非空闲状态时，会收到重置报警器的Alarm事件。
    当收到重置报警器的Alarm事件时，重新向X注册非重置报警器，让所有非重置报警器的状态变为Active状态，同时取消注册重置报警器。*/

    LOG_DEBUG("Receive alarm signal. type: %" PRId64 ", timeout: %d, xalarm id: %d, counter value: %" PRId64 ", alarm value: %" PRId64 ", idle counter value: %" PRId64 ".",
              xalarm->type,
              self->xsyncvalue_to_int64(xalarm->timeout),
              (int32_t)xalarm->xalarm_id,
              self->xsyncvalue_to_int64(alarm_event->counter_value),
              self->xsyncvalue_to_int64(alarm_event->alarm_value),
              self->get_xidle_time());

    if (xalarm->type != XAlarmType::XALARM_TYPE_RESET)
    {
        self->alarm_triggered_.emit(xalarm);
        auto reset_xalarm = self->find_xalarm_by_type(XAlarmType::XALARM_TYPE_RESET);
        if (reset_xalarm && reset_xalarm->xalarm_id == None)
        {
            XSyncIntToValue(&add, -1);
            XSyncValueAdd(&reset_xalarm->timeout, alarm_event->counter_value, add, &overflow);
            self->register_xalarm_by_xsync(reset_xalarm, XSyncNegativeTransition);
        }

        return GDK_FILTER_CONTINUE;
    }
    else
    {
        self->reset_all_xalarm();
        return GDK_FILTER_REMOVE;
    }
}
}  // namespace Kiran