/**
 * Copyright (c) 2026 ~ 2027 KylinSec Co., Ltd.
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

#include <QtTest>

class GroupsProxy;
class AccountsProxy;

namespace Kiran
{

class AccountsPlugin;
class GroupsPlugin;

class TestGroups : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testCreateGroup();

private:
    AccountsPlugin *m_accountsPlugin = nullptr;
    GroupsPlugin *m_groupsPlugin = nullptr;
    AccountsProxy *m_accountsProxy = nullptr;
    GroupsProxy *m_groupsProxy = nullptr;
};

}  // namespace Kiran