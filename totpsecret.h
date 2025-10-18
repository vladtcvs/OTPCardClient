#ifndef TOTPSECRET_H
#define TOTPSECRET_H

#include <QString>
#include <QByteArray>
#include <QPair>

#include <otpcard.h>

class TOTPSecret
{
private:
    int id;
    int digits;
    int secondsPeriod;
    int timeShift;
    QString secret_name;
    QString display_name;
    OTPCard::HashAlgorithm method;
public:
    TOTPSecret(int id, const QString& display_name, const QString& secret_name, OTPCard::HashAlgorithm method);
    TOTPSecret(int id, const QString& display_name, const QString& secret_name, OTPCard::HashAlgorithm method, int digits, int secondsPeriod, int timeShift);
    QPair<QByteArray, int> generateChallenge() const;
    QString TOTP(QByteArray HMAC) const;
    int getId() const {return id;}
    const QString& getName() const {return secret_name;}
    const QString& getDisplayName() const {return display_name;}
    OTPCard::HashAlgorithm getMethod() const {return method;}
};

#endif // TOTPSECRET_H
