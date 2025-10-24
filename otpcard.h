#ifndef OTPCARD_H
#define OTPCARD_H

#include <memory>
#include <tuple>

#include <QByteArray>

#include <infc.h>
#include <pinmanager.h>

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
        NO_AUTHORIZED,
    };

public:
    OTPCard(std::shared_ptr<INFC> nfc, std::shared_ptr<PinManager> pin);
public:
    bool checkConnection();
    QByteArray getSerial();
    size_t getMaxSecrets();
    size_t getMaxSecretValueLength();
    size_t getMaxSecretNameLength();
    bool getAlgorithmSupported(HashAlgorithm algo);

    OperationResult Auth();

    std::tuple<OperationResult, bool, QByteArray, HashAlgorithm> getSecretInfo(int id);
    OperationResult setSecret(int id, const QByteArray& name, const QByteArray& secret, HashAlgorithm method);
    std::tuple<OperationResult, QByteArray> calculateHMAC(int id, const QByteArray& challenge);
    OperationResult deleteSecret(int id);

    OperationResult setPin(const QByteArray& newPin);
    OperationResult setAdminPin(const QByteArray& adminPin, const QByteArray& newAdminPin);
    OperationResult unlockPin(const QByteArray& adminPin, const QByteArray& newPin);
private:
    std::shared_ptr<INFC> nfc;
    std::shared_ptr<PinManager> pin;
};

#endif // OTPCARD_H
