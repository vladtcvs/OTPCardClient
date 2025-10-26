#include "widget_secret_list_item.h"
#include "ui_widget_secret_list_item.h"

SecretListItem::SecretListItem(QWidget *parent)
    : QWidget{parent},
    ui(new Ui::SecretListItem)
{
    ui->setupUi(this);
    connect(ui->secret_edit, &QToolButton::clicked, this, [this]() {
        emit secretEditClicked();
    });

    connect(ui->secret_remove, &QToolButton::clicked, this, [this]() {
        emit secretRemoveClicked();
    });
}

void SecretListItem::fillContent(const QString& name, const QString& display_name, const QString& method)
{
    (void)method; // unused
    ui->secret_name->setText(name);
    ui->secret_display_name->setText(display_name);
}

void SecretListItem::mousePressEvent(QMouseEvent *event)
{
    emit secretSelected();
    QWidget::mousePressEvent(event);
}
