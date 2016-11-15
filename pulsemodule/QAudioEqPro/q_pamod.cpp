#include "q_pamod.h"

#include<pulse/context.h>
#include<pulse/mainloop.h>
#include<pulse/mainloop-api.h>

qpa::Q_PaModuleInfo::Q_PaModuleInfo(const QString& name,
        const QString& args,
        const quint32& index,
        const quint32& nUsed,
        const pa_proplist* proplist) :
    name(name),args(args),index(index),
    nUsed(nUsed),proplist(proplist) 
{}

qpa::Q_PaModuleInfo qpa::loadModule(const QString& name, const QString& args)
{
    pa_mainloop* mloop = pa_mainloop_new();
    pa_mainloop_api* mloop_api = pa_mainloop_get_api(mloop);
    pa_context* pactx = pa_context_new("qaudioeqpro",mloop_api);
    int connect_rval = pa_context_connect(pactx,NULL,PA_CONTEXT_NOFLAGS,NULL);

    if(pactx)
        pa_context_unref(pactx);

    if(mloop)
        pa_mainloop_free(mloop);
    
    return qpa::Q_PaModuleInfo("","",0,0,NULL); 
}