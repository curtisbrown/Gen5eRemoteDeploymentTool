#include "raspberrypideploy.h"

#include <QDebug>
#include <QDir>

RaspberryPiDeploy::RaspberryPiDeploy(QObject *parent, int bayNumber) :
    QObject(parent),
    m_remoteCommand(""),
    m_response(""),
    m_remoteDeploySource(""),
    m_fileToDeploy(""),
    m_remoteDeployDestination(""),
    m_controllerSubnet(""),
    m_raspberryPiIp(""),
    m_sshType(SSH_NONE),
    m_updateStatus(Enums::UPDATE_NONE),
    m_bayNumber(bayNumber),
    m_timedOut(false),
    m_remoteConnectionStatus(false)
{
    connect (&m_sshConnection, &QProcess::readyRead, this, &RaspberryPiDeploy::parseRemoteResponse);
    connect (this, &RaspberryPiDeploy::cmdFailedToStart, this, [=]() {
        this->setUpdateStatus(Enums::UPDATE_FAILED);
    });


    m_processTimeout.setInterval(30000); // 30 secs
    m_processTimeout.setSingleShot(true);

    connect (&m_pingProcess, static_cast<void (QProcess::*)(int)>(&QProcess::finished), this, &RaspberryPiDeploy::remoteActiveResponse);
    connect (&m_pingRestart, &QTimer::timeout, this, &RaspberryPiDeploy::remoteConnectionActive);
    m_pingRestart.setInterval(10000); // 10 secs
}

void RaspberryPiDeploy::executeRemoteCommand(QString ip, QString cmd, SshType sshType)
{
    emit debugMessage(m_bayNumber, Q_FUNC_INFO);

    m_remoteCommand = QString("%1/plink.exe -ssh -P 22 -pw root root@%2 -v %3").arg(m_remoteDeploySource).arg(ip).arg(cmd);
    m_sshType = sshType;

    emit debugMessage(m_bayNumber, m_remoteCommand);

    // First check if command is still running
    if (m_sshConnection.state() == QProcess::Running) {
        emit debugMessage(m_bayNumber, "waiting for previous CMD to finish...");
        if (m_sshConnection.waitForFinished()) {
            emit debugMessage(m_bayNumber, "CMD successfully finished");
        } else {
            emit debugMessage(m_bayNumber, "CMD Still running!!!!");
            emit cmdFailedToStart();
        }
    }

    // Configure channels and run
    m_sshConnection.setProcessChannelMode(QProcess::MergedChannels);
    m_sshConnection.start(m_remoteCommand);

    if (m_sshConnection.waitForStarted()) {
        emit debugMessage(m_bayNumber, "cmd started");
        emit cmdStarted();
    } else {
        emit debugMessage(m_bayNumber, "ERROR: cmd failed to start");
        emit cmdFailedToStart();
    }
}

void RaspberryPiDeploy::remoteTransferController(QString ip, QString source, QString destination)
{
    emit debugMessage(m_bayNumber, Q_FUNC_INFO);

    m_remoteCommand = QString("%1/pscp.exe -P 22 -pw root -v %2 root@%3:%4").arg(m_remoteDeploySource).arg(source).arg(ip).arg(destination);
    m_sshType = SSH_TRANSFER_BAY;

    // First check if command is still running
    if (m_sshConnection.state() == QProcess::Running) {
        emit debugMessage(m_bayNumber, "waiting for previous CMD to finish...");
        if (m_sshConnection.waitForFinished())
            emit debugMessage(m_bayNumber, "CMD successfully finished");
        else
            emit debugMessage(m_bayNumber, "CMD Still running!!!!");
    }

    // Configure channels and run
    m_sshConnection.setProcessChannelMode(QProcess::MergedChannels);
    m_sshConnection.start(m_remoteCommand);

    if (m_sshConnection.waitForStarted()) {
        emit debugMessage(m_bayNumber, "cmd started");
        emit cmdStarted();
    } else {
        emit debugMessage(m_bayNumber, "ERROR: cmd failed to start");
        emit cmdFailedToStart();
    }
}

