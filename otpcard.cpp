#include "otpcard.h"
#include <nfcget.h>

OTPCard::OTPCard(std::shared_ptr<INFC> nfc, std::shared_ptr<PinManager> pin)
{
    this->nfc = nfc;
    this->pin = pin;
}

bool OTPCard::checkConnection()
{
    NFCGet nfcget(nfc);
    return nfcget.isConnected();
}

QByteArray OTPCard::getSerial()
{
    NFCGet nfcget(nfc);
    if (!nfcget.isConnected())
        return QByteArray();
    nfc->selectOTPCard();
    auto [sw1, sw2, response] = nfc->sendAPDU(0x00, Command::GET_INFO, 0, 0, 0, {});
    QByteArray serial = response.slice(6, 4);
    return serial;
}

size_t OTPCard::getMaxSecrets()
{
    NFCGet nfcget(nfc);
    if (!nfcget.isConnected())
        return 0;
    nfc->selectOTPCard();
    auto [sw1, sw2, response] = nfc->sendAPDU(0x00, Command::GET_INFO, 0, 0, 0, {});
    return response[0];
}

size_t OTPCard::getMaxSecretValueLength()
{
    NFCGet nfcget(nfc);
    if (!nfcget.isConnected())
        return 0;
    auto [sw1, sw2, response] = nfc->sendAPDU(0x00, Command::GET_INFO, 0, 0, 0, {});
    return response[2];
}

size_t OTPCard::getMaxSecretNameLength()
{
    NFCGet nfcget(nfc);
    if (!nfcget.isConnected())
        return 0;
    auto [sw1, sw2, response] = nfc->sendAPDU(0x00, Command::GET_INFO, 0, 0, 0, {});
    return response[1];
}

bool OTPCard::getAlgorithmSupported(OTPCard::HashAlgorithm algo)
{
    NFCGet nfcget(nfc);
    if (!nfcget.isConnected())
        return false;
    auto [sw1, sw2, response] = nfc->sendAPDU(0x00, Command::GET_INFO, 0, 0, 0, {});
    switch (algo) {
    case OTPCard::HashAlgorithm::SHA1:
        return response[3];
    case OTPCard::HashAlgorithm::SHA256:
        return response[4];
    case OTPCard::HashAlgorithm::SHA512:
        return response[5];
    default:
        return false;
    }
}

OTPCard::OperationResult OTPCard::Auth()
{
    if (!pin->isValid()) {
        return OTPCard::OperationResult::INVALID_PIN;
    }
    NFCGet nfcget(nfc);
    if (!nfcget.isConnected())
        return OTPCard::OperationResult::NO_CONNECTION;

    QByteArray apdu;
    apdu.append(pin->pin().length());
    for (char c : pin->pin())
        apdu.append(c);
    auto [sw1, sw2, response] = nfc->sendAPDU(0x00, Command::PIN, 0, 0, 0, apdu);
    if (sw1 == 0x69 && sw2 == 0x82) {
        this->pin->invalid();
        return OTPCard::OperationResult::INVALID_PIN;
    }

    if (sw1 != 0x90 || sw2 != 0x00)
        return OTPCard::OperationResult::INVALID_DATA;

    return OTPCard::OperationResult::SUCCESS;
}

std::tuple<OTPCard::OperationResult, bool, QByteArray, OTPCard::HashAlgorithm> OTPCard::getSecretInfo(int id)
{
    NFCGet nfcget(nfc);
    if (!nfcget.isConnected())
        return std::make_tuple(OTPCard::OperationResult::NO_CONNECTION, false, "", HashAlgorithm::NONE);;

    QByteArray apdu;
    apdu.append(id);
    auto [sw1, sw2, response] = nfc->sendAPDU(0x00, Command::GET_SECRET_STATUS, 0, 0, 0, apdu);

    if (sw1 == 0x69 && sw2 == 0x82)
        return std::make_tuple(OTPCard::OperationResult::NO_AUTHORIZED, false, "", HashAlgorithm::NONE);

    if (sw1 != 0x90 || sw2 != 0x00)
        return std::make_tuple(OTPCard::OperationResult::INVALID_DATA, false, "", HashAlgorithm::NONE);

    bool used = response[0];
    size_t name_len = response[1];
    if ((size_t)response.length() != 3 + name_len) {
        return std::make_tuple(OTPCard::OperationResult::INVALID_DATA, false, "", HashAlgorithm::NONE);
    }
    QByteArray name;
    for (size_t i = 0; i < name_len; i++)
        name.append(response[2+i]);
    HashAlgorithm method;
    switch (response[2+name_len])
    {
    case HashAlgorithm::NONE:
    case HashAlgorithm::SHA1:
    case HashAlgorithm::SHA256:
    case HashAlgorithm::SHA512:
        method = static_cast<HashAlgorithm>(response[2+name_len]);
        break;
    default:
        return std::make_tuple(OTPCard::OperationResult::INVALID_DATA, false, "", HashAlgorithm::NONE);
    }

    return std::make_tuple(OTPCard::OperationResult::SUCCESS, used, name, method);
}

