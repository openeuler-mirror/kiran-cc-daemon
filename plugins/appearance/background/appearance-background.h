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

#include <QObject>

class QGSettings;
namespace Kiran
{
class DesktopView;
class AppearanceBackground : public QObject
{
    Q_OBJECT

public:
    AppearanceBackground(QObject *parent = nullptr);
    virtual ~AppearanceBackground() {};

    void init();

    void setBackground(const QString &path);
    void updateBackground(const bool &isShow);

private:
    void processAppearanceSettingsChanged(const QString &key);

private:
    QString m_desktopBackground;

    QGSettings *m_mateBackgroundSettings;
    QGSettings *m_appearanceSettings;

    DesktopView *m_desktopView;
};

}  // namespace Kiran