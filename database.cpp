#include "database.h"
#include <QCryptographicHash>

Database::Database(QObject *parent) : QObject(parent)
{
}

Database::~Database()
{
    if (db.isOpen())
    {
        db.close();
    }
}

static QString hashPassword(const QString &password)
{
    QByteArray hash = QCryptographicHash::hash(
        password.toUtf8(),
        QCryptographicHash::Sha256
        );
    return hash.toHex();
}

bool Database::initializeDatabase()
{
    QString targetDb = "shop.db";

    if (QFile::exists(targetDb))
    {
        qDebug() << "Database already exists, skipping template copy.";
        return true;
    }

    qDebug() << "Creating new database from template...";

    QFile templateDb(":/scripts/template.db");
    if (!templateDb.exists())
    {
        qDebug() << "Template database not found in resources!";
        return false;
    }

    if (QFile::exists(targetDb))
    {
        QFile::remove(targetDb);
    }

    if (!templateDb.copy(targetDb))
    {
        qDebug() << "Failed to create database from template:" << templateDb.errorString();
        return false;
    }

    QFile::setPermissions(targetDb, QFile::ReadOwner | QFile::WriteOwner | QFile::ReadUser | QFile::WriteUser | QFile::ReadGroup | QFile::WriteGroup);

    qDebug() << "Database created successfully from template.";
    return true;
}

bool Database::connectToDatabase(const QString &dbName)
{

    if (db.isOpen())
    {
        db.close();
    }

    db = QSqlDatabase::addDatabase("QSQLITE", "shop_connection");
    db.setDatabaseName(dbName);

    if (!db.open())
    {
        qDebug() << "Ошибка подключения к базе данных:" << db.lastError().text();
        return false;
    }

    qDebug() << "Successfully connected to database:" << dbName;
    return true;
}

bool Database::executeQuery(QSqlQuery &query, const QString &queryText)
{
    try
    {
        bool result;
        if (queryText.isEmpty())
        {
            result = query.exec();
        }
        else
        {
            result = query.exec(queryText);
        }

        if (!result)
        {
            QString errorText = query.lastError().text();
            qDebug() << "Ошибка SQL:" << errorText;

            return false;
        }

        return true;
    }
    catch (const std::exception &e)
    {
        qDebug() << "Исключение в executeQuery:" << e.what();
        throw;
    }
    catch (...)
    {
        qDebug() << "Неизвестное исключение в executeQuery";
        throw;
    }
}

QSqlQuery Database::prepareQuery(const QString &queryText)
{
    QSqlQuery query(db);
    query.prepare(queryText);
    return query;
}

User Database::authenticateUser(const QString &login, const QString &password)
{
    User user;
    user.id = -1;

    QSqlQuery query = prepareQuery(
        "SELECT id, login, password, role, created_at FROM users WHERE login = :login"
        );
    query.bindValue(":login", login);

    if (!executeQuery(query, "")) {
        return user;
    }

    if (query.next()) {
        QString storedHash = query.value("password").toString();
        QString inputHash = hashPassword(password);

        if (storedHash == inputHash) {
            user.id = query.value("id").toInt();
            user.login = query.value("login").toString();
            user.password = storedHash;
            user.role = query.value("role").toString();
            user.createdAt = query.value("created_at").toDateTime();
        }
    }

    return user;
}

QList<Product> Database::getAllProducts()
{
    QList<Product> products;

    QSqlQuery query = prepareQuery(
        "SELECT p.id, p.article, p.name, p.category_id, pc.name, "
        "p.purchase_price, p.retail_price, p.stock, p.created_at, p.updated_at "
        "FROM products p "
        "LEFT JOIN product_categories pc ON p.category_id = pc.id "
        "ORDER BY p.name");

    if (!executeQuery(query, ""))
    {
        return products;
    }

    while (query.next())
    {
        Product product;
        product.id = query.value(0).toInt();
        product.article = query.value(1).toString();
        product.name = query.value(2).toString();
        product.categoryId = query.value(3).toInt();
        product.categoryName = query.value(4).toString();
        product.purchasePrice = query.value(5).toDouble();
        product.retailPrice = query.value(6).toDouble();
        product.stock = query.value(7).toInt();
        product.createdAt = query.value(8).toDateTime();
        product.updatedAt = query.value(9).toDateTime();

        products.append(product);
    }

    return products;
}

