/* Copyright (C) 2010-2013 Vanderbilt University
 *  
 * This software was developed by the Institute for Software Integrated
 * Systems (ISIS) at Vanderbilt University, Tennessee, USA for the
 * Transformative Apps program under DARPA, Contract # HR011-10-C-0175 and
 * Contract # HR0011-12-C-0109.
 *  
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this data, including any software or models in source or binary
 * form, as well as any drawings, specifications, and documentation
 * (collectively "the Data"), to deal in the Data without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Data, and to
 * permit persons to whom the Data is furnished to do so, subject to the
 * following conditions:
 *  
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Data.
 *  
 * THE DATA IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS, SPONSORS, DEVELOPERS, CONTRIBUTORS, OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE DATA OR THE USE OR OTHER DEALINGS IN THE DATA. 
 */

/**
* This logic is derived from the logic in the serial plugin's
* SerialMessageProcessor.  It should probably eventually be moved
* out into a shared library which both plugins use.
*/

#include "TerseDecoder.h"
#include "SatcomConfigurationManager.h"
#include "BinaryOutputStream.h"

#include <fstream>

using namespace ammo::gateway;

const char DEFAULT_PRIORITY = 50;

//Mimetype string constants
const char *NEVADA_PLI_MIMETYPE = "ammo/com.aterrasys.nevada.locations";
const char *TRANSAPPS_PLI_MIMETYPE = "ammo/transapps.pli.locations";
const char *DASH_EVENT_MIMETYPE = "ammo/edu.vu.isis.ammo.dash.event";
const char *DASH_MEDIA_MIMETYPE = "ammo/edu.vu.isis.ammo.dash.media";
const char *CHAT_MESSAGE_ALL_MIMETYPE = "ammo/transapps.chat.message_groupAll";

//Serial type ID constants
const int SMS_TYPEID = 1;
const int NEVADA_PLI_TYPEID = 2;
const int TRANSAPPS_PLI_TYPEID = 5;
const int DASH_EVENT_TYPEID = 3;
const int DASH_MEDIA_TYPEID = 8;
const int CHAT_MESSAGE_ALL_TYPEID = 4;

#define ntohll(x) ( ( (int64_t)  (ntohl((int)((x << 32) >> 32))) << 32) | (uint32_t)ntohl(((int)(x >> 32)))  ) //By Runner
#define htonll(x) ntohll(x)   

TerseDecoder::TerseDecoder(ammo::gateway::GatewayConnector *connector) :
  connector(connector),
  rangeScale(SatcomConfigurationManager::getInstance().getPliRelayRangeScale()),
  timeScale(SatcomConfigurationManager::getInstance().getPliRelayTimeScale())
{
}


TerseDecoder::~TerseDecoder()
{
}

bool TerseDecoder::processMessage(const uint8_t dataType, const std::string &terseData) {
  if(connector != NULL) {
    MessageScope scope = SCOPE_GLOBAL;

    PushData pushData;
    std::string originUser;
    switch( dataType ) {
    case SMS_TYPEID:			// SMS - not implemented
      return false;
    case NEVADA_PLI_TYPEID:			// PLI
      pushData.mimeType = NEVADA_PLI_MIMETYPE;
      pushData.data = parseTerseData(NEVADA_PLI_TYPEID, terseData.data(), terseData.length(), originUser );
      pushData.uri = "serial-pli";
      pushData.originUsername = originUser;
      break;
    case TRANSAPPS_PLI_TYPEID:			// New Transapps PLI
      pushData.mimeType = TRANSAPPS_PLI_MIMETYPE;
      pushData.data = parseTerseData(TRANSAPPS_PLI_TYPEID, terseData.data(), terseData.length(), originUser );
      pushData.uri = "serial-pli";
      pushData.originUsername = originUser;
      break;
    case DASH_EVENT_TYPEID:			// Dash
      pushData.mimeType = DASH_EVENT_MIMETYPE;
      pushData.data = parseTerseData(DASH_EVENT_TYPEID, terseData.data(), terseData.length(), originUser );
      pushData.uri = "serial-dash-event";
      pushData.originUsername = originUser;
      break;
    case CHAT_MESSAGE_ALL_TYPEID:			// Chat
      pushData.mimeType = CHAT_MESSAGE_ALL_MIMETYPE;
      pushData.data = parseTerseData(CHAT_MESSAGE_ALL_TYPEID, terseData.data(), terseData.length(), originUser );
      pushData.uri = "serial-chat";
      pushData.originUsername = originUser;
      break;
    case DASH_MEDIA_TYPEID:
      pushData.mimeType = DASH_MEDIA_MIMETYPE;
      pushData.data = processDashMedia(terseData);
      pushData.uri = "serial-dash-media";
      processDashMedia(terseData);
      return false;
    default:
      LOG_ERROR("Unsupported terse data type: " << (int) dataType);
      return false;
    }

    pushData.scope = scope;
    connector->pushData(pushData);
  }
  return true;
}

