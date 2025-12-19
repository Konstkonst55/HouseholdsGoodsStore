#include "adminwindow.h"
#include "authwindow.h"
#include "ui_adminwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QHeaderView>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QDateEdit>
#include <QDialog>
#include <QFormLayout>
#include <QDialogButtonBox>
#include "addproductform.h"
#include "addsupplyform.h"
#include "salesreceiptform.h"

AdminWindow::AdminWindow(QWidget *parent, int userId)
    : QWidget(parent), ui(new Ui::AdminWindow), currentUserId(userId)
{
    ui->setupUi(this);

    setupMenuBar();

    setupPages();

    connect(ui->rbProduct, &QRadioButton::toggled, this, &AdminWindow::onNavigationChanged);
    connect(ui->rbSupply, &QRadioButton::toggled, this, &AdminWindow::onNavigationChanged);
    connect(ui->rbSale, &QRadioButton::toggled, this, &AdminWindow::onNavigationChanged);

    ui->rbProduct->setChecked(true);
    onNavigationChanged();

    connect(ui->leSearch, &QLineEdit::textChanged, this, [this](const QString &text) {
        if (ui->rbProduct->isChecked()) {
            searchProducts(text);
        } else if (ui->rbSupply->isChecked()) {
            searchSupplies(text);
        } else if (ui->rbSale->isChecked()) {
            searchSales(text);
        }
    });

    connect(salesTable, &QTableWidget::cellDoubleClicked, this, [this](int row, int column) {
        Q_UNUSED(column);
        QTableWidgetItem *idItem = salesTable->item(row, 0);
        if (idItem) {
            int saleId = idItem->text().toInt();
            showReceiptForm(saleId);
        }
    });
}

AdminWindow::~AdminWindow()
{
    delete ui;
}

void AdminWindow::setupMenuBar()
{
    menuBar = new QMenuBar(this);

    fileMenu = menuBar->addMenu("&Файл");

    QAction *openAction = new QAction("&Открыть...", this);
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &AdminWindow::onFileOpen);
    fileMenu->addAction(openAction);

    QAction *saveAsAction = new QAction("&Сохранить как...", this);
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    connect(saveAsAction, &QAction::triggered, this, &AdminWindow::onFileSaveAs);
    fileMenu->addAction(saveAsAction);

    fileMenu->addSeparator();

    QAction *exitAction = new QAction("&Выход", this);
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    fileMenu->addAction(exitAction);

    reportMenu = menuBar->addMenu("&Отчет");

    QAction *profitAction = new QAction("&Прибыль...", this);
    connect(profitAction, &QAction::triggered, this, &AdminWindow::onReportProfit);
    reportMenu->addAction(profitAction);

    QAction *popularAction = new QAction("&Популярные товары...", this);
    connect(popularAction, &QAction::triggered, this, &AdminWindow::onReportPopular);
    reportMenu->addAction(popularAction);

    helpMenu = menuBar->addMenu("&Помощь");

    QAction *referenceAction = new QAction("&Справка...", this);
    referenceAction->setShortcut(QKeySequence::HelpContents);
    connect(referenceAction, &QAction::triggered, this, &AdminWindow::onHelpReference);
    helpMenu->addAction(referenceAction);

    QAction *aboutAction = new QAction("&О программе...", this);
    connect(aboutAction, &QAction::triggered, this, &AdminWindow::onHelpAbout);
    helpMenu->addAction(aboutAction);

    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout *>(layout());
    if (mainLayout)
    {
        mainLayout->insertWidget(0, menuBar);
    }
}

void AdminWindow::setupPages()
{

    productsPage = new QWidget(this);
    suppliesPage = new QWidget(this);
    salesPage = new QWidget(this);

    setupProductsPage();
    setupSuppliesPage();
    setupSalesPage();

    ui->vlWorkflow->addWidget(productsPage);
    ui->vlWorkflow->addWidget(suppliesPage);
    ui->vlWorkflow->addWidget(salesPage);
}

