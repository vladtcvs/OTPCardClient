#ifndef NFCGET_H
#define NFCGET_H

#include <infc.h>
#include <memory>

class NFCGet
{
public:
    explicit NFCGet(const std::shared_ptr<INFC> &nfc);
    NFCGet(const NFCGet& other) = delete;
    NFCGet(NFCGet&& other) = delete;
    NFCGet& operator = (const std::shared_ptr<INFC> &nfc) = delete;
    NFCGet& operator = (std::shared_ptr<INFC> &&nfc) = delete;
    ~NFCGet();
public:
    bool isConnected() const {return connected;}
private:
    std::shared_ptr<INFC> nfc;
    bool connected;
};

#endif // NFCGET_H
