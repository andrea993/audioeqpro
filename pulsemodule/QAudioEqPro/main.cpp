#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QDebug>
#include <QObject>

#include "q_pamod.h"
//#include "signals.h"

#define MODULE_NAME "module-test"
#define MODULE_PARAMS ""

QObject* createQmlCompo(QQmlEngine& engine, const QString &localFile)
{
    QQmlComponent qml_compo(&engine,
                            QUrl(QString("qrc:/").append(localFile)));
    return qml_compo.create();
}

int main(int argc, char *argv[])
{
    //SignalWrapper sigWrapper;

    QGuiApplication app(argc,argv);
    QQmlEngine qml_engine;

    try {
        qpa::loadModule(MODULE_NAME,MODULE_PARAMS);
    } catch(qpa::PulseAudioException pae) {
        QObject* loadingDialog = createQmlCompo(qml_engine,"loading_module_failure.qml");
        qWarning(pae.what());
        app.exec();
        delete loadingDialog;
        quick_exit(1);
    }

    QObject* mainEqWindow = createQmlCompo(qml_engine,"main.qml");

    app.exec();

    //bye
    delete mainEqWindow;
    return 0;
}
