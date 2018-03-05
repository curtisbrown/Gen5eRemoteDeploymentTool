#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "guiai.h"
#include "raspberrypideploy.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);
    GuiAI guiAI;

    qmlRegisterUncreatableType<Enums>("com.CTDI.DeployTool", 1, 0, "Enums", "Cannot create instance of Enums outside of C++");
    qmlRegisterUncreatableType<RaspberryPiDeploy>("com.CTDI.DeployTool", 1, 0, "RaspberryPiDeploy", "Cannot create instance of RaspberryPiDeploy outside of C++");

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("guiAI", &guiAI);
    engine.load(QUrl(QLatin1String("qrc:/main.qml")));


    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
