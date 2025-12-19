#ifndef ADDSUPPLYFORM_H
#define ADDSUPPLYFORM_H

#include <QDialog>
#include "database.h"

namespace Ui {
class AddSupplyForm;
}

class AddSupplyForm : public QDialog
{
    Q_OBJECT

public:
    explicit AddSupplyForm(QWidget *parent = nullptr, int userId = 1);
    ~AddSupplyForm();

private slots:
    void onSaveClicked();
    void onCancelClicked();
    void onProductChanged(int index);
    void onAddNewProductClicked();
    void onQuantityChanged(int value);

private:
    Ui::AddSupplyForm *ui;
    Database db;
    int currentUserId;

    void loadProducts();
    void calculateTotal();
    bool validateForm();
};

#endif // ADDSUPPLYFORM_H
