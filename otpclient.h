#ifndef OTPCLIENT_H
#define OTPCLIENT_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class OTPClient;
}
QT_END_NAMESPACE

class OTPClient : public QMainWindow
{
    Q_OBJECT

public:
    OTPClient(QWidget *parent = nullptr);
    ~OTPClient();

private:
    Ui::OTPClient *ui;
};
#endif // OTPCLIENT_H
