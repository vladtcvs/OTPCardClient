#ifndef SECRET_NEW_H
#define SECRET_NEW_H

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
    int findEmptySlot();
private:
    Ui::SecretNew *ui;
    std::shared_ptr<TOTPSecretsManager> secretsManager;
    std::shared_ptr<PinManager> pinManager;
    std::shared_ptr<OTPCard> card;
signals:
    void cancelClicked();
    void addNewSecret(const QString& serial, int id, const QString& name, const QString& display_name,
                      OTPCard::HashAlgorithm method, int digits, int time_shift, int period);
};

#endif // SECRET_NEW_H