void AdminWindow::setupProductsPage()
{
    QVBoxLayout *layout = new QVBoxLayout(productsPage);

    QHBoxLayout *titleLayout = new QHBoxLayout();
    productsTitle = new QLabel("Товары");
    productsTitle->setStyleSheet("font-size: 16pt; font-weight: bold;");

    productsExportBtn = new QPushButton("Экспортировать");
    connect(productsExportBtn, &QPushButton::clicked, this, &AdminWindow::onExportTable);

    titleLayout->addWidget(productsTitle);
    titleLayout->addStretch();
    titleLayout->addWidget(productsExportBtn);

    productsToolBar = new QToolBar();
    productsToolBar->setIconSize(QSize(16, 16));

    QAction *addAction = productsToolBar->addAction("Добавить");
    addAction->setIcon(QIcon::fromTheme("list-add"));
    connect(addAction, &QAction::triggered, this, &AdminWindow::onAddProduct);

    QAction *editAction = productsToolBar->addAction("Редактировать");
    editAction->setIcon(QIcon::fromTheme("document-edit"));
    editAction->setEnabled(false);
    connect(editAction, &QAction::triggered, this, &AdminWindow::onEditProduct);

    QAction *deleteAction = productsToolBar->addAction("Удалить");
    deleteAction->setIcon(QIcon::fromTheme("list-remove"));
    deleteAction->setEnabled(false);
    connect(deleteAction, &QAction::triggered, this, &AdminWindow::onDeleteProduct);

    productsTable = new QTableWidget();
    productsTable->setColumnCount(9);
    QStringList productHeaders = {"ID", "Артикул", "Название", "Категория",
                                  "Закупочная цена", "Розничная цена",
                                  "Остаток", "Создан", "Обновлен"};
    productsTable->setHorizontalHeaderLabels(productHeaders);
    productsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    productsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    productsTable->horizontalHeader()->setStretchLastSection(true);
    productsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(productsTable->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &AdminWindow::onTableSelectionChanged);

    layout->addLayout(titleLayout);
    layout->addWidget(productsToolBar);
    layout->addWidget(productsTable);
}

void AdminWindow::setupSuppliesPage()
{
    QVBoxLayout *layout = new QVBoxLayout(suppliesPage);

    QHBoxLayout *titleLayout = new QHBoxLayout();
    suppliesTitle = new QLabel("Поставки");
    suppliesTitle->setStyleSheet("font-size: 16pt; font-weight: bold;");

    suppliesExportBtn = new QPushButton("Экспортировать");
    connect(suppliesExportBtn, &QPushButton::clicked, this, &AdminWindow::onExportTable);

    titleLayout->addWidget(suppliesTitle);
    titleLayout->addStretch();
    titleLayout->addWidget(suppliesExportBtn);

    suppliesToolBar = new QToolBar();
    suppliesToolBar->setIconSize(QSize(16, 16));

    QAction *addAction = suppliesToolBar->addAction("Добавить");
    addAction->setIcon(QIcon::fromTheme("list-add"));
    connect(addAction, &QAction::triggered, this, &AdminWindow::onAddSupply);

    QAction *deleteAction = suppliesToolBar->addAction("Удалить");
    deleteAction->setIcon(QIcon::fromTheme("list-remove"));
    deleteAction->setEnabled(false);
    connect(deleteAction, &QAction::triggered, this, &AdminWindow::onDeleteSupply);

    suppliesTable = new QTableWidget();
    suppliesTable->setColumnCount(11);
    QStringList supplyHeaders = {"ID", "Номер поставки", "Поставщик", "Товар",
                                 "Количество", "Цена закупки", "Сумма",
                                 "Дата поставки", "Кем создана", "Создано", "ID товара"};
    suppliesTable->setHorizontalHeaderLabels(supplyHeaders);
    suppliesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    suppliesTable->setSelectionMode(QAbstractItemView::SingleSelection);
    suppliesTable->horizontalHeader()->setStretchLastSection(true);
    suppliesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    suppliesTable->setColumnHidden(10, true);

    connect(suppliesTable->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &AdminWindow::onTableSelectionChanged);

    layout->addLayout(titleLayout);
    layout->addWidget(suppliesToolBar);
    layout->addWidget(suppliesTable);
}

