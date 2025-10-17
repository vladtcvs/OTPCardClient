#ifndef OTPCLIENT_H
#define OTPCLIENT_H

#include <QMainWindow>
#include <QTimer>
#include <QStandardItemModel>
#include <QDateTime>
#include <otpcard.h>
#include <infc.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class OTPClient;
}
QT_END_NAMESPACE

class OTPClient : public QMainWindow
{
    Q_OBJECT

public:
    OTPClient(QWidget *parent = nullptr);
    ~OTPClient();

private:
    Ui::OTPClient *ui;

    QStandardItemModel *secretsModel;

    std::shared_ptr<INFC> nfc;
    std::shared_ptr<OTPCard> card;
    QString PIN;
    bool pinValid;
    int pinExpireSeconds;
    QDateTime pinEnterTime;
    int current_id;
private:
    QString hash_method_name(OTPCard::HashAlgorithm method);

    void noCardInfo();
    void fillCardInfo();
    void listSecrets();

    void fillOTP(int id);

    bool pinExpired();
    void requestPIN();
};
#endif // OTPCLIENT_H
