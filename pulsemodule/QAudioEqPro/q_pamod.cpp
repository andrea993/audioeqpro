#include "q_pamod.h"

#include<pulse/introspect.h>
#include<pulse/context.h>
#include<pulse/mainloop.h>
#include<pulse/mainloop-api.h>

#include<string>
#include<cstring>

#define __inline static inline

//DO NOT CHANGE VALUES//
static char g_name[256] = {0}; //Module name
static char g_args[256] = {0}; //Module arguments
//DO NOT CHANGE VALUES//

__inline void clean(pa_context* ctx, pa_mainloop* mloop) 
{
	if(ctx)
		pa_context_unref(ctx);
	if(mloop)
		pa_mainloop_free(mloop);
}

static void enable_mod(pa_context *ctx,void *userdata)
{
    pa_mainloop_api *api = (pa_mainloop_api*) userdata;
    pa_operation *paop=NULL;
    pa_context_state_t state = pa_context_get_state(ctx);

    switch(state) {
    case PA_CONTEXT_AUTHORIZING:
    case PA_CONTEXT_CONNECTING:
    case PA_CONTEXT_SETTING_NAME:
    case PA_CONTEXT_UNCONNECTED:
        break;

    case PA_CONTEXT_READY:
        paop=pa_context_load_module(ctx,g_name,g_args,NULL,NULL);
        if(paop)
            pa_operation_unref(paop);
        api->quit(api,0);
        break;

    case PA_CONTEXT_TERMINATED: break;
    case PA_CONTEXT_FAILED:
        api->quit(api,-2);
        break;
    }
}

bool qpa::loadModule(const QString& name,const QString& args)
{
    pa_context* pactx = NULL;
    pa_mainloop* pamloop = NULL;
    pa_mainloop_api* pamloop_api = NULL;

    if(! (pamloop=pa_mainloop_new()) ) {
        clean(pactx,pamloop);
        throw qpa::PulseAudioException("Cannot create new mainloop");
    }

    if(! (pamloop_api=pa_mainloop_get_api(pamloop)) ) {
        clean(pactx,pamloop);
        throw qpa::PulseAudioException("Cannot get MainLoop API from MainLoop");
    }

    if(! (pactx=pa_context_new(pamloop_api,"qaudioeqpro-qt5-gui")) ) {
        clean(pactx,pamloop);
        throw qpa::PulseAudioException("Cannot create new PA context");
    }

    strncpy(g_name,name.toStdString().c_str(),strlen(name.toStdString().c_str())+1);
    strncpy(g_args,args.toStdString().c_str(),strlen(args.toStdString().c_str())+1);

    pa_context_set_state_callback(pactx,enable_mod,pamloop_api);
    if(pa_context_connect(pactx,NULL,PA_CONTEXT_NOFLAGS,NULL) < 0) {
        clean(pactx,pamloop);
        throw qpa::PulseAudioException("Cannot connect to PA server");
    }

    int loopval=-1;
    if(pa_mainloop_run(pamloop,&loopval) < 0) {
        clean(pactx,pamloop);
        throw qpa::PulseAudioException("Cannot run mainloop");
    }

    if(loopval < 0) {
        clean(pactx,pamloop);
        throw qpa::PulseAudioException("MainLoop exited with failure code");
    }

    clean(pactx,pamloop);
    return true;
}

qpa::PulseAudioException::PulseAudioException(const char* strerr): str_err(strerr)
{}

const char* qpa::PulseAudioException::what() const throw()
{
    return str_err;
}
