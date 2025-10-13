#include "otpclient.h"
#include "ui_otpclient.h"

OTPClient::OTPClient(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::OTPClient)
{
    ui->setupUi(this);
}

OTPClient::~OTPClient()
{
    delete ui;
}
