# Cloud Kitchen Management System

## Database Connectivity
For Ubuntu/Debian systems, install the mysql connection library using:
```
sudo apt update
sudo apt install libmysqlclient-dev build-essential
```
Create a .env with DB_HOST, DB_PORT, DB_USER, DB_PASSWORD and DB_NAME
Run the dbinit.sql on your machine via MySQL then make run

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
