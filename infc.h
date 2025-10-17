#ifndef INFC_H
#define INFC_H

#include <QByteArray>
#include <tuple>

class INFC
{
public:
    virtual ~INFC() {}
    virtual bool selectOTPCard() = 0;
    virtual std::tuple<int, int, QByteArray> sendAPDU(uint8_t CLA, uint8_t INS, uint8_t P1, uint8_t P2, uint8_t Le, const QByteArray& payload) = 0;
    virtual bool start() = 0;
    virtual void finish() = 0;
};

#endif // INFC_H
