#include "addsupplyform.h"
#include "ui_addsupplyform.h"
#include <QMessageBox>
#include "addproductform.h"

AddSupplyForm::AddSupplyForm(QWidget *parent, int userId)
    : QDialog(parent)
    , ui(new Ui::AddSupplyForm)
    , currentUserId(userId)
{
    ui->setupUi(this);
    ui->dteDate->setDateTime(QDateTime::currentDateTime());

    loadProducts();

    connect(ui->pbSaveSupply, &QPushButton::clicked, this, &AddSupplyForm::onSaveClicked);
    connect(ui->pbCancelSupply, &QPushButton::clicked, this, &AddSupplyForm::onCancelClicked);
    connect(ui->cbProduct, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AddSupplyForm::onProductChanged);
    connect(ui->pbAddNewProduct, &QPushButton::clicked, this, &AddSupplyForm::onAddNewProductClicked);
    connect(ui->sbQuantity, QOverload<int>::of(&QSpinBox::valueChanged), this, &AddSupplyForm::onQuantityChanged);
}

AddSupplyForm::~AddSupplyForm()
{
    delete ui;
}

void AddSupplyForm::loadProducts()
{
    ui->cbProduct->clear();
    ui->cbProduct->addItem("Выберите товар", -1);

    if (!db.connectToDatabase()) return;

    QList<Product> products = db.getAllProducts();
    foreach (const Product &product, products) {
        ui->cbProduct->addItem(product.name + " (" + product.article + ")", product.id);
    }
}

void AddSupplyForm::calculateTotal()
{
    int productId = ui->cbProduct->currentData().toInt();
    int quantity = ui->sbQuantity->value();

    if (productId > 0 && quantity > 0) {
        if (!db.connectToDatabase()) return;

        Product product = db.getProductById(productId);
        if (product.id != -1) {
            double total = product.purchasePrice * quantity;
            ui->lTotalSum->setText(QString::number(total, 'f', 2));
        }
    } else {
        ui->lTotalSum->setText("0");
    }
}

bool AddSupplyForm::validateForm()
{
    if (ui->leSupplier->text().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите поставщика");
        return false;
    }

    if (ui->cbProduct->currentData().toInt() <= 0) {
        QMessageBox::warning(this, "Ошибка", "Выберите товар");
        return false;
    }

    if (ui->sbQuantity->value() <= 0) {
        QMessageBox::warning(this, "Ошибка", "Введите количество");
        return false;
    }

    return true;
}

void AddSupplyForm::onSaveClicked()
{
    if (!validateForm()) return;

    if (!db.connectToDatabase()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось подключиться к БД");
        return;
    }

    Supply supply;
    supply.supplyNumber = db.generateSupplyNumber();
    supply.supplierName = ui->leSupplier->text();
    supply.productId = ui->cbProduct->currentData().toInt();
    supply.quantity = ui->sbQuantity->value();

    Product product = db.getProductById(supply.productId);
    if (product.id == -1) {
        QMessageBox::warning(this, "Ошибка", "Товар не найден");
        return;
    }

    supply.purchasePrice = product.purchasePrice;
    supply.supplyDate = ui->dteDate->dateTime();

    if (db.addSupply(supply, currentUserId)) {
        accept();
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось сохранить поставку");
    }
}

void AddSupplyForm::onCancelClicked()
{
    reject();
}

void AddSupplyForm::onProductChanged(int index)
{
    Q_UNUSED(index);
    calculateTotal();
}

void AddSupplyForm::onAddNewProductClicked()
{
    AddProductForm productForm(-1, this);
    if (productForm.exec() == QDialog::Accepted) {
        loadProducts();
    }
}

void AddSupplyForm::onQuantityChanged(int value)
{
    Q_UNUSED(value);
    calculateTotal();
}
