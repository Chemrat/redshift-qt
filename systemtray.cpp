#include "systemtray.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QMenu>
#include <QtCore/QProcess>
#include <QtCore/QDebug>
#include <QtCore/QTimer>
#include <QtGui/QIcon>

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
    case QSystemTrayIcon::ActivationReason::DoubleClick:
        ToggleRedshift(!_enabled);
        break;
    case QSystemTrayIcon::ActivationReason::MiddleClick:
    case QSystemTrayIcon::ActivationReason::Context:
    case QSystemTrayIcon::ActivationReason::Unknown:
        break;
    }
}

void SystemTray::onQuit()
{
    setIcon(*_iconDisabled);
    QApplication::quit();
}

void SystemTray::onRedshiftQuit(int, QProcess::ExitStatus)
{
    if (!_warnOnRedshiftQuit)
        return;

    QMessageBox::critical(0, QObject::tr("Fatal error"), QObject::tr("Redshift process has been terminated unexpectedly"));
    onQuit();
}

void SystemTray::onRedshiftOutput()
{
    if (!_redshiftProcess)
        return;

    QTextStream stream(_redshiftProcess.get());
    while (!stream.atEnd()) {
        auto line = stream.readLine();
        qDebug() << line;
        if (line.startsWith("Color temperature"))
            _colorTemp = line;
        else if (line.startsWith("Period:"))
            _period = line;
        else
            _info += "\n" + line;
    }
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

void SystemTray::onGetInfo()
{
    QMessageBox::information(nullptr, "Redshift Information", _info
                             + "\n" + _period
                             + "\n" + _colorTemp, QMessageBox::Ok);
}

bool SystemTray::CreateIcon()
{
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(0, QObject::tr("Fatal error"), QObject::tr("No system tray available"));
        return false;
    }

    CreateMenu();

    _iconEnabled  = std::make_shared<QIcon>(":/icons/redshift-status-on.png");
    _iconDisabled = std::make_shared<QIcon>(":/icons/redshift-status-off.png");

    setIcon(*_iconEnabled);

    connect(this, &QSystemTrayIcon::activated, this, &SystemTray::onClick);

    show();

    return true;
}

bool SystemTray::StartRedshift()
{
    _redshiftProcess = std::make_shared<QProcess>();
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LC_ALL", "C");
    _redshiftProcess->setProcessEnvironment(env);
    _redshiftProcess->start("redshift -v");

    connect(_redshiftProcess.get(), (void (QProcess::*)(int,QProcess::ExitStatus))&QProcess::finished, this, &SystemTray::onRedshiftQuit);
    connect(_redshiftProcess.get(), &QProcess::readyRead, this, &SystemTray::onRedshiftOutput);

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

void SystemTray::StopRedshift()
{
    if (_redshiftProcess && _redshiftProcess->pid())
    {
        _warnOnRedshiftQuit = false;
        _redshiftProcess->terminate();
        if (!_redshiftProcess->waitForFinished())
            qDebug() << "Redshift process failed to terminate";
    }
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

    auto showInfoAction = new QAction(QObject::tr("Show Info"), this);
    connect(showInfoAction, &QAction::triggered, this, &SystemTray::onGetInfo);

    auto quitAction = new QAction(QObject::tr("&Quit"), nullptr);
    connect(quitAction, &QAction::triggered, this, &SystemTray::onQuit);

    trayIconMenu->addAction(_suspendMenu.get());
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(suspendAction_10m);
    trayIconMenu->addAction(suspendAction_1h);
    trayIconMenu->addAction(suspendAction_2h);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(showInfoAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    setContextMenu(trayIconMenu);
}
