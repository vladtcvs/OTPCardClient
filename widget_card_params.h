#ifndef WIDGET_CARD_PARAMS_H
#define WIDGET_CARD_PARAMS_H

#include <QWidget>

#include <otpcard.h>

namespace Ui {
class CardParams;
}

class CardParams : public QWidget
{
    Q_OBJECT

public:
    explicit CardParams(std::shared_ptr<OTPCard> card, QWidget *parent = nullptr);
    ~CardParams();
    void fillCardInfo();
private:
    void noCardInfo();
private:
    Ui::CardParams *ui;
    std::shared_ptr<OTPCard> card;
signals:
    void change_admin_pin();
    void change_pin();
    void unlock_pin();
    void reset_card();
};

#endif // WIDGET_CARD_PARAMS_H
