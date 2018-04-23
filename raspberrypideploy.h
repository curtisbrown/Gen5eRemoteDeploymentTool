#ifndef RASPBERRYPIDEPLOY_H
#define RASPBERRYPIDEPLOY_H

#include <QObject>
#include <QProcess>
#include <QTimer>
#include <QState>
#include <QFinalState>
#include <QStateMachine>

#include "enumerations.h"

class RaspberryPiDeploy : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Enums::UpdateStatus updateStatus READ updateStatus WRITE setUpdateStatus NOTIFY updateStatusChanged)

public:
    RaspberryPiDeploy(QObject *parent = nullptr, int bayNumber = 0);

    enum SshType {
        SSH_NONE,
        SSH_CMD_BAY_CONFIG, // Remote command to configure bay for further remote activity
        SSH_CMD_BAY,        // Remote command to the bay
        SSH_CMD_PI,         // Remote command to the Pi via the bay
        SSH_TRANSFER_BAY,   // Remote file transfer to the bay
        SSH_TRANSFER_PI     // Remote file transfer to the Pi via the bay
    };

    QString raspberryPiIp() const;
    QString remoteDeploySource() const;
    QString remoteDeployDestination() const;

    void setUpdateStatus(Enums::UpdateStatus updateStatus);


signals:
    void debugMessage(int, QString);
    void updateStatusChanged();

    void cmdStarted();
    void cmdFailedToStart();

    void configCmdSuccess();
    void fileAlreadyExists();
    void mountPointAlreadyExists();

    void sshPassTransferSuccess();
    void rasPiServerTransferSuccess();
    void bayCmdSuccess();

    void noLockFile();
    void raspPiTransferFromBaySuccess();

public slots:
    Q_INVOKABLE void setControllerSubNet(const QString &controllerSubNet);
    Q_INVOKABLE void setRaspberryPiIp(const QString &raspberryPiIp);
    Q_INVOKABLE void setRemoteDeploySource(const QString &remoteDeploySource);
    Q_INVOKABLE void setRemoteDeployDestination(const QString &remoteDeployDestination);
    Enums::UpdateStatus updateStatus() const;
    void processRequest(QString ip);
    void resetPiDeploy();

private slots:
    void executeRemoteCommand(QString ip, QString cmd, SshType sshtype);
    void remoteTransferController(QString ip, QString source, QString destination);

    void parseRemoteResponse();

private:

    // Utility StateMachines
    QState *prepareControllerSubStates(QState *parent, QString ip);
    QState *transferSshPassExeToController(QState *parent, QString ip);
    QState *killRunningApp(QState *parent, QString ip);

    QProcess m_sshConnection;
    QString m_remoteCommand;
    QString m_response;
    QString m_remoteDeploySource;
    QString m_fileToDeploy;
    QString m_remoteDeployDestination;
    QString m_controllerSubnet;
    QString m_raspberryPiIp;
    SshType m_sshType;
    QTimer m_processTimeout;
    Enums::UpdateStatus m_updateStatus;
    int m_bayNumber;
    bool m_timedOut;
};

#endif // RASPBERRYPIDEPLOY_H
