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

std::tuple<OTPCard::OperationResult, CardInfo> OTPCard::getCardInfo() const
{
    NFCGet nfcget(nfc);
    if (!nfcget.isConnected())
        return std::make_tuple(OTPCard::OperationResult::NO_CONNECTION, CardInfo());
    nfc->selectOTPCard();
    auto [sw1, sw2, response] = nfc->sendAPDU(0x00, Command::GET_INFO, 0, 0, 0, {});
    if (sw1 != 0x90 || sw2 != 0x00)
        return std::make_tuple(OTPCard::OperationResult::INVALID_DATA, CardInfo());
    size_t max_secrets = response[0];
    size_t max_secret_name_len = response[1];
    size_t max_secret_value_len = response[2];
    bool sha1 = response[3];
    bool sha256 = response[4];
    bool sha512 = response[5];
    QByteArray serial = response.slice(6, 4);
    return std::make_tuple(OTPCard::OperationResult::SUCCESS, CardInfo(serial,
                                                                       max_secrets,
                                                                       max_secret_name_len,
                                                                       max_secret_value_len,
                                                                       sha1, sha256, sha512));
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
    auto [sw1, sw2, response] = nfc->sendAPDU(0x00, Command::AUTH, 0, 0, 0, apdu);
    if (sw1 == 0x69 && sw2 == 0x82) {
        this->pin->invalid();
        return OTPCard::OperationResult::INVALID_PIN;
    }

    if (sw1 != 0x90 || sw2 != 0x00)
        return OTPCard::OperationResult::INVALID_DATA;

    return OTPCard::OperationResult::SUCCESS;
}

std::tuple<OTPCard::OperationResult, bool, QByteArray, HashAlgorithm> OTPCard::getSecretInfo(int id)
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

OTPCard::OperationResult OTPCard::setSecret(int id, const QByteArray& name, const QByteArray& secret, HashAlgorithm method)
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
    auto [sw1, sw2, response] = nfc->sendAPDU(0x00, Command::SAVE_PIN, 0, 1, 0, apdu);
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
