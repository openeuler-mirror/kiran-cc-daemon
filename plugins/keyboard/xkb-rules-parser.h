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

#include <QString>
#include <QVector>

namespace xmlpp
{
class Node;
};

namespace Kiran
{
struct XkbModel
{
    QString name;
    QString description;
    QString vendor;
};

struct XkbVariant
{
    QString name;
    QString short_description;
    QString description;
};

struct XkbLayout
{
    QString name;
    QString short_description;
    QString description;
    QVector<XkbVariant> variants;
};

struct XkbOption
{
    QString name;
    QString description;
};

struct XkbOptionGroup
{
    QString name;
    QString description;
    QVector<XkbOption> options;
};

struct XkbRules
{
    QVector<XkbModel> models;
    QVector<XkbLayout> layouts;
    QVector<XkbOptionGroup> option_groups;
};

class XkbRulesParser
{
public:
    XkbRulesParser(const QString &fileName);
    virtual ~XkbRulesParser(){};

    bool parse(XkbRules &xkbRules, QString &error);

private:
    bool processConfigRegistry(const xmlpp::Node *node, XkbRules &xkb_rules, QString &error);

    bool processModels(const xmlpp::Node *node, QVector<XkbModel> &xkb_models, QString &error);
    bool processModel(const xmlpp::Node *node, XkbModel &xkb_model, QString &error);
    bool processModelConfigItem(const xmlpp::Node *node, XkbModel &xkb_model, QString &error);

    bool processLayouts(const xmlpp::Node *node, QVector<XkbLayout> &xkb_layouts, QString &error);
    bool processLayout(const xmlpp::Node *node, XkbLayout &xkb_layout, QString &error);
    bool processLayoutConfigItem(const xmlpp::Node *node, XkbLayout &xkb_layout, QString &error);
    bool processLayoutVariants(const xmlpp::Node *node, QVector<XkbVariant> &xkb_variants, QString &error);
    bool processLayoutVariant(const xmlpp::Node *node, XkbVariant &xkb_variant, QString &error);
    bool processLayoutVariantConfigItem(const xmlpp::Node *node, XkbVariant &xkb_variant, QString &error);

    bool processOptionGroups(const xmlpp::Node *node, QVector<XkbOptionGroup> &xkb_option_groups, QString &error);
    bool processOptionGroup(const xmlpp::Node *node, XkbOptionGroup &xkb_option_group, QString &error);
    bool processOptionGroupConfigItem(const xmlpp::Node *node, XkbOptionGroup &xkb_option_group, QString &error);
    bool processOption(const xmlpp::Node *node, XkbOption &xkb_option, QString &error);
    bool processOptionConfigItem(const xmlpp::Node *node, XkbOption &xkb_option, QString &error);

    bool processContentNode(const xmlpp::Node *node, QString &content, QString &error);

private:
    QString m_fileName;
};
}  // namespace Kiran