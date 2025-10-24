#include "secret_new.h"
#include "ui_secret_new.h"

SecretNew::SecretNew(std::shared_ptr<TOTPSecretsManager> secretsManager,
                     std::shared_ptr<PinManager> pinManager,
                     std::shared_ptr<OTPCard> card,
                     QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SecretNew)
    , secretsManager(secretsManager)
    , pinManager(pinManager)
    , card(card)
{
    ui->setupUi(this);
    connect(ui->cancelAdd, &QPushButton::clicked, this, [this]() {
        ui->newSecretName->setText("");
        ui->newSecretValue->setText("");
        emit cancelClicked();
    });

    connect(ui->confirmAdd, &QPushButton::clicked, this, [this, card]() {
        auto res = card->Auth();
        switch (res) {
        case OTPCard::OperationResult::INVALID_DATA:
        case OTPCard::OperationResult::NO_CONNECTION:
        case OTPCard::OperationResult::INVALID_PIN:
            return;
        default:
            break;
        }

        int new_secret_id = findEmptySlot();
        if (new_secret_id != -1) {
            QString serial = card->getSerial();
            QString secretName = ui->newSecretName->text();
            QString secretB32 = ui->newSecretValue->text();
            QByteArray secret =  fromBase32(secretB32);
            OTPCard::HashAlgorithm method = OTPCard::HashAlgorithm::SHA1;
            card->setSecret(new_secret_id, secretName.toLatin1(), secret, method);
            emit addNewSecret(serial, new_secret_id, secretName, secretName, method, 6, 0, 30);
        }

        ui->newSecretName->setText("");
        ui->newSecretValue->setText("");
    });
}

int SecretNew::findEmptySlot()
{
    auto res = card->Auth();
    switch (res) {
    case OTPCard::OperationResult::SUCCESS:
        break;
    default:
        return -1;
    }

    size_t maxSecrets = card->getMaxSecrets();
    for (size_t i = 0; i < maxSecrets; i++) {
        qDebug() << "Querying secret #" << i;
        auto [status, used, secret_name, method] = card->getSecretInfo(i);
        switch (status) {
        case OTPCard::OperationResult::SUCCESS:
            break;
        default:
            return -1;
        }
        if (used == false)
            return i;
    }
    return -1;
}

QByteArray SecretNew::fromBase32(const QString& b32)
{
    QByteArray input = b32.toLatin1();
    QByteArray output;
    unsigned int acc = 0;
    size_t bits = 0;
    for (char c : input) {
        unsigned v;
        if (c == '=')
            break;
        else if (c >= 'A' && c <= 'Z')
            v = c - 'A';
        else if (c >= '2' && c <= '7')
            v = 26 + (c - '2');
        else
            break;

        acc = (acc << 5) | v;
        bits += 5;
        if (bits >= 8) {
            unsigned char data = (acc >> (bits - 8)) & 0xFFU;
            bits -= 8;
            output.append(data);
        }
    }
    return output;
}


SecretNew::~SecretNew()
{
    delete ui;
}
