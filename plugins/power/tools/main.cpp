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

#include <qt5-log-i.h>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QTranslator>
#include <iostream>
#include "config.h"
#include "power-backlight-helper.h"

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationVersion(PROJECT_VERSION);
    QTranslator translator;
    Kiran::PowerBacklightHelper backlightHelper;

    klog_qt5_init(QString(), "kylinsec-system", PROJECT_NAME, QCoreApplication::applicationName());

    if (!translator.load(QLocale(), QCoreApplication::applicationName(), ".", KCD_INSTALL_TRANSLATIONDIR, ".qm"))
    {
        KLOG_WARNING() << "Load translator failed!";
    }
    else
    {
        app.installTranslator(&translator);
    }

    backlightHelper.init();

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addOption({"support-backlight",
                      QObject::tr("Whether the backlight device exists.")});

    parser.addOption({"get-backlight-directory",
                      QObject::tr("Get backlight monitor directory.")});

    parser.addOption({"get-brightness-value",
                      QObject::tr("Get the current brightness value.")});

    parser.addOption({"get-max-brightness-value",
                      QObject::tr("Get the max brightness value.")});

    parser.addOption({"set-brightness-value",
                      QObject::tr("Set the brightness value.")});

    parser.process(app);

    if (parser.isSet("version"))
    {
        std::cout << PROJECT_VERSION << std::endl;
        return EXIT_SUCCESS;
    }

    if (parser.isSet("support-backlight"))
    {
        std::cout << (backlightHelper.supportBacklight() ? 1 : 0);
        return EXIT_SUCCESS;
    }

    if (parser.isSet("get-backlight-directory"))
    {
        std::cout << backlightHelper.getBacklightDir().toUtf8().data();
        return EXIT_SUCCESS;
    }

    // 不支持获取和设置则直接返回
    if (!backlightHelper.supportBacklight())
    {
        std::cerr << QObject::tr("No backlights were found on your system").toUtf8().data();
        return EXIT_FAILURE;
    }

    if (parser.isSet("get-brightness-value"))
    {
        auto brightnessValue = backlightHelper.getBrightnessValue();
        if (brightnessValue >= 0)
        {
            std::cout << brightnessValue;
        }
        else
        {
            std::cerr << QObject::tr("Could not get the value of the backlight").toUtf8().data();
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }

    if (parser.isSet("get-max-brightness-value"))
    {
        auto brightnessValue = backlightHelper.getBrightnessMaxValue();
        if (brightnessValue >= 0)
        {
            std::cout << brightnessValue;
        }
        else
        {
            std::cerr << QObject::tr("Could not get the maximum value of the backlight").toUtf8().data();
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }

    if (parser.isSet("set-brightness-value"))
    {
        auto brightnessValue = parser.value("set-brightness-value").toInt();
        QString error;
        if (!backlightHelper.setBrightnessValue(brightnessValue, error))
        {
            std::cerr << error.toUtf8().data();
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }

    return EXIT_SUCCESS;
}