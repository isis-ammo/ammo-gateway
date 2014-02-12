from ammo import AndroidConnector
import uuid
import sys
import time
import datetime
import os
import optparse
import json
from ammo import AmmoMessages_pb2

from twisted.internet import reactor

def onDataReceived(connector, msg):
  messageType = AmmoMessages_pb2._MESSAGEWRAPPER_MESSAGETYPE.values_by_number[msg.type].name
  print messageType, datetime.datetime.now()

if __name__ == "__main__":
  print "Android Gateway Tester"
  
  parser = optparse.OptionParser()
  parser.add_option("-g", "--gateway", dest="gateway",
                    help="Gateway to connect to (default %default)",
                    default="127.0.0.1")
  parser.add_option("-p", "--port", dest="port", type="int",
                    help="Gateway port to connect to (default %default)",
                    default=33289)
  parser.add_option("-s", "--scope", dest="scope",
                    help="Subscription scope (either local or global; default %default)",
                    default="global")
  parser.add_option("-r", "--rate", dest="rate", type="float",
                    help="Number of seconds between pushes (default %default)",
                    default=0.5)
  parser.add_option("-a", "--latitude", dest="latitude", type="float",
                    help="Initial latitude (default %default)",
                    default=36.14459)
  parser.add_option("-o", "--longitude", dest="longitude", type="float",
                    help="Initial latitude (default %default)",
                    default=-86.79373)
  parser.add_option("", "--delta-latitude", dest="deltaLatitude", type="float",
                    help="Distance for latitude to change per cycle (default %default)",
                    default=0)
  parser.add_option("", "--delta-longitude", dest="deltaLongitude", type="float",
                    help="Distance for longitude to change per cycle (default %default)",
                    default=0)
  parser.add_option("-u", "--username", dest="username", 
                    help="Origin username for this emulated device",
                    default="user:test/" + uuid.uuid1().hex)
  parser.add_option("-b", "--heartbeat-period", dest="heartbeatPeriod", type="float",
                    help="Time between heartbeats (default %default)",
                    default=30)
  
  (options, args) = parser.parse_args()
  
  deviceName = "device:test/" + uuid.uuid1().hex
  userName = options.username
  
  connector = AndroidConnector.AndroidConnector(options.gateway, options.port, deviceName, userName, "", options.heartbeatPeriod)
  connector.setMessageQueueEnabled(False)
  connector.registerMessageCallback(onDataReceived)
  
  scope = AndroidConnector.MessageScope.GLOBAL
  if options.scope == "local":
    scope = AndroidConnector.MessageScope.LOCAL
  elif options.scope == "global":
    scope = AndroidConnector.MessageScope.GLOBAL
  else:
    print "scope must be one of: local global"
    exit(-1)
  
  try:
    connector.start()
    connector.waitForAuthentication()
    
    print "Pushing a lot of data."
    sequenceNumber = 0
    
    while True:
      postTime = time.time()
      pliJson = {}
      pliJson["name"] = userName
      pliJson["lat"] = "{:d}".format(int((options.latitude + options.deltaLatitude * sequenceNumber) * 1e6))
      pliJson["lon"] = "{:d}".format(int((options.longitude + options.deltaLongitude * sequenceNumber) * 1e6))
      pliJson["altitude"] = "0"
      pliJson["accuracy"] = "0"
      pliJson["created"] = "{:d}".format(int(postTime * 1e3))
      pliJson["modified"] = "{:d}".format(int(postTime * 1e3))
      pliJson["hops"] = "0"
      
      jsonString = json.dumps(pliJson, separators=(",", ":"))
      print jsonString
      
      itemUid = "pli:" + uuid.uuid1().hex

      connector.push(itemUid, "ammo/transapps.pli.locations", jsonString, scope)
      print "Pushed sequence number", sequenceNumber, "at", "{0:.6f}".format(time.time())
      sequenceNumber = sequenceNumber + 1
      time.sleep(options.rate)
      
  except KeyboardInterrupt:
    print "Got ^C...  Closing"
    reactor.callFromThread(reactor.stop)
  except:
    print "Unexpected error...  dying."
    reactor.callFromThread(reactor.stop)
    raise
