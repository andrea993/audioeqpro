#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "q_pamod.h"

#define MODULE_NAME "module-equalizer-sink"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc,argv);

    if(qpa::loadModule(MODULE_NAME,"")) {
        //use dlopen to load shared object
    }

    return app.exec();
}
