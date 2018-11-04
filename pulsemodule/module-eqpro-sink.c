/***
  This file is part of PulseAudio.

  Copyright 2017 Andrea Drius <andrea993 dot nokiastore at gmail dot com>

  PulseAudio is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published
  by the Free Software Foundation; either version 2.1 of the License,
  or (at your option) any later version.

  PulseAudio is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with PulseAudio; if not, see <http://www.gnu.org/licenses/>.
 ***/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <pulse/gccmacro.h>
#include <pulse/xmalloc.h>
#include <pulse/message-params.h>

#include <pulsecore/i18n.h>
#include <pulsecore/namereg.h>
#include <pulsecore/sink.h>
#include <pulsecore/module.h>
#include <pulsecore/core-util.h>
#include <pulsecore/modargs.h>
#include <pulsecore/log.h>
#include <pulsecore/rtpoll.h>
#include <pulsecore/sample-util.h>
#include <pulsecore/ltdl-helper.h>
#include <pulsecore/message-handler.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <float.h>


PA_MODULE_AUTHOR("Andrea Drius");
PA_MODULE_DESCRIPTION("Professional customizable equalizer");
PA_MODULE_VERSION("v0.20.0");
PA_MODULE_LOAD_ONCE(false);
PA_MODULE_USAGE(
        _("sink_name=<name for the sink> "
          "sink_properties=<properties for the sink> "
          "master=<name of sink to filter> "
          "rate=<sample rate> "
          "channels=<number of channels> "
          "channel_map=<channel map> "
          "use_volume_sharing=<yes or no> "
          "force_flat_volume=<yes or no> "
          "db=<filter gain in decibel>"
          "fmin=<central frequecy of the first band> "
          "octave=<octaves between bands> "
          "Nbands=<number of bands>"
          "par=<equalizer levels in the form (x1;x2;...;xn) from -1 to 1>"
          "K=<amplitude, to prevent saturation>"
          ));

#define MEMBLOCKQ_MAXLENGTH (16*1024*1024)
#define M 2
#define M2 4
#define DEFAULT_FMIN 30.0
#define DEFAULT_OCT 1.0
#define DEFAULT_DB 12.0
#define DEFAULT_K 1.0

#define MODULE_MSG_PATH "/modules/eqpro"


typedef struct __equalizerPar
{
    int N;
    double R;
    double DB;
    double K;
    double f_min;
    double **c;
    double *par;
    double ***X;
}equalizerPar;

struct userdata {
    pa_module *module;

    /* FIXME: Uncomment this and take "autoloaded" as a modarg if this is a filter */
    /* bool autoloaded; */

    pa_sink *sink;
    pa_sink_input *sink_input;

    pa_memblockq *memblockq;

    bool auto_desc;
    unsigned channels;
    equalizerPar eqp;
};

enum {
    SINK_MESSAGE_UPDATE_PARAMETERS = PA_SINK_MESSAGE_MAX
};

typedef enum __message_parameter {
   sliderchange,
	dialchange
}message_parameter;

typedef struct __asyncmsgq_data {
    message_parameter par;
	 double newval;
	 int slideridx;
}asyncmsgq_data;

static const char* const valid_modargs[] = {
    "db",
    "fmin",
    "Nbands",
    "octave",
    "par",
    "sink_name",
    "sink_properties",
    "master",
    "rate",
    "channels",
    "channel_map",
    "use_volume_sharing",
    "force_flat_volume",
    NULL
};

static double eq_filter(double u, double par[], double **c, double **x, double K, int N)
{
    double pn2,pn21,pn22,den,u0,xn2cn1;
    double xn[M2];
    int n,i;
    double y=u*K;

    for(n=0; n<N; n++)
    {
        pn2=2*par[n];
        pn21=par[n]+1;
        pn22=par[n]-1;

        den=pn21-pn22*c[n][9];
        u0=(pn22*x[n][0]+2*y)/den;
        y=(pn2*x[n][0]+(pn21*c[n][9]-pn22)*y)/den;

        xn2cn1=x[n][2]*c[n][1];

        xn[0]=x[n][0]*c[n][0]-x[n][1]+xn2cn1+c[n][2]*u0;
        xn[1]=x[n][0]*c[n][4]+x[n][2]+c[n][3]*u0;
        xn[2]=x[n][0]*c[n][6]+xn2cn1-x[n][3]+u0*c[n][5];
        xn[3]=x[n][0]*c[n][8]+u0*c[n][7];

        for (i=0; i<M2; i++)
            x[n][i]=xn[i];
    }
    return y;

}

static void eq_init(equalizerPar *eqp, double db, double f_min, int nChans, double oct, int N, double *par, double K)
{
    int n,i;
    eqp->f_min=f_min;
    eqp->DB=db;
    eqp->R=pow(2,oct);
    eqp->N=N;
    eqp->par=par;
    eqp->K=K;

    eqp->c=(double**)pa_xmalloc(eqp->N*sizeof(double*));
    for(n=0; n<eqp->N; n++)
        eqp->c[n]=(double*)pa_xmalloc(10*sizeof(double));

    eqp->X=(double***)pa_xmalloc(nChans*sizeof(double**));
    for(i=0; i<nChans; i++)
    {
        eqp->X[i]=(double**)pa_xmalloc(eqp->N*sizeof(double*));
        for(n=0; n<eqp->N; n++)
            eqp->X[i][n]=(double*)pa_xmalloc0(M2*sizeof(double));
    }

}

