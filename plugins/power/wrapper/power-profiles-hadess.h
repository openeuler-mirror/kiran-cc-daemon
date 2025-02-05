/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd.
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

#include "power-profiles.h"

class QDBusMessage;

namespace Kiran
{
class PowerProfilesHadess : public PowerProfiles
{
    Q_OBJECT

public:
    PowerProfilesHadess();
    virtual ~PowerProfilesHadess(){};

    virtual void init();
    virtual bool switchProfile(int32_t profileMode);
    virtual uint32_t holdProfile(int32_t profileMode, const QString &reason);
    virtual void releaseProfile(uint32_t cookie);
    virtual int32_t getActiveProfile();

private:
    QString porfileModeEnum2Str(int32_t profileMode);
    int32_t porfileModeStr2Enum(const QString &profileModeStr);

private Q_SLOTS:
    void processPropertiesChanged(const QDBusMessage &message);
};
}  // namespace Kiran
