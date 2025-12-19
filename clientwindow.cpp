#include "clientwindow.h"
#include "ui_clientwindow.h"
#include "authwindow.h"
#include <QMessageBox>
#include <QPushButton>
#include <QHeaderView>
#include <QStandardItem>
#include <QDebug>

ClientWindow::ClientWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ClientWindow)
    , userId(-1)
{
    ui->setupUi(this);

    productsModel = new QStandardItemModel(this);
    setupProductsTable();

    ui->cbSort->addItem("Без сортировки");
    ui->cbSort->addItem("Название (А-Я)");
    ui->cbSort->addItem("Название (Я-А)");
    ui->cbSort->addItem("Цена (по возрастанию)");
    ui->cbSort->addItem("Цена (по убыванию)");

    loadProducts();
}

ClientWindow::~ClientWindow()
{
    delete ui;
    if (cartForm) {
        cartForm->deleteLater();
    }
}

void ClientWindow::setUserId(int id){
    userId = id;
}

void ClientWindow::setupProductsTable()
{
    ui->tvProducts->setColumnCount(4);
    ui->tvProducts->setHorizontalHeaderLabels({"Наименование", "Количество на складе", "Стоимость", ""});

    ui->tvProducts->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tvProducts->setSelectionBehavior(QAbstractItemView::SelectRows);

    ui->tvProducts->horizontalHeader()->setStretchLastSection(true);
}

void ClientWindow::loadProducts()
{
    Database db;
    if (!db.connectToDatabase()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось подключиться к базе данных");
        return;
    }

    allProducts = db.getProductsForClient();
    applyFiltersAndSort("", 0);
    updateCartCount();
}

void ClientWindow::applyFiltersAndSort(const QString &searchText, int sortIndex)
{
    ui->tvProducts->setRowCount(0);
    ui->tvProducts->clearContents();

    QList<Product> filteredProducts;

    for (const Product &product : allProducts) {
        if (searchText.isEmpty()
            || product.name.contains(searchText, Qt::CaseInsensitive)
            || product.article.contains(searchText, Qt::CaseInsensitive)) {
            filteredProducts.append(product);
        }
    }

    switch (sortIndex) {
    case 1:
        std::sort(filteredProducts.begin(), filteredProducts.end(),
                  [](const Product &a, const Product &b) {
                      return a.name.localeAwareCompare(b.name) < 0;
                  });
        break;
    case 2:
        std::sort(filteredProducts.begin(), filteredProducts.end(),
                  [](const Product &a, const Product &b) {
                      return a.name.localeAwareCompare(b.name) > 0;
                  });
        break;
    case 3:
        std::sort(filteredProducts.begin(), filteredProducts.end(),
                  [](const Product &a, const Product &b) {
                      return a.retailPrice < b.retailPrice;
                  });
        break;
    case 4:
        std::sort(filteredProducts.begin(), filteredProducts.end(),
                  [](const Product &a, const Product &b) {
                      return a.retailPrice > b.retailPrice;
                  });
        break;
    default:
        break;
    }

    for (const Product &product : filteredProducts) {
        int row = ui->tvProducts->rowCount();
        ui->tvProducts->insertRow(row);

        QTableWidgetItem *nameItem = new QTableWidgetItem(product.name);
        nameItem->setData(Qt::UserRole, product.id);
        ui->tvProducts->setItem(row, 0, nameItem);

        ui->tvProducts->setItem(row, 1, new QTableWidgetItem(QString::number(product.stock)));

        ui->tvProducts->setItem(row, 2, new QTableWidgetItem(QString::number(product.retailPrice, 'f', 2) + " ₽") );

        QPushButton *addButton = new QPushButton("+");
        addButton->setEnabled(product.stock > 0);
        ui->tvProducts->setCellWidget(row, 3, addButton);

        connect(addButton, &QPushButton::clicked, this, [this, product]() {
            Database db;
            if (!db.connectToDatabase()) {
                QMessageBox::critical(this, "Ошибка", "Не удалось подключиться к базе данных");
                return;
            }

            if (!db.addToCart(userId, product.id, 1)) {
                QMessageBox::warning(this, "Ошибка", "Не удалось добавить товар в корзину");
                return;
            }

            loadProducts();
        });
    }
}

void ClientWindow::on_pbOpenCart_clicked()
{
    if (!cartForm) {
        cartForm = new ClientCartForm(userId, this);
        connect(cartForm, &ClientCartForm::finished, this, &ClientWindow::onCartClosed);
        connect(cartForm, &ClientCartForm::cartUpdated, this, &ClientWindow::updateCartCount);
    }

    cartForm->loadCartItems();
    cartForm->show();
}

void ClientWindow::onCartClosed()
{
    loadProducts();
}

void ClientWindow::updateCartCount()
{
    Database db;
    if (!db.connectToDatabase()) {
        return;
    }

    QList<CartItem> cartItems = db.getCartItems(userId);
    int totalCount = 0;
    for (const CartItem &item : cartItems) {
        totalCount += item.quantity;
    }

    ui->pbOpenCart->setText(QString("Корзина (%1)").arg(totalCount));
}

void ClientWindow::on_pbClientAccount_clicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Выход из аккаунта",
                                                              "Вы уверены, что хотите выйти из аккаунта?",
                                                              QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        this->close();

        AuthWindow *authWindow = new AuthWindow();
        authWindow->show();
    }
}


void ClientWindow::on_leSearch_textChanged(const QString &text)
{
    applyFiltersAndSort(text, ui->cbSort->currentIndex());
}

void ClientWindow::on_cbSort_currentIndexChanged(int index)
{
    applyFiltersAndSort(ui->leSearch->text(), index);
}
