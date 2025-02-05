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

#include "power-backlight-monitor-x11-gamma.h"
#include <xcb/randr.h>
#include <xcb/xcb.h>
#include <algorithm>

namespace Kiran
{
#define GAMMA_MIN_BRIGHTNESS 0
#define GAMMA_MAX_BRIGHTNESS 100

PowerBacklightMonitorX11Gamma::PowerBacklightMonitorX11Gamma(xcb_randr_output_t output,
                                                             xcb_randr_crtc_t crtc) : m_output(output),
                                                                                      m_crtc(crtc)
{
    m_xcbConnection = xcb_connect(NULL, NULL);
}

PowerBacklightMonitorX11Gamma ::~PowerBacklightMonitorX11Gamma()
{
    if (m_xcbConnection)
    {
        xcb_disconnect(m_xcbConnection);
    }
}

bool PowerBacklightMonitorX11Gamma::setBrightnessValue(int32_t brightnessValue)
{
    RETURN_VAL_IF_TRUE(m_crtc == 0, false);

    auto reply = XCB_REPLY(xcb_randr_get_crtc_gamma_size, m_xcbConnection, m_crtc);

    if (!reply)
    {
        KLOG_WARNING(power) << "Failed to get crct gamma size";
        return false;
    }

    auto size = reply->size;
    if (size == 0)
    {
        KLOG_WARNING(power) << "Gamma size is 0.";
        return false;
    }

    /*
     * The gamma-correction lookup table managed through XRR[GS]etCrtcGamma
     * is 2^n in size, where 'n' is the number of significant bits in
     * the X Color.  Because an X Color is 16 bits, size cannot be larger
     * than 2^16.
     */
    if (size > 65536)
    {
        KLOG_WARNING(power) << "Gamma correction table is impossibly large.";
        return false;
    }

    auto red = new uint16_t[size];
    auto green = new uint16_t[size];
    auto blue = new uint16_t[size];
    auto gammaInfo = getGammaInfo();
    auto gammaRed = 1.0 / gammaInfo.red;
    auto gammaGreen = 1.0 / gammaInfo.green;
    auto gammaBlue = 1.0 / gammaInfo.blue;
    auto gammaBrightness = (double)brightnessValue / 100.0;

    SCOPE_EXIT({
        delete[] red;
        delete[] green;
        delete[] blue;
    });

    for (int i = 0; i < size; i++)
    {
        if (gammaRed == 1.0 && gammaBrightness == 1.0)
        {
            red[i] = uint16_t((double)i / (double)(size - 1) * 65535.0);
        }
        else
        {
            red[i] = uint16_t(std::min(pow((double)i / (double)(size - 1),
                                           gammaRed) *
                                           gammaBrightness,
                                       (double)1.0) *
                              65535.0);
        }

        if (gammaGreen == 1.0 && gammaBrightness == 1.0)
        {
            green[i] = uint16_t((double)i / (double)(size - 1) * 65535);
        }
        else
        {
            green[i] = uint16_t(std::min(pow((double)i / (double)(size - 1),
                                             gammaGreen) *
                                             gammaBrightness,
                                         1.0) *
                                65535.0);
        }

        if (gammaBlue == 1.0 && gammaBrightness == 1.0)
        {
            blue[i] = uint16_t((double)i / (double)(size - 1) * 65535.0);
        }
        else
        {
            blue[i] = uint16_t(std::min(pow((double)i / (double)(size - 1),
                                            gammaBlue) *
                                            gammaBrightness,
                                        1.0) *
                               65535.0);
        }
    }

    auto cookie = xcb_randr_set_crtc_gamma(m_xcbConnection, m_crtc, size, red, green, blue);
    auto error = xcb_request_check(m_xcbConnection, cookie);
    if (error)
    {
        KLOG_WARNING(power) << "Failed to set crtc gamma, xcb error:" << error->error_code;
        free(error);
        return false;
    }

    return true;
}

int32_t PowerBacklightMonitorX11Gamma::getBrightnessValue()
{
    auto gammaInfo = getGammaInfo();
    RETURN_VAL_IF_TRUE(gammaInfo.brightness >= 1.0, 1);
    RETURN_VAL_IF_TRUE(gammaInfo.brightness <= 0.001, 0);
    return std::min(int32_t((gammaInfo.brightness * GAMMA_MAX_BRIGHTNESS) + 0.5), GAMMA_MAX_BRIGHTNESS);
}

bool PowerBacklightMonitorX11Gamma::getBrightnessRange(int32_t &min, int32_t &max)
{
    min = GAMMA_MIN_BRIGHTNESS;
    max = GAMMA_MAX_BRIGHTNESS;
    return true;
}

int PowerBacklightMonitorX11Gamma::findLastNonClamped(unsigned short array[], int size)
{
    int i;
    for (i = size - 1; i > 0; i--)
    {
        if (array[i] < 0xffff)
            return i;
    }
    return 0;
}

GammaInfo PowerBacklightMonitorX11Gamma::getGammaInfo()
{
    GammaInfo gammaInfo;

    RETURN_VAL_IF_TRUE(m_crtc == 0, gammaInfo);

    auto gammaSizeReply = XCB_REPLY(xcb_randr_get_crtc_gamma_size, m_xcbConnection, m_crtc);
    if (!gammaSizeReply)
    {
        KLOG_WARNING(power) << "Failed to get crct gamma size";
        return gammaInfo;
    }

    auto size = gammaSizeReply->size;
    if (!size)
    {
        KLOG_WARNING(power) << "Gamma size is 0.";
        return gammaInfo;
    }

    auto gammaReply = XCB_REPLY(xcb_randr_get_crtc_gamma, m_xcbConnection, m_crtc);
    if (!gammaReply)
    {
        KLOG_WARNING(power) << "Failed to get gamma for output" << m_output;
        return gammaInfo;
    }

    auto red = xcb_randr_get_crtc_gamma_red(gammaReply.get());
    auto green = xcb_randr_get_crtc_gamma_green(gammaReply.get());
    auto blue = xcb_randr_get_crtc_gamma_blue(gammaReply.get());

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
    auto lastRed = findLastNonClamped(red, size);
    auto lastGreen = findLastNonClamped(green, size);
    auto lastBlue = findLastNonClamped(blue, size);
    auto bestArray = red;
    auto lastBest = lastRed;
    if (lastGreen > lastBest)
    {
        lastBest = lastGreen;
        bestArray = green;
    }
    if (lastBlue > lastBest)
    {
        lastBest = lastBlue;
        bestArray = blue;
    }
    if (lastBest == 0)
        lastBest = 1;

    auto middle = lastBest / 2;
    auto i1 = (double)(middle + 1) / size;
    auto v1 = (double)(bestArray[middle]) / 65535;
    auto i2 = (double)(lastBest + 1) / size;
    auto v2 = (double)(bestArray[lastBest]) / 65535;
    if (v2 >= 0.0001)
    {
        if ((lastBest + 1) == size)
        {
            gammaInfo.brightness = v2;
        }
        else
        {
            gammaInfo.brightness = exp((log(v2) * log(i1) - log(v1) * log(i2)) / log(i1 / i2));
        }
        gammaInfo.red = log((double)(red[lastRed / 2]) / gammaInfo.brightness / 65535) / log((double)((lastRed / 2) + 1) / size);
        gammaInfo.green = log((double)(green[lastGreen / 2]) / gammaInfo.brightness / 65535) / log((double)((lastGreen / 2) + 1) / size);
        gammaInfo.blue = log((double)(blue[lastBlue / 2]) / gammaInfo.brightness / 65535) / log((double)((lastBlue / 2) + 1) / size);
    }

    KLOG_DEBUG(power) << "Gamma info: red("
                      << gammaInfo.red
                      << "), green("
                      << gammaInfo.green
                      << "), blue("
                      << gammaInfo.blue
                      << "), brightness("
                      << gammaInfo.brightness << ").";
    return gammaInfo;
}

}  // namespace Kiran
