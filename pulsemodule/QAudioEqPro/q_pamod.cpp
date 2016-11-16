#include "q_pamod.h"

#include<pulse/introspect.h>
#include<pulse/context.h>
#include<pulse/mainloop.h>
#include<pulse/mainloop-api.h>

struct mod_info {
    const char *name,*args;
};

void enable_mod(pa_context *ctx,void *userdata)
{

}

void qpa::loadModule(const QString& name,const QString& args)
{
    pa_context* pactx = NULL;
    pa_mainloop* pamloop = NULL;
    pa_mainloop_api* pamloop_api = NULL;

    if(! (pamloop=pa_mainloop_new()) )
        throw qpa::PulseAudioException("Cannot create new mainloop");

    if(! (pamloop_api=pa_mainloop_get_api(pamloop)) )
        throw qpa::PulseAudioException("Cannot get MainLoop API from MainLoop");

    if(! (pactx=pa_context_new(pamloop_api)) )
        throw qpa::PulseAudioException("Cannot create new PA context");

    mod_info modata;
    pa_context_set_state_callback(pactx,enable_mod,&modata);
    if(pa_context_connect(pactx,NULL,PA_CONTEXT_NOFLAGS,NULL) < 0)
        throw qpa::PulseAudioException("Cannot connect to PA server");

    if(pa_mainloop_run(mloop,NULL) < 0)
        throw qpa::PulseAudioException("Cannot run mainloop");

    pa_context_disconnect(pactx);

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