USE ADS_cloud_kitchen;

CREATE TABLE IF NOT EXISTS menu (
    menu_id        INT             NOT NULL AUTO_INCREMENT,
    name           VARCHAR(100)    NOT NULL,
    description    VARCHAR(255)    NULL,
    price          DECIMAL(10,2)   NOT NULL,
    prep_time      INT             NOT NULL DEFAULT 10,
    is_available   BOOLEAN         NOT NULL DEFAULT TRUE,

    PRIMARY KEY (menu_id),
    CONSTRAINT uq_menu_name       UNIQUE (name),
    CONSTRAINT chk_menu_price     CHECK (price >= 0),
    CONSTRAINT chk_menu_prep_time CHECK (prep_time >= 0)
);