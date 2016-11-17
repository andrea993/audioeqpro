#define __INCLUDED_FROM_PULSE_AUDIO
#include "config.h"
#include <pulsecore/module.h>
#include <pulsecore/log.h>
#include <pulsecore/modargs.h>
#include <pulsecore/namereg.h>
#include <pulsecore/core-util.h>
#include <pulsecore/sink.h>
#include <pulsecore/source.h>
#include <pulsecore/memblock.h>
#include <pulse/sample.h>
#include <pulse/gccmacro.h>
#include <pulse/def.h>

#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#define M 2
#define M2 4

typedef struct __equalizerPar
{
	int N;
	int nch;
	double R;
	double DB;
	double f_min;
	double **c;
	double *par;
	double ***X;
}equalizerPar;

struct userdata
{
	FILE *fp;
	pa_module *module;
	pa_core *core;
	pa_source *source;
	pa_sink* sink;
	pa_sink_input* sink_input;
	equalizerPar eqp;
};

static const char* const _valid_modargs[]=
{
	"gaindb",
	"f0",
	"octave",
	"master",
	"format",
	"rate",
	"channels",
	"channel_map",
	"use_volume_sharing",
	"force_flat_volume",
	NULL
};

void pa__done(pa_module *m);
double eq_filt(double u, double par[], double **c, double **x, int N);
void eq_init(equalizerPar *eqp, double db, double f_min,int nChans, int SR, double oct);
void eq_preproccesing(equalizerPar *eqp,double SR);


PA_MODULE_AUTHOR("Andrea Drius");
PA_MODULE_DESCRIPTION("equalizer");
PA_MODULE_VERSION("v0.00.001-alpha");

/* Called from I/O thread context */
static int sink_process_msg_cb(pa_msgobject *o, int code, void *data, int64_t offset, pa_memchunk *chunk)
{
#ifdef EQPRO_DEBUG
	pa_log("Callback: sink_process_msg_cb");
#endif
	return 0;
	return pa_sink_process_msg(o, code, data, offset, chunk);
}
static int sink_set_state_cb(pa_sink *s, pa_sink_state_t state)
{
	struct userdata *ud;
#ifdef EQPRO_DEBUG
	pa_log("Callback: sink_set_state_cb");
	pa_log(pa_sprintf_malloc("%d",state));
#endif
	pa_sink_assert_ref(s);
	pa_assert_se(ud=(struct userdata*)s->userdata);

	return 0;
}
static void sink_update_requested_latency_cb(pa_sink *s)
{
	struct userdata *ud;

#ifdef EQPRO_DEBUG
	pa_log("Callback: sink_update_requested_latency_cb");
#endif
	pa_sink_assert_ref(s);
	pa_assert_se(ud=(struct userdata*)s->userdata);

	if(!PA_SINK_IS_LINKED(ud->sink->thread_info.state) || !PA_SINK_INPUT_IS_LINKED(ud->sink_input->thread_info.state))
		return;

    /* Just hand this one over to the master sink */
	pa_sink_input_set_requested_latency_within_thread(ud->sink_input,pa_sink_get_requested_latency_within_thread(s));
}

static void sink_request_rewind_cb(pa_sink *s)
{
	struct userdata *ud;

#ifdef EQPRO_DEBUG
	pa_log("Callback: sink_request_rewind_cb");
#endif
	pa_sink_assert_ref(s);
	pa_assert_se(ud=(struct userdata*)s->userdata);

	
	if (!PA_SINK_IS_LINKED(ud->sink->thread_info.state) ||	!PA_SINK_INPUT_IS_LINKED(ud->sink_input->thread_info.state))
	return;

    /* Just hand this one over to the master sink */
#ifdef EQPRO_DEBUG
	pa_log("Do something?");
#endif
}

