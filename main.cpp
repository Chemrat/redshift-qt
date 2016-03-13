#include "systemtray.h"

#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(resources);

    QApplication a(argc, argv);
    QApplication::setQuitOnLastWindowClosed(false);

    SystemTray tray;
    QApplication::connect(&a, &QApplication::aboutToQuit, &tray, &SystemTray::StopRedshift);

    if (!tray.CreateIcon())
        return 1;

    if (!tray.StartRedshift())
        return 1;

    return a.exec();
}
