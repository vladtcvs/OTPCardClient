#include "otpclient.h"
#include "ui_otpclient.h"

#include "secret_list_item.h"
#include <ctype.h>

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

    connect(ui->newSecret, &QPushButton::clicked, this, [this]() {
        ui->pages->setCurrentIndex(5);
        ui->update->setEnabled(false);
    });

    connect(ui->cancelAdd, &QPushButton::clicked, this, [this]() {
        ui->pages->setCurrentIndex(0);
        ui->update->setEnabled(true);
        ui->newSecretName->setText("");
        ui->newSecretValue->setText("");
    });

    connect(ui->confirmAdd, &QPushButton::clicked, this, [this]() {
        if (!pinValid)
            return;

        auto res = card->Auth(PIN.toLatin1());
        switch (res) {
        case OTPCard::OperationResult::INVALID_DATA:
            return;
        case OTPCard::OperationResult::NO_CONNECTION:
            return;
        case OTPCard::OperationResult::INVALID_PIN:
            pinValid = false;
            return;
        default:
            break;
        }

        if (!pinValid)
            return;
        int new_secret_id = findEmptySlot();
        if (new_secret_id != -1) {
            QString secretName = ui->newSecretName->text();
            QString secretB32 = ui->newSecretValue->text();
            QByteArray secret =  fromBase32(secretB32);
            OTPCard::HashAlgorithm method = OTPCard::HashAlgorithm::SHA1;
            card->setSecret(new_secret_id, secretName.toLatin1(), secret, method);
        }

        ui->pages->setCurrentIndex(0);
        ui->update->setEnabled(true);
        ui->newSecretName->setText("");
        ui->newSecretValue->setText("");
        listSecrets();
    });

    ui->menuButton->setStyleSheet("QToolButton::menu-indicator { image: none; }");
    ui->menuButton->setMenu(mainMenu);

    nfc = std::make_shared<ISO7816_PCSC>();
    card = std::make_shared<OTPCard>(nfc);

    listSecrets();
}

OTPClient::~OTPClient()
{
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
    if (!pinValid) {
        return -1;
    }

    auto res = card->Auth(PIN.toLatin1());
    switch (res) {
    case OTPCard::OperationResult::INVALID_DATA:
        return -1;
    case OTPCard::OperationResult::NO_CONNECTION:
        return -1;
    case OTPCard::OperationResult::INVALID_PIN:
        pinValid = false;
        return -1;
    default:
        break;
    }

    size_t maxSecrets = card->getMaxSecrets();
    for (size_t i = 0; i < maxSecrets; i++) {
        if (!pinValid) {
            break;
        }
        qDebug() << "Querying secret #" << i;
        auto [status, used, secret_name, method] = card->getSecretInfo(i);
        switch (status) {
        case OTPCard::OperationResult::INVALID_DATA:
            return -1;
        case OTPCard::OperationResult::NO_CONNECTION:
            return -1;
        case OTPCard::OperationResult::INVALID_PIN:
            pinValid = false;
            return -1;
        default:
            break;
        }
        if (used == false)
            return i;
    }
    return -1;
}

void OTPClient::fillOTP(int id)
{
    std::shared_ptr<TOTPSecret> secret = secrets[id];
    if (pinValid) {
        QPair<QByteArray, int> res = secret->generateChallenge();
        QByteArray challenge = res.first;
        int timeout = res.second;
        qDebug() << "challenge: " << challenge.toHex(' ');
        switch (card->Auth(PIN.toLatin1())) {
        case OTPCard::OperationResult::INVALID_PIN:
            pinValid = false;
            return;
        case OTPCard::OperationResult::SUCCESS:
            break;
        default:
            return;
        }

        auto [result, hmac] = card->calculateHMAC(secret->getId(), challenge);
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
                    QString TOTP = secret->TOTP(hmac);
                    ui->OTP_VAL->setText(TOTP);
                    ui->OTP_timeout->setText(QString("%1 sec").arg(timeout));
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
    qDebug() << "Amount of secrets: " << maxSecrets;
    secretsModel->clear();
    secrets.clear();
    for (size_t i = 0; i < maxSecrets; i++) {
        if (!pinValid) {
            break;
        }
        qDebug() << "Querying secret #" << i;
        auto [status, used, secret_name, method] = card->getSecretInfo(i);
        qDebug() << "Status = " << status;
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
                TOTPSecret *secret = new TOTPSecret(i, secret_name, method);
                secrets[i] = std::shared_ptr<TOTPSecret>(secret);

                connect(item, &SecretListItem::secretEditClicked, this, [this]() {

                });
                connect(item, &SecretListItem::secretRemoveClicked, this, [this, i]() {
                    if (pinValid) {
                        switch (card->Auth(PIN.toLatin1()))
                        {
                            case OTPCard::OperationResult::SUCCESS:
                                card->deleteSecret(i);
                                listSecrets();
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
            qDebug() << "Error";
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
