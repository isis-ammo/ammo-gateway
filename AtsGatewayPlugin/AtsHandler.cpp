
#include <assert.h>

#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>

#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

#include "AtsHandler.h"

#include "log.h"

typedef std::pair<std::vector<char>::const_iterator, std::vector<char>::const_iterator > CharSpan;
typedef std::pair<std::string, CharSpan > NamedBlob;

static char curlErrorBuffer[CURL_ERROR_SIZE]; //libcurl variable for error string

AtsHandler::AtsHandler() {
 assert(sizeof(uint32_t) == 4); 

 config = AtsConfigMgr::getInstance();
 // credentials = config->getCredentialsForUser(username);
}


void AtsHandler::onConnect(GatewayConnector *sender) { }

void AtsHandler::onDisconnect(GatewayConnector *sender) { }

/**
* In response to push data request (DataPushReceiverListener)
* i.e. the client has published data to a topic
*      this is fired because the service has subscribed to the topic
* 
* types: 
*  request for chat room construction
*
*/
void AtsHandler::onDataReceived(GatewayConnector *sender, 
                                std::string uri, 
                                std::string dataType,
                                std::vector< char >& payload,
                                std::string originUsername)
{
  std::cout << "Got push data." << std::endl;
  std::cout << "  URI: " << uri << std::endl;
  std::cout << "  Data type: " << dataType << std::endl;
  std::vector<char>::iterator endIt =  (payload.size() < 128) ? payload.end() : payload.begin()+128;
  std::cout << "  Data: " << std::string(payload.begin(), endIt) << std::endl;
  std::cout << "  Origin User Name: " << originUsername << std::endl;

  if (dataType == RTC_UPLOAD_CHANNEL_MEDIA_NS) {
     std::string result = uploadMedia(dataType, payload);
     std::cout << " Push " << dataType << " result: " << result << std::endl;
     return;
  }
  if (dataType == RTC_INVITE_NS) {
     std::string result = inviteChat(dataType, payload);
     std::cout << " Push "<< dataType << " result: " << result << std::endl;
     return;
  }
  if (dataType == RTC_CREATE_CHANNEL_NS) {
     std::string data = channelCreate(dataType, payload);
     sender->pushData(uri, dataType, data);
     std::cout << " Push " << dataType << " result: " << data.substr(0,128) << std::endl;
     return;
  }
  if (dataType == RTC_SHARE_GPS_NS) {
  }
}
 

/**
* In response to pull data request (PullRequestReceiverListener) methods
* i.e. the client is initializing its content providers
*      this is fired because the service has announced its ability to provide a particular data type (topic)
* 
* types: 
*  request for chat room construction
*
*/
void AtsHandler::onDataReceived(GatewayConnector *sender,
                              std::string requestUid,
                              std::string pluginId,
                              std::string dataType, 
                              std::string query,
                              std::string projection,
                              unsigned int maxResults,
                              unsigned int startFromCount,
                              bool liveQuery)
{
  LOG_INFO("Got pull request data.");
  LOG_INFO( "  ReqId: " << requestUid);
  LOG_INFO( "  Plugin: " << pluginId);
  LOG_INFO( "  Data type: " << dataType);
  LOG_INFO( "  Query: " << query);
  LOG_INFO( "  Projection: " << projection);
  LOG_INFO( "  Start Count: " << startFromCount);
  LOG_INFO( "  Live: " << liveQuery);
  if (dataType == RTC_LIST_PEOPLE_NS) {
     std::vector<char> data = listPeople(dataType, query);
     LOG_INFO( "pull " << dataType << " result: " << std::string(data.begin(), data.begin()+128));
     sender->pullResponse(requestUid, pluginId, dataType, query, data);
     LOG_INFO( "send response: " << requestUid);
     return;
  }
  if (dataType == RTC_LIST_CHANNEL_NS) {
     std::vector<char> data = listChannels(dataType, query);
     sender->pullResponse(requestUid, pluginId, dataType, query, data);
     std::cout << " Pull " << dataType << " result: " << std::string(data.begin(), data.begin()+128) << std::endl;
     return;
  }
}

  // PullResponseReceiverListener
