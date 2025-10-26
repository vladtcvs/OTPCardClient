#include "otpclient.h"
#include "ui_otpclient.h"

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

    current_id = -1;
    current_serial = "";

    secretsManager = std::make_shared<TOTPSecretsManager>();

    pin = std::make_shared<PinManager>();
    pin->setPin("123456");

    nfc = std::make_shared<ISO7816_PCSC>();
    card = std::make_shared<OTPCard>(nfc, pin);

    // Create widgets for pages

    // Secrets List
    secretsListWidget = new SecretsList(secretsManager, pin, card, this);
    secrets_list_widget_id = ui->pages->addWidget(secretsListWidget);
    connect(secretsListWidget, &SecretsList::newSecretClicked, this, [this]() {
        ui->update->setEnabled(false);
        ui->pages->setCurrentIndex(secret_new_widget_id);
    });
    connect(secretsListWidget, &SecretsList::deleteSecretClicked, this, [this](const QString& serial, int id) {
        switch (card->Auth())
        {
        case OTPCard::OperationResult::SUCCESS:
            card->deleteSecret(id);
            secretsManager->deleteSecret(serial, id);
            populateSecretsManager(serial);
            secretsListWidget->listSecrets(serial);
            break;
        default:
            break;
        }
    });
    connect(secretsListWidget, &SecretsList::editSecretClicked, this, [this](const QString& serial, int id) {
        secretEditWidget->editSecret(serial, id);
        ui->pages->setCurrentIndex(secret_edit_widget_id);
    });
    connect(secretsListWidget, &SecretsList::generateTOTPClicked, this, [this](const QString& serial, int id) {
        showTOTPWidget->generateTOTP(serial, id);
        ui->pages->setCurrentIndex(totp_widget_id);
    });

    // New Secret
    secretNewWidget = new SecretNew(secretsManager, pin, card, this);
    secret_new_widget_id = ui->pages->addWidget(secretNewWidget);
    connect(secretNewWidget, &SecretNew::cancelClicked, this, [this]() {
        ui->update->setEnabled(true);
        ui->pages->setCurrentIndex(secrets_list_widget_id);
    });
    connect(secretNewWidget, &SecretNew::addNewSecret, this, [this](const QString& name, const QString& display_name, const QByteArray& secret,
                                                                    HashAlgorithm method, int digits, int time_shift, int period) {
        auto [res, info] = card->getCardInfo();
        auto serial = info.serial();

        res = card->Auth();
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
            card->setSecret(new_secret_id, name.toUtf8(), secret, method);
            std::shared_ptr<TOTPSecret> new_secret = std::make_shared<TOTPSecret>(serial,
                                                                                  new_secret_id,
                                                                                  display_name,
                                                                                  name,
                                                                                  method,
                                                                                  digits,
                                                                                  period,
                                                                                  time_shift);
            secretsManager->addSecret(new_secret);
        }

        ui->update->setEnabled(true);
        ui->pages->setCurrentIndex(secrets_list_widget_id);

        populateSecretsManager(serial);
        secretsListWidget->listSecrets(serial);
    });

    // show TOTP
    showTOTPWidget = new ShowTOTP(secretsManager, pin, card, this);
    totp_widget_id = ui->pages->addWidget(showTOTPWidget);
    connect(showTOTPWidget, &ShowTOTP::editClicked, this, [this](const QString& serial, int id) {
        secretEditWidget->editSecret(serial, id);
        ui->pages->setCurrentIndex(secret_edit_widget_id);
    });

    // Edit secret parameters
    secretEditWidget = new SecretEdit(secretsManager, this);
    secret_edit_widget_id = ui->pages->addWidget(secretEditWidget);
    connect(secretEditWidget, &SecretEdit::deleteClicked, this, [this](const QString& serial, int id) {
        secretsManager->deleteSecret(serial, id);
        ui->pages->setCurrentIndex(secrets_list_widget_id);
        ui->update->setEnabled(true);
        populateSecretsManager(serial);
        secretsListWidget->listSecrets(serial);
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
        populateSecretsManager(serial);
        secretsListWidget->listSecrets(serial);
    });

    // Card parameters
    cardParamsWidget = new CardParams(card, this);
    card_info_widget_id = ui->pages->addWidget(cardParamsWidget);
    connect(cardParamsWidget, &CardParams::change_pin, this, [this](){

    });
    connect(cardParamsWidget, &CardParams::change_admin_pin, this, [this](){

    });
    connect(cardParamsWidget, &CardParams::unlock_pin, this, [this](){

    });
    connect(cardParamsWidget, &CardParams::reset_card, this, [this](){

    });

    // Menu actions
    connect(home, &QAction::triggered, this, [this]() {
        auto [res, info] = card->getCardInfo();
        auto serial = info.serial();

        ui->pages->setCurrentIndex(secrets_list_widget_id);
        ui->update->setEnabled(true);
        secretsListWidget->listSecrets(serial);
    });

    connect(settings, &QAction::triggered, this, [this]() {
        ui->pages->setCurrentIndex(settings_widget_id);
    });

    connect(otpCardInfo, &QAction::triggered, this, [this]() {
        cardParamsWidget->fillCardInfo();
        ui->pages->setCurrentIndex(card_info_widget_id);
        ui->update->setEnabled(true);
    });

    connect(about, &QAction::triggered, this, [this]() {
        ui->pages->setCurrentIndex(about_widget_id);
    });

    connect(ui->update, &QToolButton::clicked, this, [this]() {
        if (card == nullptr)
            return;
        auto [res, info] = card->getCardInfo();
        auto serial = info.serial();

        int index = ui->pages->currentIndex();
        if (index == secrets_list_widget_id) {
            secretsListWidget->listSecrets(serial);
        } else if (index == totp_widget_id) {
            showTOTPWidget->generateTOTP(serial, current_id);
        } else if (index == card_info_widget_id) {
            cardParamsWidget->fillCardInfo();
        }
    });

    ui->menuButton->setStyleSheet("QToolButton::menu-indicator { image: none; }");
    ui->menuButton->setMenu(mainMenu);

    ui->pages->setCurrentIndex(secrets_list_widget_id);
    auto [res, info] = card->getCardInfo();
    auto serial = info.serial();

    populateSecretsManager(serial);
    secretsListWidget->listSecrets(serial);
}

