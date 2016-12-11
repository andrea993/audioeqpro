#include <pulse/version.h>
#include <QDir>
#include <QFile>
#include <QVector>
#include <string>

#include "q_pulseutil.h"

QString qpa::util::getPulseVersion()
{
    return QString::fromStdString(std::to_string(PA_MAJOR)+"."+std::to_string(PA_MINOR));
}

QString qpa::util::getFullPulseVersion()
{
    return QString::fromStdString(std::to_string(PA_MAJOR)+"."+std::to_string(PA_MINOR)+"."+
                                  std::to_string(PA_MICRO));
}

QString qpa::util::getModuleDir()
{
    QVector<QString> prefixes;
    prefixes.push_back("/usr/lib/");
    prefixes.push_back("/usr/local/lib/");
    prefixes.push_back("/usr/");

    for(const QString& cur_prefix : prefixes) {
        QString dirpath=cur_prefix;
        dirpath.append("pulse-").append(getPulseVersion()).append("/modules");

        QDir dirObj(dirpath);
        if(dirObj.exists())
            return dirpath;
    }

    return PATH_DOES_NOT_EXIST;
}

QString qpa::util::getModulePath(const QString &modName)
{
    QString modpath=getModuleDir();

    if(modpath == PATH_DOES_NOT_EXIST)
        return PATH_DOES_NOT_EXIST;

    QString fmod_qstr = modpath.append("/").append(modName).append(".so");

    QFile modfile(fmod_qstr);
    if(modfile.exists())
        return fmod_qstr;

    return PATH_DOES_NOT_EXIST;
}
