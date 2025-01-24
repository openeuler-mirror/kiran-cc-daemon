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

#include "xkb-rules-parser.h"
#include <libxml++/libxml++.h>
#include "lib/base/base.h"

namespace Kiran
{
XkbRulesParser::XkbRulesParser(const QString &fileName) : m_fileName(fileName)
{
}

bool XkbRulesParser::parse(XkbRules &xkbRules, QString &error)
{
    try
    {
        xmlpp::DomParser parser;
        parser.set_throw_messages(true);
        parser.parse_file(m_fileName.toLatin1().data());
        if (parser)
        {
            const auto rootNode = parser.get_document()->get_root_node();
            RETURN_VAL_IF_FALSE(processConfigRegistry(rootNode, xkbRules, error), false);
        }
    }
    catch (const std::exception &e)
    {
        error = e.what();
        return false;
    }
    return true;
}

bool XkbRulesParser::processConfigRegistry(const xmlpp::Node *node, XkbRules &xkbRules, QString &error)
{
    CHECK_XMLPP_ELEMENT(node, error);

    KLOG_DEBUG(keyboard) << "Node name is" << node->get_name().c_str();

    for (const auto &child : node->get_children())
    {
        switch (shash(child->get_name().c_str()))
        {
        case "modelList"_hash:
            RETURN_VAL_IF_FALSE(processModels(child, xkbRules.models, error), false);
            break;
        case "layoutList"_hash:
            RETURN_VAL_IF_FALSE(processLayouts(child, xkbRules.layouts, error), false);
            break;
        case "optionList"_hash:
            RETURN_VAL_IF_FALSE(processOptionGroups(child, xkbRules.option_groups, error), false);
            break;
        case "text"_hash:
            // no deal
            break;
        default:
            KLOG_DEBUG(keyboard) << "Ignore node" << child->get_name().c_str();
            break;
        }
    }
    return true;
}

bool XkbRulesParser::processModels(const xmlpp::Node *node, QVector<XkbModel> &xkb_models, QString &error)
{
    CHECK_XMLPP_ELEMENT(node, error);

    for (const auto &child : node->get_children())
    {
        switch (shash(child->get_name().c_str()))
        {
        case "model"_hash:
        {
            XkbModel xkb_model;
            RETURN_VAL_IF_FALSE(processModel(child, xkb_model, error), false);
            xkb_models.push_back(std::move(xkb_model));
        }
        break;
        case "text"_hash:
            break;
        default:
            KLOG_DEBUG(keyboard) << "Ignore node" << child->get_name().c_str();
            break;
        }
    }
    return true;
}

bool XkbRulesParser::processModel(const xmlpp::Node *node, XkbModel &xkb_model, QString &err)
{
    CHECK_XMLPP_ELEMENT(node, err);

    for (const auto &child : node->get_children())
    {
        switch (shash(child->get_name().c_str()))
        {
        case "configItem"_hash:
            RETURN_VAL_IF_FALSE(processModelConfigItem(child, xkb_model, err), false);
            break;
        case "text"_hash:
            break;
        default:
            KLOG_DEBUG(keyboard) << "Ignore node" << child->get_name().c_str();
            break;
        }
    }
    return true;
}

bool XkbRulesParser::processModelConfigItem(const xmlpp::Node *node, XkbModel &xkb_model, QString &err)
{
    CHECK_XMLPP_ELEMENT(node, err);

    for (const auto &child : node->get_children())
    {
        switch (shash(child->get_name().c_str()))
        {
        case "name"_hash:
            RETURN_VAL_IF_FALSE(processContentNode(child, xkb_model.name, err), false);
            break;
        case "description"_hash:
            RETURN_VAL_IF_FALSE(processContentNode(child, xkb_model.description, err), false);
            break;
        case "vendor"_hash:
            RETURN_VAL_IF_FALSE(processContentNode(child, xkb_model.vendor, err), false);
            break;
        case "text"_hash:
            break;
        default:
            KLOG_DEBUG(keyboard) << "Ignore node" << child->get_name().c_str();
            break;
        }
    }
    return true;
}

bool XkbRulesParser::processLayouts(const xmlpp::Node *node, QVector<XkbLayout> &xkb_layouts, QString &err)
{
    CHECK_XMLPP_ELEMENT(node, err);

    for (const auto &child : node->get_children())
    {
        switch (shash(child->get_name().c_str()))
        {
        case "layout"_hash:
        {
            XkbLayout xkb_layout;
            RETURN_VAL_IF_FALSE(processLayout(child, xkb_layout, err), false);
            xkb_layouts.push_back(std::move(xkb_layout));
        }
        break;
        case "text"_hash:
            break;
        default:
            KLOG_DEBUG(keyboard) << "Ignore node" << child->get_name().c_str();
            break;
        }
    }
    return true;
}

bool XkbRulesParser::processLayout(const xmlpp::Node *node, XkbLayout &xkb_layout, QString &err)
{
    CHECK_XMLPP_ELEMENT(node, err);

    for (const auto &child : node->get_children())
    {
        switch (shash(child->get_name().c_str()))
        {
        case "configItem"_hash:
            RETURN_VAL_IF_FALSE(processLayoutConfigItem(child, xkb_layout, err), false);
            break;
        case "variantList"_hash:
            RETURN_VAL_IF_FALSE(processLayoutVariants(child, xkb_layout.variants, err), false);
            break;
        case "text"_hash:
            break;
        default:
            KLOG_DEBUG(keyboard) << "Ignore node" << child->get_name().c_str();
            break;
        }
    }
    return true;
}

bool XkbRulesParser::processLayoutConfigItem(const xmlpp::Node *node, XkbLayout &xkb_layout, QString &err)
{
    CHECK_XMLPP_ELEMENT(node, err);

    for (const auto &child : node->get_children())
    {
        switch (shash(child->get_name().c_str()))
        {
        case "name"_hash:
            RETURN_VAL_IF_FALSE(processContentNode(child, xkb_layout.name, err), false);
            break;
        case "shortDescription"_hash:
            RETURN_VAL_IF_FALSE(processContentNode(child, xkb_layout.short_description, err), false);
            break;
        case "description"_hash:
            RETURN_VAL_IF_FALSE(processContentNode(child, xkb_layout.description, err), false);
            break;
        case "languageList"_hash:
        case "countryList"_hash:
        case "text"_hash:
            break;
        default:
            KLOG_DEBUG(keyboard) << "Ignore node" << child->get_name().c_str();
            break;
        }
    }
    return true;
}

bool XkbRulesParser::processLayoutVariants(const xmlpp::Node *node, QVector<XkbVariant> &xkb_variants, QString &err)
{
    CHECK_XMLPP_ELEMENT(node, err);

    for (const auto &child : node->get_children())
    {
        switch (shash(child->get_name().c_str()))
        {
        case "variant"_hash:
        {
            XkbVariant xkb_variant;
            RETURN_VAL_IF_FALSE(processLayoutVariant(child, xkb_variant, err), false);
            xkb_variants.push_back(std::move(xkb_variant));
        }
        break;
        case "text"_hash:
            break;
        default:
            KLOG_DEBUG(keyboard) << "Ignore node" << child->get_name().c_str();
            break;
        }
    }
    return true;
}

bool XkbRulesParser::processLayoutVariant(const xmlpp::Node *node, XkbVariant &xkb_variant, QString &err)
{
    CHECK_XMLPP_ELEMENT(node, err);

    for (const auto &child : node->get_children())
    {
        switch (shash(child->get_name().c_str()))
        {
        case "configItem"_hash:
            RETURN_VAL_IF_FALSE(processLayoutVariantConfigItem(child, xkb_variant, err), false);
            break;
        case "text"_hash:
            break;
        default:
            KLOG_DEBUG(keyboard) << "Ignore node" << child->get_name().c_str();
            break;
        }
    }
    return true;
}

bool XkbRulesParser::processLayoutVariantConfigItem(const xmlpp::Node *node, XkbVariant &xkb_variant, QString &err)
{
    CHECK_XMLPP_ELEMENT(node, err);

    for (const auto &child : node->get_children())
    {
        switch (shash(child->get_name().c_str()))
        {
        case "name"_hash:
            RETURN_VAL_IF_FALSE(processContentNode(child, xkb_variant.name, err), false);
            break;
        case "shortDescription"_hash:
            RETURN_VAL_IF_FALSE(processContentNode(child, xkb_variant.short_description, err), false);
            break;
        case "description"_hash:
            RETURN_VAL_IF_FALSE(processContentNode(child, xkb_variant.description, err), false);
            break;
        case "text"_hash:
        case "languageList"_hash:
            break;
        default:
            KLOG_DEBUG(keyboard) << "Ignore node" << child->get_name().c_str();
            break;
        }
    }
    return true;
}

bool XkbRulesParser::processOptionGroups(const xmlpp::Node *node, QVector<XkbOptionGroup> &xkb_option_groups, QString &err)
{
    CHECK_XMLPP_ELEMENT(node, err);

    for (const auto &child : node->get_children())
    {
        switch (shash(child->get_name().c_str()))
        {
        case "group"_hash:
        {
            XkbOptionGroup xkb_option_group;
            RETURN_VAL_IF_FALSE(processOptionGroup(child, xkb_option_group, err), false);
            xkb_option_groups.push_back(std::move(xkb_option_group));
        }
        break;
        case "text"_hash:
            break;
        default:
            KLOG_DEBUG(keyboard) << "Ignore node" << child->get_name().c_str();
            break;
        }
    }
    return true;
}

bool XkbRulesParser::processOptionGroup(const xmlpp::Node *node, XkbOptionGroup &xkb_option_group, QString &err)
{
    CHECK_XMLPP_ELEMENT(node, err);

    for (const auto &child : node->get_children())
    {
        switch (shash(child->get_name().c_str()))
        {
        case "configItem"_hash:
            RETURN_VAL_IF_FALSE(processOptionGroupConfigItem(child, xkb_option_group, err), false);
            break;
        case "option"_hash:
        {
            XkbOption xkb_option;
            RETURN_VAL_IF_FALSE(processOption(child, xkb_option, err), false);
            xkb_option_group.options.push_back(std::move(xkb_option));
        }
        break;
        case "text"_hash:
            break;
        default:
            KLOG_DEBUG(keyboard) << "Ignore node" << child->get_name().c_str();
            break;
        }
    }
    return true;
}

bool XkbRulesParser::processOptionGroupConfigItem(const xmlpp::Node *node, XkbOptionGroup &xkb_option_group, QString &err)
{
    CHECK_XMLPP_ELEMENT(node, err);

    for (const auto &child : node->get_children())
    {
        switch (shash(child->get_name().c_str()))
        {
        case "name"_hash:
            RETURN_VAL_IF_FALSE(processContentNode(child, xkb_option_group.name, err), false);
            break;
        case "description"_hash:
            RETURN_VAL_IF_FALSE(processContentNode(child, xkb_option_group.description, err), false);
            break;
        case "text"_hash:
            break;
        default:
            KLOG_DEBUG(keyboard) << "Ignore node" << child->get_name().c_str();
            break;
        }
    }
    return true;
}

bool XkbRulesParser::processOption(const xmlpp::Node *node, XkbOption &xkb_option, QString &err)
{
    CHECK_XMLPP_ELEMENT(node, err);

    for (const auto &child : node->get_children())
    {
        switch (shash(child->get_name().c_str()))
        {
        case "configItem"_hash:
            RETURN_VAL_IF_FALSE(processOptionConfigItem(child, xkb_option, err), false);
            break;
        case "text"_hash:
            break;
        default:
            KLOG_DEBUG(keyboard) << "Ignore node" << child->get_name().c_str();
            break;
        }
    }
    return true;
}

bool XkbRulesParser::processOptionConfigItem(const xmlpp::Node *node, XkbOption &xkb_option, QString &err)
{
    CHECK_XMLPP_ELEMENT(node, err);

    for (const auto &child : node->get_children())
    {
        switch (shash(child->get_name().c_str()))
        {
        case "name"_hash:
            RETURN_VAL_IF_FALSE(processContentNode(child, xkb_option.name, err), false);
            break;
        case "description"_hash:
            RETURN_VAL_IF_FALSE(processContentNode(child, xkb_option.description, err), false);
            break;
        case "text"_hash:
            break;
        default:
            KLOG_DEBUG(keyboard) << "Ignore node" << child->get_name().c_str();
            break;
        }
    }
    return true;
}

bool XkbRulesParser::processContentNode(const xmlpp::Node *node, QString &content, QString &error)
{
    const auto element = dynamic_cast<const xmlpp::Element *>(node);

    if (!element)
    {
        error = QString("The type of the node %1 isn't xmlpp::Element.").arg(node->get_name().c_str());
        return false;
    }

    auto text_node = element->get_child_text();

    if (!text_node)
    {
        error = QString("The node %1 hasn't xmlpp::TextNode.").arg(node->get_name().c_str());
        return false;
    }

    KLOG_DEBUG(keyboard) << "The text of node" << node->get_name().c_str() << "is" << text_node->get_name().c_str();

    content = text_node->get_content().c_str();
    return true;
}
}  // namespace  Kiran
