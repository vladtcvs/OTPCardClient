#ifndef OTPCLIENT_H
#define OTPCLIENT_H

#include <QMainWindow>
#include <QTimer>
#include <QDateTime>

#include <memory>

#include <otpcard.h>
#include <infc.h>
#include <totpsecret.h>
#include <totpsecretsmanager.h>

#include <widget_secret_edit.h>
#include <widget_show_totp.h>
#include <widget_secret_new.h>
#include <widget_secrets_list.h>
#include <widget_card_params.h>

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

    SecretEdit *secretEditWidget;
    ShowTOTP *showTOTPWidget;
    SecretsList *secretsListWidget;
    SecretNew *secretNewWidget;
    CardParams *cardParamsWidget;

    std::shared_ptr<PinManager> pin;
    std::shared_ptr<INFC> nfc;
    std::shared_ptr<OTPCard> card;
    std::shared_ptr<TOTPSecretsManager> secretsManager;
    int pinExpireSeconds;
    QDateTime pinEnterTime;
    int current_id;
    QString current_serial;

    const int settings_widget_id = 0;
    const int about_widget_id = 1;
    int card_info_widget_id;
    int secret_new_widget_id;
    int secrets_list_widget_id;
    int totp_widget_id;
    int secret_edit_widget_id;
private:
    int findEmptySlot();
    void noCardInfo();
    void fillCardInfo();
    void populateSecretsManager(const QString& serial);

    void fillOTP(const QString& serial, int id);
    void editSecret(const QString& serial, int id);

    bool pinExpired();
    void requestPIN();
};
#endif // OTPCLIENT_H