static void eq_preprocessing(equalizerPar *eqp, double SR)
{
    double v,g,cw,wcross,wc_n,fc_n,f_max,bw_n,T,tbw,c_m,d,Tpw;
    double a[3], b[3];
    int n;

    T=1.0/SR;

    g=pow(10,eqp->DB/20.0);
    wcross=pow(g,1.0/2.0/M);
    v=pow(g,1.0/M)-1;
    f_max=eqp->f_min*pow(eqp->R,eqp->N-1);

    for(n=0; n<eqp->N; n++)
    {
        fc_n=round(exp(log(eqp->f_min)+log(f_max/(double)eqp->f_min)*(n)/(double)(eqp->N-1)));
        wc_n=2*M_PI*fc_n;
        bw_n=wc_n*(sqrt(eqp->R)-1.0/sqrt(eqp->R))/wcross;

        
		  Tpw=2.0/bw_n*tan(bw_n/2.0*T);
		  cw=cos(Tpw/2.0*sqrt(4*wc_n*wc_n+1))/cos(Tpw/2.0);
		  tbw=Tpw*bw_n;
        c_m=cos(M_PI*(0.5-0.5/M));

        a[0]=4+4*c_m*tbw+tbw*tbw;
        a[1]=a[2]=1.0/a[0];
        a[1]*=2*tbw*tbw-8;
        a[2]*=a[0]-8*c_m*tbw;

        b[0]=b[1]=b[2]=tbw*v/a[0];
        b[0]*=2*tbw+4*c_m+tbw*v;
        b[1]*=2*tbw*(v+2);
        b[2]*=2*tbw-4*c_m+tbw*v;

        d=b[0]+1;
        eqp->c[n][0]=cw*(1-a[1]);
        eqp->c[n][1]=cw;
        eqp->c[n][2]=cw*(b[1]-a[1]*b[0]);
        eqp->c[n][3]=b[1]-a[1]*b[0];
        eqp->c[n][4]=-a[1];
        eqp->c[n][5]=cw*(b[2]-a[2]*b[0]);
        eqp->c[n][6]=-a[2]*cw;
        eqp->c[n][7]=b[2]-a[2]*b[0];
        eqp->c[n][8]=-a[2];
        eqp->c[n][9]=d;
    }
}

static void calcArgs(bool isfmin, bool isNbands, double *fmin, unsigned *Nbands, double *octave, double FN)
{
    double R;

    if (isfmin && isNbands){
        R=pow(FN / *fmin, 1.0 / *Nbands);
        *octave=log(R)/log(2);
        return;
    }
    if (isNbands){
        R=pow(2,*octave);
        *fmin=FN/pow(R,*Nbands);
        return;
    }
    R=pow(2,*octave);
    *Nbands=floor(log(FN/ *fmin)/log(R));

}

static int readParFromStr(char *str, unsigned N, double *out)
{
    unsigned i;
    char *s=str, *end=NULL;

    if (strlen(str) < 2)
        return -1;

    if (*s!='(')
        return -1;

    s++;
    i=0;
    while (*s!='\0' && i<N){
        out[i]=strtod(s, &end);
        if(!end || s==end)
            return -1;

        if (fabs(out[i])>1)
            return -1;

        s=end;
        if(*s=='\0' || (*s!=';' && *(s+1)!='\0') || (*s!=')' && *(s+1)=='\0'))
            return -1;

        s++;
        i++;
    }

    for (; i<N; i++)
        out[i]=0;

    return 0;

}