OTPCard::OperationResult OTPCard::setSecret(int id, const QByteArray& name, const QByteArray& secret, OTPCard::HashAlgorithm method)
{
    NFCGet nfcget(nfc);
    if (!nfcget.isConnected())
        return OTPCard::OperationResult::NO_CONNECTION;
    QByteArray apdu;
    apdu.append(id);
    apdu.append(secret.length());
    for (char c : secret)
        apdu.append(c);
    apdu.append(name.length());
    for (char c : name)
        apdu.append(c);
    apdu.append(method);
    auto [sw1, sw2, response] = nfc->sendAPDU(0x00, Command::SAVE_NEW_SECRET, 0, 0, 0, apdu);
    if (sw1 == 0x69 && sw2 == 0x82)
        return OTPCard::OperationResult::NO_AUTHORIZED;
    if (sw1 != 0x90 || sw2 != 0x00)
        return OTPCard::OperationResult::INVALID_DATA;
    return OTPCard::OperationResult::SUCCESS;
}

std::tuple<OTPCard::OperationResult, QByteArray> OTPCard::calculateHMAC(int id, const QByteArray& challenge)
{
    NFCGet nfcget(nfc);
    if (!nfcget.isConnected())
        return std::make_tuple(OTPCard::OperationResult::NO_CONNECTION, "");
    QByteArray apdu;
    apdu.append(id);
    apdu.append(challenge.length());
    for (char c : challenge)
        apdu.append(c);
    auto [sw1, sw2, response] = nfc->sendAPDU(0x00, Command::HMAC, 0, 0, 0, apdu);

    if (sw1 == 0x69 && sw2 == 0x82)
        return std::make_tuple(OTPCard::OperationResult::NO_AUTHORIZED, "");

    if (sw1 != 0x90 || sw2 != 0x00)
        return std::make_tuple(OTPCard::OperationResult::INVALID_DATA, "");

    return std::make_tuple(OTPCard::OperationResult::SUCCESS, response);
}

OTPCard::OperationResult OTPCard::deleteSecret(int id)
{
    NFCGet nfcget(nfc);
    if (!nfcget.isConnected())
        return OTPCard::OperationResult::NO_CONNECTION;
    QByteArray apdu;
    apdu.append(id);
    auto [sw1, sw2, response] = nfc->sendAPDU(0x00, Command::DELETE_SECRET, 0, 0, 0, apdu);
    if (sw1 == 0x69 && sw2 == 0x82)
        return OTPCard::OperationResult::NO_AUTHORIZED;
    if (sw1 != 0x90 || sw2 != 0x00)
        return OTPCard::OperationResult::INVALID_DATA;
    return OTPCard::OperationResult::SUCCESS;
}

OTPCard::OperationResult OTPCard::setPin(const QByteArray& newPin)
{
    NFCGet nfcget(nfc);
    if (!nfcget.isConnected())
        return OTPCard::OperationResult::NO_CONNECTION;
    QByteArray apdu;
    apdu.append(newPin.length());
    for (char c : newPin)
        apdu.append(c);
    auto [sw1, sw2, response] = nfc->sendAPDU(0x00, Command::SAVE_PIN, 0, 0, 0, apdu);
    if (sw1 == 0x69 && sw2 == 0x82)
        return OTPCard::OperationResult::NO_AUTHORIZED;
    if (sw1 != 0x90 || sw2 != 0x00)
        return OTPCard::OperationResult::INVALID_DATA;
    return OTPCard::OperationResult::SUCCESS;
}

OTPCard::OperationResult OTPCard::setAdminPin(const QByteArray& adminPin, const QByteArray& newAdminPin)
{
    NFCGet nfcget(nfc);
    if (!nfcget.isConnected())
        return OTPCard::OperationResult::NO_CONNECTION;
    QByteArray apdu;
    apdu.append(adminPin.length());
    for (char c : adminPin)
        apdu.append(c);
    apdu.append(newAdminPin.length());
    for (char c : newAdminPin)
        apdu.append(c);
    auto [sw1, sw2, response] = nfc->sendAPDU(0x00, Command::SAVE_ADMIN_PIN, 0, 0, 0, apdu);
    if (sw1 == 0x69 && sw2 == 0x82)
        return OTPCard::OperationResult::INVALID_PIN;
    if (sw1 != 0x90 || sw2 != 0x00)
        return OTPCard::OperationResult::INVALID_DATA;
    return OTPCard::OperationResult::SUCCESS;
}

OTPCard::OperationResult OTPCard::unlockPin(const QByteArray& adminPin, const QByteArray& newPin)
{
    NFCGet nfcget(nfc);
    if (!nfcget.isConnected())
        return OTPCard::OperationResult::NO_CONNECTION;
    QByteArray apdu;
    apdu.append(adminPin.length());
    for (char c : adminPin)
        apdu.append(c);
    apdu.append(newPin.length());
    for (char c : newPin)
        apdu.append(c);
    auto [sw1, sw2, response] = nfc->sendAPDU(0x00, Command::UNBLOCK_PIN, 0, 0, 0, apdu);
    if (sw1 == 0x69 && sw2 == 0x82)
        return OTPCard::OperationResult::INVALID_PIN;
    if (sw1 != 0x90 || sw2 != 0x00)
        return OTPCard::OperationResult::INVALID_DATA;
    return OTPCard::OperationResult::SUCCESS;
}
