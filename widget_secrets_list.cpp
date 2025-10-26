#include "widget_secrets_list.h"
#include "ui_widget_secrets_list.h"

#include <widget_secret_list_item.h>

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

QString SecretsList::hash_method_name(HashAlgorithm method)
{
    switch (method) {
    case HashAlgorithm::SHA1:
        return "SHA1";
    case HashAlgorithm::SHA256:
        return "SHA-256";
    case HashAlgorithm::SHA512:
        return "SHA-512";
    default:
        return "N/A";
    }
}

void SecretsList::listSecrets(const QString& serial)
{
    secretsModel->clear();

    int counter = 0;
    for (auto secret : secretsManager->getSecretsForCard(serial)) {
        qDebug() << "Querying secret #" << secret->getId();
        auto item = new SecretListItem(this);
        item->fillContent(secret->getName(), secret->getDisplayName(), hash_method_name(secret->getMethod()));

        auto model_item = new QStandardItem();
        model_item->setSizeHint(QSize(0, 48));
        secretsModel->appendRow(model_item);

        const int id = secret->getId();
        connect(item, &SecretListItem::secretEditClicked, this, [this, serial, id]() {
            emit editSecretClicked(serial, id);
        });
        connect(item, &SecretListItem::secretRemoveClicked, this, [this, serial, id]() {
            emit deleteSecretClicked(serial, id);
        });
        connect(item, &SecretListItem::secretSelected, this, [this, serial, id]() {
            emit generateTOTPClicked(serial, id);
        });
        ui->secrets->setIndexWidget(secretsModel->index(counter++, 0), item);
    }
}
