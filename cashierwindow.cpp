#include "cashierwindow.h"
#include "ui_cashierwindow.h"
#include "authwindow.h"
#include "salesreceiptform.h"
#include <QMessageBox>
#include <QPushButton>
#include <QHeaderView>

CashierWindow::CashierWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CashierWindow),
    cashierId(-1),
    salesModel(new QStandardItemModel(this))
{
    ui->setupUi(this);

    ui->twProducts->setColumnCount(4);
    ui->twProducts->setHorizontalHeaderLabels({"Наименование", "Стоимость", "Количество", ""});

    ui->twCart->setColumnCount(3);
    ui->twCart->setHorizontalHeaderLabels({"Наименование", "Количество", "Итоговая стоимость"});

    ui->twSales->setModel(salesModel);
    salesModel->setHorizontalHeaderLabels({"ID", "Дата", "Общая сумма", "Скидка", "Итог"});

    ui->twProducts->horizontalHeader()->setStretchLastSection(true);
    ui->twCart->horizontalHeader()->setStretchLastSection(true);
    ui->twSales->horizontalHeader()->setStretchLastSection(true);

    ui->twProducts->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->twCart->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->twSales->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(ui->twCart, &QTableWidget::cellDoubleClicked, this, &CashierWindow::onCartItemDoubleClicked);

    connect(ui->twSales, &QTableView::doubleClicked, this, [this](const QModelIndex &index) {
        int row = index.row();

        if (row >= 0) {
            QStandardItem *idItem = salesModel->item(row, 0);
            if (idItem) {
                int saleId = idItem->text().toInt();
                SalesReceiptForm *receiptForm = new SalesReceiptForm(saleId, this);
                receiptForm->exec();
                delete receiptForm;
            }
        }
    });

    loadProducts();
}

CashierWindow::~CashierWindow()
{
    delete ui;
}

void CashierWindow::setCashierId(int id)
{
    cashierId = id;
    loadSales();
}

void CashierWindow::setCashierName(const QString &name)
{
    cashierName = name;
    setWindowTitle(windowTitle() + " - " + name);
}

void CashierWindow::loadProducts()
{
    ui->twProducts->setRowCount(0);

    Database db;
    if (!db.connectToDatabase()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось подключиться к базе данных");
        return;
    }

    auto products = db.getAllProducts();

    for (const auto &product : products) {
        int row = ui->twProducts->rowCount();
        ui->twProducts->insertRow(row);

        ui->twProducts->setItem(row, 0, new QTableWidgetItem(product.name));
        ui->twProducts->setItem(row, 1, new QTableWidgetItem(QString::number(product.retailPrice, 'f', 2)));
        ui->twProducts->setItem(row, 2, new QTableWidgetItem(QString::number(product.stock)));

        QPushButton *addButton = new QPushButton("+");
        ui->twProducts->setCellWidget(row, 3, addButton);

        Product productCopy = product;
        connect(addButton, &QPushButton::clicked, [this, productCopy, row]() {
            if (productCopy.stock > 0) {
                addToCart(productCopy.name, productCopy.retailPrice, productCopy.stock);
                QTableWidgetItem *stockItem = ui->twProducts->item(row, 2);

                if (stockItem) {
                    int currentStock = stockItem->text().toInt();
                    if (currentStock > 0) {
                        stockItem->setText(QString::number(currentStock - 1));
                    }
                }
            }
        });
    }
}
void CashierWindow::addToCart(const QString &productName, double price, int maxQuantity)
{
    for (int i = 0; i < ui->twCart->rowCount(); ++i) {
        if (ui->twCart->item(i, 0)->text() == productName) {
            int currentQty = ui->twCart->item(i, 1)->text().toInt();
            if (currentQty < maxQuantity) {
                int newQty = currentQty + 1;
                ui->twCart->item(i, 1)->setText(QString::number(newQty));
                ui->twCart->item(i, 2)->setText(QString::number(newQty * price, 'f', 2));
                updateTotal();
            }
            return;
        }
    }

    int row = ui->twCart->rowCount();
    ui->twCart->insertRow(row);
    ui->twCart->setItem(row, 0, new QTableWidgetItem(productName));
    ui->twCart->setItem(row, 1, new QTableWidgetItem("1"));
    ui->twCart->setItem(row, 2, new QTableWidgetItem(QString::number(price, 'f', 2)));
    updateTotal();
}

void CashierWindow::updateTotal()
{
    double totalWithoutDiscount = 0.0;
    for (int i = 0; i < ui->twCart->rowCount(); ++i) {
        totalWithoutDiscount += ui->twCart->item(i, 2)->text().toDouble();
    }

    ui->lCostWithoutDiscount->setText(QString::number(totalWithoutDiscount, 'f', 2));

    double discount = ui->dsbDiscount->value();
    double totalWithDiscount = totalWithoutDiscount * (1 - discount / 100.0);
    ui->lTotal->setText(QString::number(totalWithDiscount, 'f', 2));
}