void RaspberryPiDeploy::parseRemoteResponse()
{
    m_response += QString(m_sshConnection.readAll());
    if (m_response.isEmpty()) {
        emit debugMessage(m_bayNumber, "No data available to process");
        return;
    } else {
        emit debugMessage(m_bayNumber, "..............................................");
        emit debugMessage(m_bayNumber, "RESPONSE: ");
        emit debugMessage(m_bayNumber, m_response);
        emit debugMessage(m_bayNumber, "..............................................");
    }

    // First Handle prompts if any are found
    if (m_response.contains("(y/n)")) {
        // Remove everything up to and including the prompt from the response buffer
        m_response = m_response.remove(0, m_response.indexOf("(y/n)") + strlen("(y/n)"));
        emit debugMessage(m_bayNumber, "Prompt found, answering Y");
        QByteArray cmd = "y\n"; // Press Enter key after pressing "y"
        m_sshConnection.write(cmd);
        return;
    }

    if (m_response.contains("(yes/no)")) {
        // Remove everything up to and including the prompt from the response buffer
        m_response = m_response.remove(0, m_response.indexOf("(yes/no)") + strlen("(yes/no)"));
        emit debugMessage(m_bayNumber, "Prompt found, answering yes");
        QByteArray cmd = "yes\n"; // Press Enter key after pressing "y"
        m_sshConnection.write(cmd);
        return;
    }

    // Next, decide what sort of SSH connection type it is so information sent is relavant to the connection type
    switch (m_sshType) {
    case SSH_CMD_BAY_CONFIG:
        if (m_response.contains("Server sent command exit status 0") &&
             m_response.endsWith("Disconnected: All channels closed\r\n")) {
            emit debugMessage(m_bayNumber, "CMD successfully executed");
            m_response.clear();
            emit configCmdSuccess();
        }
        if (m_response.contains("cannot create directory '/dev/pts': File exists") &&
                m_response.endsWith("Disconnected: All channels closed\r\n")) {
            emit debugMessage(m_bayNumber, "File aleady exists, moving onto next command");
            m_response.clear();
            emit fileAlreadyExists();
        }
        if (m_response.contains("mounting none on /dev/pts failed: Device or resource busy") &&
                m_response.endsWith("Disconnected: All channels closed\r\n")) {
            emit debugMessage(m_bayNumber, "mount point aleady exists");
            m_response.clear();
            emit mountPointAlreadyExists();
        }
        break;
    case SSH_CMD_BAY:
        if (m_response.contains("Server sent command exit status 0") &&
                m_response.endsWith("Disconnected: All channels closed\r\n")) {
            emit debugMessage(m_bayNumber, "CMD successfully executed");
            m_response.clear();
            emit bayCmdSuccess();
        }
        if (m_response.contains("'/var/lock/LCK..ttyUSB*': No such file or directory") &&
                m_response.endsWith("Disconnected: All channels closed\r\n")) {
            emit debugMessage(m_bayNumber, "CMD successfully executed");
            m_response.clear();
            emit noLockFile();
        }
        break;
    case SSH_CMD_PI:
        break;
    case SSH_TRANSFER_BAY:
        if (m_response.contains("100%") && m_response.contains("Sending file sshpass_Controller") &&
                m_response.endsWith("Disconnected: All channels closed\r\n")) {
            emit debugMessage(m_bayNumber, "SshPass File transfer to controller SUCCESS");
            m_response.clear();
            emit sshPassTransferSuccess();
        }
        if (m_response.contains("100%") && m_response.contains("Sending file RasPiServer") &&
                m_response.endsWith("Disconnected: All channels closed\r\n")) {
            emit debugMessage(m_bayNumber, "SshPass File transfer to controller SUCCESS");
            m_response.clear();
            emit rasPiServerTransferSuccess();
        }
        break;
    case SSH_TRANSFER_PI:
        if (m_response.contains("Server sent command exit status 0") &&
                m_response.endsWith("Disconnected: All channels closed\r\n")) {
            emit debugMessage(m_bayNumber, "CMD successfully executed");
            m_response.clear();
            emit raspPiTransferFromBaySuccess();
        }
        break;
    default:
        emit debugMessage(m_bayNumber, "Unknown ssh type, cannot answer prompts correctly");
        break;
    }
}

