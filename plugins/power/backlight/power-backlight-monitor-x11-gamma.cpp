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

#include "plugins/power/backlight/power-backlight-monitor-x11-gamma.h"

#include <X11/Xatom.h>
#include <algorithm>

namespace Kiran
{
#define GAMMA_MIN_BRIGHTNESS 0
#define GAMMA_MAX_BRIGHTNESS 100

PowerBacklightMonitorX11Gamma::PowerBacklightMonitorX11Gamma(RROutput output,
                                                             RRCrtc crtc) : output_(output),
                                                                            crtc_(crtc)
{
    this->display_ = gdk_display_get_default();
    this->xdisplay_ = GDK_DISPLAY_XDISPLAY(this->display_);
}

bool PowerBacklightMonitorX11Gamma::set_brightness_value(int32_t brightness_value)
{
    RETURN_VAL_IF_TRUE(this->crtc_ == None, false);

    auto size = XRRGetCrtcGammaSize(this->xdisplay_, this->crtc_);

    if (!size)
    {
        KLOG_WARNING("Gamma size is 0.");
        return false;
    }
    else
    {
        KLOG_DEBUG("The gamma size is %d.", size);
    }

    /*
	 * The gamma-correction lookup table managed through XRR[GS]etCrtcGamma
	 * is 2^n in size, where 'n' is the number of significant bits in
	 * the X Color.  Because an X Color is 16 bits, size cannot be larger
	 * than 2^16.
	 */
    if (size > 65536)
    {
        KLOG_WARNING("Gamma correction table is impossibly large.");
        return false;
    }

    auto crtc_gamma = XRRAllocGamma(size);
    if (!crtc_gamma)
    {
        KLOG_WARNING("Gamma allocation failed.");
        return false;
    }

    SCOPE_EXIT(
        {
            if (crtc_gamma)
            {
                XRRFreeGamma(crtc_gamma);
            }
        });

    auto gamma_info = this->get_gamma_info();
    auto gamma_red = 1.0 / gamma_info.red;
    auto gamma_green = 1.0 / gamma_info.green;
    auto gamma_blue = 1.0 / gamma_info.blue;
    auto gamma_brightness = (double)brightness_value / 100.0;

    for (int i = 0; i < size; i++)
    {
        if (gamma_red == 1.0 && gamma_brightness == 1.0)
        {
            crtc_gamma->red[i] = (double)i / (double)(size - 1) * 65535.0;
        }
        else
        {
            crtc_gamma->red[i] = std::min(pow((double)i / (double)(size - 1),
                                              gamma_red) *
                                              gamma_brightness,
                                          (double)1.0) *
                                 65535.0;
        }

        if (gamma_green == 1.0 && gamma_brightness == 1.0)
        {
            crtc_gamma->green[i] = (double)i / (double)(size - 1) * 65535.0;
        }
        else
        {
            crtc_gamma->green[i] = std::min(pow((double)i / (double)(size - 1),
                                                gamma_green) *
                                                gamma_brightness,
                                            1.0) *
                                   65535.0;
        }

        if (gamma_blue == 1.0 && gamma_brightness == 1.0)
        {
            crtc_gamma->blue[i] = (double)i / (double)(size - 1) * 65535.0;
        }
        else
        {
            crtc_gamma->blue[i] = std::min(pow((double)i / (double)(size - 1),
                                               gamma_blue) *
                                               gamma_brightness,
                                           1.0) *
                                  65535.0;
        }
    }

    XRRSetCrtcGamma(this->xdisplay_, this->crtc_, crtc_gamma);

    return true;
}

int32_t PowerBacklightMonitorX11Gamma::get_brightness_value()
{
    auto gamma_info = this->get_gamma_info();
    RETURN_VAL_IF_TRUE(gamma_info.brightness >= 1.0, 1);
    RETURN_VAL_IF_TRUE(gamma_info.brightness <= 0.001, 0);
    return std::min(int32_t((gamma_info.brightness * GAMMA_MAX_BRIGHTNESS) + 0.5), GAMMA_MAX_BRIGHTNESS);
}

bool PowerBacklightMonitorX11Gamma::get_brightness_range(int32_t &min, int32_t &max)
{
    min = GAMMA_MIN_BRIGHTNESS;
    max = GAMMA_MAX_BRIGHTNESS;
    return true;
}

int PowerBacklightMonitorX11Gamma::find_last_non_clamped(unsigned short array[], int size)
{
    int i;
    for (i = size - 1; i > 0; i--)
    {
        if (array[i] < 0xffff)
            return i;
    }
    return 0;
}

GammaInfo PowerBacklightMonitorX11Gamma::get_gamma_info()
{
    GammaInfo gamma_info;

    RETURN_VAL_IF_TRUE(this->crtc_ == 0, gamma_info);

    auto size = XRRGetCrtcGammaSize(this->xdisplay_, this->crtc_);
    if (!size)
    {
        KLOG_WARNING("Gamma size is 0.");
        return gamma_info;
    }

    auto crtc_gamma = XRRGetCrtcGamma(this->xdisplay_, this->crtc_);
    if (!crtc_gamma)
    {
        KLOG_WARNING("Failed to get gamma for output(%d).", (int)this->output_);
        return gamma_info;
    }

    /*
     * Here is a bit tricky because gamma is a whole curve for each
     * color.  So, typically, we need to represent 3 * 256 values as 3 + 1
     * values.  Therefore, we approximate the gamma curve (v) by supposing
     * it always follows the way we set it: a power function (i^g)
     * multiplied by a brightness (b).
     * v = i^g * b
     * so g = (ln(v) - ln(b))/ln(i)
     * and b can be found using two points (v1,i1) and (v2, i2):
     * b = e^((ln(v2)*ln(i1) - ln(v1)*ln(i2))/ln(i1/i2))
     * For the best resolution, we select i2 at the highest place not
     * clamped and i1 at i2/2. Note that if i2 = 1 (as in most normal
     * cases), then b = v2.
     */
    auto last_red = find_last_non_clamped(crtc_gamma->red, size);
    auto last_green = find_last_non_clamped(crtc_gamma->green, size);
    auto last_blue = find_last_non_clamped(crtc_gamma->blue, size);
    auto best_array = crtc_gamma->red;
    auto last_best = last_red;
    if (last_green > last_best)
    {
        last_best = last_green;
        best_array = crtc_gamma->green;
    }
    if (last_blue > last_best)
    {
        last_best = last_blue;
        best_array = crtc_gamma->blue;
    }
    if (last_best == 0)
        last_best = 1;

    auto middle = last_best / 2;
    auto i1 = (double)(middle + 1) / size;
    auto v1 = (double)(best_array[middle]) / 65535;
    auto i2 = (double)(last_best + 1) / size;
    auto v2 = (double)(best_array[last_best]) / 65535;
    if (v2 >= 0.0001)
    {
        if ((last_best + 1) == size)
        {
            gamma_info.brightness = v2;
        }
        else
        {
            gamma_info.brightness = exp((log(v2) * log(i1) - log(v1) * log(i2)) / log(i1 / i2));
        }
        gamma_info.red = log((double)(crtc_gamma->red[last_red / 2]) / gamma_info.brightness / 65535) / log((double)((last_red / 2) + 1) / size);
        gamma_info.green = log((double)(crtc_gamma->green[last_green / 2]) / gamma_info.brightness / 65535) / log((double)((last_green / 2) + 1) / size);
        gamma_info.blue = log((double)(crtc_gamma->blue[last_blue / 2]) / gamma_info.brightness / 65535) / log((double)((last_blue / 2) + 1) / size);
    }

    XRRFreeGamma(crtc_gamma);

    KLOG_DEBUG("Gamma info: red(%.2f), green(%.2f), blue(%.2f), brightness(%.2f).",
               gamma_info.red,
               gamma_info.green,
               gamma_info.blue,
               gamma_info.brightness);
    return gamma_info;
}

}  // namespace Kiran