void AdminWindow::setupSalesPage()
{
    QVBoxLayout *layout = new QVBoxLayout(salesPage);

    QHBoxLayout *titleLayout = new QHBoxLayout();
    salesTitle = new QLabel("Продажи");
    salesTitle->setStyleSheet("font-size: 16pt; font-weight: bold;");

    salesExportBtn = new QPushButton("Экспортировать");
    connect(salesExportBtn, &QPushButton::clicked, this, &AdminWindow::onExportTable);

    titleLayout->addWidget(salesTitle);
    titleLayout->addStretch();
    titleLayout->addWidget(salesExportBtn);

    salesTable = new QTableWidget();
    salesTable->setColumnCount(11);
    QStringList salesHeaders = {"ID", "Номер чека", "Дата продажи", "Кассир",
                                "Клиент", "Сумма", "Скидка", "Итоговая сумма",
                                "Создано", "ID кассира", "ID клиента"};
    salesTable->setHorizontalHeaderLabels(salesHeaders);
    salesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    salesTable->setSelectionMode(QAbstractItemView::SingleSelection);
    salesTable->horizontalHeader()->setStretchLastSection(true);
    salesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    salesTable->setColumnHidden(9, true);
    salesTable->setColumnHidden(10, true);

    layout->addLayout(titleLayout);
    layout->addWidget(salesTable);
}

void AdminWindow::loadProductsData()
{
    Database db;
    if (!db.connectToDatabase()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось подключиться к базе данных");
        return;
    }

    QList<Product> products = db.getAllProducts();
    updateProductsTable(products);
}

void AdminWindow::loadSuppliesData()
{
    Database db;
    if (!db.connectToDatabase()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось подключиться к базе данных");
        return;
    }

    QList<Supply> supplies = db.getAllSupplies();
    updateSuppliesTable(supplies);
}

void AdminWindow::loadSalesData()
{
    Database db;
    if (!db.connectToDatabase()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось подключиться к базе данных");
        return;
    }

    QList<Sale> sales = db.getAllSales();
    updateSalesTable(sales);
}

void AdminWindow::searchProducts(const QString &text)
{
    Database db;
    if (!db.connectToDatabase()) return;

    QList<Product> allProducts = db.getAllProducts();
    QList<Product> filteredProducts;

    if (text.isEmpty()) {
        filteredProducts = allProducts;
    } else {
        QString searchText = text.toLower();
        foreach (const Product &product, allProducts) {
            if (product.name.toLower().contains(searchText) ||
                product.article.toLower().contains(searchText)) {
                filteredProducts.append(product);
            }
        }
    }

    updateProductsTable(filteredProducts);
}

void AdminWindow::searchSupplies(const QString &text)
{
    Database db;
    if (!db.connectToDatabase()) return;

    QList<Supply> allSupplies = db.getAllSupplies();
    QList<Supply> filteredSupplies;

    if (text.isEmpty()) {
        filteredSupplies = allSupplies;
    } else {
        QString searchText = text.toLower();
        foreach (const Supply &supply, allSupplies) {
            if (supply.supplierName.toLower().contains(searchText)) {
                filteredSupplies.append(supply);
            }
        }
    }

    updateSuppliesTable(filteredSupplies);
}

void AdminWindow::searchSales(const QString &text)
{
    Database db;
    if (!db.connectToDatabase()) return;

    QList<Sale> allSales = db.getAllSales();
    QList<Sale> filteredSales;

    if (text.isEmpty()) {
        filteredSales = allSales;
    } else {
        QString searchText = text.toLower();
        foreach (const Sale &sale, allSales) {
            if (sale.cashierName.toLower().contains(searchText)) {
                filteredSales.append(sale);
            }
        }
    }

    updateSalesTable(filteredSales);
}

