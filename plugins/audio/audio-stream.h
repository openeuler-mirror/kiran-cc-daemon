/**
 * @file          /kiran-cc-daemon/plugins/audio/audio-stream.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
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
    virtual Glib::ustring application_name_get() { return this->stream_->get_application_name(); };
    virtual Glib::ustring icon_name_get() { return this->stream_->get_icon_name(); };
    virtual guint32 state_get() { return this->stream_->get_flags(); };

protected:
    virtual void SetVolume(double volume, MethodInvocation &invocation);
    virtual void SetMute(bool mute, MethodInvocation &invocation);

    // 如果属性只对外部可读，则直接返回true
    virtual bool index_setHandler(guint32 value) { return true; };
    virtual bool name_setHandler(const Glib::ustring &value) { return true; };
    virtual bool mute_setHandler(bool value) { return this->stream_->set_mute(value); };
    virtual bool volume_setHandler(double value);
    virtual bool application_name_setHandler(const Glib::ustring &value) { return true; };
    virtual bool icon_name_setHandler(const Glib::ustring &value) { return true; };
    virtual bool state_setHandler(guint32 value) { return true; };

private:
    bool dbus_register();
    void dbus_unregister();

    void on_node_info_changed_cb(PulseNodeField field);
    void on_icon_name_changed_cb(const std::string &icon_name);

private:
    std::shared_ptr<PulseStream> stream_;

    Glib::RefPtr<Gio::DBus::Connection> dbus_connect_;
    uint32_t object_register_id_;
    Glib::DBusObjectPathString object_path_;
};
}  // namespace Kiran