#ifndef SECRET_EDIT_H
#define SECRET_EDIT_H

#include <QWidget>

#include <QString>
#include <memory>
#include <totpsecretsmanager.h>
#include <otpcard.h>

namespace Ui {
class SecretEdit;
}

class SecretEdit : public QWidget
{
    Q_OBJECT

public:
    explicit SecretEdit(std::shared_ptr<TOTPSecretsManager> secretsManager, QWidget *parent = nullptr);
    ~SecretEdit();
    void editSecret(const QString& serial, int id);
private:
    QString hash_method_name(OTPCard::HashAlgorithm method);
private:
    Ui::SecretEdit *ui;
    std::shared_ptr<TOTPSecretsManager> secretsManager;
    QString current_serial;
    int current_id;
signals:
    void saveClicked(const QString& serial, int id, const QString& display_name, int digits, int time_shift, int period);
    void deleteClicked(const QString& serial, int id);
};

#endif // SECRET_EDIT_H
