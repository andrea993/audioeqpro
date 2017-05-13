#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "guimanager.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;


    engine.load(QUrl(QLatin1String("qrc:/main.qml")));

     QObject *rootObject = engine.rootObjects().first();


    QObject *sliderRow=rootObject->findChild<QObject*>("slidersRow");

    GuiManager gm(sliderRow);
    QObject::connect(sliderRow, SIGNAL(sliderChange(int,double)),&gm, SLOT(sliderChanged(int,double)));





    /*sliderRow->setProperty("oct", 1.0);
    QMetaObject::invokeMethod(sliderRow, "redrawSlider");*/









    return app.exec();
}
