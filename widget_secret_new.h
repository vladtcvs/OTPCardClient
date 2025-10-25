#ifndef WIDGET_SECRET_NEW_H
#define WIDGET_SECRET_NEW_H

#include <QWidget>

#include <otpcard.h>
#include <totpsecretsmanager.h>
#include <pinmanager.h>

namespace Ui {
class SecretNew;
}

class SecretNew : public QWidget
{
    Q_OBJECT

public:
    explicit SecretNew(std::shared_ptr<TOTPSecretsManager> secretsManager,
                       std::shared_ptr<PinManager> pinManager,
                       std::shared_ptr<OTPCard> card,
                       QWidget *parent = nullptr);
    ~SecretNew();
private:
    QByteArray fromBase32(const QString& b32);
private:
    Ui::SecretNew *ui;
    std::shared_ptr<TOTPSecretsManager> secretsManager;
    std::shared_ptr<PinManager> pinManager;
    std::shared_ptr<OTPCard> card;
signals:
    void cancelClicked();
    void addNewSecret(const QString& name, const QString& display_name, const QByteArray& secret,
                      OTPCard::HashAlgorithm method, int digits, int time_shift, int period);
};

#endif // WIDGET_SECRET_NEW_H