/* Called from Main thread context */
static int eqpro_message_handler(const char *object_path, const char *message, char *message_parameters, char **response, void *ud) {

    struct userdata *u;

    void *state = NULL, *state2=NULL;
    char *startpos = NULL, *fullpath_str;
    double arg_d;
    int64_t arg_i;
    pa_message_param* param;
    int i;
	 asyncmsgq_data* msgqd;

    pa_assert(u = (struct userdata*)ud);
    pa_assert(message);
    pa_assert(response);

    fullpath_str = pa_sprintf_malloc("%s/%d",MODULE_MSG_PATH, u->module->index);
    pa_assert(pa_safe_streq(object_path, fullpath_str));
    pa_xfree(fullpath_str);


    if(pa_streq(message, "sliderchange")) {

        if(pa_message_param_split_list(message_parameters, &startpos, NULL, &state) <= 0)
            return -PA_ERR_NOTIMPLEMENTED;

        if(pa_message_param_read_double(startpos, &arg_d, &state2) <= 0)
            return -PA_ERR_NOTIMPLEMENTED;

        if(pa_message_param_read_int64(startpos, &arg_i, &state2) <= 0)
            return -PA_ERR_NOTIMPLEMENTED;

        if(arg_i < 0 || arg_i >= u->eqp.N) {
            *response = pa_xstrdup("Cursor doesn't exists");
            return -PA_ERR_NOTIMPLEMENTED;
        }

        if(fabs(arg_d) > 1.0) {
            *response = pa_xstrdup("Cursor value out of range");
            return -PA_ERR_NOTIMPLEMENTED;
        }

		  msgqd = pa_xnew0(asyncmsgq_data,1); 
		  msgqd->par = sliderchange;
		  msgqd->slideridx = arg_i;
		  msgqd->newval = arg_d;
        pa_asyncmsgq_send(u->sink->asyncmsgq, PA_MSGOBJECT(u->sink),SINK_MESSAGE_UPDATE_PARAMETERS, msgqd, 0, NULL);

        *response=pa_xstrdup("OK");

        return PA_OK;
    }

    if(pa_streq(message, "dialchange")) {

        if(pa_message_param_read_double(message_parameters, &arg_d, &state) <= 0)
            return -PA_ERR_NOTIMPLEMENTED;

        if(arg_d < 0) {
            *response = pa_xstrdup("K value must be positive");
            return -PA_ERR_NOTIMPLEMENTED;
        }

		  msgqd = pa_xnew0(asyncmsgq_data,1); 
		  msgqd->par = dialchange;
		  msgqd->newval = arg_d;
        pa_asyncmsgq_send(u->sink->asyncmsgq, PA_MSGOBJECT(u->sink),SINK_MESSAGE_UPDATE_PARAMETERS, msgqd, 0, NULL);

        *response=pa_xstrdup("OK");

        return PA_OK;

    }

    if(pa_streq(message, "getinfo")) {
        param = pa_message_param_new();
        pa_message_param_begin_list(param);
        pa_message_param_write_int64(param, u->eqp.N);
        pa_message_param_write_double(param, u->eqp.f_min,32);
        pa_message_param_write_double(param, u->eqp.DB,32);
        pa_message_param_write_double(param, u->eqp.R, 32);
        pa_message_param_write_double(param, u->eqp.K, 32);
        for (i=0; i<u->eqp.N; i++)
            pa_message_param_write_double(param, u->eqp.par[i],32);
        pa_message_param_end_list(param);

        *response=pa_message_param_to_string(param);

        return PA_OK;
    }



    return -PA_ERR_NOTIMPLEMENTED;
}

/* Called from I/O thread context */
static int sink_process_msg_cb(pa_msgobject *o, int code, void *data, int64_t offset, pa_memchunk *chunk) {
    struct userdata *u = PA_SINK(o)->userdata;

    switch (code) {

    case PA_SINK_MESSAGE_GET_LATENCY:

        /* The sink is _put() before the sink input is, so let's
                         * make sure we don't access it in that time. Also, the
                         * sink input is first shut down, the sink second. */
        if (!PA_SINK_IS_LINKED(u->sink->thread_info.state) ||
                !PA_SINK_INPUT_IS_LINKED(u->sink_input->thread_info.state) ||
                !u->sink_input->sink) {
            *((int64_t*) data) = 0;
            return 0;
        }

        *((int64_t*) data) =

                /* Get the latency of the master sink */
                pa_sink_get_latency_within_thread(u->sink_input->sink, true) +

                /* Add the latency internal to our sink input on top */
                pa_bytes_to_usec(pa_memblockq_get_length(u->sink_input->thread_info.render_memblockq), &u->sink_input->sink->sample_spec);

        return 0;

	 case SINK_MESSAGE_UPDATE_PARAMETERS: {
		 asyncmsgq_data *d = data;
		 if (d->par ==  sliderchange) {
			u->eqp.par[d->slideridx]=d->newval;
		 }
		 else if (d->par == dialchange) {
			u->eqp.K = d->newval;
		 }
		 else {
			 pa_log("Unsupported parameter in asyncmsgq");
		 }
		 pa_xfree(d);
		 return 0;
	 }

    case PA_SINK_MESSAGE_SET_STATE: {
        pa_sink_state_t new_state = (pa_sink_state_t) PA_PTR_TO_UINT(data);

        /* When set to running or idle for the first time, request a rewind
                                                                                                                * of the master sink to make sure we are heard immediately */
        if ((new_state == PA_SINK_IDLE || new_state == PA_SINK_RUNNING) && u->sink->thread_info.state == PA_SINK_INIT) {
            pa_log_debug("Requesting rewind due to state change.");
            pa_sink_input_request_rewind(u->sink_input, 0, false, true, true);
        }
    }

    }
    return pa_sink_process_msg(o, code, data, offset, chunk);
}

/* Called from main context */
static int sink_set_state_in_main_thread_cb(pa_sink *s, pa_sink_state_t state, pa_suspend_cause_t suspend_cause) {
    struct userdata *u;

    pa_sink_assert_ref(s);
    pa_assert_se(u = s->userdata);

    if (!PA_SINK_IS_LINKED(state) ||
        !PA_SINK_INPUT_IS_LINKED(u->sink_input->state))
        return 0;

    pa_sink_input_cork(u->sink_input, state == PA_SINK_SUSPENDED);
    return 0;
}

/* Called from the IO thread. */
static int sink_set_state_in_io_thread_cb(pa_sink *s, pa_sink_state_t new_state, pa_suspend_cause_t new_suspend_cause) {
    struct userdata *u;

    pa_assert(s);
    pa_assert_se(u = s->userdata);

    /* When set to running or idle for the first time, request a rewind
     * of the master sink to make sure we are heard immediately */
    if ((new_state == PA_SINK_IDLE || new_state == PA_SINK_RUNNING) && u->sink->thread_info.state == PA_SINK_INIT) {
        pa_log_debug("Requesting rewind due to state change.");
        pa_sink_input_request_rewind(u->sink_input, 0, false, true, true);
    }

    return 0;
}

