#include "widget_show_totp.h"
#include "ui_widget_show_totp.h"

ShowTOTP::ShowTOTP(std::shared_ptr<TOTPSecretsManager> secretsManager,
                   std::shared_ptr<PinManager> pinManager,
                   std::shared_ptr<OTPCard> card,
                   QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ShowTOTP)
    , secretsManager(secretsManager)
    , pinManager(pinManager)
    , card(card)
{
    ui->setupUi(this);
    connect(ui->secret_edit, &QPushButton::clicked, this, [this]() {
        emit editClicked(current_serial, current_id);
    });
}

ShowTOTP::~ShowTOTP()
{
    delete ui;
}

void ShowTOTP::generateTOTP(const QString& serial, int id)
{
    current_serial = serial;
    current_id = id;
    std::shared_ptr<TOTPSecret> secret = secretsManager->getSecret(serial, id);
    if (secret == nullptr)
    {
        qDebug() << "Error: can not find secret in storage";
        return;
    }
    if (!pinManager->isValid())
    {
        qDebug() << "Invalid pin";
        return;
    }

    QPair<QByteArray, int> res = secret->generateChallenge();
    QByteArray challenge = res.first;
    int timeout = res.second;
    qDebug() << "challenge: " << challenge.toHex(' ');
    switch (card->Auth()) {
    case OTPCard::OperationResult::INVALID_PIN:
        pinManager->invalid();
        return;
    case OTPCard::OperationResult::SUCCESS:
        break;
    default:
        return;
    }

    auto [result, hmac] = card->calculateHMAC(secret->getId(), challenge);
    switch (result) {
    case OTPCard::OperationResult::SUCCESS:
    {
        QString TOTP = secret->TOTP(hmac);
        ui->OTP_VAL->setText(TOTP);
        ui->OTP_timeout->setText(QString("%1 sec").arg(timeout));
        ui->secret_name->setText(secret->getDisplayName());
        break;
    }
    default:
        break;
    }
}
