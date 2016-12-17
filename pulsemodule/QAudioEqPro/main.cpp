#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QDebug>

#ifdef __gnu_linux__
#include <dlfcn.h>
#else
#error Your OS is not supported for now
#endif //__gnu_linux__

#include "q_pamod.h"
#include "q_pulseutil.h"
#include "qmlutils.h"
#include "moduleobject.h"

#define MODULE_NAME "module-equalizer-sink"
#define MODULE_PARAMS ""

QObject* createQmlCompo(QQmlEngine& engine, const QString &localFile)
{
    QQmlComponent qml_compo(&engine,
                            QUrl(QString("qrc:/").append(localFile)));
    return qml_compo.create();
}

void* checkAndLoadDl()
{
    QString modpath = qpa::util::getModulePath(MODULE_NAME);
    if(modpath == PATH_DOES_NOT_EXIST)
        return NULL;

    return dlopen(modpath.toStdString().c_str(),RTLD_NOW);
}

int main(int argc, char *argv[])
{
    QmlUtils qmlutil;

    QGuiApplication app(argc,argv);
    QQmlEngine qml_engine;

    /* quit() signal handler */
    QObject::connect(&qml_engine,SIGNAL(quit())
                     ,&qmlutil,SLOT(squit()));

    /* Load and check module from PA */
    try {
        qpa::loadModule(MODULE_NAME,MODULE_PARAMS);
    } catch(qpa::PulseAudioException pae) {
        QObject* loadingDialog = createQmlCompo(qml_engine,"loading_module_failure.qml");
        qWarning(pae.what());
        app.exec();
        delete loadingDialog;
        quick_exit(1);
    }

    void* so_handler;
    if((so_handler=checkAndLoadDl()) == NULL) {
        qWarning() << "dlopen failed:" << dlerror() << '\n';
        quick_exit(1);
    }

    ModuleObject::getInstance()->setObjectHandler(so_handler);

    qml_engine.rootContext()->setContextProperty("utils",&qmlutil);
    QObject* mainEqWindow = createQmlCompo(qml_engine,"main.qml");

    app.exec();

    /* bye */
    delete mainEqWindow;
    return 0;
}
