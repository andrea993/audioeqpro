#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "pulsedriver.h"
#include "guimanager.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    engine.load(QUrl(QLatin1String("qrc:/main.qml")));


    GuiManager gm(&engine);
    QObject::connect(&app, SIGNAL(applicationStateChanged(Qt::ApplicationState)),
            &gm, SLOT(AppStateChanged(Qt::ApplicationState)));


    return app.exec();
}