std::string TerseDecoder::processDashMedia(const std::string &terseData) {
  const char *data = terseData.data();
  size_t cursor = 0;
  size_t length = terseData.length();

  std::string eventId = extractString(data, cursor, length);
  std::string filename = eventId + ".jpg";
  std::tr1::shared_ptr<const std::string> fileData = extractFile(data, cursor, length);

  //write the file out to a file on disk (in the current working directory for now)
  std::ofstream eventFile(filename.data(), std::ofstream::out | std::ofstream::binary);
  eventFile.write(fileData->data(), fileData->length());
  eventFile.close();

  uint64_t createdDate = extractInt64(data, cursor, length);
  uint64_t modifiedDate = extractInt64(data, cursor, length);

  std::ostringstream jsonStream;

  jsonStream << "{\"data\":\"" << filename  << "\",\"created_date\":\"" << createdDate  << "\",\"event_id\":\"" << eventId << "\",\"modified_date\":\"" << modifiedDate << "\",\"data_type\":\"" << "image/jpeg" << "\"}";

  BinaryOutputStream dataToSend(BinaryOutputStream::ORDER_BIG_ENDIAN);
  dataToSend.appendBytes(jsonStream.str().c_str(), jsonStream.str().length() + 1); //we want the ending null terminator, which c_str() should have
  dataToSend.appendBytes("data\0", 5);
  dataToSend.appendUInt32(fileData->length());
  dataToSend.appendBytes(fileData->data(), fileData->length());
  dataToSend.appendUInt32(fileData->length());

  return dataToSend.getString();
}

std::string TerseDecoder::extractString(const char *terse, size_t& cursor, size_t length)
{
  if(cursor + sizeof(uint16_t) > length) {
    LOG_ERROR("Not enough data to get string length (cursor=" << cursor << ", length=" << length << ")");
    return "";
  }
  uint16_t nlen = ntohs ( *(uint16_t *)&(terse[cursor]) );
  if(nlen > 0) {
    cursor += 2;
    if(cursor + nlen > length) {
      LOG_ERROR("Not enough data to get string (cursor=" << cursor << ", strlen=" << nlen << ", length=" << length << ")");
      return "";
    }
    std::ostringstream oname;
    for (uint16_t i=0; i<nlen; i++) {
      oname << terse[cursor];
      cursor += 1;
    }
    return oname.str();
  } else {
    //fall back to old-style string encoding (four-byte length, UTF-16 string) if
    //size is 0.  Hopefully, we don't receive anything legitimate with a length
    //of 0 (PLI and Chat don't seem to, but this is something to keep an eye out
    //for).
    LOG_WARN("Falling back to old string encoding");
    return extractOldStyleString(terse, cursor, length);
  }
}

std::string TerseDecoder::extractOldStyleString(const char *terse, size_t& cursor, size_t length)
{
  if(cursor + sizeof(uint32_t) > length) {
    LOG_ERROR("Not enough data to get string length (cursor=" << cursor << ", length=" << length << ")");
    return "";
  }
  uint32_t nlen = ntohl ( *(uint32_t *)&(terse[cursor]) );
  cursor += sizeof(uint32_t);

  if(cursor + nlen * 2 > length) {
    LOG_ERROR("Not enough data to get string (cursor=" << cursor << ", strlen=" << nlen << ", length=" << length << ")");
    return "";
  }
  std::ostringstream oname;
  for (uint32_t i=0; i<nlen; i++) {
    uint16_t uchar = ntohs( *(uint16_t *)&terse[cursor] );
    cursor += 2;
    oname << static_cast<uint8_t>(uchar & 0xff);
  }
  return oname.str();
}

