
#include <assert.h>

#include <iostream>
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

 // AtsConfigMgr* config = AtsConfigMgr::getInstance();
 // std::string atsAddress = config->getBaseAddress() + config->getBasePath();

 // baseServerAddr = new char[atsAddress.size() + 1];
 // strncpy(baseServerAddr, atsAddress.c_str(), atsAddress.size() + 1);
}


void AtsHandler::onConnect(GatewayConnector *sender) { }

void AtsHandler::onDisconnect(GatewayConnector *sender) { }

/**
* In response to push data (DataPushReceiverListener)
* i.e. in response to subscribing
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

  if (dataType == RTC_CHANNEL_MEDIA_UPLOAD_NS) {
     std::string result = uploadMedia(dataType, payload);
     std::cout << " Push " << dataType << " result: " << result << std::endl;
     return;
  }
  if (dataType == RTC_INVITE_NS) {
     std::string result = inviteChat(dataType, payload);
     std::cout << " Push "<< dataType << " result: " << result << std::endl;
     return;
  }
  if (dataType == RTC_CHANNEL_CREATE_NS) {
     std::string data = channelCreate(dataType, payload);
     sender->pushData(uri, dataType, data);
     std::cout << " Push " << dataType << " result: " << data.substr(0,128) << std::endl;
     return;
  }
  if (dataType == RTC_SHARE_GPS_NS) {
  }
}
 

  // PullRequestReceiverListener methods
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
  std::cout << "Got pull request data." << std::endl;
  std::cout << "  ReqId: " << requestUid << std::endl;
  std::cout << "  Plugin: " << pluginId << std::endl;
  std::cout << "  Data type: " << dataType << std::endl;
  std::cout << "  Query: " << query << std::endl;
  std::cout << "  Projection: " << projection << std::endl;
  std::cout << "  Start Count: " << startFromCount << std::endl;
  std::cout << "  Live: " << liveQuery << std::endl;
  if (dataType == RTC_PEOPLE_LIST_NS) {
     std::vector<char> data = listPeople(dataType, query);
     sender->pullResponse(requestUid, pluginId, dataType, query, data);
     // std::cout << " Pull " << dataType << " result: " << data.substr(0,128) << std::endl;
     return;
  }
  if (dataType == RTC_PEOPLE_LIST_NS) {
     std::vector<char> data = listChannels(dataType, query);
     sender->pullResponse(requestUid, pluginId, dataType, query, data);
     // std::cout << " Pull " << dataType << " result: " << data.substr(0,128) << std::endl;
     return;
  }
  // setRegisterPullInterest(gwc, RTC_CHANNEL_ACTIVATE_NS, dataHandler);
  // setRegisterPullInterest(gwc, RTC_CHANNEL_PASSIVATE_NS, dataHandler);
  // setRegisterPullInterest(gwc, RTC_CHANNEL_MEDIA_LIST_NS, dataHandler);
  // setRegisterPullInterest(gwc, RTC_CHANNEL_MEDIA_FILE_NS, dataHandler);
  // setRegisterPullInterest(gwc, RTC_CHANNEL_MEDIA_CLIP_NS, dataHandler);
  // setRegisterPullInterest(gwc, RTC_CHANNEL_AGGREGATE_NS, dataHandler);
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
    // get the name of the blob 
    end = std::find(begin, payload.end(), 0);
    // check the end value for success
    if (end != payload.end()) break;
    std::string blobName(begin, end);

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
static int parse_query(std::string query, Json::Value& meta) {
   std::vector< char > payload(query.begin(), query.end());
   std::vector< NamedBlob > media;
   parse_payload(payload, meta, media);
   return 0;
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

   res = curl_easy_setopt(curl, CURLOPT_URL, config->getPath(RTC_CHANNEL_MEDIA_UPLOAD_NS).c_str());
   if(res != CURLE_OK) { LOG_ERROR("Failed to set URL: " << curlErrorBuffer); return ""; }

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

   res = curl_easy_setopt(curl, CURLOPT_URL, config->getPath(RTC_CHANNEL_MEDIA_UPLOAD_NS).c_str());
   if(res != CURLE_OK) { LOG_ERROR("Failed to set URL: " << curlErrorBuffer); return ""; }

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

std::vector<char> AtsHandler::listPeople(std::string dataType, std::string query ) 
{
   // parse the serialized packet
   Json::Value meta;
   int rc = parse_query(query, meta);
   std::vector<char> nullRetval;
    
   CURL *curl;
   CURLcode res;
 
   res = curl_global_init(CURL_GLOBAL_ALL);

   // struct curl_httppost* formpost=NULL;
   struct curl_httppost* lastptr=NULL;

   curl = curl_easy_init();
   if (curl == 0) return nullRetval;

   res = curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curlErrorBuffer);
   if(res != CURLE_OK) { LOG_ERROR("Failed to set error buffer."); return nullRetval;
   }

   res = curl_easy_setopt(curl, CURLOPT_URL, config->getPath(RTC_PEOPLE_LIST_NS).c_str());
   if(res != CURLE_OK) { LOG_ERROR("Failed to set URL: " << curlErrorBuffer); return nullRetval; }

   // res = curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
   // if(res != CURLE_OK) { LOG_ERROR("Failed to set post data: " << curlErrorBuffer); return nullRetval; }

   std::string returnedData = "";
   res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
   if(res != CURLE_OK) { LOG_ERROR("Failed to set writer: " << curlErrorBuffer); return nullRetval;
   }

   res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &returnedData);
   if(res != CURLE_OK) { LOG_ERROR("Failed to set write data: " << curlErrorBuffer); return nullRetval; }

   res = curl_easy_perform(curl);
   if(res != CURLE_OK) { 
     LOG_ERROR("Failed to POST data: " << curlErrorBuffer); 
     curl_easy_cleanup(curl);
     return nullRetval;
   }

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
   std::string emptyRetval("");
   std::vector<char> nullRetval(emptyRetval.begin(),emptyRetval.end());
    
   CURL *curl;
   CURLcode res;
 
   res = curl_global_init(CURL_GLOBAL_ALL);

   //struct curl_httppost* formpost=NULL;
   struct curl_httppost* lastptr=NULL;

   curl = curl_easy_init();
   if (curl == 0) return nullRetval;

   res = curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curlErrorBuffer);
   if(res != CURLE_OK) { LOG_ERROR("Failed to set error buffer."); return nullRetval;
   }

   res = curl_easy_setopt(curl, CURLOPT_URL, config->getPath(RTC_PEOPLE_LIST_NS).c_str());
   if(res != CURLE_OK) { LOG_ERROR("Failed to set URL: " << curlErrorBuffer); return nullRetval; }

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

   res = curl_easy_setopt(curl, CURLOPT_URL, config->getPath(RTC_PEOPLE_LIST_NS).c_str());
   if(res != CURLE_OK) { LOG_ERROR("Failed to set URL: " << curlErrorBuffer); return ""; }

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
