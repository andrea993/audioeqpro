#ifndef MODULEOBJECT_H
#define MODULEOBJECT_H

//singleton class design
class ModuleObject
{
public:
    ~ModuleObject();
    static ModuleObject* getInstance();
    bool setObjectHandler(void* handle);
    void setBands(unsigned value);
    unsigned getBands() const;
private:
    ModuleObject();
    void* p_so_handler;
    static ModuleObject* obj;
};

#endif // MODULEOBJECT_H
