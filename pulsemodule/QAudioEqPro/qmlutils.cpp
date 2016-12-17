#include "qmlutils.h"
#include "moduleobject.h"

void QmlUtils::squit()
{
    quick_exit(0);
}

unsigned QmlUtils::getMaxBands()
{
    return 3;
}

void QmlUtils::changedBands(unsigned value)
{
    ModuleObject* obj = ModuleObject::getInstance();

    if(obj->getBands() == value)
        return;

    obj->setBands(value);
    //create bands
}