QList<Supply> Database::getAllSupplies()
{
    QList<Supply> supplies;

    QSqlQuery query = prepareQuery(
        "SELECT s.id, s.supply_number, s.supplier_name, s.product_id, p.name, "
        "s.quantity, s.purchase_price, s.total_amount, s.supply_date, "
        "s.created_by, u.login, s.created_at "
        "FROM supplies s "
        "JOIN products p ON s.product_id = p.id "
        "JOIN users u ON s.created_by = u.id "
        "ORDER BY s.supply_date DESC");

    if (!executeQuery(query, ""))
    {
        return supplies;
    }

    while (query.next())
    {
        Supply supply;
        supply.id = query.value(0).toInt();
        supply.supplyNumber = query.value(1).toString();
        supply.supplierName = query.value(2).toString();
        supply.productId = query.value(3).toInt();
        supply.productName = query.value(4).toString();
        supply.quantity = query.value(5).toInt();
        supply.purchasePrice = query.value(6).toDouble();
        supply.totalAmount = query.value(7).toDouble();
        supply.supplyDate = query.value(8).toDateTime();
        supply.createdBy = query.value(9).toInt();
        supply.createdByName = query.value(10).toString();
        supply.createdAt = query.value(11).toDateTime();

        supplies.append(supply);
    }

    return supplies;
}

QList<Sale> Database::getAllSales()
{
    QList<Sale> sales;

    QSqlQuery query = prepareQuery(
        "SELECT sa.id, sa.receipt_number, sa.sale_date, "
        "sa.cashier_id, cashier.login, sa.customer_id, customer.login, "
        "sa.total_amount, sa.discount_amount, sa.final_amount, sa.created_at "
        "FROM sales sa "
        "LEFT JOIN users cashier ON sa.cashier_id = cashier.id "
        "LEFT JOIN users customer ON sa.customer_id = customer.id "
        "ORDER BY sa.sale_date DESC");

    if (!executeQuery(query, ""))
    {
        return sales;
    }

    while (query.next())
    {
        Sale sale;
        sale.id = query.value(0).toInt();
        sale.receiptNumber = query.value(1).toString();
        sale.saleDate = query.value(2).toDateTime();
        sale.cashierId = query.value(3).toInt();
        sale.cashierName = query.value(4).toString();
        sale.customerId = query.value(5).toInt();
        sale.customerName = query.value(6).toString();
        sale.totalAmount = query.value(7).toDouble();
        sale.discountAmount = query.value(8).toDouble();
        sale.finalAmount = query.value(9).toDouble();
        sale.createdAt = query.value(10).toDateTime();

        sales.append(sale);
    }

    return sales;
}

bool Database::addProduct(const Product &product)
{
    QSqlQuery query = prepareQuery(
        "INSERT INTO products (article, name, category_id, purchase_price, retail_price, stock) "
        "VALUES (:article, :name, :category_id, :purchase_price, :retail_price, :stock)");

    query.bindValue(":article", product.article);
    query.bindValue(":name", product.name);
    query.bindValue(":category_id", product.categoryId > 0 ? product.categoryId : QVariant());
    query.bindValue(":purchase_price", product.purchasePrice);
    query.bindValue(":retail_price", product.retailPrice);
    query.bindValue(":stock", product.stock);

    return executeQuery(query, "");
}

bool Database::updateProduct(const Product &product)
{
    QSqlQuery query = prepareQuery(
        "UPDATE products SET "
        "article = :article, name = :name, category_id = :category_id, "
        "purchase_price = :purchase_price, retail_price = :retail_price, stock = :stock "
        "WHERE id = :id");

    query.bindValue(":id", product.id);
    query.bindValue(":article", product.article);
    query.bindValue(":name", product.name);
    query.bindValue(":category_id", product.categoryId > 0 ? product.categoryId : QVariant());
    query.bindValue(":purchase_price", product.purchasePrice);
    query.bindValue(":retail_price", product.retailPrice);
    query.bindValue(":stock", product.stock);

    return executeQuery(query, "");
}

bool Database::deleteProduct(int productId)
{
    QSqlQuery query = prepareQuery("DELETE FROM products WHERE id = :id");
    query.bindValue(":id", productId);

    return executeQuery(query, "");
}

