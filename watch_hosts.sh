#!/bin/bash

# close windows
if [ "$1" == "clean" ]; then
    ps aux | grep 'watch tail' | grep -v grep | awk '{print $2}' | xargs kill -9
    exit 0
fi

MN_PATH=/tmp/mininet-wifi
HOSTS_PATH=$MN_PATH/hosts
LOG_PATH=$MN_PATH/log

DELIMITER=x
WINDOW_HEIGHT=30
WINDOW_WIDTH=15
N_WINDOWS=0
X_POS=0
Y_POS=0
X_OFFSET=260
Y_OFFSET=280

for f in `ls -v $HOSTS_PATH/hosts*`
do
    gnome-terminal --geometry $WINDOW_HEIGHT$DELIMITER$WINDOW_WIDTH+$X_POS+$Y_POS -x bash -c "watch tail $f"
    X_POS=$(( $X_POS + X_OFFSET ))
    (( N_WINDOWS++ ))
    if (( $N_WINDOWS % 5 == 0 )); then
        X_POS=0
        Y_POS=$(( $Y_POS + Y_OFFSET ))
    fi
done

#gnome-terminal --geometry 30x15+0+0 -x bash -c "watch tail /tmp/mininet-wifi/hosts/hosts-sta1"
#gnome-terminal --geometry 30x15+260+0 -x bash -c "watch tail /tmp/mininet-wifi/hosts/hosts-sta2"
#gnome-terminal --geometry 30x15+520+0 -x bash -c "watch tail /tmp/mininet-wifi/hosts/hosts-sta3"
#gnome-terminal --geometry 30x15+780+0 -x bash -c "watch tail /tmp/mininet-wifi/hosts/hosts-sta4"
#gnome-terminal --geometry 30x15+1040+0	-x bash -c "watch tail /tmp/mininet-wifi/hosts/hosts-sta5"

#gnome-terminal --geometry 30x15+0+320	 -x bash -c "watch tail /tmp/mininet-wifi/log/10.0.0.1.log"
#gnome-terminal --geometry 30x15+260+320 -x bash -c "watch tail /tmp/mininet-wifi/log/10.0.0.2.log"
#gnome-terminal --geometry 30x15+520+320 -x bash -c "watch tail /tmp/mininet-wifi/log/10.0.0.3.log"
#gnome-terminal --geometry 30x15+780+320 -x bash -c "watch tail /tmp/mininet-wifi/log/10.0.0.4.log"
#gnome-terminal --geometry 30x15+1040+320 -x bash -c "watch tail /tmp/mininet-wifi/log/10.0.0.5.log"