void OTPClient::populateSecretsManager(const QString& serial)
{
    auto [res, info] = card->getCardInfo();

    res = card->Auth();
    switch (res) {
    case OTPCard::OperationResult::SUCCESS:
        break;
    default:
        return;
    }

    size_t max_secrets = info.maxSecrets();
    QList<int> used_ids;
    for (size_t i = 0; i < max_secrets; i++) {
        auto [res, used, name, method] = card->getSecretInfo(i);
        switch (res) {
        case OTPCard::OperationResult::SUCCESS:
            break;
        default:
            return;
        }

        if (!used)
            continue;

        used_ids.append(i);
        auto secret = secretsManager->getSecret(serial, i);
        if (secret == nullptr) {
            // New secret on card - create new
            secret = std::make_shared<TOTPSecret>(serial, i, name, name, method, 6, 30, 0);
            secretsManager->addSecret(secret);
        } else if (secret->getName() != name || secret->getMethod() != method) {
            // Method on card doesn't match local storage - remove old and create new
            secretsManager->deleteSecret(serial, i);
            secret = std::make_shared<TOTPSecret>(serial, i, name, name, method, 6, 30, 0);
            secretsManager->addSecret(secret);
        }
    }

    // Remove secrets which not present on card
    for (auto secret : secretsManager->getSecretsForCard(serial)) {
        int id = secret->getId();
        if (!used_ids.contains(id))
            secretsManager->deleteSecret(serial, id);
    }
}

int OTPClient::findEmptySlot()
{
    auto [res, info] = card->getCardInfo();

    res = card->Auth();
    switch (res) {
    case OTPCard::OperationResult::SUCCESS:
        break;
    default:
        return -1;
    }

    size_t maxSecrets = info.maxSecrets();
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


OTPClient::~OTPClient()
{
    delete cardParamsWidget;
    delete secretsListWidget;
    delete secretNewWidget;
    delete showTOTPWidget;
    delete secretEditWidget;
    delete ui;
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
