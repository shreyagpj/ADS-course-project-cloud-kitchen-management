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
- `make dbinit` creates the database, all tables and the sample data (run it once on an empty server)
- `make run` builds and starts the program

## Command terminal with autosuggest (trie)
`make run` starts a `kitchen>` prompt. Commands are stored in a trie (`cloud_kitchen/tries/`),
and `cloud_kitchen/terminal/` reads keys in raw mode to show live suggestions:

- grey ghost text shows the best match as you type
- `Tab` completes to the longest common prefix; `Tab Tab` lists all matches
- `→` accepts the ghost suggestion
- commands you use more often are suggested first (usage count stored in the trie node)
- unknown commands get "Did you mean" suggestions

Commands: `users add|list`, `staff add|list`, `items add|list|available|price`,
`orders place|view|queue|cook|ready|delete`, `delivery assign|complete`,
`billing generate|pay|view|delete`, `menu` (old numbered menu), `help`, `exit`.
To add a command, add one line to the `C` table in `main.c` (at most `MAXL` commands).

## Processing placed orders (priority queue)
Placing an order inserts it in the database and in the priority queue. Only PLACED orders are queued
(the queue is rebuilt from the database on start). The key is `priority * 1000000000 + order_id`, where
priority 1 = urgent, 2 = normal, 3 = low, so equal priorities are cooked oldest first.
Only PLACED orders can be deleted.

## Delivering orders (KD tree)
When an order is assigned, all delivery partners (table `delivery_partner`) are loaded into a 2-d tree.
Partners with an OUT_FOR_DELIVERY order are marked busy, and the nearest free partner to the customer is chosen.
When the order is delivered, the partner's location becomes the customer's location.
