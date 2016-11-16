#include "q_pamod.h"

#include<pulse/introspect.h>
#include<pulse/context.h>
#include<pulse/mainloop.h>
#include<pulse/mainloop-api.h>

#include<string>

struct mod_info {
    const char *name,*args;
    pa_mainloop_api *api;
};

void enable_mod(pa_context *ctx,void *userdata)
{
    mod_info *m =(mod_info*)userdata;
    pa_operation *paop=NULL;

    switch(pa_context_get_state(ctx)) {
        case PA_CONTEXT_READY:
            paop=pa_context_load_module(ctx,m->name,m->args,NULL,NULL);
            if(paop)
                pa_operation_unref(paop);
            m->api->quit(m->api,0); //IDX
            break;
        case PA_CONTEXT_FAILURE:
            m->api->quit(m->api,-2);
            break;
    }
            
    
}

void qpa::loadModule(const QString& name,const QString& args)
{
    pa_context* pactx = NULL;
    pa_mainloop* pamloop = NULL;
    pa_mainloop_api* pamloop_api = NULL;

    if(! (pamloop=pa_mainloop_new()) ) {
        goto clean;
        throw qpa::PulseAudioException("Cannot create new mainloop");
    }

    if(! (pamloop_api=pa_mainloop_get_api(pamloop)) ) {
        goto clean;
        throw qpa::PulseAudioException("Cannot get MainLoop API from MainLoop");
    }

    if(! (pactx=pa_context_new(pamloop_api)) ) {
        goto clean;
        throw qpa::PulseAudioException("Cannot create new PA context");
    }

    mod_info modata;
    modata.name=name.toStdString().c_str();
    modata.args=args.toStdString().c_str();
    modata.api=api.toStdString().c_str();

    pa_context_set_state_callback(pactx,enable_mod,&modata);
    if(pa_context_connect(pactx,NULL,PA_CONTEXT_NOFLAGS,NULL) < 0) {
        goto clean;
        throw qpa::PulseAudioException("Cannot connect to PA server");
    }

    int loopval=-1;
    if(pa_mainloop_run(mloop,&loopval) < 0) {
        goto clean;
        throw qpa::PulseAudioException("Cannot run mainloop");
    }

    if(loopval < 0) {
        goto clean;
        throw qpa::PulseAudioException("MainLoop exited with failure code");
    } 
clean:
    if(pactx)
        pa_context_unref(pactx);

    if(pamloop)
        pa_mainloop_free(pamloop);
}

qpa::PulseAudioException::PulseAudioException(const char* strerr): str_err(strerr) {}

const char* qpa::PulseAudioException::what() const throw()
{
    return str_err;
}