static int sink_input_pop_cb(pa_sink_input* in_snk, size_t sz, pa_memchunk* chunk)
{
	struct userdata *ud;
	pa_memchunk tchunk;
	size_t fs,nsamp;
	int c;
	float *src,*dst;

#ifdef EQPRO_DEBUG
	pa_log("Callback: sink_input_pop_cb");
#endif
	pa_assert(in_snk);
	pa_assert(ud=(struct userdata*)in_snk->userdata);

	

	/* Hmm, process any rewind request that might be queued up */
	pa_sink_process_rewind(ud->sink, 0);

/*	while(pa_memblockq_peek(ud->sink_input, &tchunk)<0)
	{
		
	}*/
	chunk->index=0;
	chunk->length=sz;
	chunk->memblock=pa_memblock_new(in_snk->sink->core->mempool,chunk->length);

	pa_assert(tchunk.length > 0);
	fs=pa_frame_size(&ud->sink->sample_spec);
	nsamp=sz/fs;

	/*read all buffer*/
	pa_sink_render(ud->sink,sz,&tchunk);

	src=(float*)((uint8_t*)pa_memblock_acquire(tchunk.memblock));
	dst=(float*)pa_memblock_acquire(chunk->memblock);

	while(nsamp>0)
	{
		for(c=0;c<ud->eqp.nch;c++)
		{
			*dst=(float)eq_filt(*src,ud->eqp.par,ud->eqp.c,ud->eqp.X[c],ud->eqp.N);
			src++;
			dst++;
		}
		nsamp--;
	}

	pa_memblock_release(tchunk.memblock);
	pa_memblock_release(chunk->memblock);
	pa_memblock_unref(tchunk.memblock);

	return 0;
}

static void sink_input_process_rewind_cb(pa_sink_input* in_snk,size_t sz) 
{
	struct userdata* ud;
#ifdef EQPRO_DEBUG
	pa_log("Callback: sink_input_process_rewind_cb");
#endif
	pa_assert(in_snk);
	pa_assert(ud=(struct userdata*)in_snk->userdata);
}

static void sink_input_update_max_rewind_cb(pa_sink_input* in_snk, size_t sz)
{
	struct userdata* ud;
#ifdef EQPRO_DEBUG
	pa_log("Callback: sink_input_update_max_rewind_cb");
#endif
	pa_assert(in_snk);
	pa_assert(ud=(struct userdata*)in_snk->userdata);
}

static void sink_input_update_max_request_cb(pa_sink_input* in_snk, size_t sz)
{
	struct userdata* ud;
#ifdef EQPRO_DEBUG
	pa_log("Callback: sink_input_update_max_request_cb");
#endif
	pa_assert(in_snk);
	pa_assert(ud=(struct userdata*)in_snk->userdata);
}

static void sink_input_update_sink_latency_range_cb(pa_sink_input* in_snk)
{
	struct userdata* ud;
#ifdef EQPRO_DEBUG
	pa_log("Callback: sink_input_update_sink_latency_range_cb");
#endif
	pa_assert(in_snk);
	pa_assert(ud=(struct userdata*)in_snk->userdata);
}

static void sink_input_update_sink_fixed_latency_cb(pa_sink_input* in_snk)
{
	struct userdata* ud;
#ifdef EQPRO_DEBUG
	pa_log("Callback: sink_input_update_sink_fixed_latency_cb");
#endif
	pa_assert(in_snk);
	pa_assert(ud=(struct userdata*)in_snk->userdata);
}

static void sink_input_kill_cb(pa_sink_input* in_snk) 
{
	struct userdata* ud;
#ifdef EQPRO_DEBUG
	pa_log("Callback: sink_input_kill_cb");
#endif
	pa_assert(in_snk);
	pa_assert(ud=(struct userdata*)in_snk->userdata);
}

static void sink_input_attach_cb(pa_sink_input* in_snk)
{
	struct userdata* ud;
#ifdef EQPRO_DEBUG
	pa_log("Callback: sink_input_attach_cb");
#endif
	pa_assert(in_snk);
	pa_assert(ud=(struct userdata*)in_snk->userdata);
}

static void sink_input_detach_cb(pa_sink_input* in_snk)
{
	struct userdata* ud;
#ifdef EQPRO_DEBUG
	pa_log("Callback: sink_input_detach_cb");
#endif
	pa_assert(in_snk);
	pa_assert(ud=(struct userdata*)in_snk->userdata);
}

static void sink_input_state_change_cb(pa_sink_input* in_snk, pa_sink_input_state_t state)
{
	struct userdata* ud;
#ifdef EQPRO_DEBUG
	pa_log("Callback: sink_input_state_change_cb");
#endif
	pa_assert(in_snk);
	pa_assert(ud=(struct userdata*)in_snk->userdata);
}

