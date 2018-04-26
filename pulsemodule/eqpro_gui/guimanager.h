#ifndef GUIMANAGER_H
#define GUIMANAGER_H

#include <QObject>
#include <QtDebug>

//#define EQGUI_DEBUG

class GuiManager : public QObject
{
    Q_OBJECT

private:
    QObject *sliderRow;

public:
    explicit GuiManager(QObject *sliderRow)
    {
        this->sliderRow=sliderRow;
    }

    virtual ~GuiManager() {}



signals:


public slots:
    void sliderChanged(double val, int idx)
    {
#ifdef EQGUI_DEBUG
        qDebug()<<"slider "<<idx<<" change to "<<val;
#endif

    }


};

#endif // GUIMANAGER_H
