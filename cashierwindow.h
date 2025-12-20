#ifndef CASHIERWINDOW_H
#define CASHIERWINDOW_H

#include <QWidget>
#include <QStandardItemModel>
#include "database.h"
#include "cartobserver.h"

namespace Ui {
class CashierWindow;
}

class CashierWindow : public QWidget
{
    Q_OBJECT

public:
    explicit CashierWindow(QWidget *parent = nullptr);
    ~CashierWindow();
    void setCashierId(int id);
    void setCashierName(const QString &name);
    void loadProducts();
    void loadSales();

private slots:
    void on_pbSave_clicked();
    void on_dsbDiscount_valueChanged(double arg1);
    void on_leSearchProduct_textChanged(const QString &arg1);
    void on_leSearchSale_textChanged(const QString &arg1);
    void updateTotal();
    void on_pbCashierAccount_clicked();
    void onCartItemDoubleClicked(int row, int column);

private:
    Ui::CashierWindow *ui;
    int cashierId;
    QString cashierName;
    QStandardItemModel *salesModel;
    void addToCart(const QString &productName, double price, int maxQuantity);
    void removeFromCart(int row);
    CartSubject *cartSubject;
    LoggerObserver *loggerObserver;
    UINotificationObserver *uiNotificationObserver;
};

#endif
