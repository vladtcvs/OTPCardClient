#ifndef TOTPSECRETSMANAGER_H
#define TOTPSECRETSMANAGER_H

#include <QList>

#include <totpsecret.h>

class TOTPSecretsManager
{
public:
    TOTPSecretsManager();
    QList<std::shared_ptr<TOTPSecret>> getSecrets() const {return secrets;}
    QList<std::shared_ptr<TOTPSecret>> getSecretsForCard(const QString& card_serial) const;
    std::shared_ptr<TOTPSecret> getSecret(const QString& card_serial, int id) const;
    void addSecret(std::shared_ptr<TOTPSecret> secret);
    bool deleteSecret(const QString& card_serial, int id);
private:
    QList<std::shared_ptr<TOTPSecret>> secrets;
};

#endif // TOTPSECRETSMANAGER_H
