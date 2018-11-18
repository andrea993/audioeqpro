#include <QGuiApplication>
#include <QApplication>
#include <QQmlApplicationEngine>

#include "pulsedriver.h"
#include "guimanager.h"

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);
    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    GuiManager gm(&engine);
    QObject::connect(&app, SIGNAL(applicationStateChanged(Qt::ApplicationState)),
            &gm, SLOT(AppStateChanged(Qt::ApplicationState)));

    return app.exec();
}
