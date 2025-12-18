#ifndef SALESRECEIPTFORM_H
#define SALESRECEIPTFORM_H

#include <QDialog>
#include <QPrinter>
#include "database.h"

namespace Ui {
class SalesReceiptForm;
}

class SalesReceiptForm : public QDialog
{
    Q_OBJECT

public:
    explicit SalesReceiptForm(int saleId, QWidget *parent = nullptr);
    ~SalesReceiptForm();

private slots:
    void onCloseClicked();
    void onPrintClicked();

private:
    Ui::SalesReceiptForm *ui;
    int saleId;
    Database db;

    void loadReceiptData();
    void addProductRow(const QString &name, int quantity, double price);
};

#endif // SALESRECEIPTFORM_H
