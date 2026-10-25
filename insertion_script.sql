USE ADS_cloud_kitchen;

-- Run order: staff, users, menu, orders (+ order_items), billing, delivery_partner.

-- ---------------------------------------------------------
-- STAFF
-- ---------------------------------------------------------
INSERT INTO staff (f_name, l_name, email, contact_number, role, hiring_date, salary) VALUES
('Priya',  'Shah',    'priya.shah@cloudkitchen.com',     '9876543211', 'CHEF',     '2023-02-15', 40000.00),
('Amit',   'Patel',   'amit.patel@cloudkitchen.com',     '9876543212', 'CHEF',     '2023-03-01', 38000.00),
('Neha',   'Desai',   'neha.desai@cloudkitchen.com',     '9876543213', 'CHEF',     '2023-06-20', 36000.00),
('Karan',  'Joshi',   'karan.joshi@cloudkitchen.com',    '9876543214', 'DELIVERY', '2023-07-05', 22000.00),
('Vikram', 'Solanki', 'vikram.solanki@cloudkitchen.com', '9876543215', 'DELIVERY', '2023-08-12', 21000.00),
('Sneha',  'Trivedi', 'sneha.trivedi@cloudkitchen.com',  '9876543216', 'DELIVERY', '2024-01-08', 21500.00),
('Manish', 'Parmar',  'manish.parmar@cloudkitchen.com',  NULL,         'DELIVERY', '2024-04-18', 20000.00);

-- ---------------------------------------------------------
-- USERS
-- ---------------------------------------------------------
INSERT INTO users (f_name, l_name, email, date_of_birth, contact_number, house_no, soc_name, street, latitude, longitude, city, state) VALUES
('Aarav',     'Gupta',  'aarav.gupta@gmail.com',     '1998-03-14', '9123456780', 'B-12',  'Shreeji Apartments',  'Sector 7 Main Road',  23.2250000, 72.6500000, 'Gandhinagar', 'Gujarat'),
('Diya',      'Nair',   'diya.nair@gmail.com',       '2000-07-22', '9123456781', '45',    'Krishna Residency',   'Sector 21 Road',      23.2020000, 72.6600000, 'Gandhinagar', 'Gujarat'),
('Rahul',     'Verma',  'rahul.verma@gmail.com',     '1995-11-05', '9123456782', 'A-304', 'Infocity Heights',    'Infocity Road',       23.1900000, 72.6350000, 'Gandhinagar', 'Gujarat'),
('Meera',     'Iyer',   'meera.iyer@gmail.com',      '1992-01-30', '9123456783', '7',     'Sunrise Society',     'Sector 11 Street',    23.2300000, 72.6450000, 'Gandhinagar', 'Gujarat'),
('Siddharth', 'Rao',    'siddharth.rao@gmail.com',   '1999-09-18', '9123456784', 'C-21',  'Kudasan Greens',      'Kudasan Main Road',   23.1850000, 72.6200000, 'Gandhinagar', 'Gujarat'),
('Ananya',    'Sharma', 'ananya.sharma@gmail.com',   '2001-05-09', '9123456785', '102',   'Laxmi Park',          'Sector 16 Road',      23.2100000, 72.6550000, 'Gandhinagar', 'Gujarat'),
('Kabir',     'Singh',  'kabir.singh@gmail.com',     '1997-12-25', '9123456786', '9',     'Om Bungalows',        'Sector 3 Street',     23.2400000, 72.6400000, 'Gandhinagar', 'Gujarat'),
('Isha',      'Kapoor', 'isha.kapoor@gmail.com',     NULL,         NULL,         '15',    'Raysan Residency',    'Raysan Road',         23.1700000, 72.6700000, 'Gandhinagar', 'Gujarat');

