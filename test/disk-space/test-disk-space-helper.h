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

#include <QtTest>
#include <sys/statvfs.h>

namespace Kiran
{

class TestDiskSpaceHelper : public QObject
{
    Q_OBJECT

private slots:
    void testMountHasEnoughSpace_ratioStrictlyAboveThreshold();
    void testMountHasEnoughSpace_ratioAtThresholdUsesByteExemption();
    void testMountHasEnoughSpace_absoluteGiBExemption();
    void testMountHasEnoughSpace_noSpaceWhenBelowBoth();
    void testMountHasEnoughSpace_zeroBlocksAlwaysEnough();
    void testMountHasEnoughSpace_largeBlockValues();

    void testMountIsVirtual();

    void testMountPathInIgnoreList();
    void testMountPathInIgnoreList_emptyIgnoreList();

    void testNormalizeFreePercentInOpen01();

    void testIsInSet_matchAndNoMatch();
    void testIsInSet_prefixMatchForDevPaths();
    void testIsInSet_edgeCases();

    void testCheckShouldNotify_firstTimeNotify();
    void testCheckShouldNotify_freeDropsEnough_renotify();
    void testCheckShouldNotify_freeDropsEnough_timeLimit();
    void testCheckShouldNotify_freeDropsNotEnough();
    void testCheckShouldNotify_virtualMount();
    void testCheckShouldNotify_invalidPrevSnapshot();

private:
    static struct statvfs makeBuf(fsblkcnt_t blocks, fsblkcnt_t bavail, unsigned long frsize);
};

}  // namespace Kiran
