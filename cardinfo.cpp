#include "cardinfo.h"

CardInfo::CardInfo(const QByteArray& serial,
                   size_t maxSecrets,
                   size_t maxSecretNameLen,
                   size_t maxSecretLen,
                   bool sha1_support,
                   bool sha256_support,
                   bool sha512_support)
{
    this->SHA1 = sha1_support;
    this->SHA256 = sha256_support;
    this->SHA512 = sha512_support;
    this->_serial = serial;
    this->_maxSecrets = maxSecrets;
    this->_maxSecretLen = maxSecretLen;
    this->_maxSecretNameLen = maxSecretNameLen;
}

bool CardInfo::getAlgorithmSupported(HashAlgorithm algo) const
{
    switch (algo) {
    case HashAlgorithm::SHA1:
        return SHA1;
    case HashAlgorithm::SHA256:
        return SHA256;
    case HashAlgorithm::SHA512:
        return SHA512;
    default:
        return false;
    }
}
