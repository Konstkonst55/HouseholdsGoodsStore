#ifndef ADDPRODUCTFORM_H
#define ADDPRODUCTFORM_H

#include <QDialog>
#include "database.h"

namespace Ui {
class AddProductForm;
}

class AddProductForm : public QDialog
{
    Q_OBJECT

public:
    explicit AddProductForm(int productId = -1, QWidget *parent = nullptr);
    ~AddProductForm();

private slots:
    void onSaveClicked();
    void onCancelClicked();

private:
    Ui::AddProductForm *ui;
    int productId;
    Database db;

    void loadCategories();
    void loadProductData();
    bool validateForm();
};

#endif // ADDPRODUCTFORM_H