static bool sink_input_may_move_to_cb(pa_sink_input* in_snk, pa_sink* s)
{
	struct userdata* ud;
#ifdef EQPRO_DEBUG
	pa_log("Callback: sink_input_may_move_to_cb");
#endif
	pa_assert(in_snk);
	pa_assert(ud=(struct userdata*)in_snk->userdata);
}

static void sink_input_moving_cb(pa_sink_input* in_snk, pa_sink* s_dest) 
{
	struct userdata* ud;
#ifdef EQPRO_DEBUG
	pa_log("Callback: sink_input_moving_cb");
#endif
	pa_assert(in_snk);
	pa_assert(ud=(struct userdata*)in_snk->userdata);
}

static void use_volume_sharing(pa_sink_input* in_snk)
{
	struct userdata* ud;
#ifdef EQPRO_DEBUG
	pa_log("Callback: use_volume_sharing");
#endif
	pa_assert(in_snk);
	pa_assert(ud=(struct userdata*)in_snk->userdata);
}

static void sink_input_volume_changed_cb(pa_sink_input* in_snk)
{
	struct userdata* ud;
#ifdef EQPRO_DEBUG
	pa_log("Callback: sink_input_volume_changed_cb");
#endif
	pa_assert(in_snk);
	pa_assert(ud=(struct userdata*)in_snk->userdata);
}

static void sink_input_mute_changed_cb(pa_sink_input* in_snk)
{
	struct userdata* ud;
#ifdef EQPRO_DEBUG
	pa_log("Callback: sink_input_mute_changed_cb");
#endif
	pa_assert(in_snk);
	pa_assert(ud=(struct userdata*)in_snk->userdata);
}


