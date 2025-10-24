#ifndef OTPCLIENT_H
#define OTPCLIENT_H

#include <QMainWindow>
#include <QTimer>
#include <QStandardItemModel>
#include <QDateTime>

#include <memory>

#include <otpcard.h>
#include <infc.h>
#include <totpsecret.h>
#include <totpsecretsmanager.h>

#include <secret_edit.h>
#include <show_totp.h>

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

    SecretEdit *secretEditWidget;
    ShowTOTP *showTOTPWidget;

    std::shared_ptr<PinManager> pin;
    std::shared_ptr<INFC> nfc;
    std::shared_ptr<OTPCard> card;
    std::shared_ptr<TOTPSecretsManager> secretsManager;
    int pinExpireSeconds;
    QDateTime pinEnterTime;
    int current_id;
    QString current_serial;

    const int secrets_list_widget_id = 0;
    const int settings_widget_id = 1;
    const int card_info_widget_id = 2;
    const int about_widget_id = 3;
    const int new_secret_widget_id = 4;
    int totp_widget_id;
    int secret_edit_widget_id;
private:
    QString hash_method_name(OTPCard::HashAlgorithm method);

    QByteArray fromBase32(const QString& b32);

    void noCardInfo();
    void fillCardInfo();
    void listSecrets();
    int findEmptySlot();

    void fillOTP(const QString& serial, int id);
    void editSecret(const QString& serial, int id);

    bool pinExpired();
    void requestPIN();
};
#endif // OTPCLIENT_H
