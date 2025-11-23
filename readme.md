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

g++ -std=c++17 -o loadgen load_gen_diff.cpp -Iinclude -lpthread
./loadgen 8 15 8080 get_put
./loadgen 8 15 8080 put_all
./loadgen 8 15 8080 get_popular

 taskset -c 0-2 ./server
taskset -c 3-5 ./loadgen 2 30 8080 get_popular
top -d 5