void AdminWindow::updateProductsTable(const QList<Product> &products)
{
    productsTable->setRowCount(0);

    for (int i = 0; i < products.size(); i++) {
        const Product &product = products[i];
        productsTable->insertRow(i);

        QTableWidgetItem *idItem = new QTableWidgetItem(QString::number(product.id));
        productsTable->setItem(i, 0, idItem);
        productsTable->setItem(i, 1, new QTableWidgetItem(product.article));
        productsTable->setItem(i, 2, new QTableWidgetItem(product.name));
        productsTable->setItem(i, 3, new QTableWidgetItem(product.categoryName));
        productsTable->setItem(i, 4, new QTableWidgetItem(QString::number(product.purchasePrice, 'f', 2)));
        productsTable->setItem(i, 5, new QTableWidgetItem(QString::number(product.retailPrice, 'f', 2)));
        productsTable->setItem(i, 6, new QTableWidgetItem(QString::number(product.stock)));
        productsTable->setItem(i, 7, new QTableWidgetItem(product.createdAt.toString("dd.MM.yyyy HH:mm")));
        productsTable->setItem(i, 8, new QTableWidgetItem(product.updatedAt.toString("dd.MM.yyyy HH:mm")));
    }

    productsTable->resizeColumnsToContents();
}

void AdminWindow::updateSuppliesTable(const QList<Supply> &supplies)
{
    suppliesTable->setRowCount(0);

    for (int i = 0; i < supplies.size(); i++) {
        const Supply &supply = supplies[i];
        suppliesTable->insertRow(i);

        QTableWidgetItem *idItem = new QTableWidgetItem(QString::number(supply.id));
        suppliesTable->setItem(i, 0, idItem);
        suppliesTable->setItem(i, 1, new QTableWidgetItem(supply.supplyNumber));
        suppliesTable->setItem(i, 2, new QTableWidgetItem(supply.supplierName));
        suppliesTable->setItem(i, 3, new QTableWidgetItem(supply.productName));
        suppliesTable->setItem(i, 4, new QTableWidgetItem(QString::number(supply.quantity)));
        suppliesTable->setItem(i, 5, new QTableWidgetItem(QString::number(supply.purchasePrice, 'f', 2)));
        suppliesTable->setItem(i, 6, new QTableWidgetItem(QString::number(supply.totalAmount, 'f', 2)));
        suppliesTable->setItem(i, 7, new QTableWidgetItem(supply.supplyDate.toString("dd.MM.yyyy HH:mm")));
        suppliesTable->setItem(i, 8, new QTableWidgetItem(supply.createdByName));
        suppliesTable->setItem(i, 9, new QTableWidgetItem(supply.createdAt.toString("dd.MM.yyyy HH:mm")));
        suppliesTable->setItem(i, 10, new QTableWidgetItem(QString::number(supply.productId)));
    }

    suppliesTable->resizeColumnsToContents();
}

void AdminWindow::updateSalesTable(const QList<Sale> &sales)
{
    salesTable->setRowCount(0);

    for (int i = 0; i < sales.size(); i++) {
        const Sale &sale = sales[i];
        salesTable->insertRow(i);

        QTableWidgetItem *idItem = new QTableWidgetItem(QString::number(sale.id));
        salesTable->setItem(i, 0, idItem);
        salesTable->setItem(i, 1, new QTableWidgetItem(sale.receiptNumber));
        salesTable->setItem(i, 2, new QTableWidgetItem(sale.saleDate.toString("dd.MM.yyyy HH:mm")));
        salesTable->setItem(i, 3, new QTableWidgetItem(sale.cashierName));
        salesTable->setItem(i, 4, new QTableWidgetItem(sale.customerName));
        salesTable->setItem(i, 5, new QTableWidgetItem(QString::number(sale.totalAmount, 'f', 2)));
        salesTable->setItem(i, 6, new QTableWidgetItem(QString::number(sale.discountAmount, 'f', 2)));
        salesTable->setItem(i, 7, new QTableWidgetItem(QString::number(sale.finalAmount, 'f', 2)));
        salesTable->setItem(i, 8, new QTableWidgetItem(sale.createdAt.toString("dd.MM.yyyy HH:mm")));
        salesTable->setItem(i, 9, new QTableWidgetItem(QString::number(sale.cashierId)));
        salesTable->setItem(i, 10, new QTableWidgetItem(QString::number(sale.customerId)));
    }

    salesTable->resizeColumnsToContents();
}

