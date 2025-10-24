#include "secrets_list.h"
#include "ui_secrets_list.h"

#include <secret_list_item.h>

SecretsList::SecretsList(std::shared_ptr<TOTPSecretsManager> secretsManager,
                         std::shared_ptr<PinManager> pinManager,
                         std::shared_ptr<OTPCard> card,
                         QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SecretsList)
    , secretsManager(secretsManager)
    , pinManager(pinManager)
    , card(card)
{
    ui->setupUi(this);
    secretsModel = new QStandardItemModel(this);
    ui->secrets->setModel(secretsModel);
    connect(ui->newSecret, &QPushButton::clicked, this, [this](){
        emit newSecretClicked();
    });
}

SecretsList::~SecretsList()
{
    delete secretsModel;
    delete ui;
}

QString SecretsList::hash_method_name(OTPCard::HashAlgorithm method)
{
    switch (method) {
    case OTPCard::HashAlgorithm::SHA1:
        return "SHA1";
    case OTPCard::HashAlgorithm::SHA256:
        return "SHA-256";
    case OTPCard::HashAlgorithm::SHA512:
        return "SHA-512";
    default:
        return "N/A";
    }
}

void SecretsList::listSecrets()
{
    auto res = card->Auth();
    switch (res) {
    case OTPCard::OperationResult::SUCCESS:
        break;
    case OTPCard::OperationResult::NO_CONNECTION:
        secretsModel->clear();
        return;
    default:
        return;
    }

    QString serial = card->getSerial();
    size_t maxSecrets = card->getMaxSecrets();
    qDebug() << "Amount of secrets: " << maxSecrets;
    secretsModel->clear();

    QList<int> secret_ids;
    for (size_t i = 0; i < maxSecrets; i++) {
        qDebug() << "Querying secret #" << i;
        auto [status, used, secret_name, method] = card->getSecretInfo(i);
        qDebug() << "Status = " << status;
        switch (status) {
        case OTPCard::OperationResult::SUCCESS:
            if (used) {
                secret_ids.append(i);
                auto item = new SecretListItem(this);
                auto secret = secretsManager->getSecret(serial, i);
                if (secret == nullptr)
                {
                    secret = std::make_shared<TOTPSecret>(serial, i, secret_name, secret_name, method);
                    secretsManager->addSecret(secret);
                }

                item->fillContent(secret->getDisplayName(), hash_method_name(method));

                auto model_item = new QStandardItem();
                model_item->setSizeHint(QSize(0, 48));
                secretsModel->appendRow(model_item);

                connect(item, &SecretListItem::secretEditClicked, this, [this, serial, i]() {
                    emit editSecretClicked(serial, i);
                });
                connect(item, &SecretListItem::secretRemoveClicked, this, [this, serial, i]() {
                    emit deleteSecretClicked(serial, i);
                });
                connect(item, &SecretListItem::secretSelected, this, [this, serial, i]() {
                    emit generateTOTPClicked(serial, i);
                });
                ui->secrets->setIndexWidget(secretsModel->index(i, 0), item);
            }
            break;
        default:
            break;
        }
    }

    // Remove secrets not present on card, from storage
    auto stored_secrets = secretsManager->getSecretsForCard(serial);
    for(auto secret : stored_secrets) {
        int id = secret->getId();
        if (!secret_ids.contains(id))
            secretsManager->deleteSecret(serial, id);
    }
}
