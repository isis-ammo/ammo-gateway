
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

std::string
sendChecksumsMessageData::encodeJson (void)
{
  std::ostringstream sstream;
  sstream << "{"
          << "\"Checksums\": [";
          
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
       i < root["Checksums"].size ();
       ++i)
    {
      checksums_.push_back (root["Checksums"][i].asString ());
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
