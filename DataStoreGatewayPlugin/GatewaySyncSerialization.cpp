
#include <sstream>

#include "json/value.h"
#include "json/reader.h"
#include "json/writer.h"
#include "log.h"

#include "GatewaySyncSerialization.h"
#include "DataStoreConfigManager.h"

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

std::string
requestChecksumsMessageData::encodeProtobuf (void)
{
  RequestChecksumsMessage msg;
  msg.set_tv_sec(tv_sec_);
  
  std::string serializedProtobuf = msg.SerializeAsString();
  return serializedProtobuf;
}

bool
requestChecksumsMessageData::decodeProtobuf (
  const std::string &msg)
{
  RequestChecksumsMessage parsedMsg;
  bool result = parsedMsg.ParseFromString(msg);
  
  if(!result) {
    LOG_ERROR("Protobuf parsing error in requestChecksumsMessageData");
    return false;
  }
  
  LOG_TRACE("  RequestChecksumsMessage: " << parsedMsg.DebugString());
  
  tv_sec_ = parsedMsg.tv_sec();
  
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
      
  sstream << "]}" << std::ends;
  
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

std::string
sendChecksumsMessageData::encodeProtobuf (void)
{
  SendChecksumsMessage msg;
          
  for (std::vector<std::string>::const_iterator i = checksums_.begin ();
       i != checksums_.end ();
       ++i)
    {
      msg.add_checksums(*i);
    }
      
  std::string serializedProtobuf = msg.SerializeAsString();
  return serializedProtobuf;
}

bool
sendChecksumsMessageData::decodeProtobuf (
  const std::string &msg)
{
  SendChecksumsMessage parsedMsg;
  bool result = parsedMsg.ParseFromString(msg);
  
  if(!result) {
    LOG_ERROR("Protobuf parsing error in sendChecksumsMessageData");
    return false;
  }
  
  LOG_TRACE("  SendChecksumsMessage: " << parsedMsg.DebugString());
  
  for(google::protobuf::RepeatedPtrField<const std::string>::iterator it = parsedMsg.checksums().begin(); it != parsedMsg.checksums().end(); ++it) {
    checksums_.push_back(*it);
  }
  
  return true;
}

//==============================================================

// Structure of mime types corresponding to this class and the
// base class are the same, using different key names to make
// each use case more self-evident.
requestObjectsMessageData::requestObjectsMessageData (void)
  : sendChecksumsMessageData ("MissingChecksums")
{
}

//==============================================================

std::string
sendObjectsMessageData::encodeJson (void)
{
  //Use libjson for serialization because it handles strings containing
  //quotes and other special characters (i.e. in data) correctly          
  Json::Value root;
  Json::Value objectsArray = Json::Value(Json::arrayValue);
          
  for (std::vector<dbRow>::const_iterator i = objects_.begin ();
       i != objects_.end ();
       ++i)
    {              
      Json::Value object;
      object["uri"] = i->uri_;
      object["mime_type"] = i->mime_type_;
      object["origin_user"] = i->origin_user_;
      object["tv_sec"] = (Json::UInt) i->tv_sec_;
      object["tv_usec"] = (Json::UInt) i->tv_usec_;
      object["data"] = i->data_;
      object["checksum"] = i->checksum_;
      
      objectsArray.append(object);
              
    }
    
  root["DataStoreObjects"] = objectsArray;
  
  Json::FastWriter writer;
  
  std::string retval (writer.write(root));
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
      size_t slot = objects_.size ();
      objects_.push_back (row);
      
      objects_[slot].uri_ =
        root["DataStoreObjects"][i]["uri"].asString ();
        
      objects_[slot].mime_type_ =
        root["DataStoreObjects"][i]["mime_type"].asString ();
        
      objects_[slot].origin_user_ =
        root["DataStoreObjects"][i]["origin_user"].asString ();
        
      objects_[slot].tv_sec_ =
        root["DataStoreObjects"][i]["tv_sec"].asInt ();
        
      objects_[slot].tv_usec_ =
        root["DataStoreObjects"][i]["tv_usec"].asInt ();
      
      objects_[slot].data_ =
        root["DataStoreObjects"][i]["data"].asString ();
      
      objects_[slot].checksum_ =
        root["DataStoreObjects"][i]["checksum"].asString ();
    }
    
  return true;
}

std::string 
sendObjectsMessageData::encodeProtobuf (void)
{
  SendObjectsMessage msg;
          
  for (std::vector<dbRow>::const_iterator i = objects_.begin ();
       i != objects_.end ();
       ++i)
    {              
      SendObjectsMessage::DbRow *object = msg.add_objects();
      object->set_uri(i->uri_);
      object->set_mime_type(i->mime_type_);
      object->set_origin_user(i->origin_user_);
      object->set_tv_sec(i->tv_sec_);
      object->set_tv_usec(i->tv_usec_);
      object->set_data(i->data_);
      object->set_checksum(i->checksum_);
              
    }
    
  std::string serializedProtobuf = msg.SerializeAsString();
  return serializedProtobuf;
}

bool
sendObjectsMessageData::decodeProtobuf (
  const std::string &msg)
{
  SendObjectsMessage parsedMsg;
  bool result = parsedMsg.ParseFromString(msg);
  
  if(!result) {
    LOG_ERROR("Protobuf parsing error in sendObjectsMessageData");
    return false;
  }
  
  LOG_TRACE("  SendObjectsMessage: " << parsedMsg.DebugString());
  
  for(google::protobuf::RepeatedPtrField<const SendObjectsMessage::DbRow>::iterator it = parsedMsg.objects().begin(); it != parsedMsg.objects().end(); ++it) {
    dbRow row;
    
    row.uri_ = it->uri();
    row.mime_type_ = it->mime_type();
    row.origin_user_ = it->origin_user();
    row.tv_sec_ = it->tv_sec();
    row.tv_usec_ = it->tv_usec();
    row.data_ = it->data();
    row.checksum_ = it->checksum();
    
    objects_.push_back(row);
  }
  
  return true;
}