bool Database::addSupply(const Supply &supply, int userId)
{
    try
    {
        db.transaction();

        QSqlQuery query = prepareQuery(
            "INSERT INTO supplies (supply_number, supplier_name, product_id, quantity, "
            "purchase_price, supply_date, created_by) "
            "VALUES (:supply_number, :supplier_name, :product_id, :quantity, "
            ":purchase_price, :supply_date, :created_by)");

        query.bindValue(":supply_number", supply.supplyNumber);
        query.bindValue(":supplier_name", supply.supplierName);
        query.bindValue(":product_id", supply.productId);
        query.bindValue(":quantity", supply.quantity);
        query.bindValue(":purchase_price", supply.purchasePrice);
        query.bindValue(":supply_date", supply.supplyDate);
        query.bindValue(":created_by", userId);

        if (!executeQuery(query, ""))
        {
            db.rollback();
            return false;
        }

        db.commit();
        return true;
    }
    catch (const std::exception &e)
    {
        db.rollback();
        qDebug() << "Ошибка при добавлении поставки:" << e.what();
        return false;
    }
    catch (...)
    {
        db.rollback();
        qDebug() << "Неизвестная ошибка при добавлении поставки";
        return false;
    }
}

bool Database::deleteSupply(int supplyId)
{
    QSqlQuery query = prepareQuery("DELETE FROM supplies WHERE id = :id");
    query.bindValue(":id", supplyId);

    return executeQuery(query, "");
}

Sale Database::getSaleDetails(int saleId)
{
    Sale sale;
    sale.id = -1;

    QSqlQuery query = prepareQuery(
        "SELECT sa.id, sa.receipt_number, sa.sale_date, "
        "sa.cashier_id, cashier.login, sa.customer_id, customer.login, "
        "sa.total_amount, sa.discount_amount, sa.final_amount, sa.created_at "
        "FROM sales sa "
        "LEFT JOIN users cashier ON sa.cashier_id = cashier.id "
        "LEFT JOIN users customer ON sa.customer_id = customer.id "
        "WHERE sa.id = :id");

    query.bindValue(":id", saleId);

    if (!executeQuery(query, ""))
    {
        return sale;
    }

    if (query.next())
    {
        sale.id = query.value(0).toInt();
        sale.receiptNumber = query.value(1).toString();
        sale.saleDate = query.value(2).toDateTime();
        sale.cashierId = query.value(3).toInt();
        sale.cashierName = query.value(4).toString();
        sale.customerId = query.value(5).toInt();
        sale.customerName = query.value(6).toString();
        sale.totalAmount = query.value(7).toDouble();
        sale.discountAmount = query.value(8).toDouble();
        sale.finalAmount = query.value(9).toDouble();
        sale.createdAt = query.value(10).toDateTime();
    }

    return sale;
}

QList<SaleItem> Database::getSaleItems(int saleId)
{
    QList<SaleItem> items;

    QSqlQuery query = prepareQuery(
        "SELECT si.id, si.sale_id, si.product_id, p.name, "
        "si.quantity, si.retail_price, si.total_price "
        "FROM sale_items si "
        "JOIN products p ON si.product_id = p.id "
        "WHERE si.sale_id = :sale_id");

    query.bindValue(":sale_id", saleId);

    if (!executeQuery(query, ""))
    {
        return items;
    }

    while (query.next())
    {
        SaleItem item;
        item.id = query.value(0).toInt();
        item.saleId = query.value(1).toInt();
        item.productId = query.value(2).toInt();
        item.productName = query.value(3).toString();
        item.quantity = query.value(4).toInt();
        item.retailPrice = query.value(5).toDouble();
        item.totalPrice = query.value(6).toDouble();

        items.append(item);
    }

    return items;
}

