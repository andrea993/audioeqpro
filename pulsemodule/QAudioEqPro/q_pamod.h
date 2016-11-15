#ifndef __Q_PAMOD
#define __Q_PAMOD

#include<QString>
#include<QtGlobal>
#include<pulse/introspect.h>

namespace qpa 
{
struct Q_PaModuleInfo 
{
    Q_PaModuleInfo(const QString& name,const QString& args,const quint32& index,const quint32& nUsed,const pa_proplist* proplist);
    const QString name,args;
    const quint32 index,nUsed;
    const pa_proplist* proplist;
};
Q_PaModuleInfo loadModule(const QString& name, const QString& args);
}

#endif
