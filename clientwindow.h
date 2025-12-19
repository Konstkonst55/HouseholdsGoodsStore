#ifndef CLIENTWINDOW_H
#define CLIENTWINDOW_H

#include <QWidget>
#include <QStandardItemModel>
#include "database.h"
#include "clientcartform.h"

namespace Ui {
class ClientWindow;
}

class ClientWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ClientWindow(QWidget *parent = nullptr);
    ~ClientWindow();

    void setUserId(int id);

private slots:
    void on_pbOpenCart_clicked();
    void on_pbClientAccount_clicked();
    void on_leSearch_textChanged(const QString &text);
    void on_cbSort_currentIndexChanged(int index);
    void onCartClosed();
    void updateCartCount();

private:
    Ui::ClientWindow *ui;
    int userId;
    QStandardItemModel *productsModel;
    QList<Product> allProducts;
    ClientCartForm *cartForm = nullptr;

    void loadProducts();
    void applyFiltersAndSort(const QString &searchText, int sortIndex);
    void setupProductsTable();
    void addToCart(const QString &productName, double price, int stock, int productId, int row);
};

#endif // CLIENTWINDOW_H
