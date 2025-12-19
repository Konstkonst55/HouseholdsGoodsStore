PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    login TEXT UNIQUE NOT NULL,
    password TEXT NOT NULL,
    role TEXT NOT NULL CHECK (role IN ('Администратор', 'Кассир', 'Клиент')),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS product_categories (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT UNIQUE NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS products (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    article TEXT UNIQUE NOT NULL,
    name TEXT NOT NULL,
    category_id INTEGER,
    purchase_price REAL NOT NULL CHECK (purchase_price >= 0),
    retail_price REAL NOT NULL CHECK (retail_price >= 0),
    stock INTEGER NOT NULL DEFAULT 0 CHECK (stock >= 0),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (category_id) REFERENCES product_categories(id) ON DELETE SET NULL
);

CREATE TABLE IF NOT EXISTS supplies (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    supply_number TEXT UNIQUE NOT NULL,
    supplier_name TEXT NOT NULL,
    product_id INTEGER NOT NULL,
    quantity INTEGER NOT NULL CHECK (quantity > 0),
    purchase_price REAL NOT NULL CHECK (purchase_price >= 0),
    total_amount REAL,
    supply_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    created_by INTEGER NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (product_id) REFERENCES products(id),
    FOREIGN KEY (created_by) REFERENCES users(id)
);

CREATE TABLE IF NOT EXISTS sales (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    receipt_number TEXT UNIQUE NOT NULL,
    sale_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    cashier_id INTEGER,
    customer_id INTEGER,
    total_amount REAL NOT NULL DEFAULT 0 CHECK (total_amount >= 0),
    discount_amount REAL DEFAULT 0 CHECK (discount_amount >= 0),
    final_amount REAL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (cashier_id) REFERENCES users(id),
    FOREIGN KEY (customer_id) REFERENCES users(id)
);

CREATE TABLE IF NOT EXISTS sale_items (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    sale_id INTEGER NOT NULL,
    product_id INTEGER NOT NULL,
    quantity INTEGER NOT NULL CHECK (quantity > 0),
    retail_price REAL NOT NULL CHECK (retail_price >= 0),
    total_price REAL,
    FOREIGN KEY (sale_id) REFERENCES sales(id) ON DELETE CASCADE,
    FOREIGN KEY (product_id) REFERENCES products(id)
);

CREATE TABLE IF NOT EXISTS cart_items (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL,
    product_id INTEGER NOT NULL,
    quantity INTEGER NOT NULL DEFAULT 1 CHECK (quantity > 0),
    added_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    FOREIGN KEY (product_id) REFERENCES products(id),
    UNIQUE(user_id, product_id)
);

CREATE INDEX IF NOT EXISTS idx_products_article ON products(article);
CREATE INDEX IF NOT EXISTS idx_products_category ON products(category_id);
CREATE INDEX IF NOT EXISTS idx_products_stock ON products(stock);

CREATE INDEX IF NOT EXISTS idx_supplies_date ON supplies(supply_date);
CREATE INDEX IF NOT EXISTS idx_supplies_supplier ON supplies(supplier_name);
CREATE INDEX IF NOT EXISTS idx_supplies_product ON supplies(product_id);
CREATE INDEX IF NOT EXISTS idx_supplies_created_by ON supplies(created_by);

CREATE INDEX IF NOT EXISTS idx_sales_date ON sales(sale_date);
CREATE INDEX IF NOT EXISTS idx_sales_cashier ON sales(cashier_id);
CREATE INDEX IF NOT EXISTS idx_sales_customer ON sales(customer_id);
CREATE INDEX IF NOT EXISTS idx_sales_receipt_number ON sales(receipt_number);

CREATE INDEX IF NOT EXISTS idx_sale_items_sale ON sale_items(sale_id);
CREATE INDEX IF NOT EXISTS idx_sale_items_product ON sale_items(product_id);

CREATE INDEX IF NOT EXISTS idx_cart_items_user ON cart_items(user_id);
CREATE INDEX IF NOT EXISTS idx_cart_items_product ON cart_items(product_id);

INSERT OR IGNORE INTO users (login, password, role) VALUES
('admin', 'admin123', 'Администратор'),
('cashier1', 'cashier123', 'Кассир'),
('client1', 'client123', 'Клиент');

INSERT OR IGNORE INTO product_categories (name) VALUES
('Электроника'),
('Бытовая техника'),
('Продукты питания'),
('Одежда');

CREATE TRIGGER IF NOT EXISTS calculate_supply_total_amount
AFTER INSERT ON supplies
BEGIN
    UPDATE supplies
    SET total_amount = NEW.quantity * NEW.purchase_price
    WHERE id = NEW.id;
END;

CREATE TRIGGER IF NOT EXISTS calculate_sale_final_amount
AFTER INSERT ON sales
BEGIN
    UPDATE sales
    SET final_amount = NEW.total_amount * (1 - COALESCE(NEW.discount_amount, 0) / 100.0)
    WHERE id = NEW.id;
END;

CREATE TRIGGER IF NOT EXISTS calculate_sale_item_total_price
AFTER INSERT ON sale_items
WHEN NEW.total_price IS NULL
BEGIN
    UPDATE sale_items
    SET total_price = NEW.quantity * NEW.retail_price
    WHERE id = NEW.id;
END;

CREATE TRIGGER IF NOT EXISTS update_stock_on_supply
AFTER INSERT ON supplies
BEGIN
    UPDATE products
    SET stock = stock + NEW.quantity,
        updated_at = CURRENT_TIMESTAMP
    WHERE id = NEW.product_id;
END;

CREATE TRIGGER IF NOT EXISTS update_stock_on_sale
BEFORE INSERT ON sale_items
FOR EACH ROW
BEGIN
    SELECT CASE
        WHEN (SELECT stock FROM products WHERE id = NEW.product_id) < NEW.quantity
        THEN RAISE(ABORT, 'Недостаточно товара на складе')
    END;

    UPDATE products
    SET stock = stock - NEW.quantity,
        updated_at = CURRENT_TIMESTAMP
    WHERE id = NEW.product_id;
END;

CREATE TRIGGER IF NOT EXISTS update_sale_total_amount
AFTER INSERT ON sale_items
BEGIN
    UPDATE sales
    SET total_amount = (
        SELECT COALESCE(SUM(total_price), 0)
        FROM sale_items
        WHERE sale_id = NEW.sale_id
    )
    WHERE id = NEW.sale_id;
END;

CREATE TRIGGER IF NOT EXISTS update_sale_total_amount_on_delete
AFTER DELETE ON sale_items
BEGIN
    UPDATE sales
    SET total_amount = (
        SELECT COALESCE(SUM(total_price), 0)
        FROM sale_items
        WHERE sale_id = OLD.sale_id
    )
    WHERE id = OLD.sale_id;
END;

CREATE TRIGGER IF NOT EXISTS decrease_stock_on_cart_insert
AFTER INSERT ON cart_items
FOR EACH ROW
BEGIN
    UPDATE products
    SET stock = stock - NEW.quantity,
        updated_at = CURRENT_TIMESTAMP
    WHERE id = NEW.product_id;
END;

CREATE TRIGGER IF NOT EXISTS update_stock_on_cart_update
AFTER UPDATE OF quantity ON cart_items
FOR EACH ROW
BEGIN
    UPDATE products
    SET stock = stock - (NEW.quantity - OLD.quantity),
        updated_at = CURRENT_TIMESTAMP
    WHERE id = NEW.product_id;
END;

CREATE TRIGGER IF NOT EXISTS increase_stock_on_cart_delete
AFTER DELETE ON cart_items
FOR EACH ROW
BEGIN
    UPDATE products
    SET stock = stock + OLD.quantity,
        updated_at = CURRENT_TIMESTAMP
    WHERE id = OLD.product_id;
END;