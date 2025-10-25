#include "widget_secret_new.h"
#include "ui_widget_secret_new.h"

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
        QString secretName = ui->newSecretName->text();
        QString secretB32 = ui->newSecretValue->text();
        QByteArray secret =  fromBase32(secretB32);
        OTPCard::HashAlgorithm method = OTPCard::HashAlgorithm::SHA1;
        emit addNewSecret(secretName, secretName, secret, method, 6, 0, 30);
        ui->newSecretName->setText("");
        ui->newSecretValue->setText("");
    });
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