void AdminWindow::onFileOpen()
{
    QMessageBox::information(this, "Открыть", "Функция открытия файла будет реализована позже");
}

void AdminWindow::onFileSaveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Сохранить базу данных",
                                                    "", "SQLite Database (*.db);;All Files (*)");

    if (!fileName.isEmpty())
    {

        if (QFile::exists("shop.db"))
        {
            if (QFile::copy("shop.db", fileName))
            {
                QMessageBox::information(this, "Успех",
                                         QString("База данных сохранена в:\n%1").arg(fileName));
            }
            else
            {
                QMessageBox::warning(this, "Ошибка", "Не удалось сохранить базу данных");
            }
        }
        else
        {
            QMessageBox::warning(this, "Ошибка", "Файл базы данных не найден");
        }
    }
}

void AdminWindow::onReportProfit()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Отчет о прибыли");
    QFormLayout *form = new QFormLayout(&dialog);

    QDateEdit *startDateEdit = new QDateEdit(QDate::currentDate().addDays(-30));
    startDateEdit->setCalendarPopup(true);
    QDateEdit *endDateEdit = new QDateEdit(QDate::currentDate());
    endDateEdit->setCalendarPopup(true);

    form->addRow("Начальная дата:", startDateEdit);
    form->addRow("Конечная дата:", endDateEdit);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    form->addRow(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        Database db;
        if (!db.connectToDatabase()) {
            QMessageBox::critical(this, "Ошибка", "Не удалось подключиться к БД");
            return;
        }

        ProfitReport report = db.generateProfitReport(startDateEdit->date(), endDateEdit->date());

        QString reportText = QString(
                                 "Отчет о прибыли\n"
                                 "Период: %1 - %2\n\n"
                                 "Выручка: %3 руб.\n"
                                 "Себестоимость: %4 руб.\n"
                                 "Прибыль: %5 руб."
                                 ).arg(report.startDate.toString("dd.MM.yyyy"))
                                 .arg(report.endDate.toString("dd.MM.yyyy"))
                                 .arg(report.totalRevenue, 0, 'f', 2)
                                 .arg(report.totalCost, 0, 'f', 2)
                                 .arg(report.totalProfit, 0, 'f', 2);

        QMessageBox::information(this, "Отчет о прибыли", reportText);
    }
}

void AdminWindow::onReportPopular()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Популярные товары");
    QFormLayout *form = new QFormLayout(&dialog);

    QDateEdit *startDateEdit = new QDateEdit(QDate::currentDate().addDays(-30));
    startDateEdit->setCalendarPopup(true);
    QDateEdit *endDateEdit = new QDateEdit(QDate::currentDate());
    endDateEdit->setCalendarPopup(true);

    form->addRow("Начальная дата:", startDateEdit);
    form->addRow("Конечная дата:", endDateEdit);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    form->addRow(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        Database db;
        if (!db.connectToDatabase()) {
            QMessageBox::critical(this, "Ошибка", "Не удалось подключиться к БД");
            return;
        }

        ProfitReport report = db.generateProfitReport(startDateEdit->date(), endDateEdit->date());

        QString reportText = QString("Популярные товары\nПериод: %1 - %2\n\n")
                                 .arg(report.startDate.toString("dd.MM.yyyy"))
                                 .arg(report.endDate.toString("dd.MM.yyyy"));

        if (report.popularProducts.isEmpty()) {
            reportText += "Нет данных за выбранный период";
        } else {
            for (int i = 0; i < report.popularProducts.size(); i++) {
                reportText += QString("%1. %2 - %3 шт.\n")
                                  .arg(i + 1)
                                  .arg(report.popularProducts[i].first)
                                  .arg(report.popularProducts[i].second);
            }
        }

        QMessageBox::information(this, "Популярные товары", reportText);
    }
}

