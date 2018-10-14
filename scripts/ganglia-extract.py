#!/usr/bin/python
# Filename: ganglia-extract.py

import shutil
import os
import sys
import commands
import string
sys.path.append('/usr/lib/python2.7/dist-packages/')
#to install rrdtool before importing
os.system("sudo apt-get -y install rrdtool python-rrdtool")
import rrdtool
from threading import Timer
import time
import math


if len(sys.argv)<3:
    print  'Usage: extract.py rrdsdir(e.g. /var/lib/ganglia/rrds/TestingCluster) interval(e.g. 15) startTimeAheadOfNow(optional: e.g. 30)'
    sys.exit(2)

#if this value is set to too small, rrd has no qualified data and the script will fail
startTimeAheadOfNow = 30

root=str(sys.argv[1])

interval = int(sys.argv[2])

if len(sys.argv)==4:
    startTimeAheadOfNow = int(sys.argv[3])

#every invocation of this script will cause removal of the output directory
def ensure_temp_csv_dir(f):
    if os.path.exists(f):
        shutil.rmtree(f)
    os.makedirs(f)

ensure_temp_csv_dir('ganglia-output')

now = time.time() - startTimeAheadOfNow
start = int(now)

cpu_metriclist = ('cpu_aidle', 'cpu_idle', 'cpu_nice', 'cpu_num', 'cpu_speed', 'cpu_system', 'cpu_user', 'cpu_wio')
disk_metriclist = ('disk_free', 'disk_total')
mem_metriclist = ('mem_buffers', 'mem_cached', 'mem_free', 'mem_shared', 'mem_total', 'swap_free', 'swap_total')
network_metriclist = ('pkts_in', 'pkts_out', 'bytes_in', 'bytes_out')


def extract(root, start, end, resource, metrics):
    dirs = os.listdir(root)
    
    map1=string.maketrans('.','_')
    nodefile = file('ganglia-output/' + resource + '_nodes.csv', 'a')
    summaryfile = file('ganglia-output/' + resource + '_summary.csv', 'a')
    
    numOfMetrics = len(metrics)
    
    for nodedir in dirs:
	nodeName=nodedir.translate(map1)
	f = nodefile   
        print (nodeName) 
	alldata = ['']*numOfMetrics
	for i in range(0, numOfMetrics):	
	    rrdfilename = root + '/'+ nodeName + '/' + metrics[i]+'.rrd'	
            print (rrdfilename)
	    data = rrdtool.fetch(rrdfilename, "AVERAGE", ['--start', str(start), '--end', str(end)])
#	    print data
	    alldata[i] = data
	    
	starttime = alldata[0][0][0]
	endtime = alldata[0][0][1]
	interval = alldata[0][0][2]
	num = (endtime - starttime)/interval
        
	isSummary = False
	if nodeName=='__SummaryInfo__':
	    isSummary = True
	    f = summaryfile
	
	count = 0;
	while count<num-1:
	    time = starttime + count*interval
	    f.write(str(time) + ',' + nodeName)
	    for i in range(0, numOfMetrics):	    
		value = alldata[i][2][count][0]
		f.write(',' + str(value))
	    if isSummary:
		value = alldata[i][2][count][1]
		f.write(',' + str(value))
	    count = count + 1
	    f.write('\n')
	
    nodefile.close()
    summaryfile.close()

def extractResource():
    global start
    global end
    global root
    end = start + interval -1
    print start
    print end
    extract(root, start, end, 'cpu', cpu_metriclist)
    extract(root, start, end, 'disk', disk_metriclist)
    extract(root, start, end, 'mem', mem_metriclist)
    extract(root, start, end, 'network', network_metriclist)
    start = start + interval

extractResource()
while True:
    print time.localtime()
    t = Timer(interval, extractResource)
    t.start()
    time.sleep(interval)
