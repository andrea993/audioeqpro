#ifndef SIGNALS_H
#define SIGNALS_H

#include <QObject>

class SignalWrapper : public QObject
{
    Q_OBJECT
signals:
    void closeNow();
};

#endif // SIGNALS_H
