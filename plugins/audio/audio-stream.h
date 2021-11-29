/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd. 
 * kiran-cc-daemon is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2. 
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2 
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, 
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, 
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.  
 * See the Mulan PSL v2 for more details.  
 * 
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#pragma once

#include "lib/base/base.h"

#include <audio_stream_dbus_stub.h>
#include "plugins/audio/pulse/pulse-stream.h"

namespace Kiran
{
class AudioStream : public SessionDaemon::Audio::StreamStub
{
public:
    AudioStream(std::shared_ptr<PulseStream> stream);
    virtual ~AudioStream();

    bool init(const std::string &object_path_prefix);

    std::string get_object_path() { return this->object_path_; };

    virtual guint32 index_get() { return this->stream_->get_index(); };
    virtual Glib::ustring name_get() { return this->stream_->get_name(); };
    virtual bool mute_get() { return this->stream_->get_mute(); };
    virtual double volume_get();
    virtual guint32 state_get() { return this->stream_->get_flags(); };

protected:
    // 设置音量
    virtual void SetVolume(double volume, MethodInvocation &invocation);
    // 设置静音
    virtual void SetMute(bool mute, MethodInvocation &invocation);
    // 获取属性
    virtual void GetProperty(const Glib::ustring &key, MethodInvocation &invocation);

    // 如果属性只对外部可读，则直接返回true
    virtual bool index_setHandler(guint32 value) { return true; };
    virtual bool name_setHandler(const Glib::ustring &value) { return true; };
    virtual bool mute_setHandler(bool value) { return this->stream_->set_mute(value); };
    virtual bool volume_setHandler(double value);
    virtual bool state_setHandler(guint32 value) { return true; };

private:
    bool dbus_register();
    void dbus_unregister();

    void on_node_info_changed_cb(PulseNodeField field);

private:
    std::shared_ptr<PulseStream> stream_;

    Glib::RefPtr<Gio::DBus::Connection> dbus_connect_;
    uint32_t object_register_id_;
    Glib::DBusObjectPathString object_path_;
};
}  // namespace Kiran