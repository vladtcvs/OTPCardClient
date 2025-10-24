#include "secret_edit.h"
#include "ui_secret_edit.h"

SecretEdit::SecretEdit(std::shared_ptr<TOTPSecretsManager> secretsManager, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SecretEdit)
    , secretsManager(secretsManager)
{
    ui->setupUi(this);
    connect(ui->save_secret, &QPushButton::clicked, this, [this]() {
        QString display_name = ui->secret_display_name->text();
        bool ok = true;
        int digits = ui->TOTP_digits->text().toInt(&ok);
        if (!ok)
            return;
        int shift = ui->TOTP_time_offset->text().toInt(&ok);
        if (!ok)
            return;
        int period = ui->TOTP_period->text().toInt(&ok);
        if (!ok)
            return;

        emit saveClicked(current_serial, current_id, display_name, digits, shift, period);
    });
    connect(ui->delete_secret, &QPushButton::clicked, this, [this]() {
        emit deleteClicked(current_serial, current_id);
    });
}

SecretEdit::~SecretEdit()
{
    delete ui;
}

QString SecretEdit::hash_method_name(OTPCard::HashAlgorithm method)
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

void SecretEdit::editSecret(const QString& serial, int id)
{
    current_id = id;
    current_serial = serial;
    auto secret = secretsManager->getSecret(serial, id);
    if(secret == nullptr)
    {
        qDebug() << "Can not find secret";
        return;
    }
    ui->secret_name->setText(secret->getName());
    ui->secret_display_name->setText(secret->getDisplayName());
    ui->secret_method->setText(hash_method_name(secret->getMethod()));
    ui->TOTP_digits->setText(QString::number(secret->getDigits()));
    ui->TOTP_time_offset->setText(QString::number(secret->getTimeShift()));
    ui->TOTP_period->setText(QString::number(secret->getSecondsPeriod()));
}