std::tr1::shared_ptr<const std::string> TerseDecoder::extractFile(const char *terse, size_t &cursor, size_t length) {
  LOG_TRACE("Getting file...");
  if(cursor + sizeof(uint32_t) > length) {
    LOG_ERROR("Not enough data to get file length (cursor=" << cursor << ", length=" << length << ")");
    return std::tr1::shared_ptr<const std::string>();
  }
  uint32_t fileLength = ntohl ( *(uint32_t *)&(terse[cursor]) );
  LOG_TRACE("File length: " << fileLength);
  cursor += sizeof(uint32_t);

  if(cursor + fileLength  > length) {
    LOG_ERROR("Not enough data to get file (cursor=" << cursor << ", strlen=" << fileLength << ", length=" << length << ")");
    return std::tr1::shared_ptr<std::string>();
  }
  std::tr1::shared_ptr<const std::string> file(new std::string(&terse[cursor], fileLength));
  cursor += fileLength;

  //LOG_TRACE("Got blob: " << blob);
  return file;
}

std::string TerseDecoder::extractBlob(const char *terse, size_t& cursor, size_t length)
{
  LOG_TRACE("Getting blob...");
  if(cursor + sizeof(uint16_t) > length) {
    LOG_ERROR("Not enough data to get blob length (cursor=" << cursor << ", length=" << length << ")");
    return "";
  }
  uint16_t blobLength = ntohs ( *(uint16_t *)&(terse[cursor]) );
  LOG_TRACE("Blob length: " << blobLength);
  cursor += sizeof(uint16_t);

  if(cursor + blobLength  > length) {
    LOG_ERROR("Not enough data to get blob (cursor=" << cursor << ", strlen=" << blobLength << ", length=" << length << ")");
    return "";
  }
  std::string blob(&terse[cursor], blobLength);
  cursor += blobLength;

  //LOG_TRACE("Got blob: " << blob);
  return blob;
}

int8_t TerseDecoder::extractInt8(const char* terse, size_t& cursor, size_t length)
{
  if(cursor + sizeof(int8_t) > length) {
    LOG_ERROR("Not enough data to get int8 (cursor=" << cursor << ", length=" << length << ")");
    return 0;
  }
  int8_t result = (int8_t) terse[cursor];
  cursor += sizeof(int8_t);
  return result;
}

int16_t TerseDecoder::extractInt16(const char *terse, size_t& cursor, size_t length)
{
  if(cursor + sizeof(int16_t) > length) {
    LOG_ERROR("Not enough data to get int16 (cursor=" << cursor << ", length=" << length << ")");
    return 0;
  }
  int16_t result = ntohs( *(int16_t *)&terse[cursor] );
  cursor += sizeof(int16_t);
  return result;
}

int32_t TerseDecoder::extractInt32(const char *terse, size_t& cursor, size_t length)
{
  if(cursor + sizeof(int32_t) > length) {
    LOG_ERROR("Not enough data to get int32 (cursor=" << cursor << ", length=" << length << ")");
    return 0;
  }
  int32_t result = ntohl( *(int32_t *)&terse[cursor] );
  cursor += sizeof(int32_t);
  return result;
}

int64_t TerseDecoder::extractInt64(const char *terse, size_t& cursor, size_t length)
{
  if(cursor + sizeof(int64_t) > length) {
    LOG_ERROR("Not enough data to get int64 (cursor=" << cursor << ", length=" << length << ")");
    return 0;
  }
  int64_t result = ntohll( *(int64_t *)&terse[cursor] );
  cursor += sizeof(int64_t);
  return result;
}


