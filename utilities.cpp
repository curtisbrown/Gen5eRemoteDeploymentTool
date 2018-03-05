#include "utilities.h"

#include <QTextStream>
#include <QDebug>

Utilities::Utilities(QObject *parent) :
    QObject(parent),
    m_debugLog("debugLog.txt")
{
    qDebug() << Q_FUNC_INFO;
    initialiseLogFile();
}

void Utilities::initialiseLogFile()
{
    qDebug() << "Initialising Log file";
    if (m_debugLog.exists()) {
        // Check if previous exists also
        QFile previousDebugLog("debugLogOld.txt");
        if (previousDebugLog.exists()) {
            if (previousDebugLog.remove())
                qDebug() << "Removed old log file to create a new \"previous logfile\"";
        }
        if (m_debugLog.copy("debugLogOld.txt")) {
            m_debugLog.remove();
            qDebug() << "Old log file created successfully";
        }
    } else {
        qDebug() << "Log file does not exist yet, doing nothing";
    }
}

void Utilities::debugLogMessage(int bayNumber, QString string)
{
    qDebug() << "BAY " << bayNumber << ": " << string;
    if (m_debugLog.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
        QTextStream out(&m_debugLog);
        out << endl
            << "BAY "
            << bayNumber
            << ": "
            << string;
        m_debugLog.close();
    } else {
        qWarning() << Q_FUNC_INFO << ", could not open the file " << m_debugLog.fileName();
    }
}

bool Utilities::checkFileExists(QString filename)
{
    qDebug() << Q_FUNC_INFO;
    QFile fileToCheck(filename);

    return fileToCheck.exists();
}

bool Utilities::deleteFile(QString filename)
{
    qDebug() << Q_FUNC_INFO;
    bool status = false;

    if (checkFileExists(filename)) {
        QFile file(filename);
        if (file.remove()) {
            qDebug() << "File successfully removed";
            status = true;
        }
    } else {
        qDebug() << "File doesn't exist to delete";
        return true;
    }

    return status;
}
