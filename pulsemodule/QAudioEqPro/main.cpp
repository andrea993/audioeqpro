#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <pulse/introspect.h>
//doc: https://freedesktop.org/software/pulseaudio/doxygen/introspect_8h.html

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}
