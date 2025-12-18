#include "addproductform.h"
#include "ui_addproductform.h"
#include <QMessageBox>

AddProductForm::AddProductForm(int productId, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddProductForm)
    , productId(productId)
{
    ui->setupUi(this);

    if (productId > 0) {
        ui->lHeader->setText("Редактирование товара");
    }

    loadCategories();

    if (productId > 0) {
        loadProductData();
    }

    connect(ui->pbSave, &QPushButton::clicked, this, &AddProductForm::onSaveClicked);
    connect(ui->pbCancel, &QPushButton::clicked, this, &AddProductForm::onCancelClicked);
}

AddProductForm::~AddProductForm()
{
    delete ui;
}

void AddProductForm::loadCategories()
{
    ui->cbCategory->clear();
    ui->cbCategory->addItem("Не выбрана", -1);

    if (!db.connectToDatabase()) return;

    QList<ProductCategory> categories = db.getAllCategories();
    foreach (const ProductCategory &category, categories) {
        ui->cbCategory->addItem(category.name, category.id);
    }
}

void AddProductForm::loadProductData()
{
    if (!db.connectToDatabase()) return;

    Product product = db.getProductById(productId);
    if (product.id == -1) return;

    ui->leArticle->setText(product.article);
    ui->leName->setText(product.name);

    int index = ui->cbCategory->findData(product.categoryId);
    if (index >= 0) {
        ui->cbCategory->setCurrentIndex(index);
    }

    ui->dsbPurchasePrice->setValue(product.purchasePrice);
    ui->dsbRetailPrice->setValue(product.retailPrice);
    ui->sbStock->setValue(product.stock);
}

bool AddProductForm::validateForm()
{
    if (ui->leArticle->text().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите артикул");
        return false;
    }

    if (ui->leName->text().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите наименование");
        return false;
    }

    if (ui->dsbPurchasePrice->value() <= 0) {
        QMessageBox::warning(this, "Ошибка", "Введите закупочную цену");
        return false;
    }

    if (ui->dsbRetailPrice->value() <= 0) {
        QMessageBox::warning(this, "Ошибка", "Введите розничную цену");
        return false;
    }

    if (ui->dsbRetailPrice->value() < ui->dsbPurchasePrice->value()) {
        QMessageBox::warning(this, "Ошибка", "Розничная цена не может быть меньше закупочной");
        return false;
    }

    return true;
}

void AddProductForm::onSaveClicked()
{
    if (!validateForm()) return;

    if (!db.connectToDatabase()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось подключиться к БД");
        return;
    }

    Product product;
    product.id = productId;
    product.article = ui->leArticle->text();
    product.name = ui->leName->text();
    product.categoryId = ui->cbCategory->currentData().toInt();
    product.purchasePrice = ui->dsbPurchasePrice->value();
    product.retailPrice = ui->dsbRetailPrice->value();
    product.stock = ui->sbStock->value();

    bool success = false;
    if (productId > 0) {
        success = db.updateProduct(product);
    } else {
        success = db.addProduct(product);
    }

    if (success) {
        accept();
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось сохранить товар");
    }
}

void AddProductForm::onCancelClicked()
{
    reject();
}
