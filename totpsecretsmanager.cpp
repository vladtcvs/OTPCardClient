#include "totpsecretsmanager.h"

TOTPSecretsManager::TOTPSecretsManager()
{
}

QList<std::shared_ptr<TOTPSecret>> TOTPSecretsManager::getSecretsForCard(const QString& card_serial) const
{
    QList<std::shared_ptr<TOTPSecret>> ss;
    for (auto s : secrets) {
        if (s->getCardSerial() == card_serial)
            ss.append(s);
    }
    return ss;
}

void TOTPSecretsManager::addSecret(std::shared_ptr<TOTPSecret> secret)
{
    auto card_serial = secret->getCardSerial();
    int id = secret->getId();
    for (auto s : secrets) {
        if (s->getCardSerial() == card_serial && s->getId() == id)
            return;
    }
    secrets.append(secret);
}

bool TOTPSecretsManager::deleteSecret(const QString& card_serial, int id)
{
    for (auto it = secrets.constBegin(); it != secrets.constEnd(); it++) {
        if ((*it)->getCardSerial() == card_serial && (*it)->getId() == id) {
            secrets.erase(it);
            return true;
        }
    }
    return false;
}

std::shared_ptr<TOTPSecret> TOTPSecretsManager::getSecret(const QString& card_serial, int id) const
{
    for (auto s : secrets) {
        if (s->getCardSerial() == card_serial && s->getId() == id)
            return s;
    }
    return nullptr;
}
