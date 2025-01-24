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

#include <QKeySequence>
#include <QList>
#include <QMap>
#include <QObject>

class KGlobalAccelInterface;
class KGlobalShortcutInfo;
class QGSettings;

namespace Kiran
{
class KeyListEntry;

struct SystemShortcut
{
    QString uid;
    // 分类名
    QString kind;
    // 快捷键名称
    QString name;
    // 快捷键
    QString keyCombination;
};

// 为了兼容kde和mate，这里将相关快捷键结构整合到一起
enum SystemShortcutDesktopType
{
    SYSTEM_SHORTCUT_DESKTOP_TYPE_UNKNOWN = 0,
    SYSTEM_SHORTCUT_DESKTOP_TYPE_MATE,
    SYSTEM_SHORTCUT_DESKTOP_TYPE_KDE,
};

struct SystemShortcutMate
{
    SystemShortcutMate() : gsettings(nullptr){};
    // 分类名
    QString kind;
    // 快捷键名称
    QString name;
    // 快捷键
    QString keyCombination;
    // 读写快捷键的gsettings
    QGSettings *gsettings;
    // 快捷键在gsettings中的key
    QString settingsKey;
};

struct SystemShortcutKDE
{
    // 分类名
    QString componentUniqueName;
    // 翻译的分类名
    QString componentFriendlyName;
    // 快捷键名称
    QString uniqueName;
    // 翻译的快捷键名称
    QString friendlyName;
    // 快捷键
    QList<QKeySequence> keys;
    // 默认快捷键
    QList<QKeySequence> defaultKeys;
};

struct SystemShortcutMix
{
    SystemShortcutMix() : desktopType(SYSTEM_SHORTCUT_DESKTOP_TYPE_UNKNOWN){};
    virtual ~SystemShortcutMix(){};
    QString uid;
    SystemShortcutDesktopType desktopType;
    // 这里偷懒没有用union，因为非POD类型需要自己定义构造和析够。反正也不会有太多内存开销，后续完全迁移到kf5后可以去掉对mate的兼容
    SystemShortcutMate mate;
    SystemShortcutKDE kde;
};

class SystemShortcuts : public QObject
{
    Q_OBJECT

public:
    SystemShortcuts();
    virtual ~SystemShortcuts(){};

    // 初始化
    void init();

    // 修改系统快捷键
    bool modify(const QString &uid, const QString &keyCombination);
    // 获取系统快捷键
    QSharedPointer<SystemShortcut> get(const QString &uid);
    // 通过keycomb搜索快捷键，如果存在多个快捷键有相同的keycomb，则返回第一个找到的快捷键
    QSharedPointer<SystemShortcut> getByKeycomb(const QString &keycomb);
    QList<QSharedPointer<SystemShortcut>> get();
    // 重置系统快捷键
    void reset();

Q_SIGNALS:
    // 新增系统快捷键
    void shortcutAdded(QSharedPointer<SystemShortcut> shortcut);
    // 删除系统快捷键
    void shortcutDeleted(QSharedPointer<SystemShortcut> shortcut);
    // 系统快捷键修改
    void shortcutChanged(QSharedPointer<SystemShortcut> shortcut);

private:
    // 通过kglobalaccel获取系统快捷键
    void initKSystemShortcuts();
    // 根据配置文件加载系统快捷键，主要是兼容旧的方式（marco系统快捷键）
    void initMarcoSystemShortcuts();
    bool shouldShowKey(const KeyListEntry &entry);
    QSharedPointer<SystemShortcut> mix2common(QSharedPointer<SystemShortcutMix> systemShortCutMix);
    QString keys2Str(const QList<QKeySequence> &keys);
    QStringList buildActionId(const SystemShortcutKDE &systemshortKDE);
    void processSettingsChanged(const QString &key, const QString &schemaID);

private:
    KGlobalAccelInterface *m_globalAccelInterface;
    QMap<QString, QSharedPointer<SystemShortcutMix>> m_shortcutMixs;
    QMap<QString, QGSettings *> m_gsettingsSet;
};

}  // namespace Kiran