#!/usr/bin/expect

#log_user 0

spawn nc 127.0.0.1 2009
#expect -re "Escape.*"
send "\r"
expect ">"
log_file telnetlog
send "netjsoninfo graph\r"
expect ">"
log_file
#puts $expect_out(buffer)
send "exit\r"