void AtsHandler::onDataReceived (GatewayConnector *sender,
                               std::string requestUid,
                               std::string pluginId,
                               std::string dataType, 
                               std::string uri,
                               std::vector< char > &data)
{
  std::cout << "Got pull response data." << std::endl;
  std::cout << "  ReqId: " << requestUid << std::endl;
  std::cout << "  Plugin: " << pluginId << std::endl;
  std::cout << "  Data type: " << dataType << std::endl;
  std::cout << "  Uri: " << uri << std::endl;
  std::cout << "  Data: " << std::string(data.begin(), data.end()) << std::endl;
}

/**
  The data is serialized in a particular way.
  1) a json blob of printable characters terminated by a null character
  2) a set of data blobs
     a) a null terminated string containing the name of the blob
     b) the length (a 4 byte big endian) of the blob (not including the lenth itself
     c) the bytes of the blob
     d) the length repeated
*/

static int parse_payload(std::vector<char>& payload, Json::Value& meta, std::vector<NamedBlob>& media ) {

  LOG_DEBUG("Extracting JSON metadata...");

  std::vector<char>::iterator begin = payload.begin();
  std::vector<char>::iterator end = std::find(begin, payload.end(), '\0');
  std::string json(begin, end);

  LOG_DEBUG("JSON string: " << json);

  Json::Reader jsonReader;
  bool parseSuccess = jsonReader.parse(json, meta);
  if(!parseSuccess) {
     LOG_ERROR("JSON parsing error:");
     LOG_ERROR(jsonReader.getFormatedErrorMessages());
     return -1;
  }
  LOG_DEBUG("Parsed JSON: " << meta.toStyledString());
    
  begin = ++end;
  while(end != payload.end()) { // more media?
    LOG_DEBUG("Process BLOB: ");
    // get the name of the blob 
    end = std::find(begin, payload.end(), 0);
    // check the end value for success
    if (end == payload.end()) break;
    std::string blobName(begin, end);
    LOG_DEBUG("Process BLOB: "+blobName);

    begin = ++end;
    unsigned char mediaSizeBuff[4];
    std::copy(begin, begin+4, mediaSizeBuff);
    uint32_t mediaSize = ntohl( *(reinterpret_cast<uint32_t*>(mediaSizeBuff)) );
    LOG_DEBUG("Media Data Size:" << mediaSize);

    unsigned int bytesRemaining = payload.size() - (begin-payload.begin()) - 8;
    LOG_DEBUG("Maximum valid data size: " << bytesRemaining);

    if(mediaSize > bytesRemaining) {
      LOG_ERROR("Error:  data size is larger than available data.");
      return -2;
    }
    
    CharSpan cs(begin+4, begin+4+mediaSize);
    NamedBlob nb(blobName, cs);
    media.push_back(nb);

    begin += 4 + mediaSize + 8;
  }
  return 0; 
}

/**
  There really shouldn't be any media.
*/
static int parse_query(std::vector<char> query, Json::Value& meta) {
   LOG_TRACE("parse query <vector(char)>");
   std::vector< NamedBlob > media;
   return parse_payload(query, meta, media);
}

static int parse_query(std::string query, Json::Value& meta) {
   LOG_TRACE("parse query <string>");
   std::vector<char> reform(query.begin(),query.end());
   return parse_query(reform, meta);
}

/**
  Predicates to be used by algorithms.
*/
bool isFileBlob(const NamedBlob blob) { return (blob.first == "file"); }
bool isClipBlob(const NamedBlob blob) { return (blob.first == "file"); }

/**
 a function used by curl to handle the curl log information.
 If such a function is not provided curl writes to a file.
*/
static int write_callback(char *data, size_t size, size_t nmemb, std::string *writerData) {
  std::ostringstream msg;  
  msg << "message: " << size << " x " << nmemb ;
  LOG_DEBUG(msg.str());
  if(writerData == NULL) { return 0; }
  writerData->append(data, size*nmemb);
  return size * nmemb;
}

