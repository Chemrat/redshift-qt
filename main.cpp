#include "systemtray.h"

#include <QtWidgets/QApplication>
#include <QSharedMemory>
#include <QDebug>

#include <csignal>

SystemTray *globalTray = nullptr;

void handleSignal(int signum)
{
    if (signum == SIGUSR1)
        globalTray->onSuspend();
}

bool IsInstanceAlreadyRunning(QSharedMemory &memoryLock) {
    if (!memoryLock.create(1)) {
        qDebug() << "Failed to create shared memory lock, an instance of redshift-qt might be already running\n"
                    "or it was not terminated correctly. Trying to clean up the lock...";
        memoryLock.attach();
        memoryLock.detach();

        if (!memoryLock.create(1)) {
            qDebug() << "An insance of redshift-qt is already running, exiting...";
            return true;
        }
    }

    return false;
}

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(resources);

    QSharedMemory sharedMemoryLock("redshift-qt-lock");
    if (IsInstanceAlreadyRunning(sharedMemoryLock))
        return -1;

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
