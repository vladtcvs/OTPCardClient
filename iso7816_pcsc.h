#ifndef ISO7816_PCSC_H
#define ISO7816_PCSC_H

#include <infc.h>
#include <winscard.h>

class ISO7816_PCSC : public INFC
{
public:
    ISO7816_PCSC();
    virtual ~ISO7816_PCSC();
    bool selectOTPCard() override;
    std::tuple<int, int, QByteArray> sendAPDU(uint8_t CLA, uint8_t INS, uint8_t P1, uint8_t P2, uint8_t Le, const QByteArray& payload) override;
    bool start() override;
    void finish() override;
private:
    QByteArray apdu(const QByteArray& data);
    void _finish();
private:
    SCARDCONTEXT context;
    SCARDHANDLE card;
    bool connected;
};

#endif // ISO7816_PCSC_H
