#ifndef QMLUTILS_H
#define QMLUTILS_H

#include <QObject>

class QmlUtils : public QObject
{
    Q_OBJECT
public:
    Q_INVOKABLE void changedBands(unsigned value);
    Q_INVOKABLE unsigned getMaxBands();
signals:
    void closeNow();
public slots:
    void squit();
};

#endif // SIGNALS_H
