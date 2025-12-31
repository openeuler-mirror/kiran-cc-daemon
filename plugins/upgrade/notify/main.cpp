#include <qt5-log-i.h>
#include <QCoreApplication>
#include "lib/base/misc-utils.h"
#include "notification-manager.h"
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    klog_qt5_init(QString(), "kylinsec-session", "kcd-upgrade-notify", QCoreApplication::applicationName());

    auto translator = Kiran::MiscUtils::installTranslator("kcd-upgrade-notify");

    Kiran::NotificationManager notificationManager;
    auto ret = app.exec();

    Kiran::MiscUtils::removeTranslator(translator);

    return ret;
}