#!/usr/bin/python

import sys
import ipaddr
import os
import shutil
from mininet.net import Mininet
from mininet.cli import CLI
from mininet.log import setLogLevel

""" TODO: use argparse """
if (len(sys.argv) != 3 or 
(sys.argv[1] != 'move' and sys.argv[1] != 'line') or 
not sys.argv[2].isdigit()):
    print('usage: python adhoc.py <positioning> <#stations>\n\tpositioning: move | line')
    sys.exit(0)

move = sys.argv[1] == 'move' and True or False
nStations = int(sys.argv[2])
startPos = {'x':2, 'y':100, 'z':0}
baseMac = '00:00:00:00:00:'
baseIp = ipaddr.IPAddress('10.0.0.0')
ipMask = '/8'
wirelessRange = 35
params = {}
plotGraphMaxX = 200
plotGraphMaxY = 200
mininetTmpDir = '/tmp/mininet-wifi'
hostsFilePath = mininetTmpDir + '/hosts'
logPath = mininetTmpDir + '/log'

def change_mac(mac, offset):
    return "{:012X}".format(int(mac, 16) + offset)

def topology():
    print "*** Creating network"
    net = Mininet(enable_wmediumd=True, enable_interference=True)

    print "*** Creating nodes"
    for i in range(1, nStations+1):
        #mac = baseMac + hex(i).split('x')[-1]
        #ip = baseIp + i
        params['range'] = wirelessRange
        #params['mac'] = mac
        #params['ip'] = str(ip) + ipMask
        
        if not move:
            params['position'] = str(startPos['x'] + (wirelessRange-2)*i) + ',' + str(startPos['y']) + ',' + str(startPos['z'])
        
        stationName = 'sta' + str(i)
        print("\t%s: %s" % (stationName, params))
        net.addStation(stationName, **params)

    #sta1 = net.addStation('sta1', range=35)
    #sta2 = net.addStation('sta2', range=35)
    #sta3 = net.addStation('sta3', range=35)

    print "*** Configuring propagation model"
    net.propagationModel('logDistancePropagationLossModel', exp=4.5)

    print "*** Configuring wifi nodes"
    net.configureWifiNodes()

    print "*** Creating links"
    for sta in net.stations:
        net.addHoc(sta, ssid='adhocNet')
    
    #net.addHoc(sta1, ssid='adhocNet')
    #net.addHoc(sta2, ssid='adhocNet')
    #net.addHoc(sta3, ssid='adhocNet')

    print("*** Plotting graph")
    global plotGraphMaxX
    if not move:
        plotGraphMaxX = net.stations[nStations-1].params['position'][0] + wirelessRange

    net.plotGraph(max_x=plotGraphMaxX, max_y=plotGraphMaxY)
    
    print "*** Starting network"
    net.build()

    if move:
        print("*** Adding mobility")
        net.startMobility(time=0, model='RandomDirection', max_x=plotGraphMaxX, max_y=plotGraphMaxY, min_v=1.5, max_v=2)

    print("*** Mounting 'hosts' and 'hostname' files for each station")
    if os.path.exists(hostsFilePath):
        shutil.rmtree(hostsFilePath)
    os.makedirs(hostsFilePath)
    os.makedirs(logPath)

    for i in range(1, nStations+1):
        hostsFile = open("%s/hosts-sta%d" % (hostsFilePath, i), "w")
        addr = net.stations[i-1].params['ip'][0].split("/")[0]
        hostsFile.write("%s\tsta%d\n" % (addr, i))
        hostsCmd = "mount --bind %s/hosts-sta%d /etc/hosts" % (hostsFilePath, i)
        print("\t" + hostsCmd)
        net.stations[i-1].cmd(hostsCmd)

        hostnameFile = open("%s/hostname-sta%d" % (hostsFilePath, i), "w")
        hostnameFile.write("sta%d\n" % i)
        hostnameCmd = "mount --bind %s/hostname-sta%d /etc/hostname" % (hostsFilePath, i)
        print("\t" + hostnameCmd)
        net.stations[i-1].cmd(hostnameCmd)

        hostsFile.close()
        hostnameFile.close()

    print("*** Starting OLSR and NameSPREAD in each station")
    for i in range(1, nStations+1):
        olsrCmd = "setsid ~/OONF/build/olsrd2_static sta%d-wlan0 &>/dev/null &" % i
        print("\t" + olsrCmd)
        net.stations[i-1].cmd(olsrCmd)
        
        neAddrMask = net.stations[i-1].params['ip'][0]
        maskIndex = len(neAddrMask) - neAddrMask.index('/')
        neAddr = neAddrMask[:-maskIndex]
        neCmd = "./src/namespread.o %s &" % neAddr
        print("\t" + neCmd)
        net.stations[i-1].cmd(neCmd)

    print "*** Enable monitoring of wireless traffic"
    os.system("ifconfig hwsim0 up")

    print "*** Running CLI"
    CLI(net)

    print "*** Stopping network"
    net.stop()
    shutil.rmtree(mininetTmpDir)
    os.system("mn -c")

if __name__ == '__main__':
    setLogLevel('info')
    topology()

