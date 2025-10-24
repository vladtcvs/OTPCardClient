#include "otpclient.h"
#include "ui_otpclient.h"

#include "secret_list_item.h"

#include <iso7816_pcsc.h>

OTPClient::OTPClient(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::OTPClient)
{
    ui->setupUi(this);
    QMenu *mainMenu = new QMenu();
    mainMenu->setStyleSheet("font-size: 15pt;");
    QAction *home = new QAction("Home", this);
    QAction *settings = new QAction("Settings", this);
    QAction *otpCardInfo = new QAction("Card info", this);
    QAction *about = new QAction("About", this);
    mainMenu->addAction(home);
    mainMenu->addAction(settings);
    mainMenu->addAction(otpCardInfo);
    mainMenu->addAction(about);
    ui->pages->setCurrentIndex(0);

    current_id = -1;
    current_serial = "";
    secretsModel = new QStandardItemModel(this);
    ui->secrets->setModel(secretsModel);

    secretsManager = std::make_shared<TOTPSecretsManager>();

    pin = std::make_shared<PinManager>();
    pin->setPin("123456");

    nfc = std::make_shared<ISO7816_PCSC>();
    card = std::make_shared<OTPCard>(nfc, pin);

    // Create widgets for pages

    // show TOTP
    showTOTPWidget = new ShowTOTP(secretsManager, pin, card, this);
    totp_widget_id = ui->pages->addWidget(showTOTPWidget);
    connect(showTOTPWidget, &ShowTOTP::editClicked, this, [this](const QString& serial, int id) {
        secretEditWidget->editSecret(serial, id);
        ui->pages->setCurrentIndex(secret_edit_widget_id);
    });

    connect(home, &QAction::triggered, this, [this]() {
        ui->pages->setCurrentIndex(secrets_list_widget_id);
        ui->update->setEnabled(true);
        listSecrets();
    });

    // Edit secret parameters
    secretEditWidget = new SecretEdit(secretsManager, this);
    secret_edit_widget_id = ui->pages->addWidget(secretEditWidget);
    connect(secretEditWidget, &SecretEdit::deleteClicked, this, [this](const QString& serial, int id) {
        secretsManager->deleteSecret(serial, id);
        ui->pages->setCurrentIndex(secrets_list_widget_id);
        ui->update->setEnabled(true);
        listSecrets();
    });

    connect(secretEditWidget, &SecretEdit::saveClicked, this, [this](const QString& serial, int id,
                                                                     const QString& display_name, int digits, int time_shift, int period) {
        auto secret = secretsManager->getSecret(serial, id);
        if (secret != nullptr) {
            secret->update_digits(digits);
            secret->update_period(period);
            secret->update_display_name(display_name);
            secret->update_timeShift(time_shift);
        }
        ui->pages->setCurrentIndex(secrets_list_widget_id);
        ui->update->setEnabled(true);
        listSecrets();
    });

    connect(settings, &QAction::triggered, this, [this]() {
        ui->pages->setCurrentIndex(settings_widget_id);
    });

    connect(otpCardInfo, &QAction::triggered, this, [this]() {
        ui->pages->setCurrentIndex(card_info_widget_id);
        ui->update->setEnabled(true);
        fillCardInfo();
    });

    connect(about, &QAction::triggered, this, [this]() {
        ui->pages->setCurrentIndex(about_widget_id);
    });

    connect(ui->update, &QToolButton::clicked, this, [this]() {
        if (card == nullptr)
            return;
        auto serial = card->getSerial();
        int index = ui->pages->currentIndex();
        if (index == secrets_list_widget_id) {
            listSecrets();
        } else if (index == totp_widget_id) {
            showTOTPWidget->generateTOTP(serial, current_id);
        } else if (index == card_info_widget_id) {
            fillCardInfo();
        }
    });

    connect(ui->newSecret, &QPushButton::clicked, this, [this]() {
        ui->pages->setCurrentIndex(new_secret_widget_id);
        ui->update->setEnabled(false);
    });

    connect(ui->cancelAdd, &QPushButton::clicked, this, [this]() {
        ui->pages->setCurrentIndex(secrets_list_widget_id);
        ui->update->setEnabled(true);
        ui->newSecretName->setText("");
        ui->newSecretValue->setText("");
    });

    connect(ui->confirmAdd, &QPushButton::clicked, this, [this]() {
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
            QString secretName = ui->newSecretName->text();
            QString secretB32 = ui->newSecretValue->text();
            QByteArray secret =  fromBase32(secretB32);
            OTPCard::HashAlgorithm method = OTPCard::HashAlgorithm::SHA1;
            card->setSecret(new_secret_id, secretName.toLatin1(), secret, method);
        }

        ui->pages->setCurrentIndex(secrets_list_widget_id);
        ui->update->setEnabled(true);
        ui->newSecretName->setText("");
        ui->newSecretValue->setText("");
        listSecrets();
    });

    ui->menuButton->setStyleSheet("QToolButton::menu-indicator { image: none; }");
    ui->menuButton->setMenu(mainMenu);


    listSecrets();
}

