#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QDebug>

#include <cstring>

#ifdef __gnu_linux__
#include <dlfcn.h>
#else
#error Your OS is not supported for now
#endif //__gnu_linux__

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

    void* so_handler;
    if((so_handler=checkAndLoadDl()) == NULL) {
        char dlstrerr[256];
        strncpy(dlstrerr,dlerror(),256);
        qWarning() << "dlopen failed:" << dlstrerr << '\n';
        QString fullstr("Error while dlopen() was executed: "+QString::fromUtf8(dlstrerr));
        QObject* errorDialog = createQmlCompo(qml_engine,"general_error_dialog.qml");
        errorDialog->findChild<QObject*>("editErrText")->setProperty("text",fullstr);
        app.exec();
        delete errorDialog;
        quick_exit(1);
    }

    ModuleObject::getInstance()->setObjectHandler(so_handler);
    qml_engine.rootContext()->setContextProperty("utils",&qmlutil);

    QObject* mainEqWindow = createQmlCompo(qml_engine,"main.qml");
    QObject* warningModule = createQmlCompo(qml_engine,"is_loaded.qml");

    warningModule->setParent(mainEqWindow);
    mainEqWindow->findChild<QObject*>("welcomeTextObj")->setProperty("text",QString("Running PulseAudio version: %1")
                                                                     .arg(qpa::util::getFullPulseVersion()));

    app.exec();

    /* bye */
    delete warningModule;
    delete mainEqWindow;

    return 0;
}
