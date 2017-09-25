#!/bin/bash

# remove previous log file
if [ -f ~/Dropbox/FCUP/dissertacao/namespread/telnetlog ]; then
    rm ~/Dropbox/FCUP/dissertacao/namespread/telnetlog
fi

# run 'netjsoninfo graph' in telnet session; output is saved in telnetlog file
expect telnet_expect.sh > /dev/null

# clean json and put it in /var/www/html/netjson/examples/data/mn.json
cat telnetlog | grep -v "netjsoninfo graph" | grep -v '^> $' > /var/www/html/netjson/examples/data/mn.json
