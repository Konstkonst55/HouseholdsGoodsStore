#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QList>
#include <QDateTime>
#include <QDebug>
#include <QRandomGenerator>
#include <QTextStream>
#include <QFile>

struct User {
    int id;
    QString login;
    QString password;
    QString role;
    QDateTime createdAt;
};

struct Product {
    int id;
    QString article;
    QString name;
    int categoryId;
    QString categoryName;
    double purchasePrice;
    double retailPrice;
    int stock;
    QDateTime createdAt;
    QDateTime updatedAt;
};

struct Supply {
    int id;
    QString supplyNumber;
    QString supplierName;
    int productId;
    QString productName;
    int quantity;
    double purchasePrice;
    double totalAmount;
    QDateTime supplyDate;
    int createdBy;
    QString createdByName;
    QDateTime createdAt;
};

struct Sale {
    int id;
    QString receiptNumber;
    QDateTime saleDate;
    int cashierId;
    QString cashierName;
    int customerId;
    QString customerName;
    double totalAmount;
    double discountAmount;
    double finalAmount;
    QDateTime createdAt;
};

struct SaleItem {
    int id;
    int saleId;
    int productId;
    QString productName;
    int quantity;
    double retailPrice;
    double totalPrice;
};

struct CartItem {
    int id;
    int userId;
    int productId;
    QString productName;
    double retailPrice;
    int quantity;
    QDateTime addedAt;
};

struct ProfitReport {
    QDate startDate;
    QDate endDate;
    double totalRevenue;
    double totalCost;
    double totalProfit;
    QList<QPair<QString, int>> popularProducts;
};

struct ProductCategory {
    int id;
    QString name;
    QDateTime createdAt;
};

class Database : public QObject
{
    Q_OBJECT

public:
    explicit Database(QObject *parent = nullptr);
    ~Database();

    bool connectToDatabase(const QString &dbName = "shop.db");
    bool initializeDatabase();

    User authenticateUser(const QString &login, const QString &password);

    QList<Product> getAllProducts();
    QList<Supply> getAllSupplies();
    QList<Sale> getAllSales();
    QList<ProductCategory> getAllCategories();

    bool addProduct(const Product &product);
    bool updateProduct(const Product &product);
    bool deleteProduct(int productId);

    bool addSupply(const Supply &supply, int userId);
    bool deleteSupply(int supplyId);

    Sale getSaleDetails(int saleId);
    QList<SaleItem> getSaleItems(int saleId);

    ProfitReport generateProfitReport(const QDate &startDate, const QDate &endDate);

    QList<Product> getProductsForCashier();
    int createSale(Sale &sale, const QList<SaleItem> &items);
    QList<Sale> getSalesByCashier(int cashierId);

    QList<Product> getProductsForClient();
    bool addToCart(int userId, int productId, int quantity = 1);
    bool removeFromCart(int userId, int productId);
    bool updateCartItemQuantity(int userId, int productId, int quantity);
    QList<CartItem> getCartItems(int userId);
    bool clearCart(int userId);

    bool createSaleForClient(Sale &sale, int customerId, double discountAmount);

    User getUserById(int userId);
    Product getProductById(int productId);
    QString generateReceiptNumber();

    ProductCategory getCategoryById(int categoryId);
    bool addCategory(const QString &name);
    QString generateSupplyNumber();

    bool checkProductAvailability(int productId, int requestedQuantity);

private:
    QSqlDatabase db;

    bool executeQuery(QSqlQuery &query, const QString &queryText);
    QSqlQuery prepareQuery(const QString &queryText);
};

#endif // DATABASE_H
