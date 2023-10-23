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

#include "lib/iso/iso-translation.h"

#include <glib/gi18n.h>
#include <libxml++/libxml++.h>

#include "lib/base/base.h"

#define ISO_3166 "iso_3166"
#define ISO_639 "iso_639"

#define ISO_CODES_DIR KCC_ISO_CODES_PREFIX "/share/xml/iso-codes/"
#define ISO_LOCALEDIR KCC_ISO_CODES_PREFIX "/share/locale"

namespace Kiran
{
ISOTranslation *ISOTranslation::instance_ = nullptr;
void ISOTranslation::global_init()
{
    RETURN_IF_TRUE(instance_);
    instance_ = new ISOTranslation();
    instance_->init();
}

void ISOTranslation::global_deinit()
{
    delete instance_;
    instance_ = nullptr;
}

std::string ISOTranslation::get_country_name(const std::string &code)
{
    auto iter = this->countrys_.find(code);
    return (iter == this->countrys_.end()) ? std::string() : iter->second;
}

std::string ISOTranslation::get_locale_country_name(const std::string &code)
{
    auto name = this->get_country_name(code);
    RETURN_VAL_IF_TRUE(name.length() == 0, name);
    auto locale_name = dgettext(ISO_3166, name.c_str());

    return POINTER_TO_STRING(locale_name);
}

std::string ISOTranslation::get_language_name(const std::string &code)
{
    auto iter = this->languages_.find(code);
    return (iter == this->languages_.end()) ? std::string() : iter->second;
}

std::string ISOTranslation::get_locale_string(const std::string &str, const std::string &delimeters)
{
    size_t start = 0;
    std::string result;

    for (size_t pos = 0; pos < str.length(); ++pos)
    {
        if (std::find(delimeters.begin(), delimeters.end(), str[pos]) != delimeters.end())
        {
            if (pos > start)
            {
                auto token = str.substr(start, pos - start);
                result.append(this->get_locale_string(token));
            }
            result.push_back(str[pos]);
            start = pos + 1;
        }
    }
    if (start != str.length())
    {
        auto token = str.substr(start, str.length() - start);
        result.append(this->get_locale_string(token));
    }
    return result;
}

std::string ISOTranslation::get_locale_string(const std::string &str)
{
    auto str_639 = dgettext(ISO_639, str.c_str());
    RETURN_VAL_IF_TRUE(str_639 && str_639 != str, std::string(str_639));

    auto str_3166 = dgettext(ISO_3166, str.c_str());
    RETURN_VAL_IF_TRUE(str_3166 && str_3166 != str, std::string(str_3166));

    return str;
}

void ISOTranslation::init()
{
    std::string err;

    bindtextdomain(ISO_3166, ISO_LOCALEDIR);
    bind_textdomain_codeset(ISO_3166, "UTF-8");

    bindtextdomain(ISO_639, ISO_LOCALEDIR);
    bind_textdomain_codeset(ISO_639, "UTF-8");

    if (!load_iso_file(ISO_3166, {"alpha_2_code"}, this->countrys_, err))
    {
        KLOG_WARNING("Failed to load %s: %s.", ISO_3166, err.c_str());
    }

    if (!load_iso_file(ISO_639, {"iso_639_2B_code", "iso_639_2T_code"}, this->languages_, err))
    {
        KLOG_WARNING("Failed to load %s: %s.", ISO_639, err.c_str());
    }
}

bool ISOTranslation::load_iso_file(const std::string &iso_basename,
                                   const std::vector<std::string> &code_attr_names,
                                   std::map<std::string, std::string> &result,
                                   std::string &err)
{
    try
    {
        xmlpp::DomParser parser;
        parser.set_throw_messages(true);
        parser.parse_file(ISO_CODES_DIR + iso_basename + ".xml");
        if (parser)
        {
            const auto root_node = parser.get_document()->get_root_node();
            RETURN_VAL_IF_FALSE(this->process_iso_entries(root_node, iso_basename, code_attr_names, result, err), false);
        }
    }
    catch (const std::exception &e)
    {
        err = e.what();
        return false;
    }
    return true;
}

bool ISOTranslation::process_iso_entries(const xmlpp::Node *node,
                                         const std::string &iso_basename,
                                         const std::vector<std::string> &code_attr_names,
                                         std::map<std::string, std::string> &result,
                                         std::string &err)
{
    CHECK_XMLPP_ELEMENT(node, err);

    for (const auto &child : node->get_children())
    {
        if (child->get_name() == (iso_basename + "_entry"))
        {
            RETURN_VAL_IF_FALSE(this->process_iso_entry(child, iso_basename, code_attr_names, result, err), false);
        }
    }
    return true;
}

bool ISOTranslation::process_iso_entry(const xmlpp::Node *node,
                                       const std::string &iso_basename,
                                       const std::vector<std::string> &code_attr_names,
                                       std::map<std::string, std::string> &result,
                                       std::string &err)
{
    const auto element = dynamic_cast<const xmlpp::Element *>(node);

    if (!element)
    {
        err = fmt::format("the type of the node '{0}' isn't xmlpp::Element.", node->get_name().c_str());
        return false;
    }

    const auto attributes = element->get_attributes();
    Glib::ustring name_value;

    for (const auto &attribute : attributes)
    {
        if (attribute->get_name() == "name")
        {
            name_value = attribute->get_value();
        }
    }

    if (name_value.length() > 0)
    {
        for (const auto &attribute : attributes)
        {
            auto name = attribute->get_name();
            if (std::find(code_attr_names.begin(), code_attr_names.end(), name.raw()) != code_attr_names.end())
            {
                result[attribute->get_value()] = name_value;
            }
        }
    }
    return true;
}
}  // namespace Kiran