/**
• String displayName - the human readable name of the file (this doesn't necessarily have to be unique, but
                     it is a good practice to make it unique within a given channel at least)
• String description (optional) - a description of this media file
• String channelName - the name of the channel to which this media should be associated
• Binary/Text data - the file being uploaded (in most client apps, this will not be a post parameter, but will be sent
                     via the request body as a multipart attachment)
                     Binary/Text clip - the clip to upload (in most client apps, this will not be a post parameter, but will be sent
                     via the request body as a multipart attachment)
• long   length - the size of the data being uploaded

• Note: There is no difference enforced on the server end, but the idea is that the clip will be a
reduced size version of the full file, so the server can hand back smaller files if requested by a
client. How the reduced size version is created is client specific.
*/
std::string AtsHandler::uploadMedia( std::string mediaType, std::vector< char > &payload ) 
{
   // parse the serialized packet
   Json::Value meta;
   std::vector<NamedBlob> media;
   int rc = parse_payload(payload, meta, media);
   if (rc < 0) return ""; // the parsing of the payload failed.
    
   CURL *curl;
   CURLcode res;
 
   res = curl_global_init(CURL_GLOBAL_ALL);

   struct curl_httppost* formpost=NULL;
   struct curl_httppost* lastptr=NULL;

   curl_formadd(&formpost, &lastptr,
                CURLFORM_COPYNAME, "displayName",
                CURLFORM_COPYCONTENTS, meta["displayName"].asString().c_str(),
                CURLFORM_END);
 
   curl_formadd(&formpost, &lastptr,
                CURLFORM_COPYNAME, "description",
                CURLFORM_COPYCONTENTS, meta["description"].asString().c_str(),
                CURLFORM_END);

   curl_formadd(&formpost, &lastptr,
                CURLFORM_COPYNAME, "channelName",
                CURLFORM_COPYCONTENTS, meta["channelName"].asString().c_str(),
                CURLFORM_END);

   std::vector<NamedBlob>::iterator fileIt = std::find_if(media.begin(), media.end(), isFileBlob);
   if (fileIt != media.end()) {
      NamedBlob fileBlob = *fileIt;
      char fileBuff[fileBlob.second.second - fileBlob.second.first];
      std::copy(fileBlob.second.first, fileBlob.second.second, fileBuff);
      curl_formadd(&formpost, &lastptr,
                   CURLFORM_COPYNAME, "file",
                   CURLFORM_BUFFER, fileBlob.first.c_str(),
                   CURLFORM_BUFFERPTR, fileBuff,
                   CURLFORM_BUFFERLENGTH, sizeof(fileBuff),
                   CURLFORM_CONTENTTYPE, mediaType.c_str(),
                   CURLFORM_END);
   }

   std::vector<NamedBlob>::iterator clipIt = std::find_if(media.begin(), media.end(), isClipBlob);
   if (clipIt != media.end()) {
     NamedBlob clipBlob = *clipIt; 
     char clipBuff[clipBlob.second.second - clipBlob.second.first];
     std::copy(clipBlob.second.first, clipBlob.second.second, clipBuff);
     curl_formadd(&formpost, &lastptr,
                CURLFORM_COPYNAME, "clip",
                CURLFORM_BUFFER, clipBlob.first.c_str(),
                CURLFORM_BUFFERPTR, clipBuff,
                CURLFORM_BUFFERLENGTH, sizeof(clipBuff),
                CURLFORM_CONTENTTYPE, mediaType.c_str(),
                CURLFORM_END);
   }

   curl = curl_easy_init();
   if (curl == 0) return "";

   res = curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curlErrorBuffer);
   if(res != CURLE_OK) { LOG_ERROR("Failed to set error buffer."); return ""; }

   LOG_DEBUG("url: "+ config->getUrl(RTC_UPLOAD_CHANNEL_MEDIA)+ " who: "+config->getHttpAuth());

   res = curl_easy_setopt(curl, CURLOPT_URL, config->getUrl(RTC_UPLOAD_CHANNEL_MEDIA).c_str());
   if(res != CURLE_OK) { LOG_ERROR("Failed to set URL: " << curlErrorBuffer); return ""; }

   res = curl_easy_setopt(curl, CURLOPT_USERPWD, config->getHttpAuth().c_str());
   if(res != CURLE_OK) { LOG_ERROR("Failed to set user/pass: " << curlErrorBuffer); return ""; }

   res = curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
   if(res != CURLE_OK) { LOG_ERROR("Failed to set post data: " << curlErrorBuffer); return ""; }

   std::string returnedData = "";
   res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
   if(res != CURLE_OK) { LOG_ERROR("Failed to set writer: " << curlErrorBuffer); return ""; }

   res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &returnedData);
   if(res != CURLE_OK) { LOG_ERROR("Failed to set write data: " << curlErrorBuffer); return ""; }

   res = curl_easy_perform(curl);
   if(res != CURLE_OK) { LOG_ERROR("Failed to POST data: " << curlErrorBuffer); returnedData = ""; }

   curl_easy_cleanup(curl);
   curl_formfree(formpost);

   LOG_DEBUG("Returned data from POST: " << returnedData);
   return returnedData;
}

