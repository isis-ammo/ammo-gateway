#!/usr/bin/env python

import psutil
import socket
import time
import json
import pprint
import urllib
import urllib2
import sys
import contextlib

# Set up default config (assuming everything goes well, this will be overwritten
# by loadConfig in main())
defaultConfig = { "processName": "AndroidGatewayPlugin.exe", # the process to inspect
  "sampleInterval": 30, # number of seconds between samples/updates
  "endpoint": "http://localhost/UploadStats" # endpoint to send sample data to
}

def getGatewayId():
  gatewayId = "unknown"
  try:
    with open('C:\\ProgramData\\ammo-gateway\\GatewayConfig.json') as f:
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
  
def loadConfig():
  newConfig = None
  config = None
  try:
    with open("C:\\ProgramData\\ammo-gateway\\MonitorConfig.json") as f:
      newConfig = json.load(f)
  except Exception as e:
    print "Error occurred when loading config:", e
    pass
  if newConfig != None:
    config = newConfig
  else:
    config = defaultConfig
    print "An error occurred when reading the config file; using defaults"
  
  print "Configuration:", pprint.pformat(config, indent=2)
  return config

def getProcessInfo(process):
  processInfo = {}
  
  processInfo["pid"] = process.pid
  processInfo["name"] = process.name
  processStartTime = process.create_time
  processUptime = time.time() - processStartTime
  processInfo["startTime"] = processStartTime
  processInfo["uptime"] = processUptime
  processInfo["status"] = process.status
  ioCounters = process.get_io_counters()
  processInfo["ioInfo"] = {}
  processInfo["ioInfo"]["readBytes"] = ioCounters.read_bytes
  processInfo["ioInfo"]["writeBytes"] = ioCounters.write_bytes
  processInfo["cpuPercentage"] = process.get_cpu_percent(0.1)
  memoryInfo = process.get_memory_info()
  processInfo["memInfo"] = {}
  processInfo["memInfo"]["workingSet"] = memoryInfo.rss
  processInfo["memInfo"]["privateBytes"] = memoryInfo.vms
  processInfo["connections"] = len(process.get_connections("tcp"))
  
  # print "Gateway ID:", getGatewayId()
  # print "Process", process.pid, ":", process.name
  # processStartTime = process.create_time
  # print "  Running since", processStartTime, "(uptime", time.time() - processStartTime, "sec)"
  # print "  Current status:", str(process.status)
  # print "  I/O counters:", process.get_io_counters()
  # print "  CPU utilization:", process.get_cpu_times()
  # print "  CPU percentage (over last 0.1 sec):", process.get_cpu_percent(0.1)
  # memoryInfo = process.get_memory_info()
  # print "  Memory usage: Working set:", memoryInfo.rss, "Private bytes:", memoryInfo.vms
  # 
  # networkConnections = process.get_connections()
  # print " ", len(networkConnections), "active network connections:"
  # for connection in networkConnections:
  #   print "    Connection:", connection.local_address, "to", connection.remote_address, "Status", connection.status
    
  return processInfo
  
def generateJson(processName, found, info):
  result = {}
  result["time"] = time.time()
  result["gatewayId"] = getGatewayId()
  result["processName"] = processName
  result["running"] = found
  if found:
    result["info"] = info
    
  jsonString = json.dumps(result)
  print ""
  print "JSON:", json.dumps(result, indent = 2)
  return jsonString
  
def submitSample(endpoint, jsonString):
  uploadData = urllib.urlencode({"data": jsonString})
  req = urllib2.Request(url = endpoint, data = uploadData)
  try:
    with contextlib.closing(urllib2.urlopen(req)) as f:
      print "Successfully posted sample"
  except Exception as e:
    print "Error occurred when posting sample:", e

def main():
  config = loadConfig()
  searchProcess = config["processName"]
  while True:
    found = False
    info = {}
    for process in psutil.process_iter():
      if process.name.lower() == searchProcess.lower():
        try:
          info = getProcessInfo(process)
          found = True
          break
        except psutil.error.NoSuchProcess:
          print "Process disappeared while we were gathering its stats"
    jsonToSend = generateJson(searchProcess, found, info)
    submitSample(config["endpoint"], jsonToSend)
    print ""
          
    time.sleep(config["sampleInterval"])
      

if __name__ == "__main__":
  main()