void eq_preproccesing(equalizerPar *eqp,double SR)
{
	double v,g,cw,wcross,wc_n,fc_n,f_max,bw_n,T,tbw,c_m,d;
	double a[3], b[3];	
	int n;

	T=1.0/SR;

	g=pow(10,eqp->DB/20.0);
	wcross=pow(g,1.0/2.0/M);
	v=pow(g,1.0/M)-1;
	f_max=eqp->f_min*pow(eqp->R,eqp->N-1);

	for(n=0;n<eqp->N;n++)
	{
		fc_n=round(exp(log(eqp->f_min)+log(f_max/(double)eqp->f_min)*(n-1)/(double)(eqp->N-1)));
		wc_n=2*M_PI*fc_n;
		bw_n=wc_n*(sqrt(eqp->R)-1.0/sqrt(eqp->R))/wcross;

		cw=cos(wc_n*T);
		tbw=T*bw_n;
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

double eq_filt(double u, double par[], double **c, double **x, int N)
{
	double gg,gg1,gg2,den,u0,xn2cn1;
	double xn[M2];
	int n,i;
	double y=u;

	for(n=0;n<N;n++)
	{
		gg=2*par[n];
		gg1=par[n]+1;
		gg2=par[n]-1;

		den=gg1-gg2*c[n][9];
		u0=(gg2*x[n][0]+2*y)/den;
		y=(gg*x[n][0]+(gg1*c[n][9]-gg2)*y)/den;

		xn2cn1=x[n][2]*c[n][1];

		xn[0]=x[n][0]*c[n][0]-x[n][1]+xn2cn1+c[n][2]*u0;
		xn[1]=x[n][0]*c[n][4]+x[n][2]+c[n][3]*u0;
		xn[2]=x[n][0]*c[n][6]+xn2cn1-x[n][3]+u0*c[n][5];
		xn[3]=x[n][0]*c[n][8]+u0*c[n][7];

		for (i=0;i<M2;i++)
			x[n][i]=xn[i];
	}
	return y;

}

void eq_init(equalizerPar *eqp, double db, double f_min,int nChans, int SR, double oct)
{
	int n,i;
	double FN=SR/2.0;
	eqp->f_min=f_min;
	eqp->DB=db;
	eqp->nch=nChans;
	eqp->R=pow(2,oct);
	eqp->N=floor(log(FN/f_min)/log(eqp->R));

	eqp->c=(double**)pa_xmalloc(eqp->N*sizeof(double*));
	for(n=0;n<eqp->N;n++)
		eqp->c[n]=(double*)pa_xmalloc(10*sizeof(double));

	eqp->X=(double***)pa_xmalloc(nChans*sizeof(double**));
	for(i=0;i<nChans;i++)
	{
		eqp->X[i]=(double**)pa_xmalloc(eqp->N*sizeof(double*));
		for(n=0;n<eqp->N;n++)
			eqp->X[i][n]=(double*)pa_xmalloc0(M2*sizeof(double));
	}

	eqp->par=(double*)pa_xmalloc0(eqp->N);
	for(i=0;i<eqp->N;i++) //test
		eqp->par[i]=-1;
		

		
}

int pa__init(pa_module *m) 
{
	struct userdata *ud;
	pa_modargs *ma;
	pa_sink *master=NULL;
	pa_sink_input_new_data sink_input_data;
	pa_source_new_data source_data;
	pa_sample_spec ss;
	pa_sink_new_data sink_data;
	double gaindb,f0;
	unsigned sr;
	char out[100];
	bool use_volume_sharing = true;
	bool force_flat_volume = false;
	pa_memchunk silence;

#ifdef EQPRO_DEBUG
	pa_log("module-eqpro: its started");
#endif
	
	if(!(ma=pa_modargs_new(m->argument,_valid_modargs)))
	{
		pa_log("Failed to parse module arguments.");
		goto fail;
	}

	if(!(master=(pa_sink*)pa_namereg_get(m->core,pa_modargs_get_value(ma,"master",NULL),PA_NAMEREG_SINK)))
	{
		pa_log("Master sink not found");
		goto fail;
	}
	pa_assert(master);

	ss=master->sample_spec;
	ss.format=PA_SAMPLE_FLOAT32;

	ud=pa_xnew0(struct userdata,1);
	ud->module=m;
	ud->core=m->core;
	ud->module=m;
	m->userdata=ud;


	if(pa_modargs_get_value_double(ma,"f0",&f0)<0)
	{
		pa_log("f0= expexts a boolean argument");
		goto fail;
	}

#ifdef EQPRO_DEBUG
	sprintf(out,"%d",master->sample_spec.rate);
	pa_log(out);
	sprintf(out,"%d",master->sample_spec.channels);
	pa_log(out);
	sprintf(out,"%d",master->sample_spec.format);
	pa_log(out);
#endif

#ifdef EQPRO_DEBUG
	pa_log("Create sink init");
#endif	/* Create sync init*/
	pa_sink_new_data_init(&sink_data);
	sink_data.driver=__FILE__;
	sink_data.module=m;
	sink_data.name=pa_sprintf_malloc("%s.eqpro",master->name);
	pa_sink_new_data_set_sample_spec(&sink_data,&ss);
	pa_sink_new_data_set_channel_map(&sink_data,&master->channel_map);
	pa_proplist_sets(sink_data.proplist, PA_PROP_DEVICE_MASTER_DEVICE, master->name);
	pa_proplist_sets(sink_data.proplist, PA_PROP_DEVICE_CLASS, "filter");
	ud->sink=pa_sink_new(m->core,&sink_data,(master->flags & (PA_SINK_LATENCY|PA_SINK_DYNAMIC_LATENCY)) | (use_volume_sharing ? PA_SINK_SHARE_VOLUME_WITH_MASTER : 0));
	pa_sink_new_data_done(&sink_data);

	if(!ud->sink)
	{
		pa_log("Failed to create sink");
		goto fail;
	}

	/* callbacks */
	ud->sink->parent.process_msg=sink_process_msg_cb;
	//ud->sink->set_state = sink_set_state_cb;
	ud->sink->update_requested_latency = sink_update_requested_latency_cb;
	ud->sink->request_rewind = sink_request_rewind_cb;
	/*TO DO ADD OTHERS CALLBACK AND MANAGE THE CALLBACK*/

	pa_sink_set_asyncmsgq(ud->sink, master->asyncmsgq);
#ifdef EQPRO_DEBUG
	pa_log("Create sink done");
#endif
/* Create sync done*/


	/*Create sync input init*/
	pa_sink_input_new_data_init(&sink_input_data);


	sink_input_data.driver = __FILE__;
	sink_input_data.module = m;
	pa_sink_input_new_data_set_sink(&sink_input_data, master,false);
	sink_input_data.origin_sink = ud->sink;
	pa_proplist_setf(sink_input_data.proplist, PA_PROP_MEDIA_NAME, "eqpro Sink Stream from %s", pa_proplist_gets(ud->sink->proplist, PA_PROP_DEVICE_DESCRIPTION));
	pa_proplist_sets(sink_input_data.proplist, PA_PROP_MEDIA_ROLE, "filter");
	pa_sink_input_new_data_set_sample_spec(&sink_input_data, &ss);
	pa_sink_input_new_data_set_channel_map(&sink_input_data, &master->channel_map);
	pa_sink_input_new(&ud->sink_input, m->core, &sink_input_data);
	pa_sink_input_new_data_done(&sink_input_data);

	if (!ud->sink_input)
		goto fail;

	/*callback*/
	ud->sink_input->pop=sink_input_pop_cb;
	ud->sink_input->process_rewind = sink_input_process_rewind_cb;
	ud->sink_input->update_max_rewind = sink_input_update_max_rewind_cb;
	ud->sink_input->update_max_request = sink_input_update_max_request_cb;
	ud->sink_input->update_sink_latency_range = sink_input_update_sink_latency_range_cb;
	ud->sink_input->update_sink_fixed_latency = sink_input_update_sink_fixed_latency_cb;
	ud->sink_input->kill = sink_input_kill_cb;
	ud->sink_input->attach = sink_input_attach_cb;
	ud->sink_input->detach = sink_input_detach_cb;
	ud->sink_input->state_change = sink_input_state_change_cb;
	ud->sink_input->may_move_to = sink_input_may_move_to_cb;
	ud->sink_input->moving = sink_input_moving_cb;
	ud->sink_input->volume_changed = use_volume_sharing ? NULL : sink_input_volume_changed_cb;
	ud->sink_input->mute_changed = sink_input_mute_changed_cb;
	ud->sink_input->userdata = ud;

	ud->sink->input_to_master = ud->sink_input;

	pa_sink_input_get_silence(ud->sink_input,&silence);


	/*Create sync input done*/
#ifdef EQPRO_DEBUG
	pa_log("Create sink input done");
#endif

	pa_sink_put(ud->sink);
	pa_sink_input_put(ud->sink_input);

	pa_modargs_free(ma);

	//init equalizer
	eq_init(&ud->eqp,12,30,2,44100,1);
	eq_preproccesing(&ud->eqp,44100);

	return 0;

fail:
	if(ma)
		pa_modargs_free(ma);

	pa__done(m);
	return -1;

}

void pa__done(pa_module *m) //TO DO: Free all resources
{
	struct userdata *ud;
#ifdef EQPRO_DEBUG
	pa_log("module-eqpro: its done");
#endif

	if(!(ud=(struct userdata*)m->userdata)) {
#ifdef EQPRO_DEBUG
		pa_log("(dbg)(pa__done) userdata is NULL, byebye ");
#endif
		return;
	}

	/* Releasing sinks and sink_inputs */
	if(ud->sink) 
		pa_sink_unlink(ud->sink);

	if(ud->sink_input)
		pa_sink_input_unlink(ud->sink_input);

	if(ud->sink)
		pa_sink_unref(ud->sink);

	if(ud->sink_input)
		pa_sink_input_unref(ud->sink_input);

	if(ud->eqp.par)
		pa_xfree(ud->eqp.par);

	/* Freeing resources */
	int n,i;
	for(n=0;n<ud->eqp.N;n++) {
		pa_xfree(ud->eqp.c[n]);
	}

	pa_xfree(ud->eqp.c);

	for(n=0;n<ud->eqp.nch;n++) {
		for(i=0;i<ud->eqp.N;i++) {
			pa_xfree(ud->eqp.X[n][i]);
		}
		
		pa_xfree(ud->eqp.X[n]);
	}
	
	pa_xfree(ud->eqp.X);

	pa_xfree(ud);
}
