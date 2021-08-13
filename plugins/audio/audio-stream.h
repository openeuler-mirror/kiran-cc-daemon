/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http: //www.gnu.org/licenses/>. 
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