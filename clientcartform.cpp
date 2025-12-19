#include "clientcartform.h"
#include "ui_clientcartform.h"
#include "salesreceiptform.h"
#include <QMessageBox>
#include <QHeaderView>
#include <QDebug>

ClientCartForm::ClientCartForm(int userId, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ClientCartForm)
    , userId(userId)
{
    ui->setupUi(this);
    setModal(true);

    setupTable();

    connect(ui->pbCancel, &QPushButton::clicked, this, &ClientCartForm::close);
}

ClientCartForm::~ClientCartForm()
{
    delete ui;
}

void ClientCartForm::setupTable()
{
    ui->twCart->setColumnCount(3);
    ui->twCart->setHorizontalHeaderLabels({"Наименование", "Количество", "Итог"});
    ui->twCart->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->twCart->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->twCart->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->twCart->verticalHeader()->setVisible(false);

    ui->twCart->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->twCart->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->twCart->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
}

void ClientCartForm::loadCartItems()
{
    Database db;
    if (!db.connectToDatabase()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось подключиться к базе данных");
        return;
    }

    ui->twCart->setRowCount(0);
    cartItems.clear();

    cartItems = db.getCartItems(userId);

    for (int i = 0; i < cartItems.size(); i++) {
        const CartItem &item = cartItems[i];

        ui->twCart->insertRow(i);

        QTableWidgetItem *nameItem = new QTableWidgetItem(item.productName);
        nameItem->setData(Qt::UserRole, item.productId);
        ui->twCart->setItem(i, 0, nameItem);

        QTableWidgetItem *quantityItem = new QTableWidgetItem(QString::number(item.quantity));
        quantityItem->setTextAlignment(Qt::AlignCenter);
        ui->twCart->setItem(i, 1, quantityItem);

        double total = item.retailPrice * item.quantity;
        QTableWidgetItem *totalItem = new QTableWidgetItem(QString::number(total, 'f', 2) + " ₽");
        totalItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->twCart->setItem(i, 2, totalItem);
    }

    updateTotals();
}

void ClientCartForm::updateTotals()
{
    double costWithoutDiscount = 0;

    for (const CartItem &item : cartItems) {
        costWithoutDiscount += item.retailPrice * item.quantity;
    }

    calculateDiscount();

    ui->lCostWithoutDiscount->setText(QString::number(costWithoutDiscount, 'f', 2) + " ₽");
}

void ClientCartForm::calculateDiscount()
{
    double costWithoutDiscount = 0;
    double totalQuantity = 0;

    for (const CartItem &item : cartItems) {
        costWithoutDiscount += item.retailPrice * item.quantity;
        totalQuantity += item.quantity;
    }

    double discountPercent = 0;
    if (totalQuantity >= 3 && totalQuantity <= 5) {
        discountPercent = 5;
    } else if (totalQuantity > 5) {
        discountPercent = 10;
    }

    double discountAmount = costWithoutDiscount * (discountPercent / 100);
    double totalAmount = costWithoutDiscount - discountAmount;

    ui->lDiscount->setText(QString::number(discountPercent, 'f', 1) + "%");
    ui->lTotal->setText(QString::number(totalAmount, 'f', 2) + " ₽");
}

void ClientCartForm::on_pbBuy_clicked()
{
    ui->pbBuy->setEnabled(false);

    if (cartItems.isEmpty()) {
        QMessageBox::warning(this, "Корзина пуста", "Корзина пуста.");
        ui->pbBuy->setEnabled(true);
        return;
    }

    static bool inProgress = false;
    if (inProgress) return;
    inProgress = true;

    auto reply = QMessageBox::question(
        this,
        "Подтверждение покупки",
        "Вы уверены, что хотите оформить покупку?",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply != QMessageBox::Yes) {
        inProgress = false;
        ui->pbBuy->setEnabled(true);
        return;
    }

    Database db;
    if (!db.connectToDatabase()) {
        inProgress = false;
        ui->pbBuy->setEnabled(true);
        return;
    }

    Sale sale;
    sale.customerId = userId;
    sale.discountAmount = 0;

    if (db.createSaleForClient(sale)) {
        SalesReceiptForm receiptForm(sale.id, this);
        receiptForm.exec();

        QMessageBox::information(this, "Готово", "Покупка оформлена.");

        emit cartUpdated();
        accept();
    }

    inProgress = false;
}

void ClientCartForm::on_twCart_itemDoubleClicked(QTableWidgetItem *item)
{
    if (!item) return;

    ui->twCart->setEnabled(false);

    int row = item->row();
    if (row < 0 || row >= ui->twCart->rowCount()) {
        ui->twCart->setEnabled(true);
        return;
    }

    QTableWidgetItem *nameItem = ui->twCart->item(row, 0);
    if (!nameItem) {
        ui->twCart->setEnabled(true);
        return;
    }

    QString productName = nameItem->text();
    int productId = nameItem->data(Qt::UserRole).toInt();

    auto reply = QMessageBox::question(
        this,
        "Удаление товара",
        QString("Удалить товар \"%1\" из корзины?").arg(productName),
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        Database db;
        if (db.connectToDatabase() && db.removeFromCart(userId, productId)) {
            loadCartItems();
            emit cartUpdated();
        }
    }

    ui->twCart->setEnabled(true);
}