void RaspberryPiDeploy::processRequest(QString ip)
{
    QStateMachine *stateMachine = new QStateMachine(this);
    QState *remoteCreatePseudoTerminal = prepareControllerSubStates(stateMachine, ip);
    QState *sshPassTransferToController = transferSshPassExeToController(stateMachine, ip);
    QState *transferPiFileToController = new QState(stateMachine);
    QState *killRunningAppOnPi = killRunningApp(stateMachine, ip);
    QState *remoteTransferToPi = new QState(stateMachine);
    QFinalState *done = new QFinalState(stateMachine);
    stateMachine->setInitialState(remoteCreatePseudoTerminal);

    connect(remoteCreatePseudoTerminal, &QState::entered, this, [this, ip]() {
        this->setUpdateStatus(Enums::UPDATE_IN_PROGRESS);
        emit debugMessage(m_bayNumber, QString("Creating pseudo terminal for bay with IP: %1").arg(ip));
    });
    connect(sshPassTransferToController, &QState::entered, this, [=]() {
        emit debugMessage(m_bayNumber, "Transferring sshpass exe to controller");
    });
    connect(transferPiFileToController, &QState::entered, this, [this, ip]() {
        emit debugMessage(m_bayNumber, "Transferring RasPiServer Application for remote deployment to Pi");
        this->remoteTransferController(ip, QString("%1/%2").arg(m_remoteDeploySource).arg(m_fileToDeploy), "/home/IFT");
    });
    connect(killRunningAppOnPi, &QState::entered, this, [=]() {
        emit debugMessage(m_bayNumber, "Killing Application on Raspberry Pi");
    });
    connect(remoteTransferToPi, &QState::entered, this, [this, ip]() {
        emit debugMessage(m_bayNumber, "Sending new version of the Ras Pi Server app to the Pi from the Bay");
        this->executeRemoteCommand(ip, QString("/usr/bin/sshpass -pchangeme scp /home/IFT/%1 root@%2:%3").
                                   arg(m_fileToDeploy).arg(m_raspberryPiIp).arg(m_remoteDeployDestination), SSH_TRANSFER_PI);
    });
    connect(done, &QFinalState::entered, this, [=]() {
        this->setUpdateStatus(Enums::UPDATE_SUCCESS);
        emit debugMessage(m_bayNumber, "Raspberry Pi Server Application updated successfully");
    });

    remoteCreatePseudoTerminal->addTransition(remoteCreatePseudoTerminal, &QState::finished, sshPassTransferToController);
    sshPassTransferToController->addTransition(sshPassTransferToController, &QState::finished, transferPiFileToController);
    transferPiFileToController->addTransition(this, &RaspberryPiDeploy::rasPiServerTransferSuccess, killRunningAppOnPi);
    killRunningAppOnPi->addTransition(killRunningAppOnPi, &QState::finished, remoteTransferToPi);
    remoteTransferToPi->addTransition(this, &RaspberryPiDeploy::raspPiTransferFromBaySuccess, done);

    stateMachine->start();
}

void RaspberryPiDeploy::resetPiDeploy()
{
    emit debugMessage(m_bayNumber, Q_FUNC_INFO);

    m_remoteCommand.clear();
    m_response.clear();
    m_remoteDeploySource.clear();
    m_fileToDeploy.clear();
    m_remoteDeployDestination.clear();
    m_controllerSubnet.clear();
    m_raspberryPiIp.clear();
    m_sshType = SSH_NONE,
    setUpdateStatus(Enums::UPDATE_NONE);
    m_timedOut = false;
    setRemoteConnectionStatus(false);
    m_pingRestart.stop();
}

/*
 * bool RaspberryPiDeploy::remoteConnectionActive()
 *
 * Checks to see whether the connection to the Bay is active
 * as without this connection, there is no connection to the
 * Raspberry Pi
 */
void RaspberryPiDeploy::remoteConnectionActive()
{
    QStringList pingArgs;
    pingArgs << "-n 1"
             << m_controllerSubnet
             << "."
             << QString::number(m_bayNumber);

    if (m_pingProcess.state() != QProcess::Running)
        m_pingProcess.start("ping", pingArgs);
    else
        return;

    if (!m_pingProcess.waitForStarted(3000))
        emit debugMessage(m_bayNumber, QString("start error: %1").arg(m_pingProcess.errorString()));
}

