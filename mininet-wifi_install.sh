#!/bin/bash

# backup previous instalation
foldername=$(date +%Y%m%d_%H%M%S)
backupdir=~/mininet-wifi_backups/"$foldername"

mkdir -p $backupdir

sudo mv ~/mininet-wifi $backupdir 2>/dev/null
sudo mv ~/mac80211_hwsim_mgmt $backupdir 2>/dev/null
sudo mv ~/openflow $backupdir 2>/dev/null
sudo mv ~/wmediumd $backupdir 2>/dev/null

# install
git clone https://github.com/intrig-unicamp/mininet-wifi ~/mininet-wifi
sed -i 's/Ubuntu|Debian|Fedora/Ubuntu|Debian|Fedora|LinuxMint/g' ~/mininet-wifi/util/install.sh
sudo ~/mininet-wifi/util/install.sh -Wnfvl
