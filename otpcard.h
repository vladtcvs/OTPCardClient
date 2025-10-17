#ifndef OTPCARD_H
#define OTPCARD_H

#include <memory>
#include <tuple>

#include <QByteArray>

#include <infc.h>

class OTPCard
{
public:
    enum HashAlgorithm {
        NONE = 0,
        SHA1 = 1,
        SHA256 = 2,
        SHA512 = 3,
    };

    enum Command {
        HMAC = 0x01,
        GET_SECRET_STATUS = 0x02,
        SAVE_NEW_SECRET = 0x03,
        DELETE_SECRET = 0x04,

        SAVE_PIN = 0x05,
        UNBLOCK_PIN = 0x06,
        SAVE_ADMIN_PIN = 0x07,

        GET_INFO = 0x08,
        PIN = 0x42,
    };

    enum OperationResult {
        SUCCESS = 0,
        NO_CONNECTION,
        INVALID_DATA,
        INVALID_PIN,
    };

public:
    OTPCard(std::shared_ptr<INFC> nfc);
public:
    bool checkConnection();
    QByteArray getSerial();
    size_t getMaxSecrets();
    size_t getMaxSecretValueLength();
    size_t getMaxSecretNameLength();
    bool getAlgorithmSupported(HashAlgorithm algo);

    OperationResult Auth(QByteArray pin);

    std::tuple<OperationResult, bool, QByteArray, HashAlgorithm> getSecretInfo(int id);
    OperationResult setSecret(int id, QByteArray name, QByteArray secret, HashAlgorithm method);
    std::tuple<OperationResult, QByteArray> calculateHMAC(int id, QByteArray challenge);
    OperationResult deleteSecret(int id);

    OperationResult setPin(QByteArray newPin);
    OperationResult setAdminPin(QByteArray adminPin, QByteArray newAdminPin);
    OperationResult unlockPin(QByteArray adminPin);
private:
    std::shared_ptr<INFC> nfc;
};

#endif // OTPCARD_H
