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

#include "iso-translation.h"
#include <glib/gi18n.h>
#include <libxml++/libxml++.h>
#include "lib/base/base.h"

#define ISO_3166 "iso_3166"
#define ISO_639 "iso_639"

#define ISO_CODES_DIR KCC_ISO_CODES_PREFIX "/share/xml/iso-codes/"
#define ISO_LOCALEDIR KCC_ISO_CODES_PREFIX "/share/locale"

namespace Kiran
{

ISOTranslation *ISOTranslation::m_instance = nullptr;
void ISOTranslation::globalInit()
{
    m_instance = new ISOTranslation();
    m_instance->init();
}

void ISOTranslation::globalDeinit()
{
    delete m_instance;
    m_instance = nullptr;
}

QString ISOTranslation::getLocaleCountryName(const QString &code)
{
    auto name = getCountryName(code);
    RETURN_VAL_IF_TRUE(name.length() == 0, name);
    auto localeName = dgettext(ISO_3166, name.toLatin1().data());

    return POINTER_TO_STRING(localeName);
}

QString ISOTranslation::getLocaleString(const QString &str, const QString &delimeters)
{
    size_t start = 0;
    QString result;

    for (size_t pos = 0; pos < str.length(); ++pos)
    {
        if (delimeters.contains(str.at(pos)))
        {
            if (pos > start)
            {
                auto token = str.mid(start, pos - start);
                result.append(getLocaleString(token));
            }
            result.push_back(str.at(pos));
            start = pos + 1;
        }
    }
    if (start != str.length())
    {
        auto token = str.mid(start, str.length() - start);
        result.append(getLocaleString(token));
    }
    return result;
}

QString ISOTranslation::getLocaleString(const QString &str)
{
    auto str639 = dgettext(ISO_639, str.toLatin1().data());
    RETURN_VAL_IF_TRUE(str639 && QString(str639) != str, QString(str639));

    auto str3166 = dgettext(ISO_3166, str.toLatin1().data());
    RETURN_VAL_IF_TRUE(str3166 && QString(str3166) != str, QString(str3166));

    return str;
}

void ISOTranslation::init()
{
    QString error;

    bindtextdomain(ISO_3166, ISO_LOCALEDIR);
    bind_textdomain_codeset(ISO_3166, "UTF-8");

    bindtextdomain(ISO_639, ISO_LOCALEDIR);
    bind_textdomain_codeset(ISO_639, "UTF-8");

    if (!loadISOFile(ISO_3166, {"alpha_2_code"}, m_countrys, error))
    {
        KLOG_WARNING(keyboard) << "Failed to load" << ISO_3166 << ", error is" << error;
    }

    if (!loadISOFile(ISO_639, {"iso_639_2B_code", "iso_639_2T_code"}, m_languages, error))
    {
        KLOG_WARNING(keyboard) << "Failed to load" << ISO_639 << ", error is " << error;
    }
}

bool ISOTranslation::loadISOFile(const QString &isoBasename,
                                 const QStringList &codeAttrNames,
                                 QMap<QString, QString> &result,
                                 QString &error)
{
    try
    {
        xmlpp::DomParser parser;
        parser.set_throw_messages(true);
        auto fileName = QString("%1%2.xml").arg(QString(ISO_CODES_DIR)).arg(isoBasename);
        parser.parse_file(fileName.toLatin1().data());
        if (parser)
        {
            const auto rootNode = parser.get_document()->get_root_node();
            RETURN_VAL_IF_FALSE(processISOEntries(rootNode, isoBasename, codeAttrNames, result, error), false);
        }
    }
    catch (const std::exception &e)
    {
        error = e.what();
        return false;
    }
    return true;
}

bool ISOTranslation::processISOEntries(const xmlpp::Node *node,
                                       const QString &isoBasename,
                                       const QStringList &codeAttrNames,
                                       QMap<QString, QString> &result,
                                       QString &error)
{
    CHECK_XMLPP_ELEMENT(node, error);

    for (const auto &child : node->get_children())
    {
        auto name = QString(child->get_name().c_str());
        if (name == (isoBasename + "_entry"))
        {
            RETURN_VAL_IF_FALSE(processISOEntry(child, isoBasename, codeAttrNames, result, error), false);
        }
    }
    return true;
}

bool ISOTranslation::processISOEntry(const xmlpp::Node *node,
                                     const QString &isoBasename,
                                     const QStringList &codeAttrNames,
                                     QMap<QString, QString> &result,
                                     QString &error)
{
    const auto element = dynamic_cast<const xmlpp::Element *>(node);

    if (!element)
    {
        error = QString("the type of the node %1 isn't xmlpp::Element.").arg(node->get_name().c_str());
        return false;
    }

    const auto attributes = element->get_attributes();
    QString nameValue;

    for (const auto &attribute : attributes)
    {
        if (attribute->get_name() == "name")
        {
            nameValue = attribute->get_value().c_str();
        }
    }

    if (nameValue.length() > 0)
    {
        for (const auto &attribute : attributes)
        {
            auto name = QString(attribute->get_name().c_str());
            if (codeAttrNames.contains(name))
            {
                QString key = QString(attribute->get_value().c_str());
                result[key] = nameValue;
            }
        }
    }
    return true;
}
}  // namespace Kiran