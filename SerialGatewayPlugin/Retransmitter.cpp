#include "Retransmitter.h"
#include "log.h"

Retransmitter::Retransmitter(uint8_t mySlot) :
connMatrix(mySlot, DEFAULT_EXPIRATION_TIME),
mySlotNumber(mySlot)
{

}

void Retransmitter::switchHyperperiodsIfNeeded(uint16_t hyperperiod) {
  if(hyperperiod != slotRecords.getCurrentHyperperiod()) {
    LOG_TRACE("Switching hyperperiods " << slotRecords.getCurrentHyperperiod() << " -> " << hyperperiod);
    if(abs(slotRecords.getCurrentHyperperiod() - hyperperiod) != 1) {
      LOG_ERROR("Hyperperiods differ by more than one!");
    }

    processPreviousBeforeSwap();
    connMatrix.cleanupExpired(hyperperiod);
    slotRecords.incrementSlot(hyperperiod);
  } else {
    LOG_WARN("Hyperperiod swap called but same: " << hyperperiod);
  }
}

void Retransmitter::processPreviousBeforeSwap() {
  SlotRecord *previous = slotRecords.getPreviousSlotRecord();

  LOG_TRACE("SWAP: previous is ring buffer index " << slotRecords.getPreviousIndex());

  for(int i = 0; i < previous->sendCount; i++) {
    PacketRecord *pr = previous->sentPackets[i];
    if(pr->expectToHearFrom == 0) {
      LOG_TRACE("Not requeuing packet not requiring ack");
    } else if((pr->expectToHearFrom & pr->heardFrom) == pr->heardFrom) {
      LOG_TRACE("Not requeuing fully acked packet");
    } else {
      LOG_TRACE("Requeueing");
      if(pr->resends > 0) {
        resendQueue.push(pr);
        previous->sentPackets[i] = NULL; //transfer ownership of this PacketRecord to the resend queue, so we don't delete it when this SlotRecord gets destroyed next
      }
    }
  }
}

void Retransmitter::processReceivedMessage(Message &msg, int hyperperiod) {

}

void Retransmitter::sendingPacket(Message &msg, std::string &data, uint16_t hyperperiod, uint8_t slotIndex, uint8_t indexInSlot) {

}


Message Retransmitter::createResendPacket(uint64_t bytesAvailable) {

}

Message Retransmitter::createAckPacket(uint16_t hyperperiod) {

}


ConnectivityMatrix::ConnectivityMatrix(const uint8_t mySlotId, const uint16_t expirationTime) :
mySlotId(mySlotId),
expirationTime(expirationTime)
{
  for(int i = 0; i < MAX_SLOTS; i++) {
    connectivityMatrix[i] = 0x1 << i; //each node can receive from itself
    connectivityUpdated[i] = 0;
  }
}

ConnectivityMatrix::~ConnectivityMatrix() {
}
  
void ConnectivityMatrix::receivedMessageFrom(const uint8_t theirSlotId, const uint16_t hyperperiod) {
  int before = connectivityMatrix[mySlotId];
  connectivityMatrix[mySlotId] |= (0x1 << theirSlotId);
  
  connectivityUpdated[theirSlotId] = hyperperiod;
  
  if(before != connectivityMatrix[mySlotId]) {
    LOG_TRACE("Connectivity matrix: added " << theirSlotId);
  }
}

void ConnectivityMatrix::processAckPacketPayload(const uint8_t theirSlotId, const std::vector<uint8_t> &payload) {
  for ( int i = 0; i < MAX_SLOTS; ++i ) {
    // remote is receiving directly from slot i, top bit in the ack
    // slot is set for receive info
    if ( (payload[i] & 0x00000080) == 0x00000080 )
      connectivityMatrix[theirSlotId] |= (0x1 << i);
    else
      connectivityMatrix[theirSlotId] &= ~(0x1 << i);
  }
}

bool ConnectivityMatrix::coversMyConnectivity(const uint8_t theirSlotId) {
  return (connectivityMatrix[mySlotId] &
    (connectivityMatrix[mySlotId] ^ connectivityMatrix[theirSlotId])) == 0;
}

bool ConnectivityMatrix::unionCoversMyConnectivity(const uint8_t theirSlotId, const uint8_t originalSlotId) {
  int32_t connectivityUnion = connectivityMatrix[theirSlotId] | connectivityMatrix[originalSlotId];
  
  return (connectivityMatrix[mySlotId] &
    (connectivityMatrix[mySlotId] ^ connectivityUnion)) == 0;
}

void ConnectivityMatrix::provideAckInfo(uint8_t (&acks)[MAX_SLOTS]) {
  for ( int i = 0; i < MAX_SLOTS; ++i ) {
    acks[i] |= ((connectivityMatrix[mySlotId] >> i) & 0x1) << 7;
  }
}

void ConnectivityMatrix::cleanupExpired(const uint8_t hyperperiod) {
  for( int i = 0; i < MAX_SLOTS; ++i ) {
    if ( i == mySlotId )
      continue;
    if ( (hyperperiod - connectivityUpdated[i]) > expirationTime ) {
      connectivityMatrix[mySlotId] &= ~(0x1 << i);
    }
  }
}

uint32_t ConnectivityMatrix::expectToHearFrom() {
  return 0;
}

PacketRecord::PacketRecord(uint8_t slotIndex, uint8_t slotNumber, uint16_t hyperperiod, uint8_t hopCount, std::string &packet, uint32_t expectToHearFrom, uint32_t resends) :
expectToHearFrom(expectToHearFrom),
heardFrom(0),
resends(resends),
slotIndex(slotIndex),
slotNumber(slotNumber),
hyperperiod(hyperperiod),
hopCount(hopCount),
packetType(PACKETTYPE_RESEND),
packet(packet)
{
  
}

SlotRecord::SlotRecord() :
hyperperiodId(0),
sendCount(0)
{
  for(int i = 0; i <= MAX_PACKETS_PER_SLOT; i++) {
    sentPackets[i] = NULL;
  }
  
  for(int i = 0; i  <= MAX_SLOTS; i++) {
    acks[i] = 0;
  }
}

//sentPackets elements should only be deleted here and in reset() (and should
//always be set to NULL if they are deleted)
SlotRecord::~SlotRecord() {
  for(int i = 0; i <= MAX_PACKETS_PER_SLOT; i++) {
    if(sentPackets[i] != NULL) {
      delete sentPackets[i]; 
      sentPackets[i] = NULL;
    }
  }
}

void SlotRecord::reset(int hyperperiod) {
  for(int i = 0; i <= MAX_PACKETS_PER_SLOT; i++) {
    if(sentPackets[i] != NULL) {
      delete sentPackets[i]; 
      sentPackets[i] = NULL;
    }
  }
  
  for(int i = 0; i  <= MAX_SLOTS; i++) {
    acks[i] = 0;
  }
}

void SlotRecord::setAckBit(uint8_t slotId, uint8_t indexInSlot) {
  uint8_t bits = acks[slotId];
  bits |= (0x1 << indexInSlot);
  acks[slotId]  = bits;
}

SlotRecords::SlotRecords() :
currentIndex(0)
{
  for(int i = 0; i < MAX_SLOT_HISTORY; i++) {
    ringBuffer[i] = new SlotRecord();
  }
}

SlotRecords::~SlotRecords() {
  for(int i = 0; i < MAX_SLOT_HISTORY; i++) {
    delete ringBuffer[i];
  }
}

void SlotRecords::incrementSlot(const uint16_t hyperperiod) {
  currentIndex = (currentIndex + 1) % MAX_SLOT_HISTORY;
  ringBuffer[currentIndex]->reset(hyperperiod);
}

SlotRecord *SlotRecords::getPreviousSlotRecord() {
  uint8_t previousIndex = currentIndex == 0 ? MAX_SLOT_HISTORY - 1 : currentIndex - 1;
  return ringBuffer[previousIndex];
}

void SlotRecords::addPacketRecord(PacketRecord *pr) {
  ringBuffer[currentIndex]->sentPackets[ringBuffer[currentIndex]->sendCount] = pr;
  ringBuffer[currentIndex]->sendCount++;
}

uint8_t SlotRecords::getSlotIndexWithDelta(const uint8_t delta) {
  int slotIndex = currentIndex - delta;
  if(slotIndex < 0) {
    slotIndex += MAX_SLOT_HISTORY;
  }
  return slotIndex;
}
