#ifndef PULSEDRIVER_H
#define PULSEDRIVER_H



extern "C"
{
#include <pulse/pulseaudio.h>
#include <pulse/message-params.h>
}

#include <iostream>
#include <stdexcept>
#include <string>
#include <QList>
#include <QtDebug>
#include <QObject>
#include <QStringList>

class PulseDriver;

enum class Actions{
    getSinks,
    getModules,
    sendMsg,
    subscrive //fixme
};

struct ud_t
{
    void *rawout;
    PulseDriver* pd_context;
    pa_mainloop *m;
    Actions a;
    pa_context* c;
    pa_mainloop_api *mapi;
    pa_operation* pa_op;
};


class PulseDriver : public QObject
{
    Q_OBJECT

public:

    struct Eqinfo
    {
        int nBands;
        double fmin;
        double dB;
        double r;
        double K;
        QVector<double> par;
    };


    PulseDriver(QObject* parent=0):
        QObject(parent), modulenum(-1)
    {}

    ~PulseDriver()
    {}

    void SendSliderChangeMsg(double val, int idx)
    {
        pa_message_param *param = pa_message_param_new();
        pa_message_param_begin_list(param);
        pa_message_param_write_double(param,val,32);
        pa_message_param_write_int64(param,idx);
        pa_message_param_end_list(param);

        Message mx("sliderchange",pa_message_param_to_string(param),MessageDest());
        SendMessage(mx);

        if(mx.response != "OK")
            emit module_seems_disconnected();
    }

    void SendDialChangeMsg(double val)
    {
        pa_message_param *param = pa_message_param_new();
        pa_message_param_write_double(param,val,32);

        Message mx("dialchange",pa_message_param_to_string(param),MessageDest());
        SendMessage(mx);

        if(mx.response != "OK")
            emit module_seems_disconnected();

    }

    Eqinfo RequireEqInfo()
    {
        Eqinfo info;

        Message mx( "getinfo", "", MessageDest());
        SendMessage(mx);

        void *state=NULL, *state2=NULL;
        //pa_message_param* param;
        char* startpos=NULL;

        QByteArray resp = mx.response.toLocal8Bit();
        pa_message_param_split_list(resp.data(), &startpos, NULL, &state);

        int64_t n;
        pa_message_param_read_int64(startpos, &n,&state2);
        info.nBands=n;
        pa_message_param_read_double(startpos, &info.fmin, &state2);
        pa_message_param_read_double(startpos, &info.dB, &state2);
        pa_message_param_read_double(startpos, &info.r, &state2);
        pa_message_param_read_double(startpos, &info.K, &state2);
        for (int i=0; i<info.nBands; i++)
        {
            double x;
            pa_message_param_read_double(startpos, &x, &state2);
            info.par.append(x);
        }

        return info;
    }

    QVector<int> GetEqproModules()
    {
        QList<Module> modules=GetModule();
        QVector<int> out;

        foreach (auto s, modules)
        {
            if(s.name == DRIVERNAME)
                out.append(s.idx);
        }

        return out;
    }

    // doesn't work
    void RegisterModuleDiconnectCallback()
    {
        MakeOperation<void*>(Actions::subscrive);
    }

    int getModuleNum() const { return modulenum; }
    void setModuleNum(unsigned n)
    {
        QList<Module> ml=GetModule();
        foreach (auto m, ml)
        {
            if(m.idx == int(n) && m.name == DRIVERNAME)
            {
                modulenum = n;
                return;
            }
        }
        throw std::logic_error("Module number is not correct");
    }

private:
    const QString DRIVERNAME="module-eqpro-sink";
    const QString MSGDEST="/modules/eqpro";

    int modulenum;

    struct Sink
    {
        Sink(const QString &name="", const QString &driver="", int idx=0):
            name(name), driver(driver), idx(idx)
        {}

        QString name;
        QString driver;
        int idx;
    };

    struct Message
    {
        Message(const QString& message="", const QString& param="", const QString& to="", const QString& response=""):
            to(to), message(message), param(param), response(response)
        {}

        QString to;
        QString message;
        QString param;
        QString response;
    };

    struct Module
    {
        Module(const QString &name="", int idx=0):
            name(name), idx(idx)
        {}

        QString name;
        int idx;
    };

    QString MessageDest()
    {
        if (modulenum<0)
            throw std::logic_error("No module selected");

        return MSGDEST + QString("/") + QString::number(modulenum);
    }


    template<typename T>
    T MakeOperation(Actions a, T x=T())
    {
        pa_context *context = nullptr;
        pa_mainloop *m = nullptr;
        pa_mainloop_api *mapi = nullptr;

        if (!(m = pa_mainloop_new()))
            throw std::runtime_error("pa_mainloop_new() failed.");

        mapi = pa_mainloop_get_api(m);

        if (!(context = pa_context_new(mapi,"eqpro_gui")))
            throw std::runtime_error("pa_context_new() failed.");

        ud_t ud={(void*)&x, this, m, a, context, mapi, nullptr};

        pa_context_set_state_callback(context, context_state_callback, (void*)&ud);
        pa_context_connect(context, NULL, (pa_context_flags)0, NULL);

        pa_mainloop_run(m, NULL);

        pa_context_unref(context);
        pa_mainloop_free(m);

        return x;
    }

    QList<Sink> GetSinks()
    {
        return MakeOperation<QList<Sink>>(Actions::getSinks);
    }

    QList<Module> GetModule()
    {
        return MakeOperation<QList<Module>>(Actions::getModules);
    }