ProfitReport Database::generateProfitReport(const QDate &startDate, const QDate &endDate)
{
    ProfitReport report;
    report.startDate = startDate;
    report.endDate = endDate;

    QSqlQuery query = prepareQuery(
        "SELECT "
        "SUM(si.total_price) as revenue, "
        "SUM(si.quantity * p.purchase_price) as cost "
        "FROM sale_items si "
        "JOIN products p ON si.product_id = p.id "
        "JOIN sales sa ON si.sale_id = sa.id "
        "WHERE DATE(sa.sale_date) BETWEEN :start_date AND :end_date");

    query.bindValue(":start_date", startDate.toString("yyyy-MM-dd"));
    query.bindValue(":end_date", endDate.toString("yyyy-MM-dd"));

    if (executeQuery(query, "") && query.next())
    {
        report.totalRevenue = query.value(0).toDouble();
        report.totalCost = query.value(1).toDouble();
        report.totalProfit = report.totalRevenue - report.totalCost;
    }

    query = prepareQuery(
        "SELECT p.name, SUM(si.quantity) as total_quantity "
        "FROM sale_items si "
        "JOIN products p ON si.product_id = p.id "
        "JOIN sales sa ON si.sale_id = sa.id "
        "WHERE DATE(sa.sale_date) BETWEEN :start_date AND :end_date "
        "GROUP BY p.id, p.name "
        "ORDER BY total_quantity DESC "
        "LIMIT 10");

    query.bindValue(":start_date", startDate.toString("yyyy-MM-dd"));
    query.bindValue(":end_date", endDate.toString("yyyy-MM-dd"));

    if (executeQuery(query, ""))
    {
        while (query.next())
        {
            report.popularProducts.append(
                qMakePair(query.value(0).toString(), query.value(1).toInt()));
        }
    }

    return report;
}

QList<Product> Database::getProductsForCashier()
{

    QList<Product> products;

    QSqlQuery query = prepareQuery(
        "SELECT p.id, p.article, p.name, p.category_id, pc.name, "
        "p.purchase_price, p.retail_price, p.stock, p.created_at, p.updated_at "
        "FROM products p "
        "LEFT JOIN product_categories pc ON p.category_id = pc.id "
        "WHERE p.stock > 0 "
        "ORDER BY p.name");

    if (!executeQuery(query, ""))
    {
        return products;
    }

    while (query.next())
    {
        Product product;
        product.id = query.value(0).toInt();
        product.article = query.value(1).toString();
        product.name = query.value(2).toString();
        product.categoryId = query.value(3).toInt();
        product.categoryName = query.value(4).toString();
        product.purchasePrice = query.value(5).toDouble();
        product.retailPrice = query.value(6).toDouble();
        product.stock = query.value(7).toInt();
        product.createdAt = query.value(8).toDateTime();
        product.updatedAt = query.value(9).toDateTime();

        products.append(product);
    }

    return products;
}

int Database::createSale(Sale &sale, const QList<SaleItem> &items)
{
    try
    {
        db.transaction();

        foreach (const SaleItem &item, items)
        {
            QSqlQuery checkQuery = prepareQuery(
                "SELECT stock FROM products WHERE id = :product_id");
            checkQuery.bindValue(":product_id", item.productId);

            if (!executeQuery(checkQuery, "") || !checkQuery.next())
            {
                db.rollback();
                qDebug() << "Товар не найден:" << item.productId;
                return -1;
            }

            int availableStock = checkQuery.value(0).toInt();
            if (availableStock < item.quantity)
            {
                db.rollback();
                qDebug() << QString("Недостаточно товара на складе. Товар ID: %1, Доступно: %2, Заказано: %3")
                                .arg(item.productId)
                                .arg(availableStock)
                                .arg(item.quantity);
                return -1;
            }
        }

        QSqlQuery query = prepareQuery(
            "INSERT INTO sales (receipt_number, sale_date, cashier_id, customer_id, "
            "total_amount, discount_amount) "
            "VALUES (:receipt_number, :sale_date, :cashier_id, :customer_id, "
            ":total_amount, :discount_amount)");

        sale.receiptNumber = generateReceiptNumber();
        sale.saleDate = QDateTime::currentDateTime();

        query.bindValue(":receipt_number", sale.receiptNumber);
        query.bindValue(":sale_date", sale.saleDate);
        query.bindValue(":cashier_id", sale.cashierId > 0 ? sale.cashierId : QVariant());
        query.bindValue(":customer_id", sale.customerId > 0 ? sale.customerId : QVariant());
        query.bindValue(":total_amount", sale.totalAmount);
        query.bindValue(":discount_amount", sale.discountAmount);

        if (!executeQuery(query, ""))
        {
            db.rollback();
            qDebug() << "Ошибка при вставке продажи:" << query.lastError().text();
            return -1;
        }

        int saleId = query.lastInsertId().toInt();

        foreach (const SaleItem &item, items)
        {
            query = prepareQuery(
                "INSERT INTO sale_items (sale_id, product_id, quantity, retail_price, total_price) "
                "VALUES (:sale_id, :product_id, :quantity, :retail_price, :total_price)");

            query.bindValue(":sale_id", saleId);
            query.bindValue(":product_id", item.productId);
            query.bindValue(":quantity", item.quantity);
            query.bindValue(":retail_price", item.retailPrice);
            query.bindValue(":total_price", item.totalPrice);

            if (!executeQuery(query, ""))
            {
                db.rollback();
                qDebug() << "Ошибка при вставке товара продажи:" << query.lastError().text();
                return -1;
            }
        }

        db.commit();
        sale.id = saleId;

        Sale finalSale = getSaleDetails(saleId);
        sale.finalAmount = finalSale.finalAmount;
        sale.cashierName = finalSale.cashierName;
        sale.customerName = finalSale.customerName;

        return saleId;
    }
    catch (const std::exception &e)
    {
        db.rollback();
        qDebug() << "Ошибка при создании продажи:" << e.what();
        return -1;
    }
    catch (...)
    {
        db.rollback();
        qDebug() << "Неизвестная ошибка при создании продажи";
        return -1;
    }
}

