/*
 * @Author       : tangjie02
 * @Date         : 2020-08-20 11:49:39
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-08-26 13:56:56
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/inputdevices/keyboard/iso-translation.cpp
 */

#include "plugins/inputdevices/keyboard/iso-translation.h"

#include <glib/gi18n.h>
#include <libxml++/libxml++.h>

#include "lib/log.h"

#define ISO_3166 "iso_3166"
#define ISO_639 "iso_639"

#define ISO_LOCALEDIR KCC_ISO_CODES_PREFIX "/share/locale"

namespace Kiran
{
ISOTranslation::ISOTranslation(const std::string &iso_dir) : iso_dir_(iso_dir)
{
    std::string err;

    bindtextdomain(ISO_3166, ISO_LOCALEDIR);
    bind_textdomain_codeset(ISO_3166, "UTF-8");

    bindtextdomain(ISO_639, ISO_LOCALEDIR);
    bind_textdomain_codeset(ISO_639, "UTF-8");

    if (!load_iso_file(ISO_3166, {"alpha_2_code"}, this->countrys_, err))
    {
        LOG_WARNING("failed to load %s: %s.", ISO_3166, err.c_str());
    }

    if (!load_iso_file(ISO_639, {"iso_639_2B_code", "iso_639_2T_code"}, this->languages_, err))
    {
        LOG_WARNING("failed to load %s: %s.", ISO_639, err.c_str());
    }
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
    // LOG_DEBUG("str: %s value: %s.", str.c_str(), POINTER_TO_STRING(str_639).c_str());
    RETURN_VAL_IF_TRUE(str_639 && str_639 != str, std::string(str_639));

    auto str_3166 = dgettext(ISO_3166, str.c_str());
    // LOG_DEBUG("str: %s value: %s.", str.c_str(), POINTER_TO_STRING(str_3166).c_str());
    RETURN_VAL_IF_TRUE(str_3166 && str_3166 != str, std::string(str_3166));

    return str;
}

bool ISOTranslation::load_iso_file(const std::string &iso_basename,
                                   const std::vector<std::string> &code_attr_names,
                                   std::map<std::string, std::string> &result,
                                   std::string &err)
{
    SETTINGS_PROFILE("basename: %s.", iso_basename.c_str());

    try
    {
        xmlpp::DomParser parser;
        parser.set_throw_messages(true);
        parser.parse_file(this->iso_dir_ + iso_basename + ".xml");
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