std::string AtsHandler::inviteChat( std::string mediaType, std::vector< char > &payload ) 
{
   // parse the serialized packet
   Json::Value meta;
   std::vector<NamedBlob> media;
   int rc = parse_payload(payload, meta, media);
   if (rc < 0) return "";
    
   CURL *curl;
   CURLcode res;
 
   res = curl_global_init(CURL_GLOBAL_ALL);

   struct curl_httppost* formpost=NULL;
   struct curl_httppost* lastptr=NULL;

   curl_formadd(&formpost, &lastptr,
                CURLFORM_COPYNAME, "displayName",
                CURLFORM_COPYCONTENTS, meta["displayName"].asString().c_str(),
                CURLFORM_END);

   curl = curl_easy_init();
   if (curl == 0) return "";

   res = curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curlErrorBuffer);
   if(res != CURLE_OK) { LOG_ERROR("Failed to set error buffer."); return ""; }

   LOG_DEBUG("url: "+ config->getUrl(RTC_INVITE_NS)+ " who: "+config->getHttpAuth());

   res = curl_easy_setopt(curl, CURLOPT_URL, config->getUrl(RTC_INVITE).c_str());
   if(res != CURLE_OK) { LOG_ERROR("Failed to set URL: " << curlErrorBuffer); return ""; }

   res = curl_easy_setopt(curl, CURLOPT_USERPWD, config->getHttpAuth().c_str());
   if(res != CURLE_OK) { LOG_ERROR("Failed to set user/pass: " << curlErrorBuffer); return ""; }

   res = curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
   if(res != CURLE_OK) { LOG_ERROR("Failed to set post data: " << curlErrorBuffer); return ""; }

   std::string returnedData = "";
   res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
   if(res != CURLE_OK) { LOG_ERROR("Failed to set writer: " << curlErrorBuffer); return ""; }

   res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &returnedData);
   if(res != CURLE_OK) { LOG_ERROR("Failed to set write data: " << curlErrorBuffer); return ""; }

   res = curl_easy_perform(curl);
   if(res != CURLE_OK) { 
     LOG_ERROR("Failed to POST data: " << curlErrorBuffer); 
     curl_easy_cleanup(curl);
     curl_formfree(formpost);
     return "";
   }
   curl_easy_cleanup(curl);
   curl_formfree(formpost);

   LOG_DEBUG("Returned data from POST: " << returnedData);
   return returnedData;
}

std::vector<char> AtsHandler::listPeople(std::string dataType, std::string query ) 
{
   LOG_INFO( "list people ");
   // parse the serialized packet
   Json::Value meta;
   int rc = parse_query(query, meta);
   LOG_DEBUG("parse query: "<<rc);

   std::vector<char> nullRetval;
    
   CURL *curl;
   CURLcode res;
 
   res = curl_global_init(CURL_GLOBAL_ALL);

   // if (rc < 0) return std::vector<char>();
   // struct curl_httppost* formpost=NULL;
   // struct curl_httppost* lastptr=NULL;

   curl = curl_easy_init();
   if (curl == 0) return nullRetval;

   res = curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curlErrorBuffer);
   if(res != CURLE_OK) { LOG_ERROR("Failed to set error buffer."); return nullRetval; }

   LOG_DEBUG("url: "+ config->getUrl(RTC_LIST_PEOPLE)+ " who: "+config->getHttpAuth());

   res = curl_easy_setopt(curl, CURLOPT_URL, config->getUrl(RTC_LIST_PEOPLE).c_str());
   if(res != CURLE_OK) { LOG_ERROR("Failed to set URL: " << curlErrorBuffer); return nullRetval; }

   res = curl_easy_setopt(curl, CURLOPT_USERPWD, config->getHttpAuth().c_str());
   if(res != CURLE_OK) { LOG_ERROR("Failed to set user/pass: " << curlErrorBuffer); return nullRetval; }
   LOG_DEBUG("set user pass");

   std::string returnedData = "";
   res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
   if(res != CURLE_OK) { LOG_ERROR("Failed to set writer: " << curlErrorBuffer); return nullRetval; }
   LOG_DEBUG("set write callback");

   res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &returnedData);
   if(res != CURLE_OK) { LOG_ERROR("Failed to set write data: " << curlErrorBuffer); return nullRetval; }
   LOG_DEBUG("set place to put message");

   res = curl_easy_perform(curl);
   if(res != CURLE_OK) { 
     LOG_ERROR("Failed to POST data: " << curlErrorBuffer); 
     curl_easy_cleanup(curl);
     return nullRetval;
   }
   LOG_DEBUG("sent message");

   curl_easy_cleanup(curl);
   // curl_formfree(formpost);

   LOG_DEBUG("Returned data from POST: " << returnedData);
   std::vector<char> retval(returnedData.begin(),returnedData.end());
   return retval;
}

