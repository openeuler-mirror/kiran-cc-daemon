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

#include "upgrade-history-db.h"
#include <qt5-log-i.h>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QStringList>

#include "lib/base/log.h"

#define DB_CONNECTION_PREFIX "upgrade_history_"
namespace Kiran
{
const QString UpgradeHistoryDB::DB_NAME = "upgrade_history.db";
const QString UpgradeHistoryDB::TABLE_NAME = "upgrade_history";

UpgradeHistoryDB::UpgradeHistoryDB(QObject *parent) : QObject(parent)
{
}

UpgradeHistoryDB::~UpgradeHistoryDB()
{
    QSqlDatabase::removeDatabase(DB_CONNECTION_PREFIX + getDatabasePath());
}

QString UpgradeHistoryDB::getDatabasePath() const
{
    QString dbDir = "/var/lib/kiran-cc-daemon/upgrade";
    QDir dir;
    if (!dir.exists(dbDir))
    {
        dir.mkpath(dbDir);
    }
    return dbDir + "/" + DB_NAME;
}

QSqlDatabase UpgradeHistoryDB::getDatabase()
{
    QString dbPath = getDatabasePath();
    QString connectionName = DB_CONNECTION_PREFIX + dbPath;

    QSqlDatabase db = QSqlDatabase::database(connectionName);
    if (!db.isValid())
    {
        db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
        db.setDatabaseName(dbPath);
    }

    if (!db.isOpen())
    {
        if (!db.open())
        {
            KLOG_ERROR(upgrade) << "Failed to open database: " << db.lastError().text();
        }
    }

    return db;
}

bool UpgradeHistoryDB::createTables()
{
    QSqlDatabase db = getDatabase();
    if (!db.isOpen())
    {
        KLOG_ERROR(upgrade) << "Database is not open";
        return false;
    }

    QSqlQuery query(db);
    QString createTableSQL = QString(
                                 "CREATE TABLE IF NOT EXISTS %1 ("
                                 "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                 "upgrade_time TEXT NOT NULL UNIQUE,"
                                 "result INTEGER NOT NULL,"
                                 "error_message TEXT,"
                                 "success_packages TEXT,"
                                 "failed_packages TEXT,"
                                 "created_at TEXT DEFAULT CURRENT_TIMESTAMP"
                                 ")")
                                 .arg(TABLE_NAME);

    if (!query.exec(createTableSQL))
    {
        KLOG_ERROR(upgrade) << "Failed to create table: " << query.lastError().text();
        return false;
    }

    KLOG_DEBUG(upgrade) << "Database tables created successfully";
    return true;
}

bool UpgradeHistoryDB::initialize()
{
    QSqlDatabase db = getDatabase();
    if (!db.isOpen())
    {
        KLOG_ERROR(upgrade) << "Failed to open database";
        return false;
    }

    return createTables();
}

bool UpgradeHistoryDB::addHistory(const UpgradeHistory &history)
{
    QSqlDatabase db = getDatabase();
    if (!db.isOpen())
    {
        KLOG_ERROR(upgrade) << "Database is not open";
        return false;
    }

    QSqlQuery query(db);

    // 先检查该 upgrade_time 是否已存在
    QString checkSQL = QString("SELECT id FROM %1 WHERE upgrade_time = ?").arg(TABLE_NAME);
    query.prepare(checkSQL);
    query.addBindValue(history.upgradeTime);

    bool exists = false;
    if (query.exec() && query.next())
    {
        exists = true;
    }

    bool success = false;
    if (exists)
    {
        // 如果存在，则更新该行的四个字段（result, error_message, success_packages, failed_packages）
        QString updateSQL = QString(
                                "UPDATE %1 SET "
                                "result = ?, "
                                "error_message = ?, "
                                "success_packages = ?, "
                                "failed_packages = ? "
                                "WHERE upgrade_time = ?")
                                .arg(TABLE_NAME);

        query.prepare(updateSQL);
        query.addBindValue(static_cast<int>(history.result));
        query.addBindValue(history.errorMessage);
        query.addBindValue(history.successPackages.join("\n").trimmed());
        query.addBindValue(history.failedPackages.join("\n").trimmed());
        query.addBindValue(history.upgradeTime);

        if (query.exec())
        {
            success = true;
            KLOG_DEBUG(upgrade) << "Updated upgrade history: time=" << history.upgradeTime
                                << ", result=" << static_cast<int>(history.result)
                                << ", error_message=" << history.errorMessage
                                << ", success_packages=" << history.successPackages.size()
                                << ", failed_packages=" << history.failedPackages.size();
        }
        else
        {
            KLOG_ERROR(upgrade) << "Failed to update history: " << query.lastError().text();
        }
    }
    else
    {
        // 如果不存在，则插入新行
        QString insertSQL = QString(
                                "INSERT INTO %1 (upgrade_time, result, error_message, success_packages, failed_packages) "
                                "VALUES (?, ?, ?, ?, ?)")
                                .arg(TABLE_NAME);

        query.prepare(insertSQL);
        query.addBindValue(history.upgradeTime);
        query.addBindValue(static_cast<int>(history.result));
        query.addBindValue(history.errorMessage);
        query.addBindValue(history.successPackages.join("\n").trimmed());
        query.addBindValue(history.failedPackages.join("\n").trimmed());

        if (query.exec())
        {
            success = true;
            KLOG_DEBUG(upgrade) << "Inserted upgrade history: time=" << history.upgradeTime
                                << ", result=" << static_cast<int>(history.result)
                                << ", error_message=" << history.errorMessage
                                << ", success_packages=" << history.successPackages.size()
                                << ", failed_packages=" << history.failedPackages.size();
        }
        else
        {
            KLOG_ERROR(upgrade) << "Failed to insert history: " << query.lastError().text();
        }
    }

    return success;
}

CCErrorCode UpgradeHistoryDB::getHistory(QString &history)
{
    QSqlDatabase db = getDatabase();
    if (!db.isOpen())
    {
        KLOG_ERROR(upgrade) << "Database is not open";
        return CCErrorCode::ERROR_UPGRADE_HISTORY_DB_NOT_OPEN;
    }

    QSqlQuery query(db);
    QString selectSQL = QString("SELECT upgrade_time, result, error_message, success_packages, failed_packages "
                                "FROM %1 ORDER BY upgrade_time DESC")
                            .arg(TABLE_NAME);

    if (!query.exec(selectSQL))
    {
        KLOG_ERROR(upgrade) << "Failed to query history: " << query.lastError().text();
        return CCErrorCode::ERROR_UPGRADE_HISTORY_DB_BROKEN;
    }

    QJsonArray historyArray;
    while (query.next())
    {
        QJsonObject historyItem;
        historyItem["upgrade_time"] = query.value(0).toString();

        // 将整数转换回枚举值，并在JSON中输出枚举值和字符串描述
        int resultInt = query.value(1).toInt();
        historyItem["result"] = static_cast<int>(resultInt);  // 存储枚举值（整数）
        historyItem["error_message"] = query.value(2).toString();

        // 解析包列表（使用换行符分隔）
        QStringList successPkgs = query.value(3).toString().split("\n", Qt::SkipEmptyParts);
        QStringList failedPkgs = query.value(4).toString().split("\n", Qt::SkipEmptyParts);

        QJsonArray successArray;
        for (const QString &pkg : successPkgs)
        {
            successArray.append(pkg);
        }
        historyItem["success_packages"] = successArray;

        QJsonArray failedArray;
        for (const QString &pkg : failedPkgs)
        {
            failedArray.append(pkg);
        }
        historyItem["failed_packages"] = failedArray;

        historyArray.append(historyItem);
    }

    QJsonDocument doc(historyArray);
    history = doc.toJson(QJsonDocument::Compact);
    return CCErrorCode::SUCCESS;
}
}  // namespace Kiran
