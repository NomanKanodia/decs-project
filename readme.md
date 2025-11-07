### Postgres setup
sudo service postgresql start

sudo -u postgres psql

\c mydb

\d

SELECT * FROM kv_store;

### Compile the program
g++ main.cpp src/db.cpp src/cache.cpp -Iinclude -I/usr/include/postgresql -lpq -o server

### Test the program
curl -X PUT -d "hello world" http://localhost:8080/set/greeting
curl http://localhost:8080/get/greeting
curl -X DELETE http://localhost:8080/delete/greeting



