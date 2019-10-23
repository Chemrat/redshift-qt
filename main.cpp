#include "systemtray.h"

#include <QtWidgets/QApplication>
#include <QSharedMemory>
#include <QDebug>

#include <csignal>

static SystemTray *globalTray = nullptr;

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
    QStringList argsl = QStringList();
    bool varg = false;

    for (int i = 1; i < argc; ++i){
        if(std::string(argv[i]) == "-h" || std::string(argv[i]) == "--help"){
            qInfo() <<  "Please run `redshift -h` for help output.";
            exit(-1);
        }

        if(std::string(argv[i]) == "-v")
           varg = true;

        argsl.append(argv[i]);
    }
    if(!varg)
       argsl.append("-v");
       
    Q_INIT_RESOURCE(resources);

    QSharedMemory sharedMemoryLock("redshift-qt-lock");
    if (IsInstanceAlreadyRunning(sharedMemoryLock))
        return -1;

    QApplication a(argc, argv);
    a.setApplicationName("Redshift Qt");
    QApplication::setQuitOnLastWindowClosed(false);

    SystemTray tray;
    QApplication::connect(&a, &QApplication::aboutToQuit, &tray, &SystemTray::StopRedshift);

    if (!tray.CreateIcon())
        return 1;

    if (!tray.StartRedshift(argsl))
        return 1;

    globalTray = &tray;
    signal(SIGUSR1, handleSignal);

    return a.exec();
}
