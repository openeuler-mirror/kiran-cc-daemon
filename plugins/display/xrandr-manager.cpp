/*
 * @Author       : tangjie02
 * @Date         : 2020-09-07 11:25:37
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-09-07 11:56:03
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/display/display-output.cpp
 */

#include "plugins/display/xrandr-manager.h"

#include <X11/Xatom.h>
namespace Kiran
{
#define EDID_BLOCK_SIZE 128

ScreenInfo::ScreenInfo() : min_width(0),
                           min_height(0),
                           max_width(0),
                           max_height(0)
{
}

OutputInfo::OutputInfo(RROutput output_id,
                       XRROutputInfo* output_info,
                       const std::string& output_edid) : id(output_id),
                                                         timestamp(output_info->timestamp),
                                                         crtc(output_info->crtc),
                                                         name(output_info->name, output_info->nameLen),
                                                         mm_width(output_info->mm_width),
                                                         mm_height(output_info->mm_height),
                                                         connection(output_info->connection == RR_Connected),
                                                         subpixel_order(output_info->subpixel_order),
                                                         npreferred(output_info->npreferred),
                                                         edid(output_edid)

{
    for (int32_t i = 0; i < output_info->ncrtc; ++i)
    {
        this->crtcs.push_back(output_info->crtcs[i]);
    }
    for (int32_t i = 0; i < output_info->nclone; ++i)
    {
        this->clones.push_back(output_info->clones[i]);
    }
    for (int32_t i = 0; i < output_info->nmode; ++i)
    {
        this->modes.push_back(output_info->modes[i]);
    }
}

CrtcInfo::CrtcInfo(RRCrtc crtc_id, XRRCrtcInfo* crtc_info) : id(crtc_id),
                                                             timestamp(crtc_info->timestamp),
                                                             x(crtc_info->x),
                                                             y(crtc_info->y),
                                                             width(crtc_info->width),
                                                             height(crtc_info->height),
                                                             mode(crtc_info->mode),
                                                             rotation(crtc_info->rotation),
                                                             rotations(crtc_info->rotations)
{
    for (int i = 0; i < crtc_info->noutput; ++i)
    {
        this->outputs.push_back(crtc_info->outputs[i]);
    }
    for (int i = 0; i < crtc_info->npossible; ++i)
    {
        this->possible.push_back(crtc_info->possible[i]);
    }
}

ModeInfo::ModeInfo() : id(0),
                       width(0),
                       height(0),
                       refresh_rate(0)
{
}

ModeInfo::ModeInfo(XRRModeInfo* mode_info) : id(mode_info->id),
                                             width(mode_info->width),
                                             height(mode_info->height),
                                             refresh_rate((mode_info->dotClock / (double)mode_info->hTotal) / mode_info->vTotal)
{
}

ModeInfo::ModeInfo(std::tuple<guint32, guint32, guint32, double> mode_info) : id(std::get<0>(mode_info)),
                                                                              width(std::get<1>(mode_info)),
                                                                              height(std::get<2>(mode_info)),
                                                                              refresh_rate(std::get<3>(mode_info))
{
}

XrandrManager::XrandrManager() : event_base_(0),
                                 error_base_(0),
                                 resources_(NULL)
{
    this->display_ = gdk_display_get_default();
    this->xdisplay_ = GDK_DISPLAY_XDISPLAY(this->display_);
    this->screen_ = gdk_screen_get_default();
    this->root_window_ = gdk_screen_get_root_window(this->screen_);
    this->xroot_window_ = GDK_WINDOW_XID(this->root_window_);
    this->connector_type_atom_ = XInternAtom(this->xdisplay_, "ConnectorType", FALSE);
}

XrandrManager::~XrandrManager()
{
    this->clear_xrandr();

    if (this->root_window_)
    {
        gdk_window_remove_filter(this->root_window_, &XrandrManager::window_event, this);
    }
}

XrandrManager* XrandrManager::instance_ = nullptr;
void XrandrManager::global_init()
{
    instance_ = new XrandrManager();
    instance_->init();
}

std::shared_ptr<OutputInfo> XrandrManager::get_output(RROutput id)
{
    auto iter = this->outputs_.find(id);
    if (iter != this->outputs_.end())
    {
        return iter->second;
    }
    return nullptr;
}

std::shared_ptr<CrtcInfo> XrandrManager::get_crtc(RRCrtc id)
{
    auto iter = this->crtcs_.find(id);
    if (iter != this->crtcs_.end())
    {
        return iter->second;
    }
    return nullptr;
}

std::shared_ptr<ModeInfo> XrandrManager::get_mode(RRMode id)
{
    auto iter = this->modes_.find(id);
    if (iter != this->modes_.end())
    {
        return iter->second;
    }
    return nullptr;
}

OutputInfoVec XrandrManager::get_outputs()
{
    OutputInfoVec outputs;
    for (const auto& elem : this->outputs_)
    {
        outputs.push_back(elem.second);
    }
    return outputs;
}

OutputInfoVec XrandrManager::get_connected_outputs()
{
    OutputInfoVec outputs;
    for (const auto& elem : this->outputs_)
    {
        if (elem.second->connection)
        {
            outputs.push_back(elem.second);
        }
    }
    return outputs;
}

RotationTypeVec XrandrManager::get_rotations(std::shared_ptr<CrtcInfo> crtc)
{
    RotationTypeVec rotations;

    if ((crtc->rotations & uint16_t(RotationType::ROTATION_0)) != 0)
    {
        rotations.push_back(RotationType::ROTATION_0);
    }

    if ((crtc->rotations & uint16_t(RotationType::ROTATION_90)) != 0)
    {
        rotations.push_back(RotationType::ROTATION_90);
    }

    if ((crtc->rotations & uint16_t(RotationType::ROTATION_180)) != 0)
    {
        rotations.push_back(RotationType::ROTATION_180);
    }

    if ((crtc->rotations & uint16_t(RotationType::ROTATION_270)) != 0)
    {
        rotations.push_back(RotationType::ROTATION_270);
    }

    return rotations;
}

ReflectTypeVec XrandrManager::get_reflects(std::shared_ptr<CrtcInfo> crtc)
{
    ReflectTypeVec reflects{ReflectType::REFLECT_NORMAL};

    if ((crtc->rotations & uint16_t(ReflectType::REFLECT_X)) != 0)
    {
        reflects.push_back(ReflectType::REFLECT_X);
    }

    if ((crtc->rotations & uint16_t(ReflectType::REFLECT_Y)) != 0)
    {
        reflects.push_back(ReflectType::REFLECT_Y);
    }

    if ((crtc->rotations & uint16_t(ReflectType::REFLECT_XY)) != 0)
    {
        reflects.push_back(ReflectType::REFLECT_XY);
    }
    return reflects;
}

ModeInfoVec XrandrManager::get_modes(std::shared_ptr<OutputInfo> output)
{
    ModeInfoVec modes;
    RETURN_VAL_IF_FALSE(output, modes);

    for (const auto& mode_id : output->modes)
    {
        auto mode = this->get_mode(mode_id);
        if (mode)
        {
            modes.push_back(mode);
        }
        else
        {
            LOG_WARNING("failed to get mode %u for output %s.",
                        mode_id,
                        output->name.c_str());
        }
    }
    return modes;
}

ModeInfoVec XrandrManager::get_prefer_modes(std::shared_ptr<OutputInfo> output)
{
    ModeInfoVec modes;
    RETURN_VAL_IF_FALSE(output, modes);

    for (int i = 0; i < output->npreferred; ++i)
    {
        auto mode = this->get_mode(output->modes[i]);
        if (mode)
        {
            modes.push_back(mode);
        }
        else
        {
            LOG_WARNING("failed to get mode <%d,%u> for output %s.",
                        i,
                        output->modes[i],
                        output->name.c_str());
        }
    }
    return modes;
}

std::string XrandrManager::gen_uuid(RROutput id)
{
    auto output_info = this->get_output(id);
    return this->gen_uuid(output_info);
}

std::string XrandrManager::gen_uuid(std::shared_ptr<OutputInfo> output_info)
{
    RETURN_VAL_IF_FALSE(output_info, std::string());
    if (output_info->edid.empty())
    {
        return output_info->name;
    }

    auto edid_md5 = Glib::Checksum::compute_checksum(Glib::Checksum::CHECKSUM_MD5, output_info->edid);
    // auto regex = Glib::Regex::create("-?[1-9][0-9]*$");
    // auto name = regex->replace(output_info->name, 0, "");
    return output_info->name + edid_md5;
}

void XrandrManager::init()
{
    SETTINGS_PROFILE("");

    std::string err;
    if (!this->init_xrandr(err))
    {
        LOG_WARNING("xrandr: %s.", err.c_str());
        return;
    }

    this->load_xrandr(true);

    XRRSelectInput(this->xdisplay_, this->xroot_window_, RRScreenChangeNotifyMask);
    gdk_x11_register_standard_event_type(this->display_,
                                         this->event_base_,
                                         RRNotify + 1);

    gdk_window_add_filter(this->root_window_, &XrandrManager::window_event, this);
}

bool XrandrManager::init_xrandr(std::string& err)
{
    SETTINGS_PROFILE("");
    if (XRRQueryExtension(this->xdisplay_, &this->event_base_, &this->error_base_))
    {
        int major_version = 0;
        int minor_version = 0;
        XRRQueryVersion(this->xdisplay_, &major_version, &minor_version);
        if (major_version < 1 || (major_version == 1 && minor_version < 3))
        {
            err = fmt::format("RANDR extension is too old (must be at least 1.3). current version: {0}:{1}.",
                              major_version,
                              minor_version);
            return false;
        }
    }
    else
    {
        err = "RANDR extension is not present";
        return false;
    }
    return true;
}

void XrandrManager::load_xrandr(bool polling)
{
    SETTINGS_PROFILE("");

    this->clear_xrandr();

    if (polling)
    {
        this->resources_ = XRRGetScreenResources(this->xdisplay_, this->xroot_window_);
    }
    else
    {
        this->resources_ = XRRGetScreenResourcesCurrent(this->xdisplay_, this->xroot_window_);
    }

    if (!this->resources_)
    {
        LOG_WARNING("cannot get screen resources for %0x.", this->xroot_window_);
        return;
    }

    XRRGetScreenSizeRange(this->xdisplay_,
                          this->xroot_window_,
                          &this->screen_info_.min_width,
                          &this->screen_info_.min_height,
                          &this->screen_info_.max_width,
                          &this->screen_info_.max_height);

    LOG_DEBUG("screen info: min_width: %d, min_height: %d, max_width: %d, max_height: %d.",
              this->screen_info_.min_width,
              this->screen_info_.min_height,
              this->screen_info_.max_width,
              this->screen_info_.max_height);

    this->load_outputs();
    this->load_crtcs();
    this->load_mods();
}

void XrandrManager::load_outputs()
{
    SETTINGS_PROFILE("");

    for (int32_t i = 0; i < this->resources_->noutput; ++i)
    {
        auto output_info = XRRGetOutputInfo(this->xdisplay_, this->resources_, this->resources_->outputs[i]);
        if (output_info)
        {
            auto edid = this->get_edid(this->resources_->outputs[i]);
            auto output = std::make_shared<OutputInfo>(this->resources_->outputs[i], output_info, edid);
            this->outputs_.emplace(this->resources_->outputs[i], output);
            XRRFreeOutputInfo(output_info);

            LOG_DEBUG("output(%u) name: %s, connection: %u, crtc: %u, timestamp: %u, npreferred: %d, edid length: %d.",
                      (uint32_t)output->id,
                      output->name.c_str(),
                      output->connection,
                      (uint32_t)output->crtc,
                      (uint32_t)output->timestamp,
                      output->npreferred,
                      edid.length());
        }
        else
        {
            LOG_WARNING("cannot get output info for %0x.", this->resources_->outputs[i]);
        }
    }
}
void XrandrManager::load_crtcs()
{
    SETTINGS_PROFILE("");

    for (int32_t i = 0; i < this->resources_->ncrtc; ++i)
    {
        auto crtc_info = XRRGetCrtcInfo(this->xdisplay_, this->resources_, this->resources_->crtcs[i]);
        if (crtc_info)
        {
            auto crtc = std::make_shared<CrtcInfo>(this->resources_->crtcs[i], crtc_info);
            this->crtcs_.emplace(this->resources_->crtcs[i], crtc);
            XRRFreeCrtcInfo(crtc_info);

            LOG_DEBUG("crtc(%u) x: %d, y: %d, width: %u, height: %u, mode: %u, rotation: %0x, rotations: %0x",
                      crtc->id,
                      crtc->x,
                      crtc->y,
                      crtc->width,
                      crtc->height,
                      (uint32_t)crtc->mode,
                      crtc->rotation,
                      crtc->rotations);
        }
        else
        {
            LOG_WARNING("cannot get crtc info for %0x.", this->resources_->crtcs[i]);
        }
    }
}

void XrandrManager::load_mods()
{
    SETTINGS_PROFILE("");

    for (int32_t i = 0; i < this->resources_->nmode; ++i)
    {
        auto mode = std::make_shared<ModeInfo>(&this->resources_->modes[i]);
        this->modes_.emplace(mode->id, mode);

        LOG_DEBUG("mode(%u) width: %u, height: %u refresh_rate: %f.",
                  mode->id,
                  mode->width,
                  mode->height,
                  mode->refresh_rate);
    }
}

void XrandrManager::clear_xrandr()
{
    if (this->resources_)
    {
        XRRFreeScreenResources(this->resources_);
        this->resources_ = NULL;
    }
    this->outputs_.clear();
    this->crtcs_.clear();
    this->modes_.clear();
}

bool XrandrManager::is_rotation(Rotation rotation)
{
    if ((rotation & uint16_t(RotationType::ROTATION_90)) != 0 ||
        (rotation & uint16_t(RotationType::ROTATION_270)) != 0)
    {
        return true;
    }
    return false;
}

std::string XrandrManager::get_edid(RROutput output_id)
{
    std::string result;
    result = this->get_property_string(output_id, "EDID", EDID_BLOCK_SIZE);

    if (result.empty())
    {
        result = this->get_property_string(output_id, "EDID_DATA", EDID_BLOCK_SIZE);
    }

    if (result.length() % EDID_BLOCK_SIZE == 0)
    {
        return result;
    }
    return std::string();
}

std::string XrandrManager::get_property_string(RROutput output_id, const std::string& prop_name, int32_t length)
{
    unsigned char* prop = NULL;
    int actual_format;
    unsigned long nitems, bytes_after;
    Atom actual_type;
    std::string result;

    auto atom = XInternAtom(this->xdisplay_, prop_name.c_str(), FALSE);

    auto ret = XRRGetOutputProperty(this->xdisplay_,
                                    output_id,
                                    atom,
                                    0,
                                    length,
                                    False,
                                    False,
                                    AnyPropertyType,
                                    &actual_type,
                                    &actual_format,
                                    &nitems,
                                    &bytes_after,
                                    &prop);

    if (ret == Success &&
        actual_type == XA_INTEGER &&
        actual_format == 8 &&
        prop)
    {
        result = std::string((char*)prop, nitems);
    }

    if (ret == Success && prop)
    {
        XFree(prop);
    }
    return result;
}

std::string XrandrManager::get_connector_type(RROutput output_id)
{
    unsigned char* prop = NULL;
    int actual_format;
    unsigned long nitems, bytes_after;
    Atom actual_type;
    char* connector_type_str = NULL;
    std::string result;

    auto ret = XRRGetOutputProperty(this->xdisplay_,
                                    output_id,
                                    this->connector_type_atom_,
                                    0,
                                    100,
                                    False,
                                    False,
                                    AnyPropertyType,
                                    &actual_type,
                                    &actual_format,
                                    &nitems,
                                    &bytes_after,
                                    &prop);

    LOG_DEBUG("ret: %u atom: %u type: %u format: %u prop: %p ntimes: %u.", ret, this->connector_type_atom_, actual_type, actual_format, prop, nitems);
    if (ret == Success &&
        actual_type == XA_ATOM &&
        actual_format == 32 &&
        nitems == 1 &&
        prop)
    {
        auto value = *((Atom*)prop);
        connector_type_str = XGetAtomName(this->xdisplay_, value);
    }

    if (connector_type_str)
    {
        result = std::string(connector_type_str);
        XFree(connector_type_str);
    }

    if (ret == Success && prop)
    {
        XFree(prop);
    }

    return result;
}

GdkFilterReturn XrandrManager::window_event(GdkXEvent* gdk_event, GdkEvent* event, gpointer data)
{
    XrandrManager* xrandr_manager = (XrandrManager*)data;
    if (xrandr_manager != XrandrManager::get_instance())
    {
        LOG_WARNING("the previous XrandrManager was not removed.");
        return GDK_FILTER_REMOVE;
    }

    XEvent* xevent = (XEvent*)gdk_event;

    RETURN_VAL_IF_FALSE(xrandr_manager, GDK_FILTER_CONTINUE);
    RETURN_VAL_IF_FALSE(xevent, GDK_FILTER_CONTINUE);

    if ((xevent->type - xrandr_manager->event_base_) == RRScreenChangeNotify)
    {
        xrandr_manager->load_xrandr(false);
        xrandr_manager->resources_changed_.emit();
    }
    return GDK_FILTER_CONTINUE;
}
}  // namespace Kiran