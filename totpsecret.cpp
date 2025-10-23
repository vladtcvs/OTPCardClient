#include "totpsecret.h"

#include <QDebug>
#include <QDateTime>

TOTPSecret::TOTPSecret(const QString& card_serial, int id, const QString& display_name, const QString& secret_name, OTPCard::HashAlgorithm method)
{
    this->card_serial = card_serial;
    this->id = id;
    this->secret_name = secret_name;
    this->display_name = display_name;
    this->method = method;
    this->digits = 6;
    this->secondsPeriod = 30;
    this->timeShift = 0;
}

TOTPSecret::TOTPSecret(const QString& card_serial, int id, const QString& display_name, const QString& secret_name, OTPCard::HashAlgorithm method, int digits, int secondsPeriod, int timeShift)
{
    this->card_serial = card_serial;
    this->id = id;
    this->secret_name = secret_name;
    this->display_name = display_name;
    this->method = method;
    this->digits = digits;
    this->secondsPeriod = secondsPeriod;
    this->timeShift = timeShift;
}

QPair<QByteArray, int> TOTPSecret::generateChallenge() const
{
    QDateTime asd(QDateTime::currentDateTime());
    unsigned long unixTime = asd.toSecsSinceEpoch();
    unsigned long challenge = (unixTime - timeShift) / secondsPeriod;

    // Big Endian
    unsigned char challengeBytes[8];
    for (int i = 0; i < 8; i++)
        challengeBytes[i] = (challenge >> (64-8-i*8)) & 0xFFU;

    QByteArray ch = QByteArray(reinterpret_cast<const char*>(challengeBytes), sizeof(challengeBytes));
    int time_passed = unixTime - (challenge * secondsPeriod + timeShift);
    return QPair<QByteArray, int>(ch, secondsPeriod - time_passed);
}

QString TOTPSecret::TOTP(QByteArray HMAC) const
{
    size_t len = HMAC.length();
    unsigned offset = HMAC[len-1] & 0x0FU;
    unsigned long result = (((unsigned long)(unsigned char)HMAC[offset]) << 24 |
                            ((unsigned long)(unsigned char)HMAC[offset+1]) << 16 |
                            ((unsigned long)(unsigned char)HMAC[offset+2]) << 8 |
                            ((unsigned long)(unsigned char)HMAC[offset+3])) & 0x7FFFFFFFUL;

    QString digits;
    for (int i = 0; i < this->digits; i++)
    {
        int digit = result % 10;
        result /= 10;
        digits.append("0123456789"[digit]);
    }
    std::reverse(digits.begin(), digits.end());
    return digits;
}

void TOTPSecret::update_display_name(const QString& display_name)
{
    this->display_name = display_name;
}

void TOTPSecret::update_digits(int digits)
{
    this->digits = digits;
}

void TOTPSecret::update_period(int period)
{
    this->secondsPeriod = period;
}

void TOTPSecret::update_timeShift(int timeShift)
{
    this->timeShift = timeShift;
}