std::vector<char> AtsHandler::listChannels(std::string dataType, std::string query ) 
{
   // parse the serialized packet
   Json::Value meta;
   std::vector<NamedBlob> media;
   int rc = parse_query(query, meta);
   if (rc < 0) {
      return std::vector<char>();
   }

   std::string emptyRetval("");
   std::vector<char> nullRetval(emptyRetval.begin(),emptyRetval.end());
    
   CURL *curl;
   CURLcode res;
 
   res = curl_global_init(CURL_GLOBAL_ALL);

   //struct curl_httppost* formpost=NULL;
   //struct curl_httppost* lastptr=NULL;

   curl = curl_easy_init();
   if (curl == 0) return nullRetval;

   res = curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curlErrorBuffer);
   if(res != CURLE_OK) { LOG_ERROR("Failed to set error buffer."); return nullRetval; }

   LOG_DEBUG("url: "+ config->getUrl(RTC_LIST_CHANNEL)+ " who: "+config->getHttpAuth());

   res = curl_easy_setopt(curl, CURLOPT_URL, config->getUrl(RTC_LIST_CHANNEL).c_str());
   if(res != CURLE_OK) { LOG_ERROR("Failed to set URL: " << curlErrorBuffer); return nullRetval; }

   res = curl_easy_setopt(curl, CURLOPT_USERPWD, config->getHttpAuth().c_str());
   if(res != CURLE_OK) { LOG_ERROR("Failed to set user/pass: " << curlErrorBuffer); return nullRetval; }

   // res = curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
   // if(res != CURLE_OK) { LOG_ERROR("Failed to set post data: " << curlErrorBuffer); return nullRetval; }

   std::string returnedData = "";
   res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
   if(res != CURLE_OK) { LOG_ERROR("Failed to set writer: " << curlErrorBuffer); return nullRetval; }

   res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &returnedData);
   if(res != CURLE_OK) { LOG_ERROR("Failed to set write data: " << curlErrorBuffer); return nullRetval; }

   res = curl_easy_perform(curl);
   if(res != CURLE_OK) { 
     LOG_ERROR("Failed to POST data: " << curlErrorBuffer); 
     curl_easy_cleanup(curl);
     return nullRetval; 
   }

   curl_easy_cleanup(curl);
   //curl_formfree(formpost);

   LOG_DEBUG("Returned data from POST: " << returnedData);
   std::vector<char> retval(returnedData.begin(),returnedData.end());
   return retval;
}