void RaspberryPiDeploy::remoteActiveResponse(int exitCode)
{
    if (exitCode == QProcess::CrashExit) {
        emit debugMessage(m_bayNumber, "ERROR: Issue with ping attempt");
        setRemoteConnectionStatus(false);
        return;
    }

    QString pingOutput = m_pingProcess.readAllStandardOutput();
    QStringList outputLines = pingOutput.split("\n");
    foreach (QString line, outputLines) {
        if (line.contains("0% loss")) {
            emit debugMessage(m_bayNumber, "Connection to bay active");
            setRemoteConnectionStatus(true);
            return;
        }
    }
    emit debugMessage(m_bayNumber, "ERROR: Issue with connection to bay");
    setRemoteConnectionStatus(false);
}

QState* RaspberryPiDeploy::prepareControllerSubStates(QState *parent, QString ip)
{
    QState *testState = new QState(parent);

    QState *createDir = new QState(testState);
    QState *addEntryToFstab = new QState(testState);
    QState *mountDir = new QState(testState);
    QFinalState *done = new QFinalState(testState);
    testState->setInitialState(createDir);

    connect(createDir, &QState::entered, this, [this, ip]() {
        emit debugMessage(m_bayNumber, "Creating Directory");
        this->m_pingRestart.stop();
        this->executeRemoteCommand(ip, "mkdir /dev/pts", SSH_CMD_BAY_CONFIG);
    });
    connect(addEntryToFstab, &QState::entered, this, [this, ip]() {
        emit debugMessage(m_bayNumber, "Adding entry to fstab");
        this->executeRemoteCommand(ip, "echo \"none  /dev/pts  devpts  defaults 0 0\" >> /etc/fstab", SSH_CMD_BAY_CONFIG);
    });
    connect(mountDir, &QState::entered, this, [this, ip]() {
        emit debugMessage(m_bayNumber, "Mounting directory");
        this->executeRemoteCommand(ip, "mount /dev/pts", SSH_CMD_BAY_CONFIG);
    });
    connect(done, &QFinalState::entered, this, [=](){
       this->m_pingRestart.start();
    });

    createDir->addTransition(this, &RaspberryPiDeploy::configCmdSuccess, addEntryToFstab);
    createDir->addTransition(this, &RaspberryPiDeploy::fileAlreadyExists, addEntryToFstab);

    addEntryToFstab->addTransition(this, &RaspberryPiDeploy::configCmdSuccess, mountDir);

    mountDir->addTransition(this, &RaspberryPiDeploy::configCmdSuccess, done);
    mountDir->addTransition(this, &RaspberryPiDeploy::mountPointAlreadyExists, done);

    return testState;
}

QState* RaspberryPiDeploy::transferSshPassExeToController(QState *parent, QString ip)
{
    emit debugMessage(m_bayNumber, Q_FUNC_INFO);

    QState *testState = new QState(parent);

    QState *transferSshPass = new QState(testState);
    QState *changeSshPassPermissions = new QState(testState);
    QFinalState *done = new QFinalState(testState);
    testState->setInitialState(transferSshPass);

    connect(transferSshPass, &QState::entered, this, [this, ip]() {
        emit debugMessage(m_bayNumber, QString("Deploying to sshpass to Bay with IP: %1").arg(ip));
        this->remoteTransferController(ip, QString("%1/sshpass_Controller").arg(m_remoteDeploySource), "/usr/bin/sshpass");
    });
    connect(changeSshPassPermissions, &QState::entered, this, [this, ip]() {
        emit debugMessage(m_bayNumber, QString("Changing sshpass permissions for Bay with IP: %1").arg(ip));
        this->executeRemoteCommand(ip, "chmod +x /usr/bin/sshpass", SSH_CMD_BAY);
    });

    transferSshPass->addTransition(this, &RaspberryPiDeploy::sshPassTransferSuccess, changeSshPassPermissions);
    changeSshPassPermissions->addTransition(this, &RaspberryPiDeploy::bayCmdSuccess, done);

    return testState;
}

