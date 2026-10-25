USE ADS_cloud_kitchen;

CREATE TABLE IF NOT EXISTS delivery_partner (
    staff_id   INT           NOT NULL,
    latitude   DECIMAL(10,7) NOT NULL,
    longitude  DECIMAL(10,7) NOT NULL,

    PRIMARY KEY (staff_id),
    CONSTRAINT chk_dp_latitude  CHECK (latitude BETWEEN -90 AND 90),
    CONSTRAINT chk_dp_longitude CHECK (longitude BETWEEN -180 AND 180),
    CONSTRAINT fk_dp_staff FOREIGN KEY (staff_id) REFERENCES staff (staff_id)
        ON UPDATE CASCADE ON DELETE CASCADE
);
