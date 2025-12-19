#include "salesreceiptform.h"
#include "ui_salesreceiptform.h"
#include <QMessageBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPrinter>
#include <QPrintDialog>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextTable>
#include <QTextTableFormat>
#include <QTextCharFormat>
#include <QTextBlockFormat>
#include <QTextLength>

SalesReceiptForm::SalesReceiptForm(int saleId, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SalesReceiptForm)
    , saleId(saleId)
{
    ui->setupUi(this);

    loadReceiptData();

    connect(ui->pbClose, &QPushButton::clicked, this, &SalesReceiptForm::onCloseClicked);
    connect(ui->pbPrint, &QPushButton::clicked, this, &SalesReceiptForm::onPrintClicked);
}

SalesReceiptForm::~SalesReceiptForm()
{
    delete ui;
}

void SalesReceiptForm::loadReceiptData()
{
    if (!db.connectToDatabase()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось подключиться к БД");
        return;
    }

    Sale sale = db.getSaleDetails(saleId);
    if (sale.id == -1) {
        QMessageBox::warning(this, "Ошибка", "Чек не найден");
        return;
    }

    ui->lReceiptNumber->setText(sale.receiptNumber);
    ui->lSaleDate->setText(sale.saleDate.toString("dd.MM.yyyy HH:mm"));
    ui->lCashier->setText(sale.cashierName);

    QList<SaleItem> items = db.getSaleItems(saleId);

    QLayoutItem* child;
    while ((child = ui->vlProducts->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    for (const SaleItem &item : items) {
        addProductRow(item.productName, item.quantity, item.retailPrice);
    }

    ui->lDiscount->setText(QString("%1%").arg(sale.discountAmount, 0, 'f', 1));
    ui->lTotal->setText(QString::number(sale.finalAmount, 'f', 2));
}

void SalesReceiptForm::addProductRow(const QString &name, int quantity, double price)
{
    QWidget *productRow = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout(productRow);
    layout->setContentsMargins(0, 2, 0, 2);

    QLabel *nameLabel = new QLabel(name);
    nameLabel->setStyleSheet("font-weight: normal;");
    layout->addWidget(nameLabel);

    QLabel *detailsLabel = new QLabel(
        QString("%1 x %2").arg(quantity).arg(price, 0, 'f', 2)
        );
    detailsLabel->setStyleSheet("font-weight: normal;");
    detailsLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    layout->addWidget(detailsLabel);

    ui->vlProducts->addWidget(productRow);
}

void SalesReceiptForm::onCloseClicked()
{
    accept();
}

void SalesReceiptForm::onPrintClicked()
{
    QTextDocument document;

    if (!db.connectToDatabase()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось подключиться к БД");
        return;
    }

    Sale sale = db.getSaleDetails(saleId);
    if (sale.id == -1) {
        QMessageBox::warning(this, "Ошибка", "Чек не найден");
        return;
    }

    QList<SaleItem> items = db.getSaleItems(saleId);

    QString html;
    html += "<html><body style='font-family: Arial, sans-serif;'>";

    html += "<div style='text-align: center; font-size: 14pt; font-weight: bold;'>";
    html += "МАГАЗИН БЫТОВЫХ ТОВАРОВ<br>";
    html += "ЧЕК ПРОДАЖИ<br><br>";
    html += "</div>";

    html += "<div style='font-size: 10pt;'>";
    html += QString("Чек №: %1<br>").arg(sale.receiptNumber);
    html += QString("Дата: %1<br>").arg(sale.saleDate.toString("dd.MM.yyyy HH:mm"));
    html += QString("Кассир: %1<br>").arg(sale.cashierName);

    if (!sale.customerName.isEmpty()) {
        html += QString("Клиент: %1<br>").arg(sale.customerName);
    }

    html += "<hr>";
    html += "</div>";

    html += "<table width='100%' border='1' cellpadding='2' cellspacing='0' style='font-size: 10pt;'>";
    html += "<tr style='background-color: #f0f0f0; font-weight: bold;'>";
    html += "<td>Товар</td><td align='center'>Кол-во</td><td align='right'>Сумма</td>";
    html += "</tr>";

    foreach (const SaleItem &item, items) {
        html += "<tr>";
        html += QString("<td>%1</td>").arg(item.productName);
        html += QString("<td align='center'>%1</td>").arg(item.quantity);
        html += QString("<td align='right'>%1</td>").arg(QString::number(item.totalPrice, 'f', 2));
        html += "</tr>";
    }

    html += "</table>";

    html += "<div style='text-align: right; font-size: 10pt; margin-top: 10px;'>";
    html += QString("Итого: %1 руб.<br>").arg(sale.totalAmount, 0, 'f', 2);

    if (sale.discountAmount > 0) {
        html += QString("Скидка: %1 руб.<br>").arg(sale.discountAmount, 0, 'f', 2);
    }

    html += QString("<b>К ОПЛАТЕ: %1 руб.</b><br>").arg(sale.finalAmount, 0, 'f', 2);
    html += "</div>";

    html += "<div style='text-align: center; margin-top: 20px; font-size: 10pt;'>";
    html += "<hr>";
    html += "<div style='font-size: 12pt; font-weight: bold;'>СПАСИБО ЗА ПОКУПКУ!</div>";
    html += "________________<br>";
    html += "Подпись кассира";
    html += "</div>";

    html += "</body></html>";

    document.setHtml(html);

    QPrinter printer(QPrinter::HighResolution);
    printer.setPageSize(QPageSize(QPageSize::A5));
    printer.setPageOrientation(QPageLayout::Portrait);

    QPrintDialog printDialog(&printer, this);
    if (printDialog.exec() == QDialog::Accepted) {
        document.print(&printer);
    }
}
