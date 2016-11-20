#ifndef SIGNALS_H
#define SIGNALS_H

#include <QObject>

class SignalWrapper : public QObject
{
    Q_OBJECT
signals:
    void closeNow();
public slots:
    void squit();
};

#endif // SIGNALS_H
