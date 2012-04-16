
#include <sstream>

#include "json/value.h"
#include "json/reader.h"
#include "log.h"

#include "GatewaySyncSerialization.h"

std::string
requestChecksumsMessageData::encodeJson (void)
{
  std::ostringstream sstream;
  sstream << "{"
          << "\"tv_sec\" : " << tv_sec_
          << "}";
  
  std::string retval (sstream.str ());
  return retval;
}

bool
requestChecksumsMessageData::decodeJson (const std::string &data)
{
  Json::Reader jsonReader;
  Json::Value root;
  bool parseSuccess = jsonReader.parse (data, root);

  if (!parseSuccess)
    {
      LOG_ERROR ("JSON parsing error: requestChecksumsMessageData - "
                 << jsonReader.getFormatedErrorMessages ());
                 
      return false;
    }
    
  tv_sec_ = root["tv_sec"].asInt ();
                
  return true;
}

RequestChecksumsMessage *
requestChecksumsMessageData::encodeProtobuf (void)
{
  return 0;
}

bool
requestChecksumsMessageData::decodeProtobuf (
  const RequestChecksumsMessage *msg)
{
  return true;
}

//============================================================

sendChecksumsMessageData::sendChecksumsMessageData (void)
  : jsonKey_ ("RecentChecksums")
{
}

sendChecksumsMessageData::sendChecksumsMessageData (const char *keyname)
  : jsonKey_ (keyname)
{
}

std::string
sendChecksumsMessageData::encodeJson (void)
{
  std::ostringstream sstream;
  sstream << "{"
          << "\"" << jsonKey_ << "\": [";
          
  for (std::vector<std::string>::const_iterator i = checksums_.begin ();
       i != checksums_.end ();
       ++i)
    {
      if (i != checksums_.begin ())
        {
          sstream << ",";
        }
        
      sstream << "\"" << *i << "\"";
    }
      
  sstream << "]}";
  
  std::string retval (sstream.str ());
  return retval;
}

bool
sendChecksumsMessageData::decodeJson (const std::string &data)
{
  Json::Reader jsonReader;
  Json::Value root;
  bool parseSuccess = jsonReader.parse (data, root);

  if (!parseSuccess)
    {
      LOG_ERROR ("JSON parsing error: sendChecksumsMessageData - "
                 << jsonReader.getFormatedErrorMessages ());
                 
      return false;
    }
    
  for (Json::Value::UInt i = 0;
       i < root[jsonKey_.c_str ()].size ();
       ++i)
    {
      checksums_.push_back (root[jsonKey_.c_str ()][i].asString ());
    }
    
  return true;
}

SendChecksumsMessage *
sendChecksumsMessageData::encodeProtobuf (void)
{
  return 0;
}

bool
sendChecksumsMessageData::decodeProtobuf (
  const SendChecksumsMessage *msg)
{
  return true;
}

//==============================================================

requestObjectsMessageData::requestObjectsMessageData (void)
  : sendChecksumsMessageData ("MissingChecksums")
{
}

//==============================================================

std::string
sendObjectsMessageData::encodeJson (void)
{
  std::ostringstream sstream;
  sstream << "{"
          << "\"DataStoreObjects\": [";
          
  for (std::vector<dbRow>::const_iterator i = objects_.begin ();
       i != objects_.end ();
       ++i)
    {
      if (i != objects_.begin ())
        {
          sstream << ",";
        }
        
      sstream << "{"
              << "\"uri\": \"" << i->uri_ << "\","
              << "\"mime_type\": \"" << i->mime_type_ << "\","
              << "\"origin_user\": \"" << i->origin_user_ << "\","
              << "\"tv_sec\": " << i->tv_sec_ << ","
              << "\"tv_usec\": " << i->tv_usec_ << ","
              << "\"data\": \"" << i->data_ << "\","
              << "\"checksum\": \"" << i->checksum_ << "\""
              << "}";
    }
      
  sstream << "]}";
  
  std::string retval (sstream.str ());
  return retval;
}

bool
sendObjectsMessageData::decodeJson (const std::string &data)
{
  Json::Reader jsonReader;
  Json::Value root;
  bool parseSuccess = jsonReader.parse (data, root);

  if (!parseSuccess)
    {
      LOG_ERROR ("JSON parsing error: sendObjectsMessageData - "
                 << jsonReader.getFormatedErrorMessages ());
                 
      return false;
    }
    
  for (Json::Value::UInt i = 0;
       i < root["DataStoreObjects"].size ();
       ++i)
    {
      dbRow row;
      
      row.uri_ = root["DataStoreObjects"][i]["uri"].asString ();
      row.mime_type_ = root["DataStoreObjects"][i]["mime_type"].asString ();
      row.origin_user_ = root["DataStoreObjects"][i]["origin_user"].asString ();
      row.tv_sec_ = root["DataStoreObjects"][i]["tv_sec"].asInt ();
      row.tv_usec_ = root["DataStoreObjects"][i]["tv_usec"].asInt ();
      row.data_ = root["DataStoreObjects"][i]["data"].asString ();
      row.checksum_ = root["DataStoreObjects"][i]["checksum"].asString ();
      
      objects_.push_back (row);
    }
    
  return true;
}

SendObjectsMessage *
sendObjectsMessageData::encodeProtobuf (void)
{
  return 0;
}

bool
sendObjectsMessageData::decodeProtobuf (
  const SendObjectsMessage *msg)
{
  return true;
}