OTPClient::~OTPClient()
{
    delete showTOTPWidget;
    delete secretEditWidget;
    delete ui;
}

QByteArray OTPClient::fromBase32(const QString& b32)
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

void OTPClient::noCardInfo()
{
    ui->maxSecretLength->setText("N/A");
    ui->maxSecretNameLength->setText("N/A");
    ui->maxSecrets->setText("N/A");
    ui->serialNumber->setText("N/A");
    ui->sha1support->setText("N/A");
    ui->sha256support->setText("N/A");
    ui->sha512support->setText("N/A");
}

void OTPClient::fillCardInfo()
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

    QByteArray serial = card->getSerial();
    size_t maxSecrets = card->getMaxSecrets();
    size_t maxSecretLength = card->getMaxSecretValueLength();
    size_t maxSecretNameLength = card->getMaxSecretNameLength();
    bool sha1 = card->getAlgorithmSupported(OTPCard::HashAlgorithm::SHA1);
    bool sha256 = card->getAlgorithmSupported(OTPCard::HashAlgorithm::SHA256);
    bool sha512 = card->getAlgorithmSupported(OTPCard::HashAlgorithm::SHA512);

    ui->maxSecretLength->setNum((int)maxSecretLength);
    ui->maxSecretNameLength->setNum((int)maxSecretNameLength);
    ui->maxSecrets->setNum((int)maxSecrets);
    ui->serialNumber->setText(serial.toHex());
    ui->sha1support->setText(sha1 ? "+" : "-");
    ui->sha256support->setText(sha256 ? "+" : "-");
    ui->sha512support->setText(sha512 ? "+" : "-");
}

QString OTPClient::hash_method_name(OTPCard::HashAlgorithm method)
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

int OTPClient::findEmptySlot()
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



void OTPClient::listSecrets()
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
                    ui->pages->setCurrentIndex(secret_edit_widget_id);
                    secretEditWidget->editSecret(serial, i);
                });
                connect(item, &SecretListItem::secretRemoveClicked, this, [this, serial, i]() {
                    switch (card->Auth())
                    {
                        case OTPCard::OperationResult::SUCCESS:
                            card->deleteSecret(i);
                            secretsManager->deleteSecret(serial, i);
                            listSecrets();
                            break;
                        default:
                            break;
                    }
                });
                connect(item, &SecretListItem::secretSelected, this, [this, serial, i]() {
                    ui->pages->setCurrentIndex(totp_widget_id);
                    current_id = i;
                    current_serial = serial;
                    showTOTPWidget->generateTOTP(serial, i);
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

bool OTPClient::pinExpired()
{
    return false;
    if (!pin->isValid())
        return true;
    QDateTime now = QDateTime::currentDateTime();
    return now > pinEnterTime.addSecs(pinExpireSeconds);
}

void OTPClient::requestPIN()
{

}
