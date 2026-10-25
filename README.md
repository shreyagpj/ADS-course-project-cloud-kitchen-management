# Cloud Kitchen Management System

## Setup
Install the MySQL client library (Ubuntu/Debian):
```
sudo apt update
sudo apt install libmysqlclient-dev build-essential
```
Create a `.env` in the repo root with `DB_HOST`, `DB_PORT`, `DB_USER`, `DB_PASSWORD` and `DB_NAME`.
The schema files use the database `ADS_cloud_kitchen`, so set `DB_NAME=ADS_cloud_kitchen`.

From `cloud_kitchen/`:
- `make dbinit` creates the database, all tables and the sample data
- `make run` builds and starts the program

## Command terminal with autosuggest (trie)
`make run` starts a `kitchen>` prompt. Commands are stored in a trie. If a partial command is entered, suggestions are displayed ordered by frequency and then alphabetically. Usage count of commands is stored in the trie node. If not found, "Unknown command" message is displayed.

Commands: `users add|list`, `staff add|list`, `items add|list|available|price`, `orders place|view|queue|cook|ready|delete`, `delivery assign|complete`, `billing generate|pay|view|delete`, `menu` (old numbered menu), `help`, `exit`. To add a command, add one line to the `C` table in `main.c` (at most `MAXL` commands).

## Processing placed orders (priority queue)
Placing an order inserts it in the database and in the priority queue. Only placed orders are queued and the queue is rebuilt from the database on start. The key is `priority * 1000000000 + order_id`, where priority 1 = urgent, 2 = normal, 3 = low, so equal priorities are cooked oldest first. Only placed orders can be deleted.

## Delivering orders (KD tree)
When an order is assigned, all delivery partners are loaded into a 2D tree. Partners with an OUT_FOR_DELIVERY order are marked busy, and the nearest free partner to the customer is chosen. When the order is delivered, the partner's location is updated with the customer's location.
