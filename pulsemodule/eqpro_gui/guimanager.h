#ifndef GUIMANAGER_H
#define GUIMANAGER_H

#include <QObject>
#include <QtDebug>
#include <QQmlApplicationEngine>

#include "pulsedriver.h"

#define EQGUI_DEBUG

class GuiManager : public QObject
{
    Q_OBJECT

private:
    enum class GUIstate
    {
        connected,
        disconnected,
        chooseconnection
    };

    QQmlApplicationEngine* engine;
    PulseDriver pd;

    GUIstate state;
    QObject *pageloader;

    QObject* sliderRow;
    QObject *retryButton;

    void SetState(GUIstate s)
    {
        if(state==s)
            return;

        QObject* root =engine->rootObjects().first();
        QVector<int> mds;
        QObject* combomod;
        QStringList model;

        switch (s)
        {
        case GUIstate::disconnected:
            pageloader->setProperty("source", QUrl(QLatin1String("qrc:/Page0CheckModuleForm.qml")));

            retryButton=root->findChild<QObject*>("retryButton");
            connect(retryButton, SIGNAL(clicked()),this,SLOT(RetryButtonPressed()));

            break;

        case GUIstate::chooseconnection:
            pageloader->setProperty("source",  QUrl(QLatin1String("qrc:/Page05SelectConnection.qml")));

            combomod=root->findChild<QObject*>("comboModules");

            mds=(pd.GetEqproModules());
            foreach (auto m, mds)
                model<<QString::number(m);

            combomod->setProperty("model", QVariant::fromValue(model));

            connect(combomod,SIGNAL(activated(int)),this,SLOT(EqproModuleChosen(int)));

            break;

        case GUIstate::connected:
             pageloader->setProperty("source", QUrl(QLatin1String("qrc:/Page1Form.qml")));

             QObject* page1=root->findChild<QObject*>("page1");

             PulseDriver::Eqinfo info = pd.RequireEqInfo();
             sliderRow=root->findChild<QObject*>("slidersRow");

             sliderRow->setProperty("nBands",info.nBands);
             sliderRow->setProperty("fmin",info.fmin);
             sliderRow->setProperty("r",info.r);
             sliderRow->setProperty("dB",info.dB);

             page1->setProperty("dialvalue",info.K);

             QVariant ret_x;
             QMetaObject::invokeMethod(sliderRow, "redrawSlider", Q_RETURN_ARG(QVariant, ret_x), Q_ARG(QVariant, QVariant::fromValue(info.par)));

             //pd.RegisterModuleDiconnectCallback();

             connect(sliderRow, SIGNAL(sliderChange(double,int)),this, SLOT(sliderChanged(double,int)));
             connect(page1,SIGNAL(dialChange(double)),this,SLOT(dialChanged(double)));
             break;

        }
        state=s;
    }

    void EstimateState()
    {
        QVector<int> mds=pd.GetEqproModules();

        if (mds.length() == 1)
        {
            if (pd.getModuleNum() == -1 || state != GUIstate::connected)
            {
                pd.setModuleNum(mds.first());
                SetState(GUIstate::connected);
            }
            else if(pd.getModuleNum() == mds.first() && state != GUIstate::connected)
                SetState(GUIstate::connected);
            else
                SetState(GUIstate::disconnected);
        }
        else if (mds.length() == 0)
            SetState(GUIstate::disconnected);
        else
            SetState(GUIstate::chooseconnection);
    }


public:
    explicit GuiManager(QQmlApplicationEngine* engine):
        engine(engine)
    {
        QObject* root =engine->rootObjects().first();
        pageloader=root->findChild<QObject*>("pageloader");

        EstimateState();

        connect(&pd,SIGNAL(module_seems_disconnected()),this,SLOT(ModuleSeemsDisconnect()));

    }

    virtual ~GuiManager() {}

signals:


public slots:
    void sliderChanged(double val, int idx)
    {
        pd.SendSliderChangeMsg(val,idx);
    }

    void dialChanged(double val)
    {
        pd.SendDialChangeMsg(val);
    }

    void RetryButtonPressed()
    {
        EstimateState();
    }

    void EqproModuleChosen(int n)
    {
        QObject* root =engine->rootObjects().first();
        QObject* combomod=root->findChild<QObject*>("comboModules");

        QVariant ret_x;
        QMetaObject::invokeMethod(combomod, "selectedMod", Q_RETURN_ARG(QVariant, ret_x), Q_ARG(QVariant, QVariant::fromValue(n)));

        try {
            pd.setModuleNum(ret_x.toInt());
        } catch (std::logic_error){
            SetState(GUIstate::disconnected);
            return;
        }

        SetState(GUIstate::connected);
    }

    void ModuleSeemsDisconnect()
    {
        EstimateState();
    }

    void AppStateChanged(Qt::ApplicationState as)
    {
        qDebug()<<"a state changed " << as;
        if (as ==Qt::ApplicationActive && state == GUIstate::connected)
        {
            QObject* root =engine->rootObjects().first();
            PulseDriver::Eqinfo info = pd.RequireEqInfo();
            sliderRow=root->findChild<QObject*>("slidersRow");
            sliderRow->setProperty("nBands",info.nBands);
            sliderRow->setProperty("fmin",info.fmin);
            sliderRow->setProperty("r",info.r);
            sliderRow->setProperty("dB",info.dB);

            QObject* page1=root->findChild<QObject*>("page1");
            page1->setProperty("dialvalue",info.K);

            QVariant ret_x;
            QMetaObject::invokeMethod(sliderRow, "redrawSlider", Q_RETURN_ARG(QVariant, ret_x), Q_ARG(QVariant, QVariant::fromValue(info.par)));
        }

    }



};

#endif // GUIMANAGER_H
