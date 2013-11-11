#!/usr/bin/env python

import optparse
import serial
import time
import struct
import array
import binascii


MAGIC_NUMBER_BYTES = [0xea, 0x1d, 0xad, 0xab]
HEADER_SIZE = 12


class ReaderState:
  READING_MAGIC = 0
  READING_HEADER = 1
  READING_DATA = 2

class MessageType:
  DATA = 0
  ACK_TOKEN = 1

def getTime():
  return time.clock()


class SatcomMonitor:
  def __init__(self, port, baudRate):
    self.port = port
    self.baudRate = baudRate

  def run(self):
    ser = serial.Serial(self.port, self.baudRate, parity=serial.PARITY_NONE, rtscts=1)

    state = ReaderState.READING_MAGIC
    header = ""
    payload = ""
    expectedPayloadSize = 0

    startTime = getTime()
    endTime = getTime()

    while True:
      readData = ser.read()

      if state == ReaderState.READING_MAGIC:
        if readData == MAGIC_NUMBER_BYTES[len(header)]:
          header += readData
          if len(header) == 1:
            startTime = getTime()
          elif len(header) >= 4:
            state = ReaderState.READING_HEADER
        else:
          print "Received bad magic number", readData
      elif state == ReaderState.READING_HEADER:
        header += readData
        if len(header) >= HEADER_SIZE:
          headerChecksumBytes = header[10:]
          (expectedChecksum) = struct.unpack("<H", headerChecksumBytes)
          calculatedChecksum = binascii.crc32(header[0:10]) & 0x0000ffff

          if expectedChecksum == calculatedChecksum:
            state = ReaderState.READING_DATA
            payload = ""
            expectedPayloadSize = struct.unpack("<H", header[2:4])
            pass
          else:
            print "Received corrupt header, exp", expectedChecksum, "calc", calculatedChecksum
            header = ""
      elif state == ReaderState.READING_DATA:
        payload += readData
        if len(payload) >= expectedPayloadSize:
          endTime = getTime()
          self.processMessage(header, payload, startTime, endTime)
          header = ""
          payload = ""
          state = ReaderState.READING_MAGIC
          pass
      else:
        print "Unexpected state", state
        return 0

    def processMessage(self, header, payload, startTime, endTime):
      (magicNumber, size, reserved, payloadChecksum, headerChecksum) = struct.unpack("<IHHHH")

      #validate payload checksum
      calculatedChecksum = binascii.crc32(payload) & 0x0000ffff
      if calculatedChecksum == payloadChecksum:
        messageType = struct.unpack("<B", payload[0:1])
        payloadBody = payload[1:]
        if messageType == MessageType.DATA:
          (sequenceNumber, index, count) = struct.unpack("<HHH", payloadBody[0:6])
          print startTime, "-", endTime, "DATA FRAGMENT"
          print "   sequenceNumber:", sequenceNumber
          print "   index:", index
          print "   count:", count
          print "   length:", (len(payloadBody) - 6)
          pass
        elif messageType == MessageType.ACK_TOKEN:
          ackCountShort = struct.unpack("<H", payloadBody[0:2])
          isToken = (ackCountShort >> 15) == 1
          ackCount = ackCountShort & 0x7f
          acks = array.array("<H", payloadBody[2:])
          print startTime, "-", endTime, "ACK/TOKEN PACKET"
          print "   isToken:", isToken
          print "   ackCount:", ackCount
          print "   acks:", acks
          pass
        else:
          print "Invalid message type", messageType
      else:
        print "Received corrupt data, exp", payloadChecksum, "calc", calculatedChecksum


if __name__ == "__main__":
  print "Satcom protocol monitor"

  parser = optparse.OptionParser()
  parser.add_option("-p", "--port", dest="port",
                    help="Serial port to listen on (default %default)",
                    default="COM3")
  parser.add_option("-b", "--baud-rate", dest="rate", type="int",
                    help="Baud rate (default %default)",
                    default=9600)

  (options, args) = parser.parse_args()

  monitor = SatcomMonitor(options.port, options.rate)
  monitor.run()