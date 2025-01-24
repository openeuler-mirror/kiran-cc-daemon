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

#pragma once

#include <QDBusContext>
#include <QMap>
#include <QStringList>

class QGSettings;
class KeyboardAdaptor;

namespace Kiran
{
class KeyboardManager : public QObject,
                        protected QDBusContext
{
    Q_OBJECT

    Q_PROPERTY(bool capslock_tips_enabled READ getCapslockTipsEnabled WRITE setCapslockTipsEnabled)
    Q_PROPERTY(QStringList layouts READ getLayouts WRITE setLayouts)
    Q_PROPERTY(bool modifier_lock_enabled READ getModifierLockEnabled WRITE setModifierLockEnabled)
    Q_PROPERTY(bool numlock_tips_enabled READ getNumlockTipsEnabled WRITE setNumlockTipsEnabled)
    Q_PROPERTY(QStringList options READ getOptions WRITE setOptions)
    Q_PROPERTY(int repeat_delay READ getRepeatDelay WRITE setRepeatDelay)
    Q_PROPERTY(bool repeat_enabled READ getRepeatEnabled WRITE setRepeatEnabled)
    Q_PROPERTY(int repeat_interval READ getRepeatInterval WRITE setRepeatInterval)

public:
    KeyboardManager();
    virtual ~KeyboardManager();

    static KeyboardManager *getInstance() { return m_instance; };

    static void globalInit();

    static void globalDeinit() { delete m_instance; };

public:
    bool getCapslockTipsEnabled() const { return m_capslockTipsEnabled; };
    QStringList getLayouts() const { return m_layouts; };
    bool getModifierLockEnabled() const { return m_modifierLockEnabled; };
    bool getNumlockTipsEnabled() const { return m_numlockTipsEnabled; };
    QStringList getOptions() const { return m_options; };
    int getRepeatDelay() const { return m_repeatDelay; };
    bool getRepeatEnabled() const { return m_repeatEnabled; };
    int getRepeatInterval() const { return m_repeatInterval; };

    void setCapslockTipsEnabled(bool capslockTipsEnabled);
    void setLayouts(const QStringList &layouts);
    void setModifierLockEnabled(bool modifierLockEnabled);
    void setNumlockTipsEnabled(bool numlockTipsEnabled);
    void setOptions(const QStringList &options);
    void setRepeatDelay(int repeatDelay);
    void setRepeatEnabled(bool repeatEnabled);
    void setRepeatInterval(int repeatInterval);

public Q_SLOTS:
    /* 添加键盘布局。键盘布局最多只能设置4个，如果超过4个则返回添加；
       如果布局不在GetValidLayouts返回的列表中，或者布局已经存在用户布局列表中，则返回添加失败；
       如果设置布局命令执行错误，也返回添加失败；
       否则返回添加成功。*/
    void AddLayout(const QString &layout);
    // 添加布局选项
    void AddLayoutOption(const QString &option);
    // 应用键盘布局，该键盘布局必须是已经通过AddLayout添加到键盘布局列表中的布局
    void ApplyLayout(const QString &layout);
    // 清理布局选项
    void ClearLayoutOption();
    /* 从用户布局列表中删除键盘布局，如果用户布局列表中不存在该布局，则返回删除失败；
       如果设置布局命令执行错误，也返回删除失败；
       否则返回删除成功 */
    void DelLayout(const QString &layout);
    // 删除布局选项
    void DelLayoutOption(const QString &option);
    // 获取所有合法的键盘布局列表，这个列表是从/usr/share/X11/xkb/rules/base.xml中读取。
    QString GetValidLayouts();
    // 大小写锁提示开关
    void SwitchCapsLockTips(bool enabled);
    // 数字键盘锁提示开关
    void SwitchNumLockTips(bool enabled);

private:
    void init();

    void loadFromSettings();
    void processSettingsChanged(const QString &key);
    void loadXkbRules();

    void setAllPropsToXkb();
    void setAutoRepeatToXkb();
    bool setLayoutsToXkb(const QStringList &layouts);
    bool setOptionsToXkb(const QStringList &options);

private:
    static KeyboardManager *m_instance;

    KeyboardAdaptor *m_keyboardAdaptor;
    QGSettings *m_keyboardSettings;
    QMap<QString, QString> m_validLayouts;

    bool m_modifierLockEnabled;
    bool m_capslockTipsEnabled;
    bool m_numlockTipsEnabled;
    bool m_repeatEnabled;
    int32_t m_repeatDelay;
    int32_t m_repeatInterval;
    QStringList m_layouts;
    QStringList m_options;
};

}  // namespace Kiran