/* Called from I/O thread context */
static void sink_request_rewind_cb(pa_sink *s) {
    struct userdata *u;

    pa_sink_assert_ref(s);
    pa_assert_se(u = s->userdata);

    if (!PA_SINK_IS_LINKED(u->sink->thread_info.state) ||
            !PA_SINK_INPUT_IS_LINKED(u->sink_input->thread_info.state))
        return;

    /* Just hand this one over to the master sink */
    pa_sink_input_request_rewind(u->sink_input,
                                 s->thread_info.rewind_nbytes +
                                 pa_memblockq_get_length(u->memblockq), true, false, false);
}

/* Called from I/O thread context */
static void sink_update_requested_latency_cb(pa_sink *s) {
    struct userdata *u;

    pa_sink_assert_ref(s);
    pa_assert_se(u = s->userdata);

    if (!PA_SINK_IS_LINKED(u->sink->thread_info.state) ||
            !PA_SINK_INPUT_IS_LINKED(u->sink_input->thread_info.state))
        return;

    /* Just hand this one over to the master sink */
    pa_sink_input_set_requested_latency_within_thread(
                u->sink_input,
                pa_sink_get_requested_latency_within_thread(s));
}

/* Called from main context */
static void sink_set_volume_cb(pa_sink *s) {
    struct userdata *u;

    pa_sink_assert_ref(s);
    pa_assert_se(u = s->userdata);

    if (!PA_SINK_IS_LINKED(s->state) ||
        !PA_SINK_INPUT_IS_LINKED(u->sink_input->state))
        return;

    pa_sink_input_set_volume(u->sink_input, &s->real_volume, s->save_volume, true);
}

/* Called from main context */
static void sink_set_mute_cb(pa_sink *s) {
    struct userdata *u;

    pa_sink_assert_ref(s);
    pa_assert_se(u = s->userdata);


    if (!PA_SINK_IS_LINKED(s->state) ||
        !PA_SINK_INPUT_IS_LINKED(u->sink_input->state))
        return;

    pa_sink_input_set_mute(u->sink_input, s->muted, s->save_muted);
}

/* Called from I/O thread context */
static int sink_input_pop_cb(pa_sink_input *i, size_t nbytes, pa_memchunk *chunk) {
    struct userdata *u;
    float *src, *dst;
    size_t fs;
    unsigned n, c;
    pa_memchunk tchunk;
    pa_usec_t current_latency PA_GCC_UNUSED;

    pa_sink_input_assert_ref(i);
    pa_assert(chunk);
    pa_assert_se(u = i->userdata);

    if (!PA_SINK_IS_LINKED(u->sink->thread_info.state))
        return -1;
    /* Hmm, process any rewind request that might be queued up */
    pa_sink_process_rewind(u->sink, 0);

    /* (1) IF YOU NEED A FIXED BLOCK SIZE USE
         * pa_memblockq_peek_fixed_size() HERE INSTEAD. NOTE THAT FILTERS
         * WHICH CAN DEAL WITH DYNAMIC BLOCK SIZES ARE HIGHLY
         * PREFERRED. */
    while (pa_memblockq_peek(u->memblockq, &tchunk) < 0) {
        pa_memchunk nchunk;

        pa_sink_render(u->sink, nbytes, &nchunk);
        pa_memblockq_push(u->memblockq, &nchunk);
        pa_memblock_unref(nchunk.memblock);
    }

    /* (2) IF YOU NEED A FIXED BLOCK SIZE, THIS NEXT LINE IS NOT
         * NECESSARY */
    tchunk.length = PA_MIN(nbytes, tchunk.length);
    pa_assert(tchunk.length > 0);

    fs = pa_frame_size(&i->sample_spec);
    n = (unsigned) (tchunk.length / fs);

    pa_assert(n > 0);

    chunk->index = 0;
    chunk->length = n*fs;
    chunk->memblock = pa_memblock_new(i->sink->core->mempool, chunk->length);

    pa_memblockq_drop(u->memblockq, chunk->length);

    src = (float*)pa_memblock_acquire_chunk(&tchunk);
    dst = (float*)pa_memblock_acquire(chunk->memblock);

    /* (3) PUT YOUR CODE HERE TO DO SOMETHING WITH THE DATA */


    while(n>0)
    {
        for(c=0; c<u->channels; c++)
        {
            *dst=(float)eq_filter(*src,u->eqp.par,u->eqp.c,u->eqp.X[c],u->eqp.K,u->eqp.N);
            src++;
            dst++;
        }
        n--;
    }


    pa_memblock_release(tchunk.memblock);
    pa_memblock_release(chunk->memblock);

    pa_memblock_unref(tchunk.memblock);

    /* (4) IF YOU NEED THE LATENCY FOR SOMETHING ACQUIRE IT LIKE THIS: */
    current_latency =
            /* Get the latency of the master sink */
            pa_sink_get_latency_within_thread(i->sink, false) +

            /* Add the latency internal to our sink input on top */
            pa_bytes_to_usec(pa_memblockq_get_length(i->thread_info.render_memblockq), &i->sink->sample_spec);

    return 0;
}

/* Called from I/O thread context */
static void sink_input_process_rewind_cb(pa_sink_input *i, size_t nbytes) {
    struct userdata *u;
    size_t amount = 0;

    pa_sink_input_assert_ref(i);
    pa_assert_se(u = i->userdata);

    /* If the sink is not yet linked, there is nothing to rewind */
    if (!PA_SINK_IS_LINKED(u->sink->thread_info.state))
        return;

    if (u->sink->thread_info.rewind_nbytes > 0) {
        size_t max_rewrite;

        max_rewrite = nbytes + pa_memblockq_get_length(u->memblockq);
        amount = PA_MIN(u->sink->thread_info.rewind_nbytes, max_rewrite);
        u->sink->thread_info.rewind_nbytes = 0;

        if (amount > 0) {
            pa_memblockq_seek(u->memblockq, - (int64_t) amount, PA_SEEK_RELATIVE, true);

            /* (5) PUT YOUR CODE HERE TO RESET YOUR FILTER  */
        }
    }

    pa_sink_process_rewind(u->sink, amount);
    pa_memblockq_rewind(u->memblockq, nbytes);
}

/* Called from I/O thread context */
static void sink_input_update_max_rewind_cb(pa_sink_input *i, size_t nbytes) {
    struct userdata *u;

    pa_sink_input_assert_ref(i);
    pa_assert_se(u = i->userdata);

    /* FIXME: Too small max_rewind:
         * https://bugs.freedesktop.org/show_bug.cgi?id=53709 */
    pa_memblockq_set_maxrewind(u->memblockq, nbytes);
    pa_sink_set_max_rewind_within_thread(u->sink, nbytes);
}

/* Called from I/O thread context */
static void sink_input_update_max_request_cb(pa_sink_input *i, size_t nbytes) {
    struct userdata *u;

    pa_sink_input_assert_ref(i);
    pa_assert_se(u = i->userdata);

    /* (6) IF YOU NEED A FIXED BLOCK SIZE ROUND nbytes UP TO MULTIPLES
         * OF IT HERE. THE PA_ROUND_UP MACRO IS USEFUL FOR THAT. */

    pa_sink_set_max_request_within_thread(u->sink, nbytes);
}

/* Called from I/O thread context */
static void sink_input_update_sink_latency_range_cb(pa_sink_input *i) {
    struct userdata *u;

    pa_sink_input_assert_ref(i);
    pa_assert_se(u = i->userdata);

    pa_sink_set_latency_range_within_thread(u->sink, i->sink->thread_info.min_latency, i->sink->thread_info.max_latency);
}

/* Called from I/O thread context */
static void sink_input_update_sink_fixed_latency_cb(pa_sink_input *i) {
    struct userdata *u;

    pa_sink_input_assert_ref(i);
    pa_assert_se(u = i->userdata);

    /* (7) IF YOU NEED A FIXED BLOCK SIZE ADD THE LATENCY FOR ONE
         * BLOCK MINUS ONE SAMPLE HERE. pa_usec_to_bytes_round_up() IS
         * USEFUL FOR THAT. */

    pa_sink_set_fixed_latency_within_thread(u->sink, i->sink->thread_info.fixed_latency);
}

/* Called from I/O thread context */
static void sink_input_detach_cb(pa_sink_input *i) {
    struct userdata *u;

    pa_sink_input_assert_ref(i);
    pa_assert_se(u = i->userdata);

    if (PA_SINK_IS_LINKED(u->sink->thread_info.state))
        pa_sink_detach_within_thread(u->sink);

    pa_sink_set_rtpoll(u->sink, NULL);
}

/* Called from I/O thread context */
static void sink_input_attach_cb(pa_sink_input *i) {
    struct userdata *u;

    pa_sink_input_assert_ref(i);
    pa_assert_se(u = i->userdata);

    pa_sink_set_rtpoll(u->sink, i->sink->thread_info.rtpoll);
    pa_sink_set_latency_range_within_thread(u->sink, i->sink->thread_info.min_latency, i->sink->thread_info.max_latency);

    /* (8.1) IF YOU NEED A FIXED BLOCK SIZE ADD THE LATENCY FOR ONE
         * BLOCK MINUS ONE SAMPLE HERE. SEE (7) */
    pa_sink_set_fixed_latency_within_thread(u->sink, i->sink->thread_info.fixed_latency);

    /* (8.2) IF YOU NEED A FIXED BLOCK SIZE ROUND
         * pa_sink_input_get_max_request(i) UP TO MULTIPLES OF IT
         * HERE. SEE (6) */
    pa_sink_set_max_request_within_thread(u->sink, pa_sink_input_get_max_request(i));

    /* FIXME: Too small max_rewind:
         * https://bugs.freedesktop.org/show_bug.cgi?id=53709 */
    pa_sink_set_max_rewind_within_thread(u->sink, pa_sink_input_get_max_rewind(i));

    if (PA_SINK_IS_LINKED(u->sink->thread_info.state))
        pa_sink_attach_within_thread(u->sink);

}

