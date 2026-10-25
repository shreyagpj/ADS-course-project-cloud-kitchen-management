USE ADS_cloud_kitchen;

CREATE TABLE IF NOT EXISTS orders (
    order_id         INT           NOT NULL AUTO_INCREMENT,
    user_id          INT           NOT NULL,
    chef_id          INT           NULL,
    delivery_id      INT           NULL,
    priority         TINYINT       NOT NULL DEFAULT 2,
    order_status     ENUM('PLACED','COOKING','READY','OUT_FOR_DELIVERY','DELIVERED')
                                   NOT NULL DEFAULT 'PLACED',
    order_date_time  DATETIME      NOT NULL DEFAULT CURRENT_TIMESTAMP,
    total_cost       DECIMAL(10,2) NOT NULL DEFAULT 0.00,

    PRIMARY KEY (order_id),
    CONSTRAINT chk_orders_priority CHECK (priority BETWEEN 1 AND 3),
    CONSTRAINT chk_orders_total    CHECK (total_cost >= 0),
    CONSTRAINT fk_orders_user      FOREIGN KEY (user_id)     REFERENCES users (user_id)
        ON UPDATE CASCADE ON DELETE RESTRICT,
    CONSTRAINT fk_orders_chef      FOREIGN KEY (chef_id)     REFERENCES staff (staff_id)
        ON UPDATE CASCADE ON DELETE SET NULL,
    CONSTRAINT fk_orders_delivery  FOREIGN KEY (delivery_id) REFERENCES staff (staff_id)
        ON UPDATE CASCADE ON DELETE SET NULL,
    INDEX idx_orders_user   (user_id),
    INDEX idx_orders_status (order_status, priority, order_id)
);

CREATE TABLE IF NOT EXISTS order_items (
    order_id  INT NOT NULL,
    item_id   INT NOT NULL,
    quantity  INT NOT NULL,

    PRIMARY KEY (order_id, item_id),
    CONSTRAINT chk_order_items_qty CHECK (quantity > 0),
    CONSTRAINT fk_order_items_order FOREIGN KEY (order_id) REFERENCES orders (order_id)
        ON UPDATE CASCADE ON DELETE CASCADE,
    CONSTRAINT fk_order_items_item  FOREIGN KEY (item_id)  REFERENCES menu (menu_id)
        ON UPDATE CASCADE ON DELETE RESTRICT,
    INDEX idx_order_items_item (item_id)
);
