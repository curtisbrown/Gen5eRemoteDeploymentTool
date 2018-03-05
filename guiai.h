#ifndef GUIAI_H
#define GUIAI_H

#include <QObject>
#include <QSharedPointer>

#include "utilities.h"
#include "raspberrypideploy.h"

class GuiAI : public QObject
{
    Q_OBJECT

public:
    explicit GuiAI(QObject *parent = nullptr);


    Q_INVOKABLE RaspberryPiDeploy *getPi(int index);

signals:

public slots:
    void reset();

private:
    Utilities m_utilities;
    RaspberryPiDeploy m_rasPiDeploy;
    QList<RaspberryPiDeploy*> m_piList;

};

#endif // GUIAI_H
