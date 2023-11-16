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
 * Author:     meizhigang <meizhigang@kylinsec.com.cn>
 */

#pragma once

#include <audio_dbus_proxy.h>
#include <audio_device_dbus_proxy.h>

namespace Kiran
{
enum VolumeType
{
    VOLUME_MUTE = 0,
    VOLUME_MIC_MUTE = 1,
    VOLUME_UP = 2,
    VOLUME_DOWN = 3,
};

class MediaKeysAudio
{
public:
    MediaKeysAudio();
    ~MediaKeysAudio();

    void init();

    void do_sound_action(VolumeType type);

private:
    void on_audio_ready(Glib::RefPtr<Gio::AsyncResult> &result);

    void on_sink_ready(Glib::RefPtr<Gio::AsyncResult> &result);

    void on_source_ready(Glib::RefPtr<Gio::AsyncResult> &result);

    void on_default_sink_path(Glib::RefPtr<Gio::AsyncResult> &result);

    void on_default_source_path(Glib::RefPtr<Gio::AsyncResult> &result);

    void on_state_changed();

    void on_default_sink_changed(uint32_t index);

    void on_default_source_changed(uint32_t index);

    void on_set_mute(Glib::RefPtr<Gio::AsyncResult> &result, bool muted);

    void on_set_volume(Glib::RefPtr<Gio::AsyncResult> &result, double volume, bool muted);

    std::string get_volume_action_icon(double volume);

private:
    Glib::RefPtr<Gio::Settings> settings_;

    Glib::RefPtr<SessionDaemon::AudioProxy> audio_proxy_;

    Glib::RefPtr<SessionDaemon::Audio::DeviceProxy> sink_proxy_;
    Glib::RefPtr<SessionDaemon::Audio::DeviceProxy> source_proxy_;
    Glib::RefPtr<SessionDaemon::Audio::DeviceProxy> device_proxy_;
};

}  // namespace  Kiran