std::string TerseDecoder::parseTerseData(int mt, const char *terse, size_t terseLength, std::string &originUser) {
  std::ostringstream jsonStr;
  size_t cursor = 0;
  switch(mt) {
  case NEVADA_PLI_TYPEID:			// PLI
    /*
    LID -- Java Long (8)
    UserId - Java Long (8)
    UnitId - Java Long (8)
    Name - Text : Int (2), UTF8 Char (1 byte per)
    Lat - Java Long (8)
    Lon - Java Long (8)
    Created - Java Long (8)
    Modified - Java Long (8)
    */
    /* this will need to change - as MAP PLI definition is goig to change */
    {
      uint64_t lid  = (uint64_t)extractInt64(terse, cursor, terseLength);
      uint64_t uid  = (uint64_t)extractInt64(terse, cursor, terseLength);
      uint64_t unid = (uint64_t)extractInt64(terse, cursor, terseLength);
      LOG_INFO((long) this << std::hex << "PLI: l(" << lid << ") u(" << uid << ") un(" << unid << ")");
      originUser = extractString(terse, cursor, terseLength);
      int64_t lat      = extractInt64(terse, cursor, terseLength);
      int64_t lon      = extractInt64(terse, cursor, terseLength);
      int64_t created  = extractInt64(terse, cursor, terseLength);
      int64_t modified = extractInt64(terse, cursor, terseLength);

      // JSON
      // {\"lid\":\"0\",\"lon\":\"-74888318\",\"unitid\":\"1\",\"created\":\"1320329753964\",\"name\":\"ahammer\",\"userid\":\"731\",\"lat\":\"40187744\",\"modified\":\"0\"}
      jsonStr << "{\"lid\":\"" << lid << "\",\"userid\":\"" << uid << "\",\"unitid\":\"" << unid << "\",\"name\":\"" << originUser
        << "\",\"lat\":\"" << lat << "\",\"lon\":\"" << lon << "\",\"created\":\"" << created << "\",\"modified\":\"" << modified
        << "\"}";
      latestPliTimestamps[originUser] = created;
    }

    break;
  case TRANSAPPS_PLI_TYPEID:			// Transapps (Sandeep mod...) PLI
    /*
    Name - Text : Int (2), UTF8 Char (1 byte per)
    Lat - Java Int (4)
    Lon - Java Int (4)
    Created - Java Int (4)
    */
    /* this will need to change - as MAP PLI definition is goig to change */
    {
      originUser = extractString(terse, cursor, terseLength);
      int32_t lat      = extractInt32(terse, cursor, terseLength);
      int32_t lon      = extractInt32(terse, cursor, terseLength);
      uint32_t created  = extractInt32(terse, cursor, terseLength);
      std::string groupPliBlob = extractBlob(terse, cursor, terseLength);
      parseGroupPliBlob(groupPliBlob, lat, lon, created);

      //update timestamp of last received PLI, so we know what the last 
      //received time is for delta PLI
      latestPliTimestamps[originUser] = created;

      // JSON
      // {\"lid\":\"0\",\"lon\":\"-74888318\",\"unitid\":\"1\",\"created\":\"1320329753964\",\"name\":\"ahammer\",\"userid\":\"731\",\"lat\":\"40187744\",\"modified\":\"0\"}
      jsonStr << generateTransappsPli(originUser, lat, lon, created, 0);
    }

    break;

  case DASH_EVENT_TYPEID:			// Dash-Event
    {
      std::string uuid = extractString(terse, cursor, terseLength);
      int32_t mediaCount = extractInt32(terse, cursor, terseLength);
      originUser = extractString(terse, cursor, terseLength);
      int32_t latitude = extractInt32(terse, cursor, terseLength);
      int32_t longitude = extractInt32(terse, cursor, terseLength);
      std::string title = extractString(terse, cursor, terseLength);
      std::string description = extractString(terse, cursor, terseLength);
      uint64_t created = extractInt64(terse, cursor, terseLength);

      jsonStr << "{\"created_date\":\"" << created << "\",\"modified_date\":\"" << created << "\",\"status\":\"" << 3 << "\",\"media_count\":\"" << mediaCount << "\",\"cid\":\"" << "null" << "\",\"size\":\"" << 0 << "\",\"display_name\":\"" << "<no_title>" << "\",\"category\":\"" << "null" <<"\",\"description\":\"" << description << "\",\"longitude\":\"" << longitude << "\",\"uuid\":\"" << uuid << "\",\"latitude\":\"" << latitude << "\",\"originator\":\"" << originUser << "\"}";
    }
    
    break;
  case CHAT_MESSAGE_ALL_TYPEID:			// Group-chat
    /*
    originator - Text : Int (2), UTF Char (1 byte per)
    text - Text : Int (2), UTF Char (1 byte per)
    created_date Java Long (8)
    */
    {
      std::string uuid = extractString(terse, cursor, terseLength);
      std::string originator = extractString(terse, cursor, terseLength);
      originUser = originator;
      std::string text = extractString(terse, cursor, terseLength);
      uint32_t created = extractInt32(terse, cursor, terseLength);
      uint64_t createdMillis = 1000 * static_cast<uint64_t>(created);
      //ACE_Utils::UUID *uuid = ACE_Utils::UUID_GENERATOR::instance ()->generate_UUID ();
      // JSON
      // "{\"created_date\":\"1339572928976\",\"text\":\"Wwwww\",\"modified_date\":\"1339572928984\",\"status\":\"21\",\"receipts\":\"0\",\"group_id\":\"All\",\"media_count\":\"0\",\"longitude\":\"0\",\"uuid\":\"9bf10c58-9154-4be8-8f63-e6a79a5ecbc1\",\"latitude\":\"0\",\"originator\":\"mark\"}"
      jsonStr << "{\"created_date\":\"" << createdMillis << "\",\"text\":\"" << text << "\",\"modified_date\":\"" << createdMillis << "\",\"status\":\"21\",\"receipts\":\"0\",\"group_id\":\"All\",\"media_count\":\"0\",\"longitude\":\"0\",\"uuid\":\"" << uuid << "\",\"latitude\":\"0\",\"originator\":\""<< originator << "\"}";
    }


  }
  LOG_TRACE((long) this << jsonStr.str() );

  return jsonStr.str();
}

