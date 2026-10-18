USE ADS_cloud_kitchen;

CREATE TABLE IF NOT EXISTS users (
    user_id         INT             NOT NULL AUTO_INCREMENT,
    f_name          VARCHAR(100)    NOT NULL,
    l_name          VARCHAR(100)    NOT NULL,
    email           VARCHAR(255)    NOT NULL,
    date_of_birth   DATE            NULL,
    contact_number  VARCHAR(20)     NULL,
    house_no        VARCHAR(20)     NULL,
    soc_name        VARCHAR(150)    NULL,
    street          VARCHAR(150)    NULL,
    latitude        DECIMAL(10,7)   NULL,
    longitude       DECIMAL(10,7)   NULL,
    city            VARCHAR(100)    NULL,
    state           VARCHAR(100)    NULL,

    PRIMARY KEY (user_id),
    CONSTRAINT uq_users_email UNIQUE (email),
    CONSTRAINT chk_users_latitude  CHECK (latitude BETWEEN -90 AND 90 OR latitude IS NULL),
    CONSTRAINT chk_users_longitude CHECK (longitude BETWEEN -180 AND 180 OR longitude IS NULL)
);
