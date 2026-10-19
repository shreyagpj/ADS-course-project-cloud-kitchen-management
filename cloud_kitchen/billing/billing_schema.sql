USE ADS_cloud_kitchen;

CREATE TABLE IF NOT EXISTS billing (
    bill_id         INT           NOT NULL AUTO_INCREMENT,
    order_id        INT           NOT NULL,
    user_id         INT           NOT NULL,
    total_amount    DECIMAL(10,2) NOT NULL,
    address         VARCHAR(255)  NULL,
    payment_status  ENUM('PENDING','PAID') NOT NULL DEFAULT 'PENDING',
    payment_method  ENUM('CASH','CARD','UPI','NET_BANKING') NULL,
    transaction_id  VARCHAR(64)   NULL,
    payment_time    DATETIME      NULL,

    PRIMARY KEY (bill_id),
    UNIQUE KEY uq_billing_order       (order_id),
    UNIQUE KEY uq_billing_transaction (transaction_id),
    CONSTRAINT chk_billing_total CHECK (total_amount >= 0),
    CONSTRAINT chk_billing_paid  CHECK (payment_status = 'PENDING' OR payment_time IS NOT NULL),
    CONSTRAINT fk_billing_order FOREIGN KEY (order_id) REFERENCES orders (order_id)
        ON UPDATE CASCADE ON DELETE RESTRICT,
    CONSTRAINT fk_billing_user  FOREIGN KEY (user_id)  REFERENCES users (user_id)
        ON UPDATE CASCADE ON DELETE RESTRICT,
    INDEX idx_billing_user (user_id)
);