void TerseDecoder::parseGroupPliBlob(std::string groupPliBlob, int32_t baseLat, int32_t baseLon, uint32_t baseTime) {
  size_t cursor = 0;
  const char *groupPliBlobArray = groupPliBlob.data();
  size_t groupPliBlobLength = groupPliBlob.length();

  int8_t count = extractInt8(groupPliBlobArray, cursor, groupPliBlobLength);
  for(int i = 0; i < count; ++i) {
    std::string originUsername = extractString(groupPliBlobArray, cursor, groupPliBlobLength);
    int16_t dLat = extractInt16(groupPliBlobArray, cursor, groupPliBlobLength);
    int16_t dLon = extractInt16(groupPliBlobArray, cursor, groupPliBlobLength);
    int8_t dCreatedTime = extractInt8(groupPliBlobArray, cursor, groupPliBlobLength);
    int8_t hopCount = extractInt8(groupPliBlobArray, cursor, groupPliBlobLength);

    int32_t latitude = (baseLat - dLat) * rangeScale;
    int32_t longitude = (baseLon - dLon) * rangeScale;
    uint32_t createdTime = (baseTime - dCreatedTime) * timeScale;

    TimestampMap::iterator it = latestPliTimestamps.find(originUsername);
    if(it != latestPliTimestamps.end() && createdTime < it->second) {
      //received delta PLI is older than the one we already have; discard it
      LOG_TRACE("Dropping PLI relay message from " << originUsername << " because it's too old");
    } else {
      //received delta PLI is newer than the one we have or we haven't gotten
      //one before, update map and send it
      latestPliTimestamps[originUsername] = createdTime;
      std::string pliString = generateTransappsPli(originUsername, latitude, longitude, createdTime, hopCount);

      PushData pushData;
      pushData.mimeType = TRANSAPPS_PLI_MIMETYPE;
      pushData.data = pliString;
      pushData.uri = "serial-pli";
      pushData.originUsername = originUsername;
      pushData.scope = SCOPE_GLOBAL;

      LOG_TRACE("Sending group PLI relay message: " << pushData.data);
      connector->pushData(pushData);
    }
  }
}

std::string TerseDecoder::generateTransappsPli(std::string originUser, int32_t lat, int32_t lon, uint32_t created, int8_t hopCount) {
  std::ostringstream jsonStr;
  jsonStr << "{\"name\":\"" << originUser
    << "\",\"lat\":\"" << lat << "\",\"lon\":\"" << lon
    << "\",\"altitude\":\"" << 0 << "\",\"accuracy\":\"" << 0
    << "\",\"created\":\"" << 1000*(uint64_t)created << "\",\"modified\":\"" << 1000*(uint64_t)created
    << "\",\"hops\":\"" << (int) hopCount
    << "\"}";
  return jsonStr.str();
}
