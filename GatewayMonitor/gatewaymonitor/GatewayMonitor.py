#!/usr/bin/env python

import psutil
import socket
import time
import json

def getGatewayId():
  gatewayId = "unknown"
  with open('C:\\ProgramData\\ammo-gateway\\GatewayConfig.json') as f:
    try:
      # HACK: removing comments from JSON.  To be more robust, we should find
      # a JSON parser that handles comments (this code will break if // is in
      # a string somewhere)
      jsonConfig = ""
      for line in f:
        (beginning, middle, end) = line.rpartition("//")
        if middle != "": # we found a comment
          jsonConfig = jsonConfig + beginning
        else: # we didn't find a comment
          jsonConfig = jsonConfig + end
      gatewayConfig = json.loads(jsonConfig)
      gatewayId = gatewayConfig["CrossGatewayId"]
    except IOError:
      pass
  return gatewayId

def getProcessInfo(process):
  print "Gateway ID:", getGatewayId()
  print "Process", process.pid, ":", process.name
  processStartTime = process.create_time
  print "  Running since", processStartTime, "(uptime", time.time() - processStartTime, "sec)"
  print "  Current status:", str(process.status)
  print "  I/O counters:", process.get_io_counters()
  print "  CPU utilization:", process.get_cpu_times()
  print "  CPU percentage (over last 0.1 sec):", process.get_cpu_percent(0.1)
  memoryInfo = process.get_memory_info()
  print "  Memory usage: Working set:", memoryInfo.rss, "Private bytes:", memoryInfo.vms
  
  networkConnections = process.get_connections()
  print " ", len(networkConnections), "active network connections:"
  for connection in networkConnections:
    print "    Connection:", connection.local_address, "to", connection.remote_address, "Status", connection.status

def main():
  while True:
    for process in psutil.process_iter():
      if process.name.lower() == "AndroidGatewayPlugin.exe".lower():
        getProcessInfo(process)
    time.sleep(10)
      

if __name__ == "__main__":
  main()
