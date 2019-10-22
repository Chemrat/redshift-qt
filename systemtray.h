#pragma once

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
    void onRedshiftQuit(int exitCode, QProcess::ExitStatus exitStatus);
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
    QString _errText;

    QIcon _iconEnabled;
    QIcon _iconDisabled;
    QAction *_suspendMenu = nullptr;

    QProcess *_redshiftProcess = nullptr;
    QTimer *_suspendTimer = nullptr;

    bool _warnOnRedshiftQuit = true;
    bool _enabled = false;
};
