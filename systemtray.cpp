#include "systemtray.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QMenu>
#include <QtCore/QProcess>
#include <QtCore/QDebug>
#include <QtCore/QTimer>

#include <signal.h>

SystemTray::SystemTray()
{
    _suspendTimer = std::make_shared<QTimer>(this);
    connect(_suspendTimer.get(), &QTimer::timeout, this, &SystemTray::onTimeout);
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

void SystemTray::onSuspend()
{
    ToggleRedshift(!_enabled);
}

void SystemTray::onSuspend10minutes()
{
    ToggleRedshift(false);
    _suspendTimer->start(10*60*1000);
}

void SystemTray::onSuspend1hour()
{
    ToggleRedshift(false);
    _suspendTimer->start(60*60*1000);
}

void SystemTray::onSuspend2hours()
{
    ToggleRedshift(false);
    _suspendTimer->start(120*60*1000);
}

void SystemTray::onTimeout()
{
    ToggleRedshift(true);
}

bool SystemTray::CreateIcon()
{
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(0, QObject::tr("Fatal error"), QObject::tr("No system tray available"));
        return false;
    }

    CreateMenu();

    _iconEnabled  = std::make_shared<QIcon>(":/icons/redshift-status-on.svg");
    _iconDisabled = std::make_shared<QIcon>(":/icons/redshift-status-off.svg");
    setIcon(*_iconEnabled);

    connect(this, &QSystemTrayIcon::activated, this, &SystemTray::onClick);

    show();

    return true;
}

bool SystemTray::StartRedshift()
{
    _redshiftProcess = std::make_shared<QProcess>();
    _redshiftProcess->start("redshift");

    if (!_redshiftProcess->waitForStarted(5000))
    {
        QMessageBox::critical(0, QObject::tr("Fatal error"), QObject::tr("Failed to start redshift"));
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

    if (enable == _enabled)
        return;

    _enabled = enable;
    _suspendMenu->setChecked(!enable);
    setIcon(enable ? *_iconEnabled : *_iconDisabled);
    qDebug() << "Redshift status change: " << (enable ? "enabled" : "disabled");
    kill(_redshiftProcess->pid(), SIGUSR1);
}

void SystemTray::CreateMenu()
{
    // Lifetime of this menu is the same as the application's
    auto trayIconMenu = new QMenu();

    _suspendMenu = std::make_shared<QAction>(QObject::tr("Suspended"), this);
    _suspendMenu->setCheckable(true);
    _suspendMenu->setChecked(false);
    connect(_suspendMenu.get(), &QAction::triggered, this, &SystemTray::onSuspend);

    auto suspendAction_10m = new QAction(QObject::tr("Suspend for 10 minutes"), this);
    auto suspendAction_1h = new QAction(QObject::tr("Suspend for 1 hour"), this);
    auto suspendAction_2h = new QAction(QObject::tr("Suspend for 2 hours"), this);

    connect(suspendAction_10m, &QAction::triggered, this, &SystemTray::onSuspend10minutes);
    connect(suspendAction_1h, &QAction::triggered, this, &SystemTray::onSuspend1hour);
    connect(suspendAction_2h, &QAction::triggered, this, &SystemTray::onSuspend2hours);

    auto quitAction = new QAction(QObject::tr("&Quit"), nullptr);
    connect(quitAction, &QAction::triggered, this, &SystemTray::onQuit);

    trayIconMenu->addAction(_suspendMenu.get());
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(suspendAction_10m);
    trayIconMenu->addAction(suspendAction_1h);
    trayIconMenu->addAction(suspendAction_2h);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    setContextMenu(trayIconMenu);
}
