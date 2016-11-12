#include "config.h"
#include <pulsecore/module.h>
#include <pulsecore/log.h>
#include <pulsecore/modargs.h>
#include <pulsecore/namereg.h>
#include <pulsecore/core-util.h>
#include <pulsecore/sink.h>
#include <pulsecore/source.h>
#include <pulse/gccmacro.h>

#include <stdbool.h>
#include <stdio.h>

typedef struct __equalizer
{

}equalizer;

struct userdata
{
	FILE *fp;
	pa_module *module;
	pa_core *core;
	pa_source *source;
	pa_sink* sink;
	pa_sink_input* sink_input;
	equalizer eq;
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

PA_MODULE_AUTHOR("Stefanoz");
PA_MODULE_DESCRIPTION("lol");
PA_MODULE_VERSION("v0.00.001-cacca_rel");


/* Called from I/O thread context */
static int sink_process_msg_cb(pa_msgobject *o, int code, void *data, int64_t offset, pa_memchunk *chunk)
{
	pa_log("sink_process_msg_cb");
	return pa_sink_process_msg(o, code, data, offset, chunk);
}
static int sink_set_state_cb(pa_sink *s, pa_sink_state_t state)
{
	struct userdata *u;

	pa_log("sink_set_state_cb");
	pa_sink_assert_ref(s);
	pa_assert_se(u=(struct userdata*)s->userdata);

	/*
	 ...
	 */
	return 0;
}
static void sink_update_requested_latency_cb(pa_sink *s)
{
	struct userdata *u;
	
	pa_log("sink_update_requested_latency_cb");
	pa_sink_assert_ref(s);
	pa_assert_se(u=(struct userdata*)s->userdata);

	/*
	 ...
	 */
}

static void sink_request_rewind_cb(pa_sink *s)
{
	struct userdata *u;
	
	pa_log("sink_request_rewind_cb");
	pa_sink_assert_ref(s);
	pa_assert_se(u=(struct userdata*)s->userdata);

}

int pa__init(pa_module *m) 
{
	struct userdata *ud;
	pa_modargs *ma;
	pa_sink *master=NULL;
	pa_sink_input_new_data sink_input_data;
	pa_source_new_data source_data;
	pa_sample_spec source_ss;
	pa_sink_new_data sink_data;
	double gaindb,f0;
	unsigned sr;
	char out[100];

	
	pa_log("its started");

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

	source_ss=master->sample_spec;


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

	sprintf(out,"%d",master->sample_spec.rate);
	pa_log(out);
	sprintf(out,"%d",master->sample_spec.channels);
	pa_log(out);
	sprintf(out,"%d",master->sample_spec.format);
	pa_log(out);

	/* Create source */
	/*pa_source_new_data_init(&source_data);
		source_data.driver==__FILE__;
		source_data.module=m;
		source_data.name=pa_sprintf_malloc("%s.eqpro",master->name);
		pa_source_new_data_set_sample_spec(&source_data,&master->sample_spec);
		pa_source_new_data_set_channel_map(&source_data,&master->channel_map);
		pa_proplist_sets(source_data.proplist, PA_PROP_DEVICE_MASTER_DEVICE, master->name);
		pa_proplist_sets(source_data.proplist, PA_PROP_DEVICE_CLASS, "filter");
		ud->source=pa_source_new(m->core,&source_data,master->flags);
		pa_source_new_data_done(&source_data); 
	
	
	if(!ud->source)
	{
		pa_log("Failed to create source.");
		goto fail;
	}

*/

/* Create sync init*/
   pa_sink_new_data_init(&sink_data);
		sink_data.driver=__FILE__;
		sink_data.module=m;
		sink_data.name=pa_sprintf_malloc("%s.eqpro",master->name);
		pa_sink_new_data_set_sample_spec(&sink_data,&master->sample_spec);
		pa_sink_new_data_set_channel_map(&sink_data,&master->channel_map);
		pa_proplist_sets(sink_data.proplist, PA_PROP_DEVICE_MASTER_DEVICE, master->name);
		pa_proplist_sets(sink_data.proplist, PA_PROP_DEVICE_CLASS, "filter");
		ud->sink=pa_sink_new(m->core,&sink_data,master->flags);
	pa_sink_new_data_done(&sink_data);

	if(!ud->sink)
	{
		pa_log("Failed to create sink");
		goto fail;
	}

	/* callbacks */
	ud->sink->parent.process_msg=sink_process_msg_cb;
	ud->sink->set_state = sink_set_state_cb;
	ud->sink->update_requested_latency = sink_update_requested_latency_cb;
	ud->sink->request_rewind = sink_request_rewind_cb;
	/*TO DO ADD OTHERS CALLBACK AND MANAGE THE CALLBACK*/
	
	pa_sink_set_asyncmsgq(ud->sink, master->asyncmsgq);
/* Create sync done*/


/*Create sync input init*/
	pa_sink_input_new_data_init(&sink_input_data);
		
	
	sink_input_data.driver = __FILE__;
	sink_input_data.module = m;
	pa_sink_input_new_data_set_sink(&sink_input_data, master,false);
	sink_input_data.origin_sink = ud->sink;
	pa_proplist_setf(sink_input_data.proplist, PA_PROP_MEDIA_NAME, "eqpro Sink Stream from %s", pa_proplist_gets(ud->sink->proplist, PA_PROP_DEVICE_DESCRIPTION));
	pa_proplist_sets(sink_input_data.proplist, PA_PROP_MEDIA_ROLE, "filter");
	pa_sink_input_new_data_set_sample_spec(&sink_input_data, &master->sample_spec);
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
/*Create sync input done*/







	pa_sink_put(ud->sink);
	pa_sink_input_put(ud->sink_input);


	pa_modargs_free(ma);

	return 0;

fail:
	if(ma)
		pa_modargs_free(ma);
	return -1;

}                                                                                                                                                                                              
void pa__done(pa_module *mod) 
{
	pa_log("its done");                                                                                                                                                                    
}
