/**
 * Copyright (c) 2020 ~ 2025 KylinSec Co., Ltd.
 * kiran-cc-daemon is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     yuanxing <yuanxing@kylinsec.com.cn>
 */

#pragma once

#include <error-i.h>
#include <upgrade-i.h>
#include <QDateTime>
#include <QObject>
#include <QString>
#include <QStringList>

class QSqlDatabase;
class QSqlQuery;

namespace Kiran
{
/**
 * @brief 升级历史记录数据库管理类
 * 使用SQLite数据库存储升级历史记录
 */
class UpgradeHistoryDB : public QObject
{
    Q_OBJECT

public:
    explicit UpgradeHistoryDB(QObject *parent = nullptr);
    ~UpgradeHistoryDB();

    /**
     * @brief 初始化数据库（创建表等）
     * @return 是否成功
     */
    bool initialize();

    /**
     * @brief 添加升级历史记录
     * @param history 升级历史记录结构体
     * @return 是否成功
     */
    bool addHistory(const UpgradeHistory &history);

    /**
     * @brief 获取所有升级历史记录
     * @param history JSON格式的历史记录
     * @return 是否成功
     */
    CCErrorCode getHistory(QString &history);

private:
    /**
     * @brief 获取数据库路径
     * @return 数据库文件路径
     */
    QString getDatabasePath() const;

    /**
     * @brief 创建数据库表
     * @return 是否成功
     */
    bool createTables();

    /**
     * @brief 获取数据库连接
     * @return 数据库连接对象
     */
    QSqlDatabase getDatabase();

private:
    static const QString DB_NAME;
    static const QString TABLE_NAME;
};
}  // namespace Kiran
