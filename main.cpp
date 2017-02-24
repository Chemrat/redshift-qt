#include "systemtray.h"

#include <QtWidgets/QApplication>

#include <signal.h>

SystemTray* globalTray;

void handleSignal(int signum)
{
    if (signum == SIGUSR1)
        globalTray->onSuspend();
}

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

    globalTray = &tray;
    signal(SIGUSR1, handleSignal);

    return a.exec();
}