QList<Sale> Database::getSalesByCashier(int cashierId)
{
    QList<Sale> sales;

    QSqlQuery query = prepareQuery(
        "SELECT sa.id, sa.receipt_number, sa.sale_date, "
        "sa.cashier_id, cashier.login, sa.customer_id, customer.login, "
        "sa.total_amount, sa.discount_amount, sa.final_amount, sa.created_at "
        "FROM sales sa "
        "LEFT JOIN users cashier ON sa.cashier_id = cashier.id "
        "LEFT JOIN users customer ON sa.customer_id = customer.id "
        "WHERE sa.cashier_id = :cashier_id "
        "ORDER BY sa.sale_date DESC");

    query.bindValue(":cashier_id", cashierId);

    if (!executeQuery(query, ""))
    {
        return sales;
    }

    while (query.next())
    {
        Sale sale;
        sale.id = query.value(0).toInt();
        sale.receiptNumber = query.value(1).toString();
        sale.saleDate = query.value(2).toDateTime();
        sale.cashierId = query.value(3).toInt();
        sale.cashierName = query.value(4).toString();
        sale.customerId = query.value(5).toInt();
        sale.customerName = query.value(6).toString();
        sale.totalAmount = query.value(7).toDouble();
        sale.discountAmount = query.value(8).toDouble();
        sale.finalAmount = query.value(9).toDouble();
        sale.createdAt = query.value(10).toDateTime();

        sales.append(sale);
    }

    return sales;
}

QList<Product> Database::getProductsForClient()
{
    return getProductsForCashier();
}

bool Database::addToCart(int userId, int productId, int quantity)
{
    try
    {
        db.transaction();

        QSqlQuery query = prepareQuery(
            "INSERT INTO cart_items (user_id, product_id, quantity) "
            "VALUES (:user_id, :product_id, :quantity) "
            "ON CONFLICT(user_id, product_id) DO UPDATE SET "
            "quantity = quantity + :quantity");

        query.bindValue(":user_id", userId);
        query.bindValue(":product_id", productId);
        query.bindValue(":quantity", quantity);

        if (!executeQuery(query, ""))
        {
            db.rollback();
            return false;
        }

        db.commit();
        return true;
    }
    catch (...)
    {
        db.rollback();
        return false;
    }
}

bool Database::removeFromCart(int userId, int productId)
{
    QSqlQuery query = prepareQuery(
        "DELETE FROM cart_items WHERE user_id = :user_id AND product_id = :product_id");

    query.bindValue(":user_id", userId);
    query.bindValue(":product_id", productId);

    return executeQuery(query, "");
}

