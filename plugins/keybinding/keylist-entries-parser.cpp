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

#include "plugins/keybinding/keylist-entries-parser.h"

#include <libxml++/libxml++.h>

#include "lib/base/base.h"

namespace Kiran
{
KeyListEntriesParser::KeyListEntriesParser(const std::string &key_entry_dir) : key_entry_dir_(key_entry_dir)
{
}

bool KeyListEntriesParser::parse(std::vector<KeyListEntries> &keys, std::string &err)
{
    try
    {
        Glib::Dir dir(this->key_entry_dir_);
        for (const auto &file_name : dir)
        {
            auto file_path = Glib::build_filename(this->key_entry_dir_, file_name);
            xmlpp::DomParser parser;
            parser.set_throw_messages(true);
            parser.parse_file(file_path);
            if (parser)
            {
                const auto root_node = parser.get_document()->get_root_node();
                KeyListEntries keylist_entries;
                if (!this->process_keylist_entries(root_node, keylist_entries, err))
                {
                    KLOG_WARNING_KEYBINDING("Failed to paerse %s: %s. ignore it.", file_path.c_str(), err.c_str());
                    continue;
                }
                keys.push_back(std::move(keylist_entries));
            }
        }
    }
    catch (const Glib::Error &e)
    {
        err = e.what().raw();
        return false;
    }
    return true;
}

bool KeyListEntriesParser::process_keylist_entries(const xmlpp::Node *node, KeyListEntries &keylist_entries, std::string &err)
{
    const auto element = dynamic_cast<const xmlpp::Element *>(node);

    if (!element)
    {
        err = fmt::format("The type of the node '{0}' isn't xmlpp::Element.", node->get_name().c_str());
        return false;
    }

    for (const auto &child : element->get_attributes())
    {
        switch (shash(child->get_name().c_str()))
        {
        case "schema"_hash:
            keylist_entries.schema = child->get_value().raw();
            break;
        case "package"_hash:
            keylist_entries.package = child->get_value().raw();
            break;
        case "name"_hash:
            keylist_entries.name = child->get_value().raw();
            break;
        case "wm_name"_hash:
            keylist_entries.wm_name = child->get_value().raw();
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
            KeyListEntry keylist_entry;
            RETURN_VAL_IF_FALSE(this->process_keylist_entry(child, keylist_entry, err), false);
            keylist_entries.entries_.push_back(std::move(keylist_entry));
        }
        break;
        case "text"_hash:
            // no deal
            break;
        default:
            KLOG_DEBUG_KEYBINDING("Ignore node: %s.", child->get_name().c_str());
            break;
        }
    }
    return true;
}

bool KeyListEntriesParser::process_keylist_entry(const xmlpp::Node *node, KeyListEntry &keylist_entry, std::string &err)
{
    const auto element = dynamic_cast<const xmlpp::Element *>(node);

    if (!element)
    {
        err = fmt::format("The type of the node '{0}' isn't xmlpp::Element.", node->get_name().c_str());
        return false;
    }

    for (const auto &child : element->get_attributes())
    {
        switch (shash(child->get_name().c_str()))
        {
        case "name"_hash:
            keylist_entry.name = child->get_value().raw();
            break;
        case "description"_hash:
            keylist_entry.description = child->get_value().raw();
            break;
        case "schema"_hash:
            keylist_entry.schema = child->get_value().raw();
            break;
        case "key"_hash:
            keylist_entry.key = child->get_value().raw();
            break;
        case "value"_hash:
            keylist_entry.value = child->get_value().raw();
            break;
        case "comparison"_hash:
            keylist_entry.comparison = child->get_value().raw();
            break;
        default:
            break;
        }
    }
    return true;
}
}  // namespace Kiran