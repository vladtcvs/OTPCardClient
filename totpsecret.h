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
    QString card_serial;
    OTPCard::HashAlgorithm method;
public:
    TOTPSecret(const QString& card_serial, int id, const QString& display_name, const QString& secret_name, OTPCard::HashAlgorithm method);
    TOTPSecret(const QString& card_serial, int id, const QString& display_name, const QString& secret_name, OTPCard::HashAlgorithm method, int digits, int secondsPeriod, int timeShift);
    void update_display_name(const QString& display_name);
    void update_digits(int digits);
    void update_period(int period);
    void update_timeShift(int timeShift);
    QPair<QByteArray, int> generateChallenge() const;
    QString TOTP(QByteArray HMAC) const;
    int getId() const {return id;}
    const QString& getName() const {return secret_name;}
    const QString& getDisplayName() const {return display_name;}
    const QString& getCardSerial() const {return card_serial;}
    int getDigits() const {return digits;}
    int getTimeShift() const {return timeShift;}
    int getSecondsPeriod() const {return secondsPeriod;}
    OTPCard::HashAlgorithm getMethod() const {return method;}
};

#endif // TOTPSECRET_H
