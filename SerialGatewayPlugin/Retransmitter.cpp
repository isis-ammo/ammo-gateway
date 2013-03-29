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
  LOG_TRACE("Received packet type " << msg.packetType << " hyperperiod " << hyperperiod);

  switchHyperperiodsIfNeeded(hyperperiod);

  //set ack bit for current hyperperiod for providing an ack back to sender
  slotRecords.setAckBit(msg.slotId, msg.indexInSlot);

  if(msg.packetType == PACKETTYPE_ACK) {
    LOG_TRACE("Received ack packet");

    //verify that the ack packet's length matches what we think the ack packet's length should be
    if(msg.data.length == MAX_SLOTS) {
      uint8_t ackPayloadArray[MAX_SLOTS];
      for(int i = 0; i <= MAX_SLOTS; i++) {
        ackPayloadArray[i] = static_cast<uint8_t>(msg.data[i]);
      }


      uint8_t theirAckBitsForMe = ackPayloadArray[mySlotNumber] & 0x7F;
      if(theirAckBitsForMe != 0) {
        connMatrix.receivedMessageFrom(hyperperiod, msg.slotId);
      }

      connMatrix.processAckPacketPayload(msg.slotId, ackPayloadArray);

      //can also use their ack information to tell which packets sent in the last hyperperiod were actually received by them
      if(hyperperiod == msg.hyperperiod || hyperperiod == msg.hyperperiod + 1) {
        SlotRecord *previous = slotRecords.getPreviousSlotRecord();

        for(int i = 0; i < previous->sendCount; i++) {
          if((theirAckBitsForMe & 0x1) != 0) {
            //They received a packet in the position in the slot with index 'index'.  Record it in the map.
            PacketRecord *stats = previous->sentPackets[i];
            stats->heardFrom |= (0x1 << msg.slotId);
          }

          theirAckBitsForMe = theirAckBitsForMe >> 1;
          //TODO: probably should generate a Gateway ack message here if we sent the original message
        }
      } else {
        LOG_DEBUG("Spurious ack recieved in hyperperiod " << hyperperiod << "; sent in hyperperiod " << msg.hyperperiod << ".  Ignoring.");
      }
    } else {
      LOG_ERROR("Ack packet length mismatch (is max slot size incorrect?).  Dropping ack.");
    }
  } else if(msg.packetType == PACKETTYPE_NORMAL) {
    LOG_TRACE("Received normal packet");

    //Save for relaying if original sender doesn't have the same connectivity matrix as we do
    if(!connMatrix.coversMyConnectivity(msg.slotId)) {
      PacketRecord *pr = new PacketRecord(msg.indexInSlot, msg.slotId, msg.hyperperiod, msg, connMatrix.expectToHearFrom(), DEFAULT_RESENDS);
      --(pr->hopCount);
      pr->packetType = PACKETTYPE_RELAY;
      LOG_TRACE("Hopcount: Normal packet decrementing to " << pr->hopCount);
      if(pr->hopCount > 0) {
        LOG_TRACE("Adding message for relay");
        resendQueue.push(pr);
        LOG_TRACE("  Resend queue size: " << resendQueue.size());
      } else {
        //not actually going to do anything with this message, so trash it
        delete pr;
      }
    }
    //TODO: Deliver message
  } else if(msg.packetType == PACKETTYPE_RESEND || msg.packetType == PACKETTYPE_RELAY) {
    LOG_TRACE("Received resend/relay packet");
    uint8_t originalSlot = msg.data[1];
    uint8_t originalIndex = msg.data[0];

    uint16_t originalHyperperiod = *(reinterpret_cast<uint16_t *>(&msg.data.data[2]));

    LOG_TRACE("Resend packet:  original slot " << originalSlot << " index " << originalIndex << " HP " << originalHyperperiod << "...  current HP " << hyperperiod);

    int16_t hpDelta = hyperperiod - originalHyperperiod;

    if(hpDelta < MAX_SLOT_HISTORY) {
      LOG_TRACE("Message is within dup window");

      //find the ack record for that hyperperiod
      uint8_t slotIndex = slotRecords.getSlotIndexWithDelta(hpDelta);
      uint8_t ackByte = slotRecords.getAckByte(slotIndex, originalSlot);

      //if it's not a packet from me, and I didn't ack it earlier, it's not a duplicate
      if( (originalSlot != mySlotNumber) && (ackByte & (0x1 << originalIndex)) == 0) {
        LOG_TRACE("Received packet I haven't seen before");
        slotRecords.setAckBit(slotIndex, originalSlot, originalIndex);

        //TODO: modify message to remove original UID and deliver it

        //Add for further relay if union of connectivity vecotr of original sender and relayer is not a superset of my connectivty vector
        if(!connMatrix.unionCoversMyConnectivity(msg.slotId, originalSlot)) {
          //TODO: use modified message here (without original UID in message payload)
          PacketRecord *pr = new PacketRecord(originalIndex, originalSlot, originalHyperperiod, msg, connMatrix.expectToHearFrom(), DEFAULT_RESENDS);
          --(pr->hopCount);
          LOG_TRACE("Hopcount: Normal packet decrementing to " << pr->hopCount);
          if(pr->hopCount > 0) {
            LOG_TRACE("Adding message for relay");
            resendQueue.push(pr);
            LOG_TRACE("  Resend queue size: " << resendQueue.size());
          } else {
            //not actually going to do anything with this message, so trash it
            delete pr;
          }
        }
      } else {
        LOG_DEBUG("Filtered duplicate message (S: " << originalSlot << " I: " << originalIndex << " H: " << originalHyperperiod);
      }
    }
  }
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

void ConnectivityMatrix::processAckPacketPayload(const uint8_t theirSlotId, const uint8_t (&payload)[MAX_SLOTS]) {
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

PacketRecord::PacketRecord(uint8_t slotIndex, uint8_t slotNumber, uint16_t hyperperiod, Message &packet, uint32_t expectToHearFrom, uint32_t resends) :
expectToHearFrom(expectToHearFrom),
heardFrom(0),
resends(resends),
slotIndex(slotIndex),
slotNumber(slotNumber),
hyperperiod(hyperperiod),
hopCount(packet.hopCount),
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
