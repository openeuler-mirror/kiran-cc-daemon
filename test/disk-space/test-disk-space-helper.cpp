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

#include "test-disk-space-helper.h"

#include "plugins/disk-space/disk-space-helper.h"

namespace Kiran
{

struct statvfs TestDiskSpaceHelper::makeBuf(fsblkcnt_t blocks, fsblkcnt_t bavail, unsigned long frsize)
{
    struct statvfs b{};
    b.f_blocks = blocks;
    b.f_bavail = bavail;
    b.f_frsize = frsize;
    return b;
}

void TestDiskSpaceHelper::testMountHasEnoughSpace_ratioStrictlyAboveThreshold()
{
    struct statvfs b = makeBuf(100, 10, 4096);
    QVERIFY(DiskSpaceHelper::mountHasEnoughSpace(b, 0.05, 2));
}

void TestDiskSpaceHelper::testMountHasEnoughSpace_ratioAtThresholdUsesByteExemption()
{
    struct statvfs b = makeBuf(100, 5, 4096);
    QVERIFY(!DiskSpaceHelper::mountHasEnoughSpace(b, 0.05, 2));
}

void TestDiskSpaceHelper::testMountHasEnoughSpace_absoluteGiBExemption()
{
    struct statvfs b = makeBuf(100000, 1000, 3 * 1024 * 1024);
    QVERIFY(DiskSpaceHelper::mountHasEnoughSpace(b, 0.05, 2));
}

void TestDiskSpaceHelper::testMountHasEnoughSpace_noSpaceWhenBelowBoth()
{
    struct statvfs b = makeBuf(10000, 100, 4096);
    QVERIFY(!DiskSpaceHelper::mountHasEnoughSpace(b, 0.05, 2));
}

void TestDiskSpaceHelper::testMountHasEnoughSpace_zeroBlocksAlwaysEnough()
{
    struct statvfs b = makeBuf(0, 0, 4096);
    QVERIFY(DiskSpaceHelper::mountHasEnoughSpace(b, 0.05, 2));
}

void TestDiskSpaceHelper::testMountHasEnoughSpace_largeBlockValues()
{
    // 测试大块数量（潜在溢出测试），同时覆盖「比例不满足但字节豁免满足」路径。
    struct statvfs b = makeBuf(1000000000LL, 50000000LL, 4096);  // 5% 可用，约 204.8 GiB
    QVERIFY(DiskSpaceHelper::mountHasEnoughSpace(b, 0.05, 2));
}

// ==================== mountIsVirtual 测试 ====================

void TestDiskSpaceHelper::testMountIsVirtual()
{
    struct statvfs z = makeBuf(0, 0, 0);
    QVERIFY(DiskSpaceHelper::mountIsVirtual(z));
    struct statvfs n = makeBuf(100, 50, 512);
    QVERIFY(!DiskSpaceHelper::mountIsVirtual(n));
}

// ==================== mountPathInIgnoreList 测试 ====================

void TestDiskSpaceHelper::testMountPathInIgnoreList()
{
    QStringList ign{QStringLiteral("/srv/a"), QStringLiteral("/mnt/b"), QStringLiteral("/data/c")};
    // 精确匹配
    QVERIFY(DiskSpaceHelper::mountPathInIgnoreList(QStringLiteral("/srv/a"), ign));
    QVERIFY(DiskSpaceHelper::mountPathInIgnoreList(QStringLiteral("/mnt/b"), ign));
    QVERIFY(DiskSpaceHelper::mountPathInIgnoreList(QStringLiteral("/data/c"), ign));
    // 不匹配
    QVERIFY(!DiskSpaceHelper::mountPathInIgnoreList(QStringLiteral("/srv/a/sub"), ign));
    QVERIFY(!DiskSpaceHelper::mountPathInIgnoreList(QStringLiteral("/srv/other"), ign));
}

void TestDiskSpaceHelper::testMountPathInIgnoreList_emptyIgnoreList()
{
    QStringList ign;  // 空列表
    QVERIFY(!DiskSpaceHelper::mountPathInIgnoreList(QStringLiteral("/mnt/data"), ign));
}

// ==================== normalizeFreePercentInOpen01 测试 ====================

void TestDiskSpaceHelper::testNormalizeFreePercentInOpen01()
{
    double out = -1.0;
    QVERIFY(DiskSpaceHelper::normalizeFreePercentInOpen01(0.03, 0.05, &out));
    QCOMPARE(out, 0.03);

    // v = 0.0 应该返回 true（半开区间允许）
    QVERIFY(DiskSpaceHelper::normalizeFreePercentInOpen01(0.0, 0.05, &out));
    QCOMPARE(out, 0.0);

    // v = 1.0 应该返回 false（半开区间拒绝）
    QVERIFY(!DiskSpaceHelper::normalizeFreePercentInOpen01(1.0, 0.05, &out));
    QCOMPARE(out, 0.05);

    QVERIFY(!DiskSpaceHelper::normalizeFreePercentInOpen01(-0.1, 0.05, &out));
    QCOMPARE(out, 0.05);

    QVERIFY(DiskSpaceHelper::normalizeFreePercentInOpen01(0.999, 0.05, &out));
    QCOMPARE(out, 0.999);
}

// ==================== isInSet 测试 ====================

void TestDiskSpaceHelper::testIsInSet_matchAndNoMatch()
{
    const char* const set[] = {"nfs", "smbfs", "tmpfs", nullptr};
    QVERIFY(DiskSpaceHelper::isInSet(QByteArray("nfs"), set));
    QVERIFY(DiskSpaceHelper::isInSet(QByteArray("smbfs"), set));
    QVERIFY(!DiskSpaceHelper::isInSet(QByteArray("ext4"), set));
}

void TestDiskSpaceHelper::testIsInSet_prefixMatchForDevPaths()
{
    // /dev/ 开头的集合项支持前缀匹配
    const char* const set[] = {"/dev/loop", "/dev/vn", nullptr};
    QVERIFY(DiskSpaceHelper::isInSet(QByteArray("/dev/loop"), set));
    QVERIFY(DiskSpaceHelper::isInSet(QByteArray("/dev/loop0"), set));
    QVERIFY(DiskSpaceHelper::isInSet(QByteArray("/dev/vn"), set));
    QVERIFY(!DiskSpaceHelper::isInSet(QByteArray("/dev/sda"), set));
}

void TestDiskSpaceHelper::testIsInSet_edgeCases()
{
    // 空值返回 false
    const char* const set[] = {"nfs", "smbfs", nullptr};
    QVERIFY(!DiskSpaceHelper::isInSet(QByteArray(""), set));

    // 空集合（只有 nullptr）
    const char* const emptySet[] = {nullptr};
    QVERIFY(!DiskSpaceHelper::isInSet(QByteArray("nfs"), emptySet));
}

// ==================== checkShouldNotify 测试 ====================

void TestDiskSpaceHelper::testCheckShouldNotify_firstTimeNotify()
{
    // 首次通知：无历史记录
    DiskSpaceHelper::NotifyCheckInput input;
    input.hasRecord = false;
    input.currentBuf = makeBuf(100, 5, 4096);  // 5% 可用
    input.currentTime = 1000;
    input.notifyAgainThreshold = 0.01;
    input.minNotifyPeriodMinutes = 10;

    DiskSpaceHelper::NotifyCheckResult result = DiskSpaceHelper::checkShouldNotify(input);
    QVERIFY(result.shouldNotify);
    QVERIFY(result.updateRecord);
    QCOMPARE(result.newNotifyTime, 1000LL);
}

void TestDiskSpaceHelper::testCheckShouldNotify_freeDropsEnough_renotify()
{
    // 空闲比例再降足够，且超过时间间隔 → 弹窗
    DiskSpaceHelper::NotifyCheckInput input;
    input.hasRecord = true;
    input.lastNotifyTime = 1000;
    input.lastBuf = makeBuf(100, 10, 4096);  // 上次：10% 可用
    input.currentBuf = makeBuf(100, 3, 4096);  // 当前：3% 可用，再降 7%
    input.currentTime = 2000;  // 1000 秒后 > 10 分钟
    input.notifyAgainThreshold = 0.05;  // 再降超过 5% 就提醒
    input.minNotifyPeriodMinutes = 10;

    DiskSpaceHelper::NotifyCheckResult result = DiskSpaceHelper::checkShouldNotify(input);
    QVERIFY(result.shouldNotify);
    QVERIFY(result.updateRecord);
    QCOMPARE(result.newNotifyTime, 2000LL);
}

void TestDiskSpaceHelper::testCheckShouldNotify_freeDropsEnough_timeLimit()
{
    // 空闲比例再降足够，但未超过时间间隔 → 不弹窗，但更新快照
    DiskSpaceHelper::NotifyCheckInput input;
    input.hasRecord = true;
    input.lastNotifyTime = 1000;
    input.lastBuf = makeBuf(100, 10, 4096);  // 上次：10% 可用
    input.currentBuf = makeBuf(100, 3, 4096);  // 当前：3% 可用，再降 7%
    input.currentTime = 1100;  // 100 秒后 < 10 分钟
    input.notifyAgainThreshold = 0.05;
    input.minNotifyPeriodMinutes = 10;

    DiskSpaceHelper::NotifyCheckResult result = DiskSpaceHelper::checkShouldNotify(input);
    QVERIFY(!result.shouldNotify);
    QVERIFY(result.updateRecord);  // 仍然更新快照
    QCOMPARE(result.newNotifyTime, 1000LL);  // 保持原通知时间
}

void TestDiskSpaceHelper::testCheckShouldNotify_freeDropsNotEnough()
{
    // 空闲比例再降不足 → 不弹窗，不更新快照
    DiskSpaceHelper::NotifyCheckInput input;
    input.hasRecord = true;
    input.lastNotifyTime = 1000;
    input.lastBuf = makeBuf(100, 5, 4096);  // 上次：5% 可用
    input.currentBuf = makeBuf(100, 4, 4096);  // 当前：4% 可用，再降 1%
    input.currentTime = 2000;
    input.notifyAgainThreshold = 0.05;  // 需要再降超过 5%
    input.minNotifyPeriodMinutes = 10;

    DiskSpaceHelper::NotifyCheckResult result = DiskSpaceHelper::checkShouldNotify(input);
    QVERIFY(!result.shouldNotify);
    QVERIFY(!result.updateRecord);  // 不更新快照
}

void TestDiskSpaceHelper::testCheckShouldNotify_virtualMount()
{
    // 虚拟挂载（f_blocks == 0）→ 不通知
    DiskSpaceHelper::NotifyCheckInput input;
    input.hasRecord = false;
    input.currentBuf = makeBuf(0, 0, 4096);  // 虚拟挂载
    input.currentTime = 1000;
    input.notifyAgainThreshold = 0.01;
    input.minNotifyPeriodMinutes = 10;

    DiskSpaceHelper::NotifyCheckResult result = DiskSpaceHelper::checkShouldNotify(input);
    QVERIFY(!result.shouldNotify);
    QVERIFY(!result.updateRecord);
}

void TestDiskSpaceHelper::testCheckShouldNotify_invalidPrevSnapshot()
{
    // 上次快照异常（f_blocks == 0）→ 视为首次通知
    DiskSpaceHelper::NotifyCheckInput input;
    input.hasRecord = true;
    input.lastNotifyTime = 1000;
    input.lastBuf = makeBuf(0, 0, 4096);  // 异常快照
    input.currentBuf = makeBuf(100, 5, 4096);  // 当前正常
    input.currentTime = 2000;
    input.notifyAgainThreshold = 0.01;
    input.minNotifyPeriodMinutes = 10;

    DiskSpaceHelper::NotifyCheckResult result = DiskSpaceHelper::checkShouldNotify(input);
    QVERIFY(result.shouldNotify);
    QVERIFY(result.updateRecord);
    QCOMPARE(result.newNotifyTime, 2000LL);
}

}  // namespace Kiran

QTEST_MAIN(Kiran::TestDiskSpaceHelper)
