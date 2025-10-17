#include "nfcget.h"

NFCGet::NFCGet(const std::shared_ptr<INFC> &nfc)
{
    connected = false;
    this->nfc = nfc;
    if (!this->nfc->start())
        return;
    connected = nfc->selectOTPCard();
}

NFCGet::~NFCGet()
{
    this->nfc->finish();
}
