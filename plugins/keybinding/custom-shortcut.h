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

#include <QMap>
#include <QObject>

class QSettings;
class KActionCollection;
class QAction;

namespace Kiran
{

struct CustomShortCut
{
    CustomShortCut() = default;
    CustomShortCut(const QString &u,
                   const QString &n,
                   const QString a,
                   const QString &k) : uid(u),
                                       name(n),
                                       action(a),
                                       keyComb(k) {}
    CustomShortCut(const QString &n,
                   const QString a,
                   const QString &k) : name(n),
                                       action(a),
                                       keyComb(k) {}

    // 快捷键的uid，对应keyfile文件中的group name
    QString uid;
    // 快捷键名称
    QString name;
    // 触发快捷键后执行的命令
    QString action;
    // 按键组合
    QString keyComb;
};

class CustomShortcuts : public QObject
{
    Q_OBJECT

public:
    CustomShortcuts();
    virtual ~CustomShortcuts();

    // 初始化
    void init();

    // 添加自定义快捷键
    bool add(QSharedPointer<CustomShortCut> shortcut);
    // 修改自定义快捷键，如果uid不存在则返回错误
    bool modify(QSharedPointer<CustomShortCut> shortcut);
    // 删除自定义快捷键
    bool remove(const QString &uid);
    // 获取自定义快捷键
    QSharedPointer<CustomShortCut> get(const QString &uid);
    // 通过keycomb搜索快捷键，如果存在多个快捷键有相同的keycomb，则返回第一个找到的快捷键
    QSharedPointer<CustomShortCut> getByKeyComb(const QString &keyComb);
    // 获取所有自定义快捷键
    QMap<QString, QSharedPointer<CustomShortCut>> get();
    // 是否存在快捷键
    bool exists(const QString &uid);

private:
    // 生成自定义快捷键唯一ID
    QString genUid();
    // 校验自定义快捷键是否合法
    bool checkValid(QSharedPointer<CustomShortCut> shortcut);
    // 向kglobalaccel服务注册快捷键
    void registerShortcut(QSharedPointer<CustomShortCut> shortcut);
    void unregisterShortcut(QSharedPointer<CustomShortCut> shortcut);
    void triggerAction(bool checked, QAction *action);

private:
    QString m_confFilePath;
    QSettings *m_settings;
    KActionCollection *m_actionCollection;
};
}  // namespace Kiran