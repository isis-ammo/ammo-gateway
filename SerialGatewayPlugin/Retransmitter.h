#ifndef RETRANSMITTER_H
#define RETRANSMITTER_H

#include <stdint.h>
#include <vector>
#include <string>

#include "SerialServiceHandler.h"

const size_t MAX_SLOTS = 16;
const size_t MAX_PACKETS_PER_SLOT = 7;

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
  ~SlotRecord();
  void reset(int hyperperiod);
  void setAckBit(int slotId, int indexInSlot);
  
  uint16_t hyperperiodId;
  PacketRecord *sentPackets[MAX_PACKETS_PER_SLOT];
  unsigned int sendCount;
  uint8_t acks[MAX_SLOTS];
};

#endif //RETRANSMITTER_H
