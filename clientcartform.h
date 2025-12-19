#ifndef CLIENTCARTFORM_H
#define CLIENTCARTFORM_H

#include <QDialog>
#include <QTableWidgetItem>
#include "database.h"

namespace Ui {
class ClientCartForm;
}

class ClientCartForm : public QDialog
{
    Q_OBJECT

public:
    explicit ClientCartForm(int userId, QWidget *parent = nullptr);
    ~ClientCartForm();

    void loadCartItems();

signals:
    void cartUpdated();

private slots:
    void on_pbBuy_clicked();
    void on_twCart_itemDoubleClicked(QTableWidgetItem *item);

private:
    Ui::ClientCartForm *ui;
    int userId;
    QList<CartItem> cartItems;

    void setupTable();
    void updateTotals();
    void calculateDiscount();
};

#endif // CLIENTCARTFORM_H
