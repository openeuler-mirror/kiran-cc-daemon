/**
 * Copyright (c) 2020 ~ 2026 KylinSec Co., Ltd.
 * kiran-cc-daemon is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:    gaobo <gaobo@kylinsec.com.cn>
 */

#pragma once

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <sys/statvfs.h>
#include <time.h>

#include <QtGlobal>

namespace Kiran
{
namespace DiskSpaceHelper
{

constexpr qint64 kGiB = 1024LL * 1024 * 1024;

/**
 * @brief 判断挂载是否为虚拟挂载
 * @param buf statvfs 结构体
 * @return true 表示是虚拟挂载，不参与低空间判断
 */
inline bool mountIsVirtual(const struct statvfs& buf)
{
    return buf.f_blocks == 0;
}

/**
 * @brief 判断挂载是否有足够空间
 * @param buf statvfs 结构体
 * @param freePercentNotify 空闲百分比阈值
 * @param freeSizeGbNoNotify 空闲字节数豁免阈值（GiB）
 * @return true 表示空间充足，无需告警
 */
inline bool mountHasEnoughSpace(const struct statvfs& buf, double freePercentNotify, int freeSizeGbNoNotify)
{
    if (buf.f_blocks == 0)
        return true;
    const double freeFrac = static_cast<double>(buf.f_bavail) / static_cast<double>(buf.f_blocks);
    if (freeFrac > freePercentNotify)
        return true;
    const qint64 availBytes = static_cast<qint64>(buf.f_frsize) * static_cast<qint64>(buf.f_bavail);
    if (availBytes > static_cast<qint64>(freeSizeGbNoNotify) * kGiB)
        return true;
    return false;
}

/**
 * @brief 判断挂载路径是否在忽略列表中
 * @param path 挂载路径
 * @param ignorePaths 忽略路径列表
 * @return true 表示路径在忽略列表中（精确匹配）
 */
inline bool mountPathInIgnoreList(const QString& path, const QStringList& ignorePaths)
{
    return ignorePaths.contains(path);
}

/**
 * @brief 规范化空闲百分比配置值
 * @param v 输入值
 * @param fallback 回退默认值
 * @param out 输出值
 * @return true 表示输入值有效，false 表示使用了回退值
 *
 * 有效范围为 [0, 1)，即允许 0 但拒绝 1
 */
inline bool normalizeFreePercentInOpen01(double v, double fallback, double* out)
{
    if (v >= 0.0 && v < 1.0)
    {
        *out = v;
        return true;
    }
    *out = fallback;
    return false;
}

/**
 * @brief 判断值是否在给定的字符串集合中
 * @param value 待判断的值
 * @param set 字符串集合（以 nullptr 结尾）
 * @return true 表示值在集合中
 *
 * 匹配规则：
 * - 精确匹配返回 true
 * - 若集合项以 "/dev/" 开头，则支持前缀匹配
 */
inline bool isInSet(const QByteArray& value, const char* const* set)
{
    if (value.isEmpty())
        return false;
    for (int i = 0; set[i] != nullptr; ++i)
    {
        const QByteArray candidate(set[i]);
        if (value == candidate)
            return true;
        if (candidate.startsWith("/dev/") && value.startsWith(candidate))
            return true;
    }
    return false;
}

/**
 * @brief 通知判断的输入参数
 */
struct NotifyCheckInput
{
    bool hasRecord = false;            // 是否有历史通知记录
    time_t lastNotifyTime = 0;         // 上次通知时间（有记录时有效）
    struct statvfs lastBuf {};         // 上次快照（有记录时有效）
    struct statvfs currentBuf {};      // 当前快照
    time_t currentTime = 0;            // 当前时间
    double notifyAgainThreshold = 0.0; // free-percent-notify-again 阈值
    int minNotifyPeriodMinutes = 0;    // 最小通知间隔（分钟）
};

/**
 * @brief 通知判断的结果
 */
struct NotifyCheckResult
{
    bool shouldNotify = false;        // 是否应该弹出通知
    bool updateRecord = false;        // 是否应该更新记录
    time_t newNotifyTime = 0;         // 新的通知时间
    struct statvfs newSnapshot {};    // 新的快照
};

/**
 * @brief 判断是否应该弹出低空间通知
 * @param input 通知判断输入参数
 * @return 通知判断结果
 *
 * 判断逻辑：
 * 1. 无历史记录 -> 首次通知
 * 2. 有历史记录：
 *    - 空闲比例相对上次快照再降超过 notifyAgainThreshold -> 检查时间间隔
 *      - 距上次通知超过 minNotifyPeriodMinutes -> 弹窗
 *      - 否则 -> 不弹窗但更新快照
 *    - 空闲比例再降不足 -> 不弹窗，不更新快照
 */
inline NotifyCheckResult checkShouldNotify(const NotifyCheckInput& input)
{
    NotifyCheckResult result;

    // 虚拟挂载，不应通知
    if (input.currentBuf.f_blocks == 0)
        return result;

    const double currentFree = static_cast<double>(input.currentBuf.f_bavail) /
                               static_cast<double>(input.currentBuf.f_blocks);

    // 首次低于阈值：直接通知，写入快照
    if (!input.hasRecord)
    {
        result.shouldNotify = true;
        result.updateRecord = true;
        result.newNotifyTime = input.currentTime;
        result.newSnapshot = input.currentBuf;
        return result;
    }

    // 上次快照异常，视为首次
    if (input.lastBuf.f_blocks == 0)
    {
        result.shouldNotify = true;
        result.updateRecord = true;
        result.newNotifyTime = input.currentTime;
        result.newSnapshot = input.currentBuf;
        return result;
    }

    const double prevFree = static_cast<double>(input.lastBuf.f_bavail) /
                            static_cast<double>(input.lastBuf.f_blocks);

    // 空闲比例再降是否超过阈值
    const double freeDrop = prevFree - currentFree;
    if (freeDrop <= input.notifyAgainThreshold)
    {
        // 空闲比例再降不足：不通知，不更新快照
        result.shouldNotify = false;
        result.updateRecord = false;
        return result;
    }

    // 空闲比例再降超过阈值：检查时间间隔
    const double elapsedSeconds = difftime(input.currentTime, input.lastNotifyTime);
    const double minIntervalSeconds = static_cast<double>(input.minNotifyPeriodMinutes) * 60.0;

    if (elapsedSeconds > minIntervalSeconds)
    {
        // 超过最小间隔：弹窗
        result.shouldNotify = true;
        result.newNotifyTime = input.currentTime;
    }
    else
    {
        // 未超过最小间隔：不弹窗
        result.shouldNotify = false;
        result.newNotifyTime = input.lastNotifyTime;
    }

    // 无论是否弹窗，都更新快照（已跨 notify-again 阈值）
    result.updateRecord = true;
    result.newSnapshot = input.currentBuf;

    return result;
}

}  // namespace DiskSpaceHelper
}  // namespace Kiran
