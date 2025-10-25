#ifndef WIDGET_SECRET_LIST_ITEM_H
#define WIDGET_SECRET_LIST_ITEM_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class SecretListItem;
}
QT_END_NAMESPACE

class SecretListItem : public QWidget
{
    Q_OBJECT
public:
    explicit SecretListItem(QWidget *parent = nullptr);
    void fillContent(const QString& name, const QString& method);

protected:
    void mousePressEvent(QMouseEvent *event) override;
private:
    Ui::SecretListItem *ui;
signals:
    void secretEditClicked();
    void secretRemoveClicked();
    void secretSelected();
};

#endif // WIDGET_SECRET_LIST_ITEM_H
