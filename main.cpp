#include "infodialog.h"
#include "systemtray.h"

#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(resources);

    QApplication a(argc, argv);
    QApplication::setQuitOnLastWindowClosed(false);

    SystemTray tray;
    if (!tray.CreateIcon())
        return 1;

    if (!tray.StartRedshift())
        return 1;

    return a.exec();
}