QState *RaspberryPiDeploy::killRunningApp(QState *parent, QString ip)
{
    emit debugMessage(m_bayNumber, Q_FUNC_INFO);

    QState *testState = new QState(parent);

    QState *removeLockFiles = new QState(testState);
    QState *killAppUsb0 = new QState(testState);
    QState *killAppUsb1 = new QState(testState);
    QFinalState *done = new QFinalState(testState);
    testState->setInitialState(removeLockFiles);

    connect(removeLockFiles, &QState::entered, this, [this, ip]() {
        emit debugMessage(m_bayNumber, QString("Removing lock Files for Bay with IP: %1").arg(ip));
        this->executeRemoteCommand(ip, "rm /var/lock/LCK..ttyUSB*", SSH_CMD_BAY);
    });
    connect(killAppUsb0, &QState::entered, this, [this, ip]() {
        emit debugMessage(m_bayNumber, "Attempting to Kill Rasp Pi app via USB connection 0");
        this->executeRemoteCommand(ip, "echo \"0#\" | microcom -p /dev/ttyUSB0", SSH_CMD_BAY);
    });
    connect(killAppUsb1, &QState::entered, this, [this, ip]() {
        emit debugMessage(m_bayNumber, "Attempting to Kill Rasp Pi app via USB connection 1");
        this->executeRemoteCommand(ip, "echo \"0#\" | microcom -p /dev/ttyUSB1", SSH_CMD_BAY);
    });

    removeLockFiles->addTransition(this, &RaspberryPiDeploy::bayCmdSuccess, killAppUsb0);
    removeLockFiles->addTransition(this, &RaspberryPiDeploy::noLockFile, killAppUsb0);
    killAppUsb0->addTransition(this, &RaspberryPiDeploy::bayCmdSuccess, killAppUsb1);
    killAppUsb1->addTransition(this, &RaspberryPiDeploy::bayCmdSuccess, done);

    return testState;
}

bool RaspberryPiDeploy::remoteConnectionStatus() const
{
    return m_remoteConnectionStatus;
}

void RaspberryPiDeploy::setRemoteConnectionStatus(bool remoteConnectionStatus)
{
    m_remoteConnectionStatus = remoteConnectionStatus;
    emit remoteConnectionStatusChanged();
}

Enums::UpdateStatus RaspberryPiDeploy::updateStatus() const
{
    return m_updateStatus;
}

void RaspberryPiDeploy::setUpdateStatus(Enums::UpdateStatus updateStatus)
{
    m_updateStatus = updateStatus;
    emit updateStatusChanged();
}

void RaspberryPiDeploy::setControllerSubNet(const QString &controllerSubNet)
{
    m_controllerSubnet = controllerSubNet;
    m_pingRestart.start();
}

QString RaspberryPiDeploy::remoteDeployDestination() const
{
    return m_remoteDeployDestination;
}

void RaspberryPiDeploy::setRemoteDeployDestination(const QString &remoteDeployDestination)
{
    emit debugMessage(m_bayNumber, Q_FUNC_INFO);
    m_remoteDeployDestination = remoteDeployDestination;
}

QString RaspberryPiDeploy::remoteDeploySource() const
{
    return m_remoteDeploySource;
}

void RaspberryPiDeploy::setRemoteDeploySource(const QString &remoteDeployLocation)
{
    emit debugMessage(m_bayNumber, Q_FUNC_INFO);
    emit debugMessage(m_bayNumber, "Splitting into location and fileName");

    QStringList list = remoteDeployLocation.split("/");
    m_fileToDeploy = list.last();

    QString temp = remoteDeployLocation;
    temp = temp.remove(QString("/%1").arg(m_fileToDeploy));
    m_remoteDeploySource = temp;
}

QString RaspberryPiDeploy::raspberryPiIp() const
{
    return m_raspberryPiIp;
}

void RaspberryPiDeploy::setRaspberryPiIp(const QString &raspberryPiIp)
{
    emit debugMessage(m_bayNumber, Q_FUNC_INFO);
    m_raspberryPiIp = raspberryPiIp;
}
