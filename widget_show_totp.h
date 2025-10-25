#ifndef WIDGET_SHOW_TOTP_H
#define WIDGET_SHOW_TOTP_H

#include <QWidget>

#include <totpsecretsmanager.h>
#include <pinmanager.h>

namespace Ui {
class ShowTOTP;
}

class ShowTOTP : public QWidget
{
    Q_OBJECT

public:
    explicit ShowTOTP(std::shared_ptr<TOTPSecretsManager> secretsManager,
                      std::shared_ptr<PinManager> pinManager,
                      std::shared_ptr<OTPCard> card,
                      QWidget *parent = nullptr);
    ~ShowTOTP();
    void generateTOTP(const QString& serial, int id);

private:
    Ui::ShowTOTP *ui;
    QString current_serial;
    int current_id;
    std::shared_ptr<TOTPSecretsManager> secretsManager;
    std::shared_ptr<PinManager> pinManager;
    std::shared_ptr<OTPCard> card;

signals:
    void editClicked(const QString& serial, int id);
};

#endif // WIDGET_SHOW_TOTP_H
