# Cloud Kitchen Management System

## Database Connectivity
For Ubuntu/Debian systems, install the mysql connection library using:
```
sudo apt update
sudo apt install libmysqlclient-dev build-essential
```
Create a .env with DB_HOST, DB_PORT, DB_USER, DB_PASSWORD and DB_NAME. Run the dbinit.sql on your machine via MySQL then make run

## Command terminal with autosuggest (trie)
`make run` starts a `kitchen>` prompt. Commands are stored in a trie (`cloud_kitchen/trie/`),
and `cloud_kitchen/terminal/` reads keys in raw mode to show live suggestions:

- grey ghost text shows the best match as you type
- `Tab` completes to the longest common prefix; `Tab Tab` lists all matches
- `→` accepts the ghost suggestion
- commands you use more often are suggested first (usage count stored in the trie node)
- unknown commands get "Did you mean" suggestions

Commands: `users add|list`, `staff add|list`, `items add|list`,
`orders place|view|queue|cook|ready`, `delivery assign|complete`,
`billing generate|pay|view`, `menu` (old numbered menu), `help`, `exit`.
To add a command, add one line to the `COMMANDS` table in `main.c`.

## Processing placed orders (priority queue)
Placing an order inserts it both in the database as well as the priority queue. Only placed orders are present in the queue, and priority is assigned as `priority * 100000 + order_id` where priority 1 = urgent, 2 = normal, 3 = low. For orders having the same priority, the ones placed earlier get completed first.

## Delivering orders (KD tree)
A K-Dimensional tree is used to map delivery parteners to orders based on their geo coordinates. Delivery drivers who are not currently busy will get assigned the location closest to them.
