#ifndef ADMINWINDOW_H
#define ADMINWINDOW_H

#include <QWidget>
#include <QMenuBar>
#include <QAction>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QToolBar>
#include <QRadioButton>
#include <QComboBox>
#include "database.h"

namespace Ui
{
    class AdminWindow;
}

class AdminWindow : public QWidget
{
    Q_OBJECT

public:
    explicit AdminWindow(QWidget *parent = nullptr, int userId = 1);
    ~AdminWindow();

private slots:
    void onFileOpen();
    void onFileSaveAs();

    void onReportProfit();
    void onReportPopular();

    void onHelpAbout();
    void onHelpReference();

    void onNavigationChanged();

    void onAddProduct();
    void onEditProduct();
    void onDeleteProduct();
    void onAddSupply();
    void onDeleteSupply();
    void onExportTable();

    void onTableSelectionChanged();

    void on_pbAdminAccount_clicked();

private:
    Ui::AdminWindow *ui;
    int currentUserId;

    QMenuBar *menuBar;
    QMenu *fileMenu;
    QMenu *reportMenu;
    QMenu *helpMenu;

    QTableWidget *productsTable;
    QTableWidget *suppliesTable;
    QTableWidget *salesTable;

    QToolBar *productsToolBar;
    QToolBar *suppliesToolBar;

    QWidget *productsPage;
    QWidget *suppliesPage;
    QWidget *salesPage;

    QLabel *productsTitle;
    QLabel *suppliesTitle;
    QLabel *salesTitle;

    QPushButton *productsExportBtn;
    QPushButton *suppliesExportBtn;
    QPushButton *salesExportBtn;

    QAction *profitReportAction;
    QAction *popularReportAction;

    void setupMenuBar();
    void setupPages();
    void setupProductsPage();
    void setupSuppliesPage();
    void setupSalesPage();

    void loadProductsData();
    void loadSuppliesData();
    void loadSalesData();

    void showAddProductForm(int productId = -1);
    void showAddSupplyForm();
    void showReceiptForm(int saleId);
    void loadCategoriesToCombo(QComboBox *combo);
    void loadProductsToCombo(QComboBox *combo);

    void searchProducts(const QString &text);
    void searchSupplies(const QString &text);
    void searchSales(const QString &text);

    void updateProductsTable(const QList<Product> &products);
    void updateSuppliesTable(const QList<Supply> &supplies);
    void updateSalesTable(const QList<Sale> &sales);

    QString tableToCSV(QTableWidget *table);
    void saveTableToCSV(QTableWidget *table);

    int getSelectedRowId(QTableWidget *table, int column = 0);
};

#endif // ADMINWINDOW_H
