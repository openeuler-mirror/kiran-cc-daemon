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

#include "keylist-entries-parser.h"
#include <libxml++/libxml++.h>
#include <QDir>
#include "lib/base/base.h"

namespace Kiran
{
KeyListEntriesParser::KeyListEntriesParser(const QString &keyEntryDir) : m_keyEntryDir(keyEntryDir)
{
}

bool KeyListEntriesParser::parse(QVector<KeyListEntries> &keys, QString &error)
{
    try
    {
        QDir dir(m_keyEntryDir);

        for (const auto &fileInfo : dir.entryInfoList(QDir::Files))
        {
            auto filePath = fileInfo.absoluteFilePath();
            xmlpp::DomParser parser;
            parser.set_throw_messages(true);
            parser.parse_file(filePath.toStdString());
            if (parser)
            {
                const auto rootNode = parser.get_document()->get_root_node();
                KeyListEntries keylistEntries;
                if (!processKeylistEntries(rootNode, keylistEntries, error))
                {
                    KLOG_WARNING(keybinding) << "Failed to paerse" << filePath << ":" << error << ". ignore it.";
                    continue;
                }
                keys.push_back(std::move(keylistEntries));
            }
        }
    }
    catch (const std::exception &e)
    {
        error = e.what();
        return false;
    }
    return true;
}

bool KeyListEntriesParser::processKeylistEntries(const xmlpp::Node *node, KeyListEntries &keylistEntries, QString &error)
{
    const auto element = dynamic_cast<const xmlpp::Element *>(node);

    if (!element)
    {
        error = QString("The type of the node '%1' isn't xmlpp::Element.").arg(node->get_name().c_str());
        return false;
    }

    for (const auto &child : element->get_attributes())
    {
        switch (shash(child->get_name().c_str()))
        {
        case "schema"_hash:
            keylistEntries.schema = child->get_value().c_str();
            break;
        case "package"_hash:
            keylistEntries.package = child->get_value().c_str();
            break;
        case "name"_hash:
            keylistEntries.name = child->get_value().c_str();
            break;
        case "wm_name"_hash:
            keylistEntries.wmName = child->get_value().c_str();
            break;
        default:
            break;
        }
    }

    for (const auto &child : node->get_children())
    {
        switch (shash(child->get_name().c_str()))
        {
        case "KeyListEntry"_hash:
        {
            KeyListEntry keylistEntry;
            RETURN_VAL_IF_FALSE(processKeylistEntry(child, keylistEntry, error), false);
            keylistEntries.entries.push_back(std::move(keylistEntry));
        }
        break;
        case "text"_hash:
            // no deal
            break;
        default:
            KLOG_DEBUG(keybinding) << "Ignore node" << child->get_name().c_str();
            break;
        }
    }
    return true;
}

bool KeyListEntriesParser::processKeylistEntry(const xmlpp::Node *node, KeyListEntry &keylistEntry, QString &error)
{
    const auto element = dynamic_cast<const xmlpp::Element *>(node);

    if (!element)
    {
        error = QString("The type of the node '%1' isn't xmlpp::Element.").arg(node->get_name().c_str());
        return false;
    }

    for (const auto &child : element->get_attributes())
    {
        switch (shash(child->get_name().c_str()))
        {
        case "name"_hash:
            keylistEntry.name = child->get_value().c_str();
            break;
        case "description"_hash:
            keylistEntry.description = child->get_value().c_str();
            break;
        case "schema"_hash:
            keylistEntry.schema = child->get_value().c_str();
            break;
        case "key"_hash:
            keylistEntry.key = child->get_value().c_str();
            break;
        case "value"_hash:
            keylistEntry.value = child->get_value().c_str();
            break;
        case "comparison"_hash:
            keylistEntry.comparison = child->get_value().c_str();
            break;
        default:
            break;
        }
    }
    return true;
}
}  // namespace Kiran