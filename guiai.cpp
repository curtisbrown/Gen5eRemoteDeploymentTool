#include "guiai.h"

#include <QDebug>

GuiAI::GuiAI(QObject *parent) :
    QObject(parent),
    m_utilities(this)
{
    qDebug() << Q_FUNC_INFO;
    qDebug() << "Creating 16 Raspberry Pi connections";

    for (int i = 0; i < 16; ++i) {
        RaspberryPiDeploy *newPi = new RaspberryPiDeploy(this, i + 1);
        connect(newPi, &RaspberryPiDeploy::debugMessage, &m_utilities, &Utilities::debugLogMessage);
        m_piList << newPi;
    }
}

RaspberryPiDeploy *GuiAI::getPi(int index)
{
    if (index >= 16) {
        qFatal("Pi index out of bound");
    }
    return m_piList.at(index);
}

void GuiAI::reset()
{
    qDebug() << Q_FUNC_INFO;
    for (int i = 0; i < 16; ++i) {
        m_piList.at(i)->resetPiDeploy();
    }
}

