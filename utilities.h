#ifndef UTILITIES_H
#define UTILITIES_H

#include <QObject>
#include <QFile>

class Utilities : public QObject
{
    Q_OBJECT
public:
    explicit Utilities(QObject *parent = nullptr);

signals:

public slots:
    void initialiseLogFile();
    void debugLogMessage(int bayNumber, QString string);

    bool checkFileExists(QString filename);
    bool deleteFile(QString filename);

private:
    QFile m_debugLog;
};

#endif // UTILITIES_H
