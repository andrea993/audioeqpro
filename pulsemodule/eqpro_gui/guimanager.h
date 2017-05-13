#ifndef GUIMANAGER_H
#define GUIMANAGER_H

#include <QObject>

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
    void sliderChanged(int s, double val)
    {

    }


};

#endif // GUIMANAGER_H