/* Called from main context */
static void sink_input_kill_cb(pa_sink_input *i) {
    struct userdata *u;

    pa_sink_input_assert_ref(i);
    pa_assert_se(u = i->userdata);

    /* The order here matters! We first kill the sink so that streams
         * can properly be moved away while the sink input is still conneted
         * to the master. */
    pa_sink_input_cork(u->sink_input, true);
    pa_sink_unlink(u->sink);
    pa_sink_input_unlink(u->sink_input);

    pa_sink_input_unref(u->sink_input);
    u->sink_input = NULL;

    pa_sink_unref(u->sink);
    u->sink = NULL;

    pa_module_unload_request(u->module, true);
}

/* Called from main context */
static void sink_input_moving_cb(pa_sink_input *i, pa_sink *dest) {
    struct userdata *u;

    pa_sink_input_assert_ref(i);
    pa_assert_se(u = i->userdata);

    if (dest) {
        pa_sink_set_asyncmsgq(u->sink, dest->asyncmsgq);
        pa_sink_update_flags(u->sink, PA_SINK_LATENCY|PA_SINK_DYNAMIC_LATENCY, dest->flags);
    } else
        pa_sink_set_asyncmsgq(u->sink, NULL);

    if (u->auto_desc && dest) {
        const char *z;
        pa_proplist *pl;

        pl = pa_proplist_new();
        z = pa_proplist_gets(dest->proplist, PA_PROP_DEVICE_DESCRIPTION);
        pa_proplist_setf(pl, PA_PROP_DEVICE_DESCRIPTION, "Virtual Sink %s on %s",
                         pa_proplist_gets(u->sink->proplist, "device.vsink.name"), z ? z : dest->name);

        pa_sink_update_proplist(u->sink, PA_UPDATE_REPLACE, pl);
        pa_proplist_free(pl);
    }
}

/* Called from main context */
static void sink_input_volume_changed_cb(pa_sink_input *i) {
    struct userdata *u;

    pa_sink_input_assert_ref(i);
    pa_assert_se(u = i->userdata);

    pa_sink_volume_changed(u->sink, &i->volume);
}

/* Called from main context */
static void sink_input_mute_changed_cb(pa_sink_input *i) {
    struct userdata *u;

    pa_sink_input_assert_ref(i);
    pa_assert_se(u = i->userdata);

    pa_sink_mute_changed(u->sink, i->muted);
}

