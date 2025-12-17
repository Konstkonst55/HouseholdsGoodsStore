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

namespace Ui
{
    class AdminWindow;
}

class AdminWindow : public QWidget
{
    Q_OBJECT

public:
    explicit AdminWindow(QWidget *parent = nullptr);
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

private:
    Ui::AdminWindow *ui;

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

    void setupMenuBar();
    void setupPages();
    void setupProductsPage();
    void setupSuppliesPage();
    void setupSalesPage();

    void loadProductsData();
    void loadSuppliesData();
    void loadSalesData();

    QString tableToCSV(QTableWidget *table);
    void saveTableToCSV(QTableWidget *table);

    int getSelectedRowId(QTableWidget *table, int column = 0);
};

#endif // ADMINWINDOW_H
