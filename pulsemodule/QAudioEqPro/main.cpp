#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "q_pamod.h"

int main(int argc, char *argv[])
{
    if(qpa::loadModule("module-equalizer-sink",""))
        return -1;

    QGuiApplication app(argc,argv);
    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    app.exec();

    return 0;
}
