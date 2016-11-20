#ifndef Q_PULSEUTIL_H
#define Q_PULSEUTIL_H

#include <QString>

#define PATH_DOES_NOT_EXIST QString("__null__path__")

namespace qpa {
namespace util {
QString getPulseVersion(); //returns x.y (MAJOR.MINOR)
QString getFullPulseVersion(); //returns x.y.z (MAJOR.MINOR.MICRO)
QString getModuleDir(); //will determine prefix itself
QString getModulePath(const QString& modName); //returns module path /pa/th/module.so
}
}

#endif // Q_PULSEUTIL_H
