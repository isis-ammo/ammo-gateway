#ifndef RETRANSMITTER_H
#define RETRANSMITTER_H

#include <stdint.h>
#include <vector>
#include <string>

#include "SerialServiceHandler.h"

const size_t MAX_SLOTS = 16;
const size_t MAX_PACKETS_PER_SLOT = 7;
const size_t MAX_SLOT_HISTORY = 16;

class ConnectivityMatrix {
public:
  ConnectivityMatrix(const uint8_t mySlotId, const uint16_t expirationTime);
  virtual ~ConnectivityMatrix();
  
  void receivedMessageFrom(const uint8_t theirSlotId, const uint16_t hyperperiod);
  void processAckPacketPayload(const uint8_t theirSlotId, const std::vector<uint8_t> &payload);
  
  bool coversMyConnectivity(const uint8_t theirSlotId);
  bool unionCoversMyConnectivity(const uint8_t theirSlotId, const uint8_t originalSlotId);
  
  void provideAckInfo(uint8_t (&acks)[]);
  
  void cleanupExpired(const uint8_t hyperperiod);
  
  uint32_t expectToHearFrom();
  
private:
  uint8_t mySlotId;
  uint8_t expirationTime;
  
  uint32_t connectivityMatrix[MAX_SLOTS];
  uint16_t connectivityUpdated[MAX_SLOTS];
};

struct PacketRecord {
public:
  PacketRecord(uint8_t slotIndex, uint8_t slotNumber, uint16_t hyperperiod, uint8_t hopCount, std::string &packet, uint32_t expectToHearFrom, uint32_t resends);
  
  uint32_t expectToHearFrom;
  uint32_t heardFrom;
  uint32_t resends;
  
  //UID components
  uint8_t  slotIndex;       //the index of this packet within its slot
  uint8_t  slotNumber;      //the slot number of the sending device
  uint16_t hyperperiod;     //current GPS time / cycle duration; low order 16 bits
  
  uint8_t hopCount;
  uint16_t packetType;
  std::string packet;
};

struct SlotRecord {
public:
  SlotRecord();
  virtual ~SlotRecord();
  void reset(int hyperperiod);
  void setAckBit(uint8_t slotId, uint8_t indexInSlot);
  
  uint16_t hyperperiodId;
  PacketRecord *sentPackets[MAX_PACKETS_PER_SLOT];
  unsigned int sendCount;
  uint8_t acks[MAX_SLOTS];
};

class SlotRecords {
public:
  SlotRecords();
  virtual ~SlotRecords();
  
  void incrementSlot(const uint16_t hyperperiod);
  inline uint16_t getCurrentHyperperiod() { return ringBuffer[currentIndex]->hyperperiodId; }
  inline uint16_t getPreviousIndex() { return currentIndex == 0 ? MAX_SLOT_HISTORY - 1 : currentIndex - 1; }
  SlotRecord *getPreviousSlotRecord();
  
  inline void setAckBit(const uint8_t slotId, const uint8_t indexInSlot) { ringBuffer[currentIndex]->setAckBit(slotId, indexInSlot); }
  inline void setAckBit(const uint8_t slotIndex, const uint8_t originalSlot, const uint8_t originalIndex) { ringBuffer[slotIndex]->acks[originalSlot] |= (0x1 << originalIndex); }
  inline uint8_t getAckByte(const uint8_t slotIndex, const uint8_t originalSlot) { return ringBuffer[slotIndex]->acks[originalSlot]; }
  inline unsigned int getCurrentSendCount() { return ringBuffer[currentIndex]->sendCount; }
  
  void addPacketRecord(PacketRecord *pr);
  uint8_t getSlotIndexWithDelta(const uint8_t delta);
  
private:
  SlotRecord *ringBuffer[MAX_SLOT_HISTORY];
  uint8_t currentIndex;
};

#endif //RETRANSMITTER_H                                                     
