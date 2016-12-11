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

#include "q_pamod.h"
#include "q_pulseutil.h"
#include "signals.h"

#define MODULE_NAME "module-equalizer-sink"
#define MODULE_PARAMS ""

/* SignalWrapper definitions */
/* slot */
void SignalWrapper::squit()
{
    quick_exit(0);
}
/* --- */

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
    SignalWrapper sigWrapper;

    QGuiApplication app(argc,argv);
    QQmlEngine qml_engine;

    /* quit() signal handler */
    QObject::connect(&qml_engine,SIGNAL(quit())
                     ,&sigWrapper,SLOT(squit()));

    /* Load and check module from PA */
    try {
        qpa::loadModule(MODULE_NAME,MODULE_PARAMS);
    } catch(qpa::PulseAudioException pae) {
        QObject* loadingDialog = createQmlCompo(qml_engine,"loading_module_failure.qml");
        qWarning() << pae.what() << '\n';
        app.exec();
        delete loadingDialog;
        quick_exit(1);
    }

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

    dlclose(so_handler);

    QObject* mainEqWindow = createQmlCompo(qml_engine,"main.qml");

    app.exec();

    /* bye */
    delete mainEqWindow;
    return 0;
}
