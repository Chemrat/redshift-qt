#pragma once

#include <memory>

#include <QtWidgets/QSystemTrayIcon>

class QIcon;
class QProcess;

class SystemTray
        : public QSystemTrayIcon
{
public:
    SystemTray();

    bool CreateIcon();
    bool StartRedshift();
    void ToggleRedshift(bool enable = true);

private:
    void onClick(QSystemTrayIcon::ActivationReason);
    void onQuit();

private:
    std::shared_ptr<QIcon> _iconEnabled;
    std::shared_ptr<QIcon> _iconDisabled;

    std::shared_ptr<QProcess> _redshiftProcess;
    bool _enabled = false;
};