    void SendMessage(Message& message)
    {
        message=MakeOperation<Message>(Actions::sendMsg,message);
    }

    static void get_sink_info_callback(pa_context *c, const pa_sink_info *l, int is_last, void *userdata)
    {
        qDebug()<<"sink callback";
        ud_t* ud=(ud_t*)userdata;

        QList<Sink>* list=(QList<Sink>*)ud->rawout;

        if(is_last > 0)
            return;

        list->append(Sink(l->name,l->driver,l->index));
    }

    static void get_module_info_callback(pa_context *c, const pa_module_info *l, int is_last, void *userdata)
    {
        qDebug()<<"module callback";
        ud_t* ud=(ud_t*)userdata;

        QList<Module>* list=(QList<Module>*)ud->rawout;

        if(is_last > 0)
            return;

        list->append(Module(l->name,l->index));
    }

    static void get_content_string_callback(pa_context *c, int success, char *response, void *userdata)
    {
         qDebug()<<"get_content_string_callback";

         ud_t* ud=(ud_t*)userdata;
         Message* mx =(Message*)ud->rawout;

         if(!success)
         {
             qDebug()<<"not success, send emit";
             ud->mapi->quit(ud->mapi,0);
             emit ud->pd_context->module_seems_disconnected();
             return;
         }

         if(response)
         {
             mx->response=response;
             qDebug()<<mx->response;
         }
    }

    static void context_subscribe_callback(pa_context *c, pa_subscription_event_type_t t, uint32_t idx, void *userdata)
    {
        qDebug()<<"context_subscribe_callback";

        ud_t* ud=(ud_t*)userdata;

        if (idx == (int)ud->pd_context->getModuleNum())
        {
            switch (t & PA_SUBSCRIPTION_EVENT_TYPE_MASK)
            {
            case PA_SUBSCRIPTION_EVENT_NEW:
            case PA_SUBSCRIPTION_EVENT_CHANGE:
                break;
            case PA_SUBSCRIPTION_EVENT_REMOVE:
                emit ud->pd_context->module_seems_disconnected();
            }

        }

    }


    static void context_state_callback(pa_context *c, void *userdata)
    {
        assert(c);

        ud_t* ud=(ud_t*)userdata;
        Message* mx;
        qDebug()<<"context state";


        switch (pa_context_get_state(c))
        {
        case PA_CONTEXT_CONNECTING:
        case PA_CONTEXT_AUTHORIZING:
        case PA_CONTEXT_SETTING_NAME:
            break;

        case PA_CONTEXT_READY:
            qDebug()<<"Ready";
            switch(ud->a)
            {
            case Actions::getSinks:
                ud->pa_op = pa_context_get_sink_info_list(c, get_sink_info_callback, userdata);
                break;
            case Actions::getModules:
                ud->pa_op = pa_context_get_module_info_list(c, get_module_info_callback, userdata);
                break;
            case Actions::sendMsg:
                mx=(Message*)ud->rawout;
                ud->pa_op = pa_context_send_message_to_object(c,
                                                              mx->to.toLocal8Bit().constData(),
                                                              mx->message.toLocal8Bit().constData(),
                                                              mx->param.toLocal8Bit().constData(),
                                                              get_content_string_callback,
                                                              userdata);
                break;
            case Actions::subscrive:
                pa_context_set_subscribe_callback(c, context_subscribe_callback, userdata);
                ud->pa_op = pa_context_subscribe(c,
                                                 (pa_subscription_mask)(PA_SUBSCRIPTION_MASK_SINK|
                                                 PA_SUBSCRIPTION_MASK_SOURCE|
                                                 PA_SUBSCRIPTION_MASK_SINK_INPUT|
                                                 PA_SUBSCRIPTION_MASK_SOURCE_OUTPUT|
                                                 PA_SUBSCRIPTION_MASK_MODULE|
                                                 PA_SUBSCRIPTION_MASK_CLIENT|
                                                 PA_SUBSCRIPTION_MASK_SAMPLE_CACHE|
                                                 PA_SUBSCRIPTION_MASK_SERVER|
                                                 PA_SUBSCRIPTION_MASK_CARD),
                                                 NULL,userdata);
                break;
            }

            pa_operation_set_state_callback(ud->pa_op, get_operation_state, userdata);
            break;

        case PA_CONTEXT_TERMINATED:
            qDebug()<<"terminated";
            break;

        case PA_CONTEXT_FAILED:
            ud->mapi->quit(ud->mapi,0); //stop mainloop
            emit ud->pd_context->module_seems_disconnected();
            break;
        default:
            throw std::runtime_error("Connection failure: " + std::string(pa_strerror(pa_context_errno(c))));

        }

    }

    static void get_operation_state(pa_operation* pa_op, void* userdata)
    {
        ud_t* ud=(ud_t*)userdata;

        qDebug()<<"op state "<< (int)ud->a;
        switch(pa_operation_get_state(pa_op))
        {
        case PA_OPERATION_RUNNING:
            qDebug()<<"Op running";
            break;

        case PA_OPERATION_CANCELLED:
            qDebug()<<"Op canc";
        case PA_OPERATION_DONE:
            qDebug()<<"Op done canc";
            pa_operation_unref(ud->pa_op);
            ud->mapi->quit(ud->mapi,0); //stop mainloop
            break;

        }
    }


private slots:

signals:
    void module_seems_disconnected();

};

#endif // PULSEDRIVER_H
