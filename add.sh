#/bin/sh
redis-cli -h 218.30.115.72 -p 6378 sadd server_list "$1"