-- ---------------------------------------------------------
-- MENU
-- ---------------------------------------------------------
INSERT INTO menu (name, description, price, prep_time, is_available) VALUES
('Paneer Butter Masala', 'Paneer in a rich tomato butter gravy',      220.00, 20, TRUE),
('Dal Tadka',            'Yellow lentils tempered with ghee',         150.00, 15, TRUE),
('Veg Biryani',          'Basmati rice cooked with vegetables',       180.00, 25, TRUE),
('Butter Naan',          'Soft tandoor bread with butter',             40.00, 10, TRUE),
('Masala Dosa',          'Crispy dosa with potato filling',           110.00, 15, TRUE),
('Pav Bhaji',            'Spicy mashed vegetables with buttered pav', 130.00, 15, TRUE),
('Gujarati Thali',       'Dal, kadhi, sabzi, rotli, rice and sweet',  250.00, 30, TRUE),
('Margherita Pizza',     'Classic cheese pizza',                      200.00, 20, TRUE),
('Veg Burger',           'Crispy veg patty burger with fries',        120.00, 12, TRUE),
('Gulab Jamun',          'Two pieces of warm gulab jamun',             60.00,  5, TRUE),
('Masala Chai',          'Hot Indian spiced tea',                      30.00,  5, TRUE),
('Chole Bhature',        'Spicy chickpeas with fried bread',          140.00, 20, FALSE);

-- ---------------------------------------------------------
-- ORDERS  (priority: 1 = urgent, 2 = normal, 3 = low)
-- total_cost = sum of price * quantity of the order's items
-- ---------------------------------------------------------
INSERT INTO orders (order_id, user_id, chef_id, delivery_staff_id, priority, order_status, order_date_time, total_cost) VALUES
(1, (SELECT user_id FROM users WHERE email = 'aarav.gupta@gmail.com'),
    (SELECT staff_id FROM staff WHERE email = 'priya.shah@cloudkitchen.com'),
    (SELECT staff_id FROM staff WHERE email = 'karan.joshi@cloudkitchen.com'),
    2, 'DELIVERED', '2026-10-20 12:30:00', 600.00),
(2, (SELECT user_id FROM users WHERE email = 'diya.nair@gmail.com'),
    (SELECT staff_id FROM staff WHERE email = 'amit.patel@cloudkitchen.com'),
    (SELECT staff_id FROM staff WHERE email = 'vikram.solanki@cloudkitchen.com'),
    2, 'DELIVERED', '2026-10-20 13:10:00', 390.00),
(3, (SELECT user_id FROM users WHERE email = 'rahul.verma@gmail.com'),
    (SELECT staff_id FROM staff WHERE email = 'neha.desai@cloudkitchen.com'),
    (SELECT staff_id FROM staff WHERE email = 'sneha.trivedi@cloudkitchen.com'),
    1, 'DELIVERED', '2026-10-20 19:00:00', 240.00),
(4, (SELECT user_id FROM users WHERE email = 'meera.iyer@gmail.com'),
    (SELECT staff_id FROM staff WHERE email = 'priya.shah@cloudkitchen.com'),
    (SELECT staff_id FROM staff WHERE email = 'karan.joshi@cloudkitchen.com'),
    1, 'OUT_FOR_DELIVERY', '2026-10-21 12:00:00', 520.00),
(5, (SELECT user_id FROM users WHERE email = 'siddharth.rao@gmail.com'),
    (SELECT staff_id FROM staff WHERE email = 'amit.patel@cloudkitchen.com'),
    NULL,
    2, 'READY', '2026-10-21 12:10:00', 260.00),
(6, (SELECT user_id FROM users WHERE email = 'ananya.sharma@gmail.com'),
    (SELECT staff_id FROM staff WHERE email = 'neha.desai@cloudkitchen.com'),
    NULL,
    1, 'COOKING', '2026-10-21 12:20:00', 480.00),
(7, (SELECT user_id FROM users WHERE email = 'kabir.singh@gmail.com'),
    NULL, NULL,
    2, 'PLACED', '2026-10-21 12:25:00', 230.00),
(8, (SELECT user_id FROM users WHERE email = 'isha.kapoor@gmail.com'),
    NULL, NULL,
    3, 'PLACED', '2026-10-21 12:30:00', 500.00);

