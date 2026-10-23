USE ADS_cloud_kitchen;

CREATE TABLE IF NOT EXISTS staff (
    staff_id       INT             NOT NULL AUTO_INCREMENT,
    f_name         VARCHAR(100)    NOT NULL,
    l_name         VARCHAR(100)    NOT NULL,
    email          VARCHAR(255)    NOT NULL,
    contact_number VARCHAR(20)     NULL,
    role           ENUM('CHEF', 'DELIVERY') NOT NULL,
    hiring_date    DATE            NOT NULL,
    salary         DECIMAL(10,2)   NOT NULL,

    PRIMARY KEY (staff_id),
    CONSTRAINT uq_staff_email  UNIQUE (email),
    CONSTRAINT chk_staff_salary CHECK (salary >= 0)
);
