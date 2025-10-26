#ifndef CARDINFO_H
#define CARDINFO_H

#include <QByteArray>

enum HashAlgorithm {
    NONE = 0,
    SHA1 = 1,
    SHA256 = 2,
    SHA512 = 3,
};

class CardInfo
{
public:
    CardInfo() {}
    CardInfo(const QByteArray& serial,
             size_t maxSecrets,
             size_t maxSecretNameLen,
             size_t maxSecretLen,
             bool sha1_support,
             bool sha256_support,
             bool sha512_support);

    QByteArray serial() const {return _serial;}
    size_t maxSecrets() const {return _maxSecrets;}
    size_t maxSecretNameLen() const {return _maxSecretNameLen;}
    size_t maxSecretLen() const {return _maxSecretLen;}
    bool getAlgorithmSupported(HashAlgorithm algo) const;
private:
    QByteArray _serial;
    size_t _maxSecrets;
    size_t _maxSecretNameLen;
    size_t _maxSecretLen;
    bool SHA1;
    bool SHA256;
    bool SHA512;
};

#endif // CARDINFO_H
