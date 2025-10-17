#include <QDebug>
#include "iso7816_pcsc.h"

ISO7816_PCSC::ISO7816_PCSC()
{
}

ISO7816_PCSC::~ISO7816_PCSC()
{
    _finish();
}

bool ISO7816_PCSC::selectOTPCard()
{
    const unsigned char AID[] = {0xA0, 0x00, 0x00, 0x00, 0x02, 0x02, 0x01, 0x01};
    auto [sw1, sw2, res] = sendAPDU(0, 0xA4, 0x04, 0x00, 0, QByteArray(reinterpret_cast<const char*>(AID), sizeof(AID)));
    return (sw1 == 0x90 && sw2 == 0x00);
}

std::tuple<int, int, QByteArray> ISO7816_PCSC::sendAPDU(uint8_t CLA, uint8_t INS, uint8_t P1, uint8_t P2, uint8_t Le, const QByteArray& payload)
{
    QByteArray send;
    send.append(CLA);
    send.append(INS);
    send.append(P1);
    send.append(P2);
    int Lc = payload.length();
    if (Lc == 0) {
        send.append(1);
        send.append((char)0);
    } else {
        send.append(Lc);
        send.append(payload);
    }
    if (Le > 0)
        send.append(Le);

    QByteArray data = apdu(send);
    if (data.length() < 2)
    {
        return std::make_tuple(0,0, QByteArray());
    }
    int sw1 = (unsigned char)data[data.length()-2];
    int sw2 = (unsigned char)data[data.length()-1];
    return std::make_tuple(sw1, sw2, data.slice(0, data.length()-2));
}

bool ISO7816_PCSC::start()
{
    LONG result = SCardEstablishContext(SCARD_SCOPE_SYSTEM, nullptr, nullptr, &context);
    if (result != SCARD_S_SUCCESS) {
        qWarning() << "Failed to establish context:" << result;
        return false;
    }

    // List readers
    char readers[4096] = {0};
    size_t readersLen = sizeof(readers) - 1;
    result = SCardListReaders(context, nullptr, readers, &readersLen);
    if (result != SCARD_S_SUCCESS) {
        qWarning() << "No readers found:" << QString("%1").arg(result, 8, 16);;
        return false;
    }

    QStringList readerList;
    size_t begin = 0;
    while (begin < sizeof(readers)) {
        if (readers[begin] == 0) {
            begin++;
            continue;
        }
        size_t end;
        for (end = begin + 1; end < sizeof(readers); end++) {
            if (readers[end] == 0)
                break;
        }
        QString reader(readers + begin);
        if (reader.length() > 0)
            readerList.append(reader);
        begin = end + 1;
    }

    if (readerList.isEmpty()) {
        qWarning() << "No smartcard readers detected.";
        return false;
    }

    for (auto reader : readerList) {
        qDebug() << "Using reader:" << reader;

        // Connect
        size_t activeProtocol;
        result = SCardConnect(context, reader.toUtf8().data(),
                              SCARD_SHARE_SHARED,
                              SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
                              &card,
                              &activeProtocol);
        if (result != SCARD_S_SUCCESS) {
            qWarning() << "Cannot connect to card:" << QString("%1").arg(result, 8, 16);
        } else {
            connected = true;
            return true;
        }
    }
    return false;
}

void ISO7816_PCSC::_finish()
{
    if (connected) {
        SCardDisconnect(card, SCARD_LEAVE_CARD);
        SCardReleaseContext(context);
    }
    connected = false;
}

void ISO7816_PCSC::finish()
{
    _finish();
}

QByteArray ISO7816_PCSC::apdu(const QByteArray& data)
{
    if (!connected)
        return QByteArray();

    unsigned char resp[256];
    size_t respLen = sizeof(resp);
    auto result = SCardTransmit(card, SCARD_PCI_T1, reinterpret_cast<const unsigned char*>(data.data()), data.length(), nullptr, resp, &respLen);
    if (result == SCARD_S_SUCCESS) {
        qDebug() << "Response:" << QByteArray(reinterpret_cast<char*>(resp), respLen).toHex(' ');
    } else {
        qWarning() << "Transmit failed:" << QString("%1").arg(result, 8, 16);
    }

    return QByteArray(reinterpret_cast<const char*>(resp), respLen);
}