-- ---------------------------------------------------------
-- ORDER ITEMS
-- ---------------------------------------------------------
INSERT INTO order_items (order_id, item_id, quantity) VALUES
(1, (SELECT menu_id FROM menu WHERE name = 'Paneer Butter Masala'), 2),
(1, (SELECT menu_id FROM menu WHERE name = 'Butter Naan'),          4),
(2, (SELECT menu_id FROM menu WHERE name = 'Masala Dosa'),          3),
(2, (SELECT menu_id FROM menu WHERE name = 'Masala Chai'),          2),
(3, (SELECT menu_id FROM menu WHERE name = 'Veg Biryani'),          1),
(3, (SELECT menu_id FROM menu WHERE name = 'Gulab Jamun'),          1),
(4, (SELECT menu_id FROM menu WHERE name = 'Margherita Pizza'),     2),
(4, (SELECT menu_id FROM menu WHERE name = 'Gulab Jamun'),          2),
(5, (SELECT menu_id FROM menu WHERE name = 'Pav Bhaji'),            2),
(6, (SELECT menu_id FROM menu WHERE name = 'Veg Burger'),           4),
(7, (SELECT menu_id FROM menu WHERE name = 'Dal Tadka'),            1),
(7, (SELECT menu_id FROM menu WHERE name = 'Butter Naan'),          2),
(8, (SELECT menu_id FROM menu WHERE name = 'Gujarati Thali'),       2);

-- ---------------------------------------------------------
-- BILLING  (one bill per order)
-- ---------------------------------------------------------
INSERT INTO billing (order_id, user_id, total_amount, address, payment_status, payment_method, transaction_id, payment_time) VALUES
(1, (SELECT user_id FROM users WHERE email = 'aarav.gupta@gmail.com'),   600.00, 'B-12, Shreeji Apartments, Sector 7 Main Road, Gandhinagar',   'PAID',    'UPI',         'TXN20261020001', '2026-10-20 12:31:00'),
(2, (SELECT user_id FROM users WHERE email = 'diya.nair@gmail.com'),     390.00, '45, Krishna Residency, Sector 21 Road, Gandhinagar',         'PAID',    'CARD',        'TXN20261020002', '2026-10-20 13:11:00'),
(3, (SELECT user_id FROM users WHERE email = 'rahul.verma@gmail.com'),   240.00, 'A-304, Infocity Heights, Infocity Road, Gandhinagar',        'PAID',    'CASH',        NULL,             '2026-10-20 19:40:00'),
(4, (SELECT user_id FROM users WHERE email = 'meera.iyer@gmail.com'),    520.00, '7, Sunrise Society, Sector 11 Street, Gandhinagar',          'PAID',    'UPI',         'TXN20261021001', '2026-10-21 12:01:00'),
(5, (SELECT user_id FROM users WHERE email = 'siddharth.rao@gmail.com'), 260.00, 'C-21, Kudasan Greens, Kudasan Main Road, Gandhinagar',      'PAID',    'NET_BANKING', 'TXN20261021002', '2026-10-21 12:11:00'),
(6, (SELECT user_id FROM users WHERE email = 'ananya.sharma@gmail.com'), 480.00, '102, Laxmi Park, Sector 16 Road, Gandhinagar',              'PENDING', NULL,          NULL,             NULL),
(7, (SELECT user_id FROM users WHERE email = 'kabir.singh@gmail.com'),   230.00, '9, Om Bungalows, Sector 3 Street, Gandhinagar',             'PENDING', 'CASH',        NULL,             NULL),
(8, (SELECT user_id FROM users WHERE email = 'isha.kapoor@gmail.com'),   500.00, '15, Raysan Residency, Raysan Road, Gandhinagar',             'PENDING', NULL,          NULL,             NULL);

-- ---------------------------------------------------------
-- DELIVERY PARTNERS  (current locations; Karan is busy with order 4)
-- ---------------------------------------------------------
INSERT INTO delivery_partner (staff_id, latitude, longitude) VALUES
((SELECT staff_id FROM staff WHERE email = 'karan.joshi@cloudkitchen.com'),    23.2290000, 72.6430000),
((SELECT staff_id FROM staff WHERE email = 'vikram.solanki@cloudkitchen.com'), 23.2020000, 72.6600000),
((SELECT staff_id FROM staff WHERE email = 'sneha.trivedi@cloudkitchen.com'),  23.1900000, 72.6350000),
((SELECT staff_id FROM staff WHERE email = 'manish.parmar@cloudkitchen.com'),  23.2100000, 72.6500000);