int pa__init(pa_module*m) {
    struct userdata *u;
    pa_sample_spec ss;
    pa_channel_map map;
    pa_modargs *ma;
    pa_sink *master=NULL;
    pa_sink_input_new_data sink_input_data;
    pa_sink_new_data sink_data;
    bool use_volume_sharing = true;
    bool force_flat_volume = false;
    pa_memchunk silence;
    double fmin, octave, db, K;
    unsigned Nbands = 0;
    double* par=NULL;
    char *str=NULL, *fullpath_str=NULL;
    bool isfmin=true, isNbands=true, isoctave=true;
    int ret;

    pa_assert(m);

    if (!(ma = pa_modargs_new(m->argument, valid_modargs))) {
        pa_log("Failed to parse module arguments.");
        goto fail;
    }

    if (!(master = pa_namereg_get(m->core, pa_modargs_get_value(ma, "master", NULL), PA_NAMEREG_SINK))) {
        pa_log("Master sink not found");
        goto fail;
    }

    pa_assert(master);

    ss = master->sample_spec;
    ss.format = PA_SAMPLE_FLOAT32;
    map = master->channel_map;
    if (pa_modargs_get_sample_spec_and_channel_map(ma, &ss, &map, PA_CHANNEL_MAP_DEFAULT) < 0) {
        pa_log("Invalid sample format specification or channel map");
        goto fail;
    }

    if (pa_modargs_get_value_boolean(ma, "use_volume_sharing", &use_volume_sharing) < 0) {
        pa_log("use_volume_sharing= expects a boolean argument");
        goto fail;
    }

    if (pa_modargs_get_value_boolean(ma, "force_flat_volume", &force_flat_volume) < 0) {
        pa_log("force_flat_volume= expects a boolean argument");
        goto fail;
    }

	 /* FIXME pa_modargs_get_value* can't detect whether the user gave an invalid argument to the module or whether the user left the argument blank, I'm working around this using strange values but it is exploitable */
	 
 _Pragma("GCC diagnostic push")
 _Pragma("GCC diagnostic ignored \"-Wfloat-equal\"")
	 db=DBL_EPSILON;
    ret=pa_modargs_get_value_double(ma, "db", &db);
    if (ret < 0) {
        pa_log("db= expects a double argument");
        goto fail;
    }
	 if (db == DBL_EPSILON)
		 db=DEFAULT_DB;
	 if (db <= 0) {
		 pa_log("db must be positive");
		 goto fail;
	 }

	 K=DBL_EPSILON;
    ret=pa_modargs_get_value_double(ma, "K", &K);
    if (ret < 0) {
        pa_log("K= expects a double argument");
        goto fail;
    }
	 if (K == DBL_EPSILON)
		 K = DEFAULT_K;
    if (K<0) {
        pa_log("K= expects a positive double");
        goto fail;
    }

	 fmin = DBL_EPSILON;
    ret=pa_modargs_get_value_double(ma, "fmin", &fmin);
    if (ret < 0) {
        pa_log("fmin= expects a double argument");
        goto fail;
    }
	 if (fmin == DBL_EPSILON)
	 {
		 fmin = DEFAULT_FMIN;
		 isfmin = false;
	 }
    if (fmin <= 0 || fmin>=ss.rate/2.0) {
        pa_log("fmin= expects a positive double less than (sampling rate)/2");
        goto fail;
    }

	 octave = DBL_EPSILON;
    ret=pa_modargs_get_value_double(ma, "octave", &octave);
    if (ret < 0) {
        pa_log("octave= expects a double argument");
        goto fail;
    }
	 if (octave == DBL_EPSILON) {
		 octave = DEFAULT_OCT;
		 isoctave = false;
	 }
    if (octave <= 0) {
        pa_log("octave= expects a positive double");
        goto fail;
    }

	 Nbands = UINT_MAX;
    ret=pa_modargs_get_value_u32(ma, "Nbands", &Nbands);
    if (ret < 0) {
        pa_log("Nbands= expects an unsigned argument");
        goto fail;
    }
	 if (Nbands == UINT_MAX)
		isNbands = false;

    if (isoctave && isNbands && isfmin) {
        pa_log("You can choose up to two arguments between: octave, Nbands, fmin");
        goto fail;
    }
_Pragma("GCC diagnostic pop") \

    calcArgs(isfmin, isNbands, &fmin, &Nbands, &octave, ss.rate/2.0);


    par=pa_xmalloc0(Nbands*sizeof(double));
    if ((str=pa_xstrdup(pa_modargs_get_value(ma, "par", NULL)))) {
        if (readParFromStr(str, Nbands, par) < 0) {
            pa_log("par= expects an array of double in the format (x1;x2,...;xn) from -1 to 1");
            goto fail;
        }
    }

    if (use_volume_sharing && force_flat_volume) {
        pa_log("Flat volume can't be forced when using volume sharing.");
        goto fail;
    }

    u = pa_xnew0(struct userdata, 1);
    u->module = m;
    m->userdata = u;
    u->channels = ss.channels;

    /* Create sink */
    pa_sink_new_data_init(&sink_data);
    sink_data.driver = __FILE__;
    sink_data.module = m;
    if (!(sink_data.name = pa_xstrdup(pa_modargs_get_value(ma, "sink_name", NULL))))
        sink_data.name = pa_sprintf_malloc("%s.vsink", master->name);
    pa_sink_new_data_set_sample_spec(&sink_data, &ss);
    pa_sink_new_data_set_channel_map(&sink_data, &map);
    pa_proplist_sets(sink_data.proplist, PA_PROP_DEVICE_MASTER_DEVICE, master->name);
    pa_proplist_sets(sink_data.proplist, PA_PROP_DEVICE_CLASS, "filter");
    pa_proplist_sets(sink_data.proplist, "device.vsink.eqpro", sink_data.name);

    if (pa_modargs_get_proplist(ma, "sink_properties", sink_data.proplist, PA_UPDATE_REPLACE) < 0) {
        pa_log("Invalid properties");
        pa_sink_new_data_done(&sink_data);
        goto fail;
    }

    if ((u->auto_desc = !pa_proplist_contains(sink_data.proplist, PA_PROP_DEVICE_DESCRIPTION))) {
        const char *z;

        z = pa_proplist_gets(master->proplist, PA_PROP_DEVICE_DESCRIPTION);
        pa_proplist_setf(sink_data.proplist, PA_PROP_DEVICE_DESCRIPTION, "EqualizerPro Sink %s on %s", sink_data.name, z ? z : master->name);
    }

    u->sink = pa_sink_new(m->core, &sink_data, (master->flags & (PA_SINK_LATENCY|PA_SINK_DYNAMIC_LATENCY))
                          | (use_volume_sharing ? PA_SINK_SHARE_VOLUME_WITH_MASTER : 0));
    pa_sink_new_data_done(&sink_data);

    if (!u->sink) {
        pa_log("Failed to create sink.");
        goto fail;
    }

    u->sink->parent.process_msg = sink_process_msg_cb;
    u->sink->set_state_in_main_thread = sink_set_state_in_main_thread_cb;
    u->sink->set_state_in_io_thread = sink_set_state_in_io_thread_cb;
    u->sink->update_requested_latency = sink_update_requested_latency_cb;
    u->sink->request_rewind = sink_request_rewind_cb;
    pa_sink_set_set_mute_callback(u->sink, sink_set_mute_cb);
    if (!use_volume_sharing) {
        pa_sink_set_set_volume_callback(u->sink, sink_set_volume_cb);
        pa_sink_enable_decibel_volume(u->sink, true);
    }
    /* Normally this flag would be enabled automatically be we can force it. */
    if (force_flat_volume)
        u->sink->flags |= PA_SINK_FLAT_VOLUME;
    u->sink->userdata = u;

    pa_sink_set_asyncmsgq(u->sink, master->asyncmsgq);

    /* Create sink input */
    pa_sink_input_new_data_init(&sink_input_data);
    sink_input_data.driver = __FILE__;
    sink_input_data.module = m;
    pa_sink_input_new_data_set_sink(&sink_input_data, master, false, true);
    sink_input_data.origin_sink = u->sink;
    pa_proplist_setf(sink_input_data.proplist, PA_PROP_MEDIA_NAME, "Virtual Sink Stream from %s", pa_proplist_gets(u->sink->proplist, PA_PROP_DEVICE_DESCRIPTION));
    pa_proplist_sets(sink_input_data.proplist, PA_PROP_MEDIA_ROLE, "filter");
    pa_sink_input_new_data_set_sample_spec(&sink_input_data, &ss);
    pa_sink_input_new_data_set_channel_map(&sink_input_data, &map);
    sink_input_data.flags |= PA_SINK_INPUT_START_CORKED;
    pa_sink_input_new(&u->sink_input, m->core, &sink_input_data);
    pa_sink_input_new_data_done(&sink_input_data);

    if (!u->sink_input)
        goto fail;

    u->sink_input->pop = sink_input_pop_cb;
    u->sink_input->process_rewind = sink_input_process_rewind_cb;
    u->sink_input->update_max_rewind = sink_input_update_max_rewind_cb;
    u->sink_input->update_max_request = sink_input_update_max_request_cb;
    u->sink_input->update_sink_latency_range = sink_input_update_sink_latency_range_cb;
    u->sink_input->update_sink_fixed_latency = sink_input_update_sink_fixed_latency_cb;
    u->sink_input->kill = sink_input_kill_cb;
    u->sink_input->attach = sink_input_attach_cb;
    u->sink_input->detach = sink_input_detach_cb;
    u->sink_input->moving = sink_input_moving_cb;
    u->sink_input->volume_changed = use_volume_sharing ? NULL : sink_input_volume_changed_cb;
    u->sink_input->mute_changed = sink_input_mute_changed_cb;
    u->sink_input->userdata = u;

    u->sink->input_to_master = u->sink_input;

    pa_sink_input_get_silence(u->sink_input, &silence);
    u->memblockq = pa_memblockq_new("module-virtual-sink memblockq", 0, MEMBLOCKQ_MAXLENGTH, 0, &ss, 1, 1, 0, &silence);
    pa_memblock_unref(silence.memblock);

    //init eq
    eq_init(&u->eqp,db,fmin,u->channels,octave,(int)Nbands,par, K);
    eq_preprocessing(&u->eqp,ss.rate);

    /* The order here is important. The input must be put first,
         * otherwise streams might attach to the sink before the sink
         * input is attached to the master. */

    pa_sink_input_put(u->sink_input);
    pa_sink_put(u->sink);
    pa_sink_input_cork(u->sink_input, false);

    pa_modargs_free(ma);
    if (str)
        pa_xfree(str);



    /* Messages handling */
    fullpath_str = pa_sprintf_malloc("%s/%d",MODULE_MSG_PATH, m->index);
    if (pa_hashmap_get(m->core->message_handlers, fullpath_str) != NULL) {
        pa_log("the communication path is in use");
        goto fail;
    }

    pa_message_handler_register(m->core, fullpath_str, "communication with eqpro_gui", eqpro_message_handler, (void*)u);
    pa_xfree(fullpath_str);

    return 0;

fail:
    if (ma)
        pa_modargs_free(ma);
    if (par)
        pa_xfree(par);
    if (str)
        pa_xfree(str);

    pa_xfree(fullpath_str);

    pa__done(m);

    return -1;
}

