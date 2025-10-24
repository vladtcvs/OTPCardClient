#ifndef PINMANAGER_H
#define PINMANAGER_H

#include <QString>
#include <QByteArray>

class PinManager
{
public:
    PinManager();
    void setPin(const QString& newpin);

    const QByteArray& pin() const;
    bool isValid() const;
    void invalid();
private:
    bool _valid;
    QByteArray _pin;
};

#endif // PINMANAGER_H
