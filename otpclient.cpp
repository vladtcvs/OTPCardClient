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
    secretsModel = new QStandardItemModel(this);
    ui->secrets->setModel(secretsModel);

    PIN = QString("123456");
    pinValid = true;

    connect(home, &QAction::triggered, this, [this]() {
        ui->pages->setCurrentIndex(0);
        ui->update->setEnabled(true);
        listSecrets();
    });

    connect(settings, &QAction::triggered, this, [this]() {
        ui->pages->setCurrentIndex(2);
    });

    connect(otpCardInfo, &QAction::triggered, this, [this]() {
        ui->pages->setCurrentIndex(3);
        ui->update->setEnabled(true);
        fillCardInfo();
    });

    connect(about, &QAction::triggered, this, [this]() {
        ui->pages->setCurrentIndex(4);
    });

    connect(ui->update, &QToolButton::clicked, this, [this]() {
        switch (ui->pages->currentIndex())
        {
        case 0:
            listSecrets();
            current_id = -1;
            break;
        case 1:
            fillOTP(current_id);
            break;
        case 3:
            fillCardInfo();
            current_id = -1;
            break;
        default:
            current_id = -1;
            break;
        }
    });

    ui->menuButton->setStyleSheet("QToolButton::menu-indicator { image: none; }");
    ui->menuButton->setMenu(mainMenu);

    nfc = std::make_shared<ISO7816_PCSC>();
    card = std::make_shared<OTPCard>(nfc);
}

OTPClient::~OTPClient()
{
    delete ui;
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

void OTPClient::fillOTP(int id)
{
    if (pinValid) {
        QDateTime asd(QDateTime::currentDateTime());
        unsigned long unixTime = asd.toSecsSinceEpoch();
        unsigned long challenge = unixTime / 30U;

        // Big Endian
        unsigned char challengeBytes[8];
        for (int i = 0; i < 8; i++)
            challengeBytes[i] = (challenge >> (64-8-i*8)) & 0xFFU;

        auto challengeArray = QByteArray(reinterpret_cast<const char*>(challengeBytes), sizeof(challengeBytes));
        qDebug() << "challenge: " << challengeArray.toHex(' ');
        switch (card->Auth(PIN.toLatin1())) {
        case OTPCard::OperationResult::INVALID_PIN:
            pinValid = false;
            return;
        case OTPCard::OperationResult::SUCCESS:
            break;
        default:
            return;
        }

        auto [result, hmac] = card->calculateHMAC(id, challengeArray);
        switch (result) {
            case OTPCard::OperationResult::INVALID_PIN:
                pinValid = false;
                break;
            case OTPCard::OperationResult::INVALID_DATA:
                break;
            case OTPCard::OperationResult::NO_CONNECTION:
                break;
            case OTPCard::OperationResult::SUCCESS:
                {
                    // Big Endian
                    size_t len = hmac.length();
                    unsigned offset = hmac[len-1] & 0x0FU;
                    unsigned long result = (((unsigned long)(unsigned char)hmac[offset]) << 24 |
                                            ((unsigned long)(unsigned char)hmac[offset+1]) << 16 |
                                            ((unsigned long)(unsigned char)hmac[offset+2]) << 8 |
                                            ((unsigned long)(unsigned char)hmac[offset+3])) & 0x7FFFFFFFUL;
                    qDebug() << "Result = " << result << QString("%1").arg(result, 8, 16);
                    int digits[6] = {
                        (int)((result / 100000) % 10),
                        (int)((result / 10000) % 10),
                        (int)((result / 1000) % 10),
                        (int)((result / 100) % 10),
                        (int)((result / 10) % 10),
                        (int)(result % 10),
                    };
                    QString TOTP1 = QString("%1%2%3").arg(digits[0]).arg(digits[1]).arg(digits[2]);
                    QString TOTP2 = QString("%1%2%3").arg(digits[3]).arg(digits[4]).arg(digits[5]);
                    ui->OTP1->setText(TOTP1);
                    ui->OTP2->setText(TOTP2);
                }
                break;
        }
    }
}

void OTPClient::listSecrets()
{
    bool error = false;
    auto res = card->Auth(PIN.toLatin1());
    switch (res) {
    case OTPCard::OperationResult::INVALID_DATA:
        error = true;
        return;
    case OTPCard::OperationResult::NO_CONNECTION:
        error = true;
        return;
    case OTPCard::OperationResult::INVALID_PIN:
        error = true;
        pinValid = false;
        return;
    default:
        break;
    }

    size_t maxSecrets = card->getMaxSecrets();
    secretsModel->clear();
    for (size_t i = 0; i < maxSecrets; i++) {
        if (!pinValid) {
            break;
        }
        qDebug() << "Querying secret #" << i;
        auto [status, used, secret_name, method] = card->getSecretInfo(i);
        switch (status) {
        case OTPCard::OperationResult::INVALID_DATA:
            error = true;
            break;
        case OTPCard::OperationResult::NO_CONNECTION:
            error = true;
            break;
        case OTPCard::OperationResult::INVALID_PIN:
            error = true;
            pinValid = false;
            break;
        case OTPCard::OperationResult::SUCCESS:
            if (used) {
                auto item = new SecretListItem(this);
                item->fillContent(secret_name, hash_method_name(method));
                auto model_item = new QStandardItem();
                model_item->setSizeHint(QSize(0, 48));
                secretsModel->appendRow(model_item);

                connect(item, &SecretListItem::secretEditClicked, this, [this]() {

                });
                connect(item, &SecretListItem::secretRemoveClicked, this, [this, i]() {
                    if (pinValid) {
                        switch (card->Auth(PIN.toLatin1()))
                        {
                            case OTPCard::OperationResult::SUCCESS:
                                card->deleteSecret(i);
                                break;
                            case OTPCard::OperationResult::INVALID_PIN:
                                pinValid = false;
                                break;
                            default:
                                break;
                        }
                    }
                });
                connect(item, &SecretListItem::secretSelected, this, [this, i]() {
                    ui->pages->setCurrentIndex(1);
                    current_id = i;
                    fillOTP(i);
                });
                ui->secrets->setIndexWidget(secretsModel->index(i, 0), item);
            }
            break;
        }

        if (!pinValid || error) {
            break;
        }
    }
}

bool OTPClient::pinExpired()
{
    return false;
    if (!pinValid)
        return true;
    QDateTime now = QDateTime::currentDateTime();
    return now > pinEnterTime.addSecs(pinExpireSeconds);
}

void OTPClient::requestPIN()
{

}