void AdminWindow::onHelpAbout()
{
    QMessageBox::about(this, "О программе",
                       "Магазин бытовых товаров\n"
                       "Версия 1.0\n\n"
                       "Административный модуль для управления магазином.\n"
                       "Управление товарами, поставками и продажами.");
}

void AdminWindow::onHelpReference()
{
    QMessageBox::information(this, "Справка",
                             "Административный модуль магазина бытовых товаров\n\n"
                             "Основные функции:\n"
                             "1. Товары - управление товарами магазина\n"
                             "2. Поставки - учет поставок товаров\n"
                             "3. Продажи - просмотр истории продаж\n\n"
                             "Используйте навигационные кнопки для переключения между разделами.");
}

void AdminWindow::onNavigationChanged()
{

    productsPage->hide();
    suppliesPage->hide();
    salesPage->hide();

    if (ui->rbProduct->isChecked())
    {
        productsPage->show();
        loadProductsData();
    }
    else if (ui->rbSupply->isChecked())
    {
        suppliesPage->show();
        loadSuppliesData();
    }
    else if (ui->rbSale->isChecked())
    {
        salesPage->show();
        loadSalesData();
    }
}

void AdminWindow::onAddProduct()
{
    showAddProductForm();
}

void AdminWindow::onEditProduct()
{
    int productId = getSelectedRowId(productsTable);
    if (productId > 0) {
        showAddProductForm(productId);
    }
}

void AdminWindow::showAddProductForm(int productId)
{
    AddProductForm form(productId, this);
    if (form.exec() == QDialog::Accepted) {
        loadProductsData();
    }
}

void AdminWindow::onDeleteProduct()
{
    int productId = getSelectedRowId(productsTable);
    if (productId > 0)
    {
        QMessageBox::StandardButton reply = QMessageBox::question(this, "Удалить товар",
                                                                  QString("Вы уверены, что хотите удалить товар ID: %1?").arg(productId),
                                                                  QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes)
        {
            Database db;
            if (!db.connectToDatabase())
            {
                QMessageBox::critical(this, "Ошибка", "Не удалось подключиться к базе данных");
                return;
            }

            if (db.deleteProduct(productId))
            {
                loadProductsData();
                QMessageBox::information(this, "Успех", "Товар удален");
            }
            else
            {
                QMessageBox::warning(this, "Ошибка", "Не удалось удалить товар");
            }
        }
    }
}

void AdminWindow::onAddSupply()
{
    showAddSupplyForm();
}

void AdminWindow::showAddSupplyForm()
{
    AddSupplyForm form(this, currentUserId);
    if (form.exec() == QDialog::Accepted) {
        loadSuppliesData();
    }
}

void AdminWindow::showReceiptForm(int saleId)
{
    SalesReceiptForm form(saleId, this);
    form.exec();
}

void AdminWindow::onDeleteSupply()
{
    int supplyId = getSelectedRowId(suppliesTable);
    if (supplyId > 0)
    {
        QMessageBox::StandardButton reply = QMessageBox::question(this, "Удалить поставку",
                                                                  QString("Вы уверены, что хотите удалить поставку ID: %1?").arg(supplyId),
                                                                  QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes)
        {
            Database db;
            if (!db.connectToDatabase())
            {
                QMessageBox::critical(this, "Ошибка", "Не удалось подключиться к базе данных");
                return;
            }

            if (db.deleteSupply(supplyId))
            {
                loadSuppliesData();
                QMessageBox::information(this, "Успех", "Поставка удалена");
            }
            else
            {
                QMessageBox::warning(this, "Ошибка", "Не удалось удалить поставку");
            }
        }
    }
}

