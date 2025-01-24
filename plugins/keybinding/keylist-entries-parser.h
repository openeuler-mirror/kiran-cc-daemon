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

#include <QVector>

namespace xmlpp
{
class Node;
};

namespace Kiran
{
struct KeyListEntry
{
    // gsettings的键名(key)；值为按键组合，例如<ctrl>O
    QString name;
    // 按键组合描述
    QString description;
    // 如果schema:key  comparison value 成立，则entry可用
    QString schema;
    QString key;
    QString value;
    QString comparison;
};

struct KeyListEntries
{
    // gsettings路径
    QString schema;
    // domainname，可以翻译description字段中的内容
    QString package;
    // 按键绑定所属分类
    QString name;
    // 窗口管理器按键绑定组名
    QString wmName;
    // 具体的按键绑定列表
    QVector<KeyListEntry> entries;
};

// 这里是为了兼容mate-control-center中已有的按键绑定配置
class KeyListEntriesParser
{
public:
    KeyListEntriesParser(const QString &keyEntryDir);
    virtual ~KeyListEntriesParser(){};

    bool parse(QVector<KeyListEntries> &keys, QString &error);

private:
    bool processKeylistEntries(const xmlpp::Node *node, KeyListEntries &keylistEntries, QString &error);
    bool processKeylistEntry(const xmlpp::Node *node, KeyListEntry &keylistEntry, QString &error);

private:
    QString m_keyEntryDir;
};

}  // namespace Kiran
