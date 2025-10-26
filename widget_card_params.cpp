#include "widget_card_params.h"
#include "ui_widget_card_params.h"

CardParams::CardParams(std::shared_ptr<OTPCard> card, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CardParams)
    , card(card)
{
    ui->setupUi(this);
    noCardInfo();
    connect(ui->resetCard, &QPushButton::clicked, this, [this](){
        emit reset_card();
    });
    connect(ui->changePin, &QPushButton::clicked, this, [this](){
        emit change_pin();
    });
    connect(ui->changeAdminPin, &QPushButton::clicked, this, [this](){
        emit change_admin_pin();
    });
    connect(ui->unlockCard, &QPushButton::clicked, this, [this](){
        emit unlock_pin();
    });
}

CardParams::~CardParams()
{
    delete ui;
}

void CardParams::noCardInfo()
{
    ui->maxSecretLength->setText("N/A");
    ui->maxSecretNameLength->setText("N/A");
    ui->maxSecrets->setText("N/A");
    ui->serialNumber->setText("N/A");
    ui->sha1support->setText("N/A");
    ui->sha256support->setText("N/A");
    ui->sha512support->setText("N/A");
}

void CardParams::fillCardInfo()
{
    if (!card) {
        noCardInfo();
        return;
    }

    if (!card->checkConnection())
    {
        noCardInfo();
        return;
    }

    auto [res, info] = card->getCardInfo();
    if (res != OTPCard::OperationResult::SUCCESS) {
        noCardInfo();
        return;
    }
    QByteArray serial = info.serial();
    size_t maxSecrets = info.maxSecrets();
    size_t maxSecretLength = info.maxSecretLen();
    size_t maxSecretNameLength = info.maxSecretNameLen();
    bool sha1 = info.getAlgorithmSupported(HashAlgorithm::SHA1);
    bool sha256 = info.getAlgorithmSupported(HashAlgorithm::SHA256);
    bool sha512 = info.getAlgorithmSupported(HashAlgorithm::SHA512);

    ui->maxSecretLength->setNum((int)maxSecretLength);
    ui->maxSecretNameLength->setNum((int)maxSecretNameLength);
    ui->maxSecrets->setNum((int)maxSecrets);
    ui->serialNumber->setText(serial.toHex());
    ui->sha1support->setText(sha1 ? "+" : "-");
    ui->sha256support->setText(sha256 ? "+" : "-");
    ui->sha512support->setText(sha512 ? "+" : "-");
}
