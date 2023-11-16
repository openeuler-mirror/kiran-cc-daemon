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

#include "plugins/keybinding/media-keys/media-keys-audio.h"
#include "audio-i.h"
#include "lib/base/base.h"
#include "lib/osdwindow/osd-window.h"
#include "media-keys-i.h"

namespace Kiran
{
#define IMAGE_VOLUME_MUTED "osd-audio-volume-muted"
#define IMAGE_VOLUME_OFF "osd-audio-volume-off"
#define IMAGE_VOLUME_LOW "osd-audio-volume-low"
#define IMAGE_VOLUME_MEDIUM "osd-audio-volume-medium"
#define IMAGE_VOLUME_HIGH "osd-audio-volume-high"

// 默认音量步进
#define VOLUME_STEP_DEFAULT 6

MediaKeysAudio::MediaKeysAudio()
{
    this->settings_ = Gio::Settings::create(MEDIAKEYS_SCHEMA_ID);
}

MediaKeysAudio::~MediaKeysAudio()
{
}

void MediaKeysAudio::init()
{
    SessionDaemon::AudioProxy::createForBus(Gio::DBus::BUS_TYPE_SESSION,
                                            Gio::DBus::PROXY_FLAGS_NONE,
                                            AUDIO_DBUS_NAME,
                                            AUDIO_OBJECT_PATH,
                                            sigc::mem_fun(this, &MediaKeysAudio::on_audio_ready));
}

void MediaKeysAudio::on_audio_ready(Glib::RefPtr<Gio::AsyncResult> &result)
{
    try
    {
        this->audio_proxy_ = SessionDaemon::AudioProxy::createForBusFinish(result);

        this->audio_proxy_->state_changed().connect(sigc::mem_fun(this, &MediaKeysAudio::on_state_changed));
        this->audio_proxy_->DefaultSinkChange_signal.connect(sigc::mem_fun(this, &MediaKeysAudio::on_default_sink_changed));
        this->audio_proxy_->DefaultSourceChange_signal.connect(sigc::mem_fun(this, &MediaKeysAudio::on_default_source_changed));

        this->audio_proxy_->GetDefaultSink(sigc::mem_fun(this, &MediaKeysAudio::on_default_sink_path));
        this->audio_proxy_->GetDefaultSource(sigc::mem_fun(this, &MediaKeysAudio::on_default_source_path));
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING_KEYBINDING("Cannot connect to %s: %s.", AUDIO_OBJECT_PATH, e.what().c_str());
        return;
    }
}
void MediaKeysAudio::on_sink_ready(Glib::RefPtr<Gio::AsyncResult> &result)
{
    try
    {
        this->sink_proxy_ = SessionDaemon::Audio::DeviceProxy::createForBusFinish(result);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING_KEYBINDING("Cannot connect to %s: %s.", AUDIO_OBJECT_PATH, e.what().c_str());
        return;
    }
}

void MediaKeysAudio::on_source_ready(Glib::RefPtr<Gio::AsyncResult> &result)
{
    try
    {
        this->source_proxy_ = SessionDaemon::Audio::DeviceProxy::createForBusFinish(result);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING_KEYBINDING("Cannot connect to %s: %s.", AUDIO_OBJECT_PATH, e.what().c_str());
        return;
    }
}

void MediaKeysAudio::on_default_sink_path(Glib::RefPtr<Gio::AsyncResult> &result)
{
    try
    {
        Glib::ustring sink_path;
        this->audio_proxy_->GetDefaultSink_finish(sink_path, result);
        RETURN_IF_TRUE(sink_path.empty());

        SessionDaemon::Audio::DeviceProxy::createForBus(Gio::DBus::BUS_TYPE_SESSION,
                                                        Gio::DBus::PROXY_FLAGS_NONE,
                                                        AUDIO_DBUS_NAME,
                                                        sink_path,
                                                        sigc::mem_fun(this, &MediaKeysAudio::on_sink_ready));
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING_KEYBINDING("%s.", e.what().c_str());
    }
}

void MediaKeysAudio::on_default_source_path(Glib::RefPtr<Gio::AsyncResult> &result)
{
    try
    {
        Glib::ustring source_path;
        this->audio_proxy_->GetDefaultSource_finish(source_path, result);
        RETURN_IF_TRUE(source_path.empty());

        SessionDaemon::Audio::DeviceProxy::createForBus(Gio::DBus::BUS_TYPE_SESSION,
                                                        Gio::DBus::PROXY_FLAGS_NONE,
                                                        AUDIO_DBUS_NAME,
                                                        source_path,
                                                        sigc::mem_fun(this, &MediaKeysAudio::on_source_ready));
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING_KEYBINDING("%s.", e.what().c_str());
        return;
    }
}

void MediaKeysAudio::on_state_changed()
{
    this->audio_proxy_->GetDefaultSink(sigc::mem_fun(this, &MediaKeysAudio::on_default_sink_path));
    this->audio_proxy_->GetDefaultSource(sigc::mem_fun(this, &MediaKeysAudio::on_default_source_path));
}

void MediaKeysAudio::on_default_sink_changed(uint32_t index)
{
    this->audio_proxy_->GetDefaultSink(sigc::mem_fun(this, &MediaKeysAudio::on_default_sink_path));
}

void MediaKeysAudio::on_default_source_changed(uint32_t index)
{
    this->audio_proxy_->GetDefaultSource(sigc::mem_fun(this, &MediaKeysAudio::on_default_source_path));
}

void MediaKeysAudio::do_sound_action(VolumeType type)
{
    RETURN_IF_FALSE(this->audio_proxy_);

    bool is_input = (type == VOLUME_MIC_MUTE) ? true : false;
    if (is_input)
    {
        this->device_proxy_ = this->source_proxy_;
    }
    else
    {
        this->device_proxy_ = this->sink_proxy_;
    }
    RETURN_IF_FALSE(this->device_proxy_);

    bool muted_last = this->device_proxy_->mute_get();
    bool muted = muted_last;
    double volume_last = this->device_proxy_->volume_get();
    double volume = volume_last;

    uint32_t volume_step = this->settings_->get_int(MEDIAKEYS_SCHEMA_VOLUME_STEP);
    if (volume_step <= 0 || volume_step > 100)
    {
        volume_step = VOLUME_STEP_DEFAULT;
    }

    double volume_step_abs = volume_step / 100.0;

    switch (type)
    {
    case VOLUME_MUTE:
    case VOLUME_MIC_MUTE:
    {
        muted = !muted;
    }
    break;
    case VOLUME_DOWN:
    {
        if (volume < volume_step_abs)
        {
            volume = 0.0;
            muted = true;
        }
        else
        {
            volume = CLAMP((volume - volume_step_abs), 0.0, 1.0);
            muted = false;
        }
    }
    break;
    case VOLUME_UP:
    {
        if (muted)
        {
            muted = false;
            if (volume < EPS)
            {
                volume = volume_step_abs;
            }
        }
        else
        {
            volume = CLAMP((volume + volume_step_abs), 0.0, 1.0);
        }
    }
    break;
    default:
        break;
    }

    KLOG_DEBUG_KEYBINDING("Last mute volume:<%d, %f>, new mute volume:<%d, %f>",
                          muted_last, volume_last, muted, volume);

    if (muted != muted_last)
    {
        try
        {
            Gio::SlotAsyncReady res = sigc::bind(sigc::mem_fun(this, &MediaKeysAudio::on_set_mute), muted);
            this->device_proxy_->SetMute(muted, res);
        }
        catch (const Glib::Error &e)
        {
            KLOG_WARNING_KEYBINDING("%s.", e.what().c_str());
        }

        RETURN_IF_TRUE(muted);
    }

    try
    {
        Gio::SlotAsyncReady res = sigc::bind(sigc::mem_fun(this, &MediaKeysAudio::on_set_volume), volume, muted);
        this->device_proxy_->SetVolume(volume, res);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING_KEYBINDING("%s.", e.what().c_str());
    }
}

void MediaKeysAudio::on_set_mute(Glib::RefPtr<Gio::AsyncResult> &result, bool muted)
{
    try
    {
        this->device_proxy_->SetMute_finish(result);
        if (muted)
        {
            OSDWindow::get_instance()->dialog_show(IMAGE_VOLUME_MUTED);
        }
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING_KEYBINDING("%s.", e.what().c_str());
        return;
    }
}

void MediaKeysAudio::on_set_volume(Glib::RefPtr<Gio::AsyncResult> &result, double volume, bool muted)
{
    try
    {
        this->device_proxy_->SetVolume_finish(result);

        if (muted)
        {
            OSDWindow::get_instance()->dialog_show(IMAGE_VOLUME_MUTED);
        }
        else
        {
            OSDWindow::get_instance()->dialog_show(this->get_volume_action_icon(volume));
        }
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING_KEYBINDING("%s.", e.what().c_str());
        return;
    }
}

std::string MediaKeysAudio::get_volume_action_icon(double volume)
{
    if (volume < 0.25)
    {
        return IMAGE_VOLUME_OFF;
    }
    else if (volume < 0.5)
    {
        return IMAGE_VOLUME_LOW;
    }
    else if (volume < 0.75)
    {
        return IMAGE_VOLUME_MEDIUM;
    }
    else
    {
        return IMAGE_VOLUME_HIGH;
    }
}

}  // namespace  Kiran
