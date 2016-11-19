#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQmlContext>

#include "q_pamod.h"

#define MODULE_NAME "module-equalizer-sink"
#define MODULE_PARAMS ""

QObject* createQmlCompo(QQmlEngine& engine, const QString &localFile)
{
    QQmlComponent qml_compo(&engine,
                            QUrl::fromLocalFile(localFile));
    return qml_compo.create();
}

int main(int argc, char *argv[])
{
    QGuiApplication app(argc,argv);
    QQmlEngine qml_engine;
    QObject* loadingDialog = createQmlCompo(qml_engine,"loading_module.qml");
    QQmlContext* ctx = qml_engine.rootContext();
    ctx->setContextProperty("sig",nullptr);

    if(qpa::loadModule(MODULE_NAME,MODULE_PARAMS)) {
        //use dlopen to load shared object
    }

    return app.exec();
}