int pa__get_n_used(pa_module *m) {
    struct userdata *u;

    pa_assert(m);
    pa_assert_se(u = m->userdata);

    return pa_sink_linked_by(u->sink);
}

void pa__done(pa_module*m) {
    struct userdata *u;
    int n, i;
    char *fullpath_str;

    pa_assert(m);

    if (!(u = m->userdata))
        return;

    fullpath_str = pa_sprintf_malloc("%s/%d",MODULE_MSG_PATH, m->index);
    if (pa_hashmap_get(m->core->message_handlers, fullpath_str)) {
        pa_message_handler_unregister(m->core, fullpath_str);
    }
    pa_xfree(fullpath_str);

    /* See comments in sink_input_kill_cb() above regarding
         * destruction order! */

    if (u->sink_input)
        pa_sink_input_cork(u->sink_input, true);

    if (u->sink)
        pa_sink_unlink(u->sink);

    if (u->sink_input)
        pa_sink_input_unlink(u->sink_input);

    if (u->sink_input)
        pa_sink_input_unref(u->sink_input);

    if (u->sink)
        pa_sink_unref(u->sink);

    if (u->memblockq)
        pa_memblockq_free(u->memblockq);

    /*free equalizerPar resources*/
    if (u->eqp.par)
        pa_xfree(u->eqp.par);


    for(n=0; n<u->eqp.N; n++) {
        pa_xfree(u->eqp.c[n]);
    }

    pa_xfree(u->eqp.c);

    for(n=0; n<(int)u->channels; n++) {
        for(i=0; i<u->eqp.N; i++) {
            pa_xfree(u->eqp.X[n][i]);
        }

        pa_xfree(u->eqp.X[n]);
    }

    pa_xfree(u->eqp.X);

    /*free userdata*/
    pa_xfree(u);
}


