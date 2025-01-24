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

#include "display-util.h"
#include <kscreen/edid.h>
#include <kscreen/output.h>
#include <QRegExp>
#include "lib/base/base.h"

namespace Kiran
{
QString DisplayUtil::rotationEnum2str(DisplayRotationType rotation)
{
    switch (rotation)
    {
    case DisplayRotationType::DISPLAY_ROTATION_90:
        return "left";
    case DisplayRotationType::DISPLAY_ROTATION_180:
        return "inverted";
    case DisplayRotationType::DISPLAY_ROTATION_270:
        return "right";
    default:
        return "normal";
    }
}

QString DisplayUtil::reflectEnum2str(DisplayReflectType reflect)
{
    switch (reflect)
    {
    case DisplayReflectType::DISPLAY_REFLECT_X:
        return "x";
    case DisplayReflectType::DISPLAY_REFLECT_Y:
        return "y";
    case DisplayReflectType::DISPLAY_REFLECT_XY:
        return "xy";
    default:
        return "normal";
        break;
    }
}

DisplayRotationType DisplayUtil::rotationStr2Enum(const QString &str)
{
    switch (shash(str.toLatin1().data()))
    {
    case "left"_hash:
        return DisplayRotationType::DISPLAY_ROTATION_90;
    case "inverted"_hash:
        return DisplayRotationType::DISPLAY_ROTATION_180;
    case "right"_hash:
        return DisplayRotationType::DISPLAY_ROTATION_270;
    default:
        return DisplayRotationType::DISPLAY_ROTATION_0;
    }
    return DisplayRotationType::DISPLAY_ROTATION_0;
}

DisplayReflectType DisplayUtil::reflectStr2Enum(const QString &str)
{
    switch (shash(str.toLatin1().data()))
    {
    case "x"_hash:
        return DisplayReflectType::DISPLAY_REFLECT_X;
    case "y"_hash:
        return DisplayReflectType::DISPLAY_REFLECT_Y;
    case "xy"_hash:
        return DisplayReflectType::DISPLAY_REFLECT_XY;
    default:
        return DisplayReflectType::DISPLAY_REFLECT_NORMAL;
    }
    return DisplayReflectType::DISPLAY_REFLECT_NORMAL;
}

QString DisplayUtil::generateOutputUID(KScreen::OutputPtr output)
{
    RETURN_VAL_IF_FALSE(output, QString());

    auto edid = output->edid();

    if (edid && edid->isValid())
    {
        auto edidMd5 = edid->hash();
        auto outputName = output->name();
        auto outputNamePrefix = outputName.replace(QRegExp("-[1-9][0-9]*$"), "");
        return QString("%1-%2").arg(outputNamePrefix).arg(edidMd5);
    }
    else
    {
        // 虚拟机的EDID可能为空，这里我们将最佳分辨率加入唯一标识，以便在后续虚拟机大小发生变化时可以对分辨率进行重新设置。
        auto preferModes = output->preferredModes();
        if (preferModes.size() >= 1)
        {
            auto preferMode = output->mode(preferModes[0]);
            return QString("%1-%2x%3")
                .arg(output->name())
                .arg(preferMode->size().width())
                .arg(preferMode->size().height());
        }
        else
        {
            return output->name();
        }
    }
}
}  // namespace Kiran