void CashierWindow::on_pbCashierAccount_clicked()
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


void CashierWindow::onCartItemDoubleClicked(int row, int column)
{
    Q_UNUSED(column);
    removeFromCart(row);
}

void CashierWindow::removeFromCart(int row)
{
    if (row >= 0 && row < ui->twCart->rowCount()) {
        QString productName = ui->twCart->item(row, 0)->text();
        int quantity = ui->twCart->item(row, 1)->text().toInt();

        ui->twCart->removeRow(row);

        updateTotal();

        for (int i = 0; i < ui->twProducts->rowCount(); ++i) {
            if (ui->twProducts->item(i, 0)->text() == productName) {
                QTableWidgetItem *stockItem = ui->twProducts->item(i, 2);
                if (stockItem) {
                    int currentStock = stockItem->text().toInt();
                    stockItem->setText(QString::number(currentStock + quantity));
                }
                break;
            }
        }
    }
}

void CashierWindow::on_dsbDiscount_valueChanged(double arg1)
{
    Q_UNUSED(arg1);
    updateTotal();
}

void CashierWindow::on_pbSave_clicked()
{
    if (ui->twCart->rowCount() == 0) {
        QMessageBox::warning(this, "Ошибка", "Корзина пуста!");
        return;
    }

    double totalWithoutDiscount = ui->lCostWithoutDiscount->text().toDouble();
    double discount = ui->dsbDiscount->value();

    Database db;
    if (!db.connectToDatabase()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось подключиться к базе данных");
        return;
    }

    QList<SaleItem> saleItems;
    for (int i = 0; i < ui->twCart->rowCount(); ++i) {
        QString productName = ui->twCart->item(i, 0)->text();
        int quantity = ui->twCart->item(i, 1)->text().toInt();
        double price = ui->twCart->item(i, 2)->text().toDouble() / quantity;

        auto allProducts = db.getAllProducts();
        Product product;
        for (const auto &p : allProducts) {
            if (p.name == productName) {
                product = p;
                break;
            }
        }

        if (product.id > 0) {
            if (!db.checkProductAvailability(product.id, quantity)) {
                QMessageBox::warning(this, "Ошибка",
                                     QString("Недостаточно товара '%1' на складе. Доступно: %2")
                                         .arg(productName)
                                         .arg(product.stock));
                return;
            }

            SaleItem item;
            item.productId = product.id;
            item.productName = productName;
            item.quantity = quantity;
            item.retailPrice = price;
            item.totalPrice = price * quantity;
            saleItems.append(item);
        }
    }

    Sale sale;
    sale.saleDate = QDateTime::currentDateTime();
    sale.cashierId = cashierId;
    sale.totalAmount = totalWithoutDiscount;
    sale.discountAmount = discount;

    int saleId = db.createSale(sale, saleItems);
    if (saleId != -1) {
        SalesReceiptForm form(saleId, this);
        form.exec();

        ui->twCart->setRowCount(0);
        ui->dsbDiscount->setValue(0.0);
        loadProducts();
        loadSales();
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось сохранить продажу!");
    }
}

void CashierWindow::on_leSearchProduct_textChanged(const QString &arg1)
{
    for (int i = 0; i < ui->twProducts->rowCount(); ++i) {
        bool match = ui->twProducts->item(i, 0)->text().contains(arg1, Qt::CaseInsensitive);
        ui->twProducts->setRowHidden(i, !match);
    }
}

void CashierWindow::loadSales()
{
    if (cashierId == -1) return;

    salesModel->removeRows(0, salesModel->rowCount());

    Database db;
    if (!db.connectToDatabase()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось подключиться к базе данных");
        return;
    }

    auto sales = db.getSalesByCashier(cashierId);

    for (const auto &sale : sales) {
        QList<QStandardItem*> rowItems;
        rowItems << new QStandardItem(QString::number(sale.id));
        rowItems << new QStandardItem(sale.saleDate.toString("dd.MM.yyyy HH:mm"));
        rowItems << new QStandardItem(QString::number(sale.totalAmount, 'f', 2));
        rowItems << new QStandardItem(QString::number(sale.discountAmount, 'f', 1) + "%");
        rowItems << new QStandardItem(QString::number(sale.finalAmount, 'f', 2));
        salesModel->appendRow(rowItems);
    }
}

void CashierWindow::on_leSearchSale_textChanged(const QString &arg1)
{
    for (int i = 0; i < salesModel->rowCount(); ++i) {
        bool match = false;
        for (int j = 0; j < salesModel->columnCount(); ++j) {
            if (salesModel->item(i, j)->text().contains(arg1, Qt::CaseInsensitive)) {
                match = true;
                break;
            }
        }
        ui->twSales->setRowHidden(i, !match);
    }
}
