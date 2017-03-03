#pragma once

#include <memory>

#include <QtWidgets/QSystemTrayIcon>
#include <QtCore/QProcess>

class QIcon;
class QProcess;
class QAction;
class QTimer;

class SystemTray
        : public QSystemTrayIcon
{
public:
    SystemTray();

    bool CreateIcon();
    bool StartRedshift();
    void ToggleRedshift(bool enable = true);
    void StopRedshift();
    void onSuspend();

private:
    void CreateMenu();

private:
    void onClick(QSystemTrayIcon::ActivationReason);
    void onQuit();
    void onRedshiftQuit(int, QProcess::ExitStatus);
    void onRedshiftOutput();

    // FIXME: this looks bad
    void onSuspend10minutes();
    void onSuspend1hour();
    void onSuspend2hours();

    void onTimeout();

    void onGetInfo();

private:
    QString _colorTemp;
    QString _period;
    QString _info;

    std::shared_ptr<QIcon> _iconEnabled;
    std::shared_ptr<QIcon> _iconDisabled;
    std::shared_ptr<QAction> _suspendMenu;

    std::shared_ptr<QProcess> _redshiftProcess;
    std::shared_ptr<QTimer> _suspendTimer;

    bool _warnOnRedshiftQuit = true;
    bool _enabled = false;
};