void AdminWindow::onExportTable()
{
    QTableWidget *currentTable = nullptr;
    QString defaultName;

    if (ui->rbProduct->isChecked())
    {
        currentTable = productsTable;
        defaultName = "товары.csv";
    }
    else if (ui->rbSupply->isChecked())
    {
        currentTable = suppliesTable;
        defaultName = "поставки.csv";
    }
    else if (ui->rbSale->isChecked())
    {
        currentTable = salesTable;
        defaultName = "продажи.csv";
    }

    if (currentTable)
    {
        saveTableToCSV(currentTable);
    }
}

void AdminWindow::onTableSelectionChanged()
{

    bool hasSelection = false;

    if (ui->rbProduct->isChecked())
    {
        hasSelection = !productsTable->selectedItems().isEmpty();

        QList<QAction *> actions = productsToolBar->actions();
        if (actions.size() >= 3)
        {
            actions[1]->setEnabled(hasSelection);
            actions[2]->setEnabled(hasSelection);
        }
    }
    else if (ui->rbSupply->isChecked())
    {
        hasSelection = !suppliesTable->selectedItems().isEmpty();

        QList<QAction *> actions = suppliesToolBar->actions();
        if (actions.size() >= 2)
        {
            actions[1]->setEnabled(hasSelection);
        }
    }
}

void AdminWindow::on_pbAdminAccount_clicked()
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

QString AdminWindow::tableToCSV(QTableWidget *table)
{
    QString csv;
    QTextStream stream(&csv);

    for (int i = 0; i < table->columnCount(); i++)
    {
        if (!table->isColumnHidden(i))
        {
            stream << "\"" << table->horizontalHeaderItem(i)->text() << "\"";
            if (i < table->columnCount() - 1)
            {
                stream << ";";
            }
        }
    }
    stream << "\n";

    for (int row = 0; row < table->rowCount(); row++)
    {
        for (int col = 0; col < table->columnCount(); col++)
        {
            if (!table->isColumnHidden(col))
            {
                QTableWidgetItem *item = table->item(row, col);
                if (item && !item->text().isEmpty())
                {
                    stream << "\"" << item->text() << "\"";
                }
                else
                {
                    stream << "\"\"";
                }

                if (col < table->columnCount() - 1)
                {
                    stream << ";";
                }
            }
        }
        stream << "\n";
    }

    return csv;
}

void AdminWindow::saveTableToCSV(QTableWidget *table)
{
    QString defaultName;
    if (ui->rbProduct->isChecked())
    {
        defaultName = "товары.csv";
    }
    else if (ui->rbSupply->isChecked())
    {
        defaultName = "поставки.csv";
    }
    else if (ui->rbSale->isChecked())
    {
        defaultName = "продажи.csv";
    }

    QString fileName = QFileDialog::getSaveFileName(this, "Экспорт таблицы",
                                                    defaultName, "CSV Files (*.csv);;All Files (*)");

    if (!fileName.isEmpty())
    {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream stream(&file);

            stream << tableToCSV(table);
            file.close();

            QMessageBox::information(this, "Успех",
                                     QString("Таблица успешно экспортирована в:\n%1").arg(fileName));
        }
        else
        {
            QMessageBox::warning(this, "Ошибка", "Не удалось сохранить файл");
        }
    }
}

int AdminWindow::getSelectedRowId(QTableWidget *table, int column)
{
    QList<QTableWidgetItem *> selectedItems = table->selectedItems();
    if (!selectedItems.isEmpty())
    {

        int row = selectedItems.first()->row();
        QTableWidgetItem *idItem = table->item(row, column);
        if (idItem)
        {
            return idItem->text().toInt();
        }
    }
    return -1;
}