bool Database::updateCartItemQuantity(int userId, int productId, int quantity)
{
    if (quantity <= 0)
        return removeFromCart(userId, productId);

    QSqlQuery query = prepareQuery(
        "UPDATE cart_items SET quantity = :quantity "
        "WHERE user_id = :user_id AND product_id = :product_id");

    query.bindValue(":user_id", userId);
    query.bindValue(":product_id", productId);
    query.bindValue(":quantity", quantity);

    return executeQuery(query, "");
}

QList<CartItem> Database::getCartItems(int userId)
{
    QList<CartItem> cartItems;

    QSqlQuery query = prepareQuery(
        "SELECT ci.id, ci.user_id, ci.product_id, p.name, p.retail_price, ci.quantity, ci.added_at "
        "FROM cart_items ci "
        "JOIN products p ON ci.product_id = p.id "
        "WHERE ci.user_id = :user_id "
        "ORDER BY ci.added_at DESC");

    query.bindValue(":user_id", userId);

    if (!executeQuery(query, ""))
    {
        return cartItems;
    }

    while (query.next())
    {
        CartItem item;
        item.id = query.value(0).toInt();
        item.userId = query.value(1).toInt();
        item.productId = query.value(2).toInt();
        item.productName = query.value(3).toString();
        item.retailPrice = query.value(4).toDouble();
        item.quantity = query.value(5).toInt();
        item.addedAt = query.value(6).toDateTime();

        cartItems.append(item);
    }

    return cartItems;
}

bool Database::clearCart(int userId)
{
    QSqlQuery query = prepareQuery(
        "DELETE FROM cart_items WHERE user_id = :user_id");

    query.bindValue(":user_id", userId);

    return executeQuery(query, "");
}

bool Database::createSaleForClient(Sale &sale)
{
    try
    {
        db.transaction();

        QList<CartItem> cartItems = getCartItems(sale.customerId);
        if (cartItems.isEmpty())
        {
            db.rollback();
            return false;
        }

        double totalAmount = 0;
        QList<SaleItem> saleItems;

        for (const CartItem &cartItem : cartItems)
        {
            SaleItem item;
            item.productId = cartItem.productId;
            item.quantity = cartItem.quantity;
            item.retailPrice = cartItem.retailPrice;
            item.totalPrice = cartItem.quantity * cartItem.retailPrice;

            totalAmount += item.totalPrice;
            saleItems.append(item);
        }

        sale.receiptNumber = generateReceiptNumber();
        sale.saleDate = QDateTime::currentDateTime();
        sale.totalAmount = totalAmount;

        QSqlQuery query = prepareQuery(
            "INSERT INTO sales (receipt_number, sale_date, customer_id, total_amount, discount_amount) "
            "VALUES (:receipt_number, :sale_date, :customer_id, :total_amount, :discount_amount)");

        query.bindValue(":receipt_number", sale.receiptNumber);
        query.bindValue(":sale_date", sale.saleDate);
        query.bindValue(":customer_id", sale.customerId);
        query.bindValue(":total_amount", sale.totalAmount);
        query.bindValue(":discount_amount", sale.discountAmount);

        if (!executeQuery(query, ""))
        {
            db.rollback();
            return false;
        }

        int saleId = query.lastInsertId().toInt();

        for (const SaleItem &item : saleItems)
        {
            query = prepareQuery(
                "INSERT INTO sale_items (sale_id, product_id, quantity, retail_price, total_price) "
                "VALUES (:sale_id, :product_id, :quantity, :retail_price, :total_price)");

            query.bindValue(":sale_id", saleId);
            query.bindValue(":product_id", item.productId);
            query.bindValue(":quantity", item.quantity);
            query.bindValue(":retail_price", item.retailPrice);
            query.bindValue(":total_price", item.totalPrice);

            if (!executeQuery(query, ""))
            {
                db.rollback();
                return false;
            }
        }

        // 🔥 ОЧИСТКА КОРЗИНЫ → триггер вернёт ничего (уже продано)
        query = prepareQuery("DELETE FROM cart_items WHERE user_id = :user_id");
        query.bindValue(":user_id", sale.customerId);

        if (!executeQuery(query, ""))
        {
            db.rollback();
            return false;
        }

        db.commit();
        sale.id = saleId;
        return true;
    }
    catch (...)
    {
        db.rollback();
        return false;
    }
}

