#!/usr/bin/python

import sys
import ipaddr
import os
import shutil
from mininet.net import Mininet
from mininet.cli import CLI
from mininet.log import setLogLevel
from random import randint

usageMsg = 'usage: python adhoc.py <positioning> <#stations|case#> <area> \n\tpositioning: move | line | random | case'

""" TODO: use argparse """
if (len(sys.argv) < 3 or 
(sys.argv[1] != 'move' and sys.argv[1] != 'line' and sys.argv[1] != 'random' and sys.argv[1] != 'case') or 
not sys.argv[2].isdigit()):
    print(usageMsg)
    sys.exit(0)

line = sys.argv[1] == 'line' and True or False
move = sys.argv[1] == 'move' and True or False
random = sys.argv[1] == 'random' and True or False
case = sys.argv[1] == 'case' and True or False

nStations = int(sys.argv[2])
startPos = {'x':2, 'y':100, 'z':0}
baseMac = '00:00:00:00:00:'
baseIp = ipaddr.IPAddress('10.0.0.0')
ipMask = '/8'
wirelessRange = 33
params = {}
coords = []
plotGraphMaxX = 200
plotGraphMaxY = 200

if ((move or random) and len(sys.argv) == 4):
    plotGraphMaxX = int(sys.argv[3])
    plotGraphMaxY = int(sys.argv[3])

if (case):
    if (len(sys.argv) != 3):
        print(usageMsg)
        sys.exit(0)
    else:
        caseNum = int(sys.argv[2])
        if (caseNum == 2):
            coords = [l.rstrip('\n') for l in open('scenarios/2/100_10.txt')]
            nStations = 10
            plotGraphMaxX = 100
            plotGraphMaxY = 100
        elif (caseNum == 3):
            coords = [l.rstrip('\n') for l in open('scenarios/3/316_30.txt')]
            nStations = 30
            plotGraphMaxX = 316
            plotGraphMaxY = 316
        elif (caseNum == 4):
            coords = [l.rstrip('\n') for l in open('scenarios/4/316_100.txt')]
            nStations = 100
            plotGraphMaxX = 316
            plotGraphMaxY = 316
        elif (caseNum == 5):
            coords = [l.rstrip('\n') for l in open('scenarios/5/1000_100.txt')]
            nStations = 100
            plotGraphMaxX = 1000
            plotGraphMaxY = 1000
        else:
            print('Valid case numbers: 2, 3, 4 or 5.')
            sys.exit(0)

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
        #params['range'] = wirelessRange
        #params['mac'] = mac
        #params['ip'] = str(ip) + ipMask
        
        if line:
            params['position'] = str(startPos['x'] + (wirelessRange-2)*i) + ',' + str(startPos['y']) + ',' + str(startPos['z'])
        elif random:
            global plotGraphMaxX
            params['position'] = str(randint(0, plotGraphMaxX)) + ',' + str(randint(0, plotGraphMaxY)) + ', 0'           
        elif case:
            params['position'] = coords[i-1]                       

        stationName = 'sta' + str(i)
        print("\t%s: %s" % (stationName, params))
        net.addStation(stationName, **params)

    """
    if not move:
        sta1 = net.addStation('sta1', position='100,50,0')
        sta2 = net.addStation('sta2', position='100,60,0')
    """

    print "*** Configuring propagation model"
    #net.propagationModel('logNormalShadowingPropagationLossModel', exp=4.5, variance=4)
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
    #global plotGraphMaxX
    if line:
        plotGraphMaxX = net.stations[nStations-1].params['position'][0] + wirelessRange

    net.plotGraph(max_x=plotGraphMaxX, max_y=plotGraphMaxY)

    if move:
        print("*** Adding mobility")
        net.startMobility(time=0, model='RandomWayPoint', max_x=plotGraphMaxX, max_y=plotGraphMaxY, min_v=1.5, max_v=2)
        #net.mobility(net.stations[0], 'start', time=2, position='45,90,0')
        #net.mobility(net.stations[0], 'stop', time=6, position='140,90,0')
        #net.stopMobility(time=10)

    print "*** Starting network"
    net.build()

    print("*** Mounting 'hosts' and 'hostname' files for each station")
    if os.path.exists(mininetTmpDir):
        shutil.rmtree(mininetTmpDir)
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
        neAddrMask = net.stations[i-1].params['ip'][0]
        maskIndex = len(neAddrMask) - neAddrMask.index('/')
        neAddr = neAddrMask[:-maskIndex]
        neCmd = "./src/namespread.o %s &" % neAddr
        print("\t" + neCmd)
        net.stations[i-1].cmd(neCmd)

        olsrCmd = "setsid ~/OONF/build/olsrd2_static sta%d-wlan0 lo &>/dev/null &" % i
        print("\t" + olsrCmd)
        net.stations[i-1].cmd(olsrCmd)

    print "*** Enable monitoring of wireless traffic"
    os.system("ifconfig hwsim0 up")
    os.system("wireshark -i hwsim0 -k &")

    print "*** Running CLI"
    CLI(net)

    print "*** Stopping network"
    os.system("ps aux | grep olsrd2 | grep -v grep | awk '{print($2)}' | xargs sudo kill -9")
    net.stop()
    os.system("mn -c")

if __name__ == '__main__':
    setLogLevel('info')
    topology()
