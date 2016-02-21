#include "systemtray.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QMenu>
#include <QtCore/QProcess>
#include <QtCore/QDebug>

#include <signal.h>

SystemTray::SystemTray()
{

}

void SystemTray::onClick(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
    case QSystemTrayIcon::ActivationReason::Trigger:
        ToggleRedshift(!_enabled);
        break;
    }
}

void SystemTray::onQuit()
{
    setIcon(*_iconDisabled);
    if (_redshiftProcess && _redshiftProcess->pid())
    {
        _redshiftProcess->terminate();
        if (!_redshiftProcess->waitForFinished())
            qDebug() << "Redshift process failed to terminate";
    }

    QApplication::quit();
}

bool SystemTray::CreateIcon()
{
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(0, QObject::tr("Systray"),
                              QObject::tr("I couldn't detect any system tray "
                                          "on this system."));
        return false;
    }

    auto quitAction = new QAction(QObject::tr("&Quit"), nullptr);
    connect(quitAction, &QAction::triggered, this, &SystemTray::onQuit);

    auto trayIconMenu = new QMenu();
    trayIconMenu->addAction(quitAction);

    _iconEnabled  = std::make_shared<QIcon>(":/resources/redshift-status-on.svg");
    _iconDisabled = std::make_shared<QIcon>(":/resources/redshift-status-off.svg");

    connect(this, &QSystemTrayIcon::activated, this, &SystemTray::onClick);

    setContextMenu(trayIconMenu);
    setIcon(*_iconEnabled);

    show();

    return true;
}

bool SystemTray::StartRedshift()
{
    if (_redshiftProcess)
        return false;

    _redshiftProcess = std::make_shared<QProcess>();
    _redshiftProcess->start("redshift");

    if (!_redshiftProcess->waitForStarted(5000))
    {
        qDebug() << "Failed to start redshift";
        return false;
    }

    _enabled = true;

    return true;
}

void SystemTray::ToggleRedshift(bool enable)
{
    if (!_redshiftProcess)
    {
        qDebug() << "QProcess pointer is null";
        return;
    }

    if (enable)
    {
        _enabled = true;
        setIcon(*_iconEnabled);
        qDebug() << "Enabling redshift";
        kill(_redshiftProcess->pid(), SIGUSR1);
    }
    else
    {
        _enabled = false;
        setIcon(*_iconDisabled);
        qDebug() << "Disabling redshift";
        kill(_redshiftProcess->pid(), SIGUSR1);
    }
}