User Database::getUserById(int userId)
{
    User user;
    user.id = -1;

    QSqlQuery query = prepareQuery(
        "SELECT id, login, password, role, created_at FROM users WHERE id = :id");
    query.bindValue(":id", userId);

    if (!executeQuery(query, ""))
    {
        return user;
    }

    if (query.next())
    {
        user.id = query.value(0).toInt();
        user.login = query.value(1).toString();
        user.password = query.value(2).toString();
        user.role = query.value(3).toString();
        user.createdAt = query.value(4).toDateTime();
    }

    return user;
}

Product Database::getProductById(int productId)
{
    Product product;
    product.id = -1;

    QSqlQuery query = prepareQuery(
        "SELECT p.id, p.article, p.name, p.category_id, pc.name, "
        "p.purchase_price, p.retail_price, p.stock, p.created_at, p.updated_at "
        "FROM products p "
        "LEFT JOIN product_categories pc ON p.category_id = pc.id "
        "WHERE p.id = :id");

    query.bindValue(":id", productId);

    if (!executeQuery(query, ""))
    {
        return product;
    }

    if (query.next())
    {
        product.id = query.value(0).toInt();
        product.article = query.value(1).toString();
        product.name = query.value(2).toString();
        product.categoryId = query.value(3).toInt();
        product.categoryName = query.value(4).toString();
        product.purchasePrice = query.value(5).toDouble();
        product.retailPrice = query.value(6).toDouble();
        product.stock = query.value(7).toInt();
        product.createdAt = query.value(8).toDateTime();
        product.updatedAt = query.value(9).toDateTime();
    }

    return product;
}

QString Database::generateReceiptNumber()
{
    QString prefix = "CHK";
    QDateTime now = QDateTime::currentDateTime();
    QString timestamp = now.toString("yyyyMMddHHmmss");

    QString random = QString::number(QRandomGenerator::global()->bounded(10000)).rightJustified(4, '0');

    return prefix + timestamp + random;
}

QList<ProductCategory> Database::getAllCategories()
{
    QList<ProductCategory> categories;

    QSqlQuery query = prepareQuery(
        "SELECT id, name, created_at FROM product_categories ORDER BY name");

    if (!executeQuery(query, "")) {
        return categories;
    }

    while (query.next()) {
        ProductCategory category;
        category.id = query.value(0).toInt();
        category.name = query.value(1).toString();
        category.createdAt = query.value(2).toDateTime();

        categories.append(category);
    }

    return categories;
}

ProductCategory Database::getCategoryById(int categoryId)
{
    ProductCategory category;
    category.id = -1;

    QSqlQuery query = prepareQuery(
        "SELECT id, name, created_at FROM product_categories WHERE id = :id");
    query.bindValue(":id", categoryId);

    if (!executeQuery(query, "")) {
        return category;
    }

    if (query.next()) {
        category.id = query.value(0).toInt();
        category.name = query.value(1).toString();
        category.createdAt = query.value(2).toDateTime();
    }

    return category;
}

bool Database::addCategory(const QString &name)
{
    QSqlQuery query = prepareQuery(
        "INSERT INTO product_categories (name) VALUES (:name)");
    query.bindValue(":name", name);

    return executeQuery(query, "");
}

QString Database::generateSupplyNumber()
{
    QString prefix = "SUP";
    QDateTime now = QDateTime::currentDateTime();
    QString timestamp = now.toString("yyyyMMddHHmmss");

    QString random = QString::number(QRandomGenerator::global()->bounded(10000)).rightJustified(4, '0');

    return prefix + timestamp + random;
}

bool Database::checkProductAvailability(int productId, int requestedQuantity)
{
    try
    {
        QSqlQuery query = prepareQuery(
            "SELECT stock FROM products WHERE id = :product_id");
        query.bindValue(":product_id", productId);

        if (!executeQuery(query, ""))
        {
            return false;
        }

        if (query.next())
        {
            int availableStock = query.value(0).toInt();
            return availableStock >= requestedQuantity;
        }

        return false;
    }
    catch (const std::exception &e)
    {
        qDebug() << "Ошибка при проверке доступности товара:" << e.what();
        return false;
    }
    catch (...)
    {
        qDebug() << "Неизвестная ошибка при проверке доступности товара";
        return false;
    }
}
