USE ADS_cloud_kitchen;

CREATE TABLE IF NOT EXISTS delivery (
    delivery_id     INT             NOT NULL AUTO_INCREMENT,
    customer_name   VARCHAR(100)    NOT NULL,
    address         VARCHAR(255)    NOT NULL,
    menu_id         INT             NOT NULL,
    quantity        INT             NOT NULL DEFAULT 1,
    priority        TINYINT         NOT NULL DEFAULT 2,
    status          ENUM('PENDING', 'OUT_FOR_DELIVERY', 'DELIVERED', 'CANCELLED') NOT NULL DEFAULT 'PENDING',
    staff_id        INT             NULL,
    order_time      TIMESTAMP       NOT NULL DEFAULT CURRENT_TIMESTAMP,
    delivered_time  TIMESTAMP       NULL,

    PRIMARY KEY (delivery_id),
    CONSTRAINT fk_delivery_menu  FOREIGN KEY (menu_id)  REFERENCES menu(menu_id),
    CONSTRAINT fk_delivery_staff FOREIGN KEY (staff_id) REFERENCES staff(staff_id) ON DELETE SET NULL,
    CONSTRAINT chk_delivery_quantity CHECK (quantity > 0),
    CONSTRAINT chk_delivery_priority CHECK (priority BETWEEN 1 AND 3)
);