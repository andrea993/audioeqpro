#include <dlfcn.h>
#include "moduleobject.h"

ModuleObject* ModuleObject::obj = nullptr;

ModuleObject::~ModuleObject()
{
    if(p_so_handler)
        dlclose(p_so_handler);
    if(obj)
        delete obj;
}

ModuleObject::ModuleObject() :
    p_so_handler(nullptr)
{}

ModuleObject* ModuleObject::getInstance()
{
    if(!obj)
        obj = new ModuleObject;

    return obj;
}

bool ModuleObject::setObjectHandler(void *handle)
{
    if(!p_so_handler && handle)
        p_so_handler = handle;
    else
        return false;

    return true;
}

unsigned ModuleObject::getBands() const
{
    return 1;
}

void ModuleObject::setBands(unsigned value)
{

}
