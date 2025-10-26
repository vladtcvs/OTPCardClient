#ifndef WIDGET_SECRETS_LIST_H
#define WIDGET_SECRETS_LIST_H

#include <QWidget>
#include <totpsecretsmanager.h>
#include <pinmanager.h>
#include <QStandardItemModel>

namespace Ui {
class SecretsList;
}

class SecretsList : public QWidget
{
    Q_OBJECT

public:
    explicit SecretsList(std::shared_ptr<TOTPSecretsManager> secretsManager,
                         std::shared_ptr<PinManager> pinManager,
                         std::shared_ptr<OTPCard> card,
                         QWidget *parent = nullptr);
    ~SecretsList();

    void listSecrets(const QString& serial);
private:
    QString hash_method_name(HashAlgorithm method);
private:
    Ui::SecretsList *ui;
    std::shared_ptr<TOTPSecretsManager> secretsManager;
    std::shared_ptr<PinManager> pinManager;
    std::shared_ptr<OTPCard> card;
    QStandardItemModel *secretsModel;
signals:
    void newSecretClicked();
    void deleteSecretClicked(const QString& serial, int id);
    void editSecretClicked(const QString& serial, int id);
    void generateTOTPClicked(const QString& serial, int id);
};

#endif // WIDGET_SECRETS_LIST_H
