#include "pinmanager.h"

PinManager::PinManager()
{
    _pin = QByteArray();
    _valid = false;
}

bool PinManager::isValid() const
{
    return _valid;
}

void PinManager::invalid()
{
    _valid = false;
    _pin = QByteArray();
}

const QByteArray& PinManager::pin() const
{
    return _pin;
}

void PinManager::setPin(const QString& newpin)
{
    _valid = true;
    _pin = newpin.toUtf8();
}