std::string AtsHandler::channelCreate(std::string dataType, std::vector< char > &payload ) 
{
   // parse the serialized packet
   Json::Value meta;
   std::vector<NamedBlob> media;
   int rc = parse_payload(payload, meta, media);
   if (rc < 0) return ""; // the parsing of the payload failed.
    
   CURL *curl;
   CURLcode res;
 
   res = curl_global_init(CURL_GLOBAL_ALL);

   struct curl_httppost* formpost=NULL;
   struct curl_httppost* lastptr=NULL;

   curl = curl_easy_init();
   if (curl == 0) return "";

   res = curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curlErrorBuffer);
   if(res != CURLE_OK) { LOG_ERROR("Failed to set error buffer."); return "";
   }

   LOG_DEBUG("url: "+ config->getUrl(RTC_CREATE_CHANNEL)+ " who: "+config->getHttpAuth());

   res = curl_easy_setopt(curl, CURLOPT_URL, config->getUrl(RTC_CREATE_CHANNEL).c_str());
   if(res != CURLE_OK) { LOG_ERROR("Failed to set URL: " << curlErrorBuffer); return ""; }

   res = curl_easy_setopt(curl, CURLOPT_USERPWD, config->getHttpAuth().c_str());
   if(res != CURLE_OK) { LOG_ERROR("Failed to set user/pass: " << curlErrorBuffer); return ""; }

   //  name=Channel1 description="Test Channel" type=GROUP

   curl_formadd(&formpost, &lastptr,
                CURLFORM_COPYNAME, "name",
                CURLFORM_COPYCONTENTS, meta["name"].asString().c_str(),
                CURLFORM_END);

   curl_formadd(&formpost, &lastptr,
                CURLFORM_COPYNAME, "description",
                CURLFORM_COPYCONTENTS, meta["description"].asString().c_str(),
                CURLFORM_END);

   curl_formadd(&formpost, &lastptr,
                CURLFORM_COPYNAME, "type",
                CURLFORM_COPYCONTENTS, meta["type"].asString().c_str(),
                CURLFORM_END);

   res = curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
   if(res != CURLE_OK) { LOG_ERROR("Failed to set post data: " << curlErrorBuffer); return ""; }

   std::string returnedData = "";
   res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
   if(res != CURLE_OK) { LOG_ERROR("Failed to set writer: " << curlErrorBuffer); return ""; }

   res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &returnedData);
   if(res != CURLE_OK) { LOG_ERROR("Failed to set write data: " << curlErrorBuffer); return ""; }

   res = curl_easy_perform(curl);
   if(res != CURLE_OK) { LOG_ERROR("Failed to POST data: " << curlErrorBuffer); returnedData = ""; }

   curl_easy_cleanup(curl);
   curl_formfree(formpost);

   LOG_DEBUG("Returned data from POST: " << returnedData);

   return returnedData;
}

/**
 String channelName - the name of the channel to post the message to
 Integer lat - lattitude E6
 Integer lon - longitude E6
 Integer zoom - zoom level
*/
std::string AtsHandler::centerMap(std::string dataType, std::vector< char > &query ) 
{
   // parse the serialized packet
   Json::Value meta;
   int rc = parse_query(query, meta);
   if (rc < 0) return ""; // the parsing of the payload failed.
    
   CURL *curl;
   CURLcode res;
 
   res = curl_global_init(CURL_GLOBAL_ALL);

   struct curl_httppost* formpost=NULL;
   struct curl_httppost* lastptr=NULL;

   curl = curl_easy_init();
   if (curl == 0) return "";

   res = curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curlErrorBuffer);
   if(res != CURLE_OK) { LOG_ERROR("Failed to set error buffer."); return "";
   }

   LOG_DEBUG("url: "+ config->getUrl(RTC_CENTER_MAP)+ " who: "+config->getHttpAuth());

   res = curl_easy_setopt(curl, CURLOPT_URL, config->getUrl(RTC_CENTER_MAP).c_str());
   if(res != CURLE_OK) { LOG_ERROR("Failed to set URL: " << curlErrorBuffer); return ""; }

   res = curl_easy_setopt(curl, CURLOPT_USERPWD, config->getHttpAuth().c_str());
   if(res != CURLE_OK) { LOG_ERROR("Failed to set user/pass: " << curlErrorBuffer); return ""; }

   //  name=Channel1 description="Test Channel" type=GROUP

   curl_formadd(&formpost, &lastptr,
                CURLFORM_COPYNAME, "channelName",
                CURLFORM_COPYCONTENTS, meta["channelName"].asString().c_str(),
                CURLFORM_END);

   curl_formadd(&formpost, &lastptr,
                CURLFORM_COPYNAME, "lat",
                CURLFORM_COPYCONTENTS, meta["lat"].asString().c_str(),
                CURLFORM_END);

   curl_formadd(&formpost, &lastptr,
                CURLFORM_COPYNAME, "lon",
                CURLFORM_COPYCONTENTS, meta["lon"].asString().c_str(),
                CURLFORM_END);

   res = curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
   if(res != CURLE_OK) { LOG_ERROR("Failed to set post data: " << curlErrorBuffer); return ""; }

   std::string returnedData = "";
   res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
   if(res != CURLE_OK) { LOG_ERROR("Failed to set writer: " << curlErrorBuffer); return ""; }

   res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &returnedData);
   if(res != CURLE_OK) { LOG_ERROR("Failed to set write data: " << curlErrorBuffer); return ""; }

   res = curl_easy_perform(curl);
   if(res != CURLE_OK) { LOG_ERROR("Failed to POST data: " << curlErrorBuffer); returnedData = ""; }

   curl_easy_cleanup(curl);
   curl_formfree(formpost);

   LOG_DEBUG("Returned data from POST: " << returnedData);

   return returnedData;
}
