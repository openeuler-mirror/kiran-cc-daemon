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
#include <QStringList>

namespace xmlpp
{
class Node;
};

namespace Kiran
{
class ISOTranslation : public QObject
{
    Q_OBJECT

public:
    ISOTranslation(){};
    virtual ~ISOTranslation(){};

    static ISOTranslation *getInstance() { return m_instance; };

    static void globalInit();

    static void globalDeinit();

public:
    // 通过code获得城市/地区的名字
    QString getCountryName(const QString &code) { return m_countrys.value(code); };
    // 通过code获得城市/地区的名字，并对名字进行本地化
    QString getLocaleCountryName(const QString &code);

    // 通过code获得语言名
    QString getLanguageName(const QString &code) { return m_languages.value(code); };
    // 对字符串进行本地化
    QString getLocaleString(const QString &str, const QString &delimeters);
    QString getLocaleString(const QString &str);

private:
    void init();

    bool loadISOFile(const QString &isoBasename,
                     const QStringList &codeAttrNames,
                     QMap<QString, QString> &result,
                     QString &error);

    bool processISOEntries(const xmlpp::Node *node,
                           const QString &isoBasename,
                           const QStringList &codeAttrNames,
                           QMap<QString, QString> &result,
                           QString &error);

    bool processISOEntry(const xmlpp::Node *node,
                         const QString &isoBasename,
                         const QStringList &codeAttrNames,
                         QMap<QString, QString> &result,
                         QString &error);

private:
    static ISOTranslation *m_instance;

    // 格式：<code, name>
    QMap<QString, QString> m_countrys;
    // 格式：<code, name>
    QMap<QString, QString> m_languages;
};
}  // namespace Kiran