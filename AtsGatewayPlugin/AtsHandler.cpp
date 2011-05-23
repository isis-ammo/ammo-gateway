
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
   CURLcode res = curl_global_init(CURL_GLOBAL_ALL);
   if(res != CURLE_OK) { 
     LOG_ERROR("Failed to configure curl.");
     return ;
   }
}

std::string IntToStr(int tmp)
{
    std::ostringstream out;
    out << tmp;
    return out.str();
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
   CURLcode res; 
   LOG_INFO( "Got push data.");
   LOG_INFO( "  URI: " << uri);
   LOG_INFO( "  Data type: " << dataType);
   std::vector<char>::iterator endIt =  (payload.size() < 128) ? payload.end() : payload.begin()+128;
   LOG_INFO( "  Data: " << std::string(payload.begin(), endIt));
   LOG_INFO( "  Origin User Name: " << originUsername);
   LOG_DEBUG("  User: " << config->getUsername(originUsername));

   CURL *curl = curl_easy_init();
   if (curl == 0) { LOG_ERROR("Failed to initialize curl."); return; }

   res = curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curlErrorBuffer);
   if(res != CURLE_OK) { LOG_ERROR("Failed to set error buffer."); return; }

   res = curl_easy_setopt(curl, CURLOPT_USERPWD, config->getHttpAuth(originUsername).c_str());
   if(res != CURLE_OK) { LOG_ERROR("Failed to set user/pass: " << curlErrorBuffer); return; }

   if (dataType == PLI_POST_LOC_NS) {
     std::string result = postLocation(curl, dataType, payload);
     LOG_INFO(" Push " << dataType << " result: " << result);
     return;
   }
   if (dataType == PLI_POST_LOCS_NS) {
     std::string result = postLocations(curl, dataType, payload);
     LOG_INFO(" Push " << dataType << " result: " << result);
     return;
   }
   if (dataType == RTC_UPLOAD_CHANNEL_MEDIA_NS) {
      std::string result = uploadMedia(curl, dataType, payload);
      LOG_INFO(" Push " << dataType << " result: " << result);
      return;
   }
   if (dataType == RTC_INVITE_NS) {
      std::string result = inviteChat(curl, dataType, payload);
      LOG_INFO(" Push "<< dataType << " result: " << result);
      return;
   }
   if (dataType == RTC_CREATE_CHANNEL_NS) {
      std::string data = channelCreate(curl, dataType, payload);
      sender->pushData(uri, dataType, data);
      LOG_INFO(" Push " << dataType << " result: " << data.substr(0,128));
      return;
   }
   if (dataType == RTC_SHARE_GPS_NS) {
   }
       if (dataType == PLI_POST_LOC_NS)  {
               LOG_INFO("POSTING LOCATION:");
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
   CURLcode res; 
   LOG_INFO("Got pull request data.");
   LOG_INFO( "  ReqId: " << requestUid);
   LOG_INFO( "  Plugin: " << pluginId);
   LOG_INFO( "  Data type: " << dataType);
   LOG_INFO( "  Query: " << query);
   LOG_INFO( "  Projection: " << projection);
   LOG_INFO( "  Start Count: " << startFromCount);
   LOG_INFO( "  Live: " << liveQuery);

   CURL *curl = curl_easy_init();
   if (curl == 0) { LOG_ERROR("Failed to initialize curl."); return; }

   res = curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curlErrorBuffer);
   if(res != CURLE_OK) { LOG_ERROR("Failed to set error buffer."); return; }

   res = curl_easy_setopt(curl, CURLOPT_USERPWD, config->getHttpAuth().c_str());
   if(res != CURLE_OK) { LOG_ERROR("Failed to set user/pass: " << curlErrorBuffer); return; }

   if (dataType == RTC_LIST_PEOPLE_NS) {
      std::vector<char> data = listPeople(curl, dataType, query);
      LOG_INFO( "pull " << dataType << " result: " << std::string(data.begin(), data.begin()+128));
      sender->pullResponse(requestUid, pluginId, dataType, query, data);
      LOG_INFO( "send response: " << requestUid);
      return;
   }
   if (dataType == RTC_LIST_CHANNEL_NS) {
      std::vector<char> data = listChannels(curl, dataType, query);
      sender->pullResponse(requestUid, pluginId, dataType, query, data);
      LOG_INFO(" Pull " << dataType << " result: " << std::string(data.begin(), data.begin()+128));
      return;
   }
   if (dataType == PLI_LIST_LOC_NS){
               std::vector<char> data = listLocations(curl, dataType, query);
               LOG_INFO(" Pull " << dataType << " result: " << std::string(data.begin(), data.begin()+128));
      sender->pullResponse(requestUid, pluginId, dataType, query, data);
      LOG_INFO(" Pull " << dataType << " result: " << std::string(data.begin(), data.begin()+128));
      return;
       }
       if (dataType == PLI_LIST_UNIT_NS){
               std::vector<char> data = listUnits(curl, dataType, query);
      sender->pullResponse(requestUid, pluginId, dataType, query, data);
      LOG_DEBUG(" Pull " << dataType << " result: " << std::string(data.begin(), data.begin()+128));
      return;
       }
       if (dataType == PLI_MEMBERS_NS){
               std::vector<char> data = listMembers(curl, dataType, query);
      sender->pullResponse(requestUid, pluginId, dataType, query, data);
      LOG_INFO(" Pull " << dataType << " result: " << std::string(data.begin(), data.begin()+128));
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
   CURLcode res; 
   LOG_INFO( "Got pull response data." );
   LOG_INFO( "  ReqId: " << requestUid );
   LOG_INFO( "  Plugin: " << pluginId );
   LOG_INFO( "  Data type: " << dataType );
   LOG_INFO( "  Uri: " << uri );
   LOG_INFO( "  Data: " << std::string(data.begin(), data.end()) );

   CURL *curl = curl_easy_init();
   if (curl == 0) { LOG_ERROR("Failed to initialize curl."); return; }

   res = curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curlErrorBuffer);
   if(res != CURLE_OK) { LOG_ERROR("Failed to set error buffer."); return; }

   res = curl_easy_setopt(curl, CURLOPT_USERPWD, config->getHttpAuth().c_str());
   if(res != CURLE_OK) { LOG_ERROR("Failed to set user/pass: " << curlErrorBuffer); return; }

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
     if (end == payload.end()){
               break;
               }
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
std::string AtsHandler::uploadMedia(CURL *curl, std::string mediaType, std::vector< char > &payload ) 
{
   CURLcode res;
   // parse the serialized packet
   Json::Value meta;
   std::vector<NamedBlob> media;
   int rc = parse_payload(payload, meta, media);
   if (rc < 0) return ""; // the parsing of the payload failed.
    
   std::string url = config->getUrl(RTC_UPLOAD_CHANNEL_MEDIA);
   LOG_DEBUG("url: "+ url);
   res = curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
   if(res != CURLE_OK) { LOG_ERROR("Failed to set URL: " << curlErrorBuffer); return ""; }

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

std::string AtsHandler::inviteChat(CURL *curl, std::string mediaType, std::vector< char > &payload ) 
{
   CURLcode res;
   // parse the serialized packet
   Json::Value meta;
   std::vector<NamedBlob> media;
   int rc = parse_payload(payload, meta, media);
   if (rc < 0) return "";
    
   std::string url = config->getUrl(RTC_INVITE);
   LOG_DEBUG("url: "+ url);
   res = curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
   if(res != CURLE_OK) { LOG_ERROR("Failed to set URL: " << curlErrorBuffer); return ""; }

   struct curl_httppost* formpost=NULL;
   struct curl_httppost* lastptr=NULL;

   curl_formadd(&formpost, &lastptr,
                CURLFORM_COPYNAME, "displayName",
                CURLFORM_COPYCONTENTS, meta["displayName"].asString().c_str(),
                CURLFORM_END);

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

std::vector<char> AtsHandler::listPeople(CURL *curl, std::string dataType, std::string query ) 
{
   CURLcode res;
   // if (rc < 0) return std::vector<char>();
   LOG_INFO( "list people ");
   // parse the serialized packet
   Json::Value meta;
   int rc = parse_query(query, meta);
   LOG_DEBUG("parse query: "<<rc);

   std::vector<char> nullRetval;
    
   // struct curl_httppost* formpost=NULL;
   // struct curl_httppost* lastptr=NULL;

   std::string url = config->getUrl(RTC_LIST_PEOPLE);
   LOG_DEBUG("url: "+ url);
   res = curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
   if(res != CURLE_OK) { LOG_ERROR("Failed to set URL: " << curlErrorBuffer); return nullRetval; }

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

std::vector<char> AtsHandler::listLocations(CURL *curl, std::string dataType, std::string query ) 
{
   CURLcode res;
   LOG_DEBUG( "list locations ");
   // parse the serialized packet
   Json::Value meta;
   int rc = parse_query(query, meta);
   LOG_DEBUG("parse query: "<<rc);
       LOG_DEBUG("query: " << query);
       LOG_DEBUG("meta: "  << meta.toStyledString());

   std::vector<char> nullRetval;
    
   struct curl_httppost* formpost=NULL;
   struct curl_httppost* lastptr=NULL;

       std::string unitid = IntToStr(meta["unitId"].asInt());

   std::string url = config->getUrl(PLI_LIST_LOC);
   LOG_DEBUG("url: "+ url);
   res = curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
   if(res != CURLE_OK) { LOG_ERROR("Failed to set URL: " << curlErrorBuffer); return nullRetval; }

       curl_formadd(&formpost, &lastptr,
                CURLFORM_COPYNAME, "unitId",
               CURLFORM_COPYCONTENTS, unitid.c_str(),
                CURLFORM_END);

       res = curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
   if(res != CURLE_OK) { LOG_ERROR("Failed to set post data: " << curlErrorBuffer); return nullRetval; }
  
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
   curl_formfree(formpost);

   LOG_DEBUG("Returned data from POST: " << returnedData);
   std::vector<char> retval(returnedData.begin(),returnedData.end());
   return retval;
}


std::vector<char> AtsHandler::listMembers(CURL *curl, std::string dataType, std::string query ) 
{
   CURLcode res;
 
   LOG_INFO( "list members ");
   // parse the serialized packet
   Json::Value meta;
   int rc = parse_query(query, meta);
       
   LOG_DEBUG("parse query: "<<rc);
       LOG_DEBUG("query: " << query);
       LOG_DEBUG("meta: "  << meta.toStyledString());

   std::vector<char> nullRetval;
    
   struct curl_httppost* formpost=NULL;
   struct curl_httppost* lastptr=NULL;

   std::string url = config->getUrl(PLI_MEMBERS);
   LOG_DEBUG("url: "+ url);
   res = curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
   if(res != CURLE_OK) { LOG_ERROR("Failed to set URL: " << curlErrorBuffer); return nullRetval; }

  
       std::string unitid = IntToStr(meta["unitId"].asInt());

       

       LOG_DEBUG ("META[UNITID]:" <<  unitid);


       curl_formadd(&formpost, &lastptr,
                CURLFORM_COPYNAME, "unitId",
               CURLFORM_COPYCONTENTS, unitid.c_str(),
                CURLFORM_END);

  

   res = curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
   if(res != CURLE_OK) { LOG_ERROR("Failed to set post data: " << curlErrorBuffer); return nullRetval; }


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
   curl_formfree(formpost);

   LOG_DEBUG("Returned data from POST: " << returnedData);
   std::vector<char> retval(returnedData.begin(),returnedData.end());
   return retval;
}
std::vector<char> AtsHandler::listUnits(CURL *curl, std::string dataType, std::string query ) 
{
   CURLcode res;
   std::string returnedData = "";
 
   LOG_DEBUG( "list units ");
   // parse the serialized packet
   Json::Value meta;
   int rc = parse_query(query, meta);
   LOG_DEBUG("parse query: "<<rc);

   std::vector<char> nullRetval;
    
   // struct curl_httppost* formpost=NULL;
   // struct curl_httppost* lastptr=NULL;

   std::string url = config->getUrl(PLI_LIST_UNIT);
   LOG_DEBUG("url: "+ url);
   res = curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
   if(res != CURLE_OK) { LOG_ERROR("Failed to set URL: " << curlErrorBuffer); return nullRetval; }

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

std::vector<char> AtsHandler::listChannels(CURL *curl, std::string dataType, std::string query ) 
{
   CURLcode res;
   // parse the serialized packet
   Json::Value meta;
   std::vector<NamedBlob> media;
   int rc = parse_query(query, meta);
   //if (rc < 0) {
   //   return std::vector<char>();
   //}

   std::string emptyRetval("");
   std::vector<char> nullRetval(emptyRetval.begin(),emptyRetval.end());
    
   //struct curl_httppost* formpost=NULL;
   //struct curl_httppost* lastptr=NULL;

   std::string url = config->getUrl(RTC_LIST_CHANNEL);
   LOG_DEBUG("url: "+ url);
   res = curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
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


std::string AtsHandler::channelCreate(CURL *curl, std::string dataType, std::vector< char > &payload ) 
{
   CURLcode res;
   // parse the serialized packet
   Json::Value meta;
   std::vector<NamedBlob> media;
   int rc = parse_payload(payload, meta, media);
   if (rc < 0) return ""; // the parsing of the payload failed.
    
   struct curl_httppost* formpost=NULL;
   struct curl_httppost* lastptr=NULL;

   std::string url = config->getUrl(RTC_CREATE_CHANNEL);
   LOG_DEBUG("url: "+ url);
   res = curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
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

/**
 String channelName - the name of the channel to post the message to
 Integer lat - lattitude E6
 Integer lon - longitude E6
 Integer zoom - zoom level
*/
std::string AtsHandler::centerMap(CURL *curl, std::string dataType, std::vector< char > &query ) 
{
   CURLcode res;
   // parse the serialized packet
   Json::Value meta;
   int rc = parse_query(query, meta);
   if (rc < 0) return ""; // the parsing of the payload failed.
    
   struct curl_httppost* formpost=NULL;
   struct curl_httppost* lastptr=NULL;

   std::string url = config->getUrl(RTC_CENTER_MAP);
   LOG_DEBUG("url: "+ url);
   res = curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
   if(res != CURLE_OK) { LOG_ERROR("Failed to set URL: " << curlErrorBuffer); return ""; }

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

size_t print_httppost_callback(void *arg, const char *buf, size_t len)
{
  fwrite(buf, len, 1, stdout);
  (*(size_t *) arg) += len;
  return len;
}



std::string AtsHandler::postLocation(CURL *curl, std::string dataType, std::vector< char > &payload ) 
{
   CURLcode res;
   // parse the serialized packet
   Json::Value meta;
   int rc = parse_query(payload, meta);
   if (rc < 0) return ""; // the parsing of the payload failed.
    
   struct curl_httppost* formpost=NULL;
   struct curl_httppost* lastptr=NULL;

   std::string url = config->getUrl(PLI_POST_LOC);
   LOG_DEBUG("url: "+ url);
   res = curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
   if(res != CURLE_OK) { LOG_ERROR("Failed to set URL: " << curlErrorBuffer); return ""; }

   std::ostringstream lat, lon;
   lat << meta["lat"].asInt();
   lon << meta["lon"].asInt();

   curl_formadd(&formpost, &lastptr,
                CURLFORM_COPYNAME, "lat",
                CURLFORM_COPYCONTENTS, lat.str().c_str(),
                CURLFORM_END);

   curl_formadd(&formpost, &lastptr,
                CURLFORM_COPYNAME, "lon",
                CURLFORM_COPYCONTENTS, lon.str().c_str(),
                CURLFORM_END);

   res = curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
   if(res != CURLE_OK) { LOG_ERROR("Failed to set post data: " << curlErrorBuffer); return ""; }

   std::string returnedData = "";
   res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
   if(res != CURLE_OK) { LOG_ERROR("Failed to set writer: " << curlErrorBuffer); return ""; }

   res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &returnedData);
   if(res != CURLE_OK) { LOG_ERROR("Failed to set write data: " << curlErrorBuffer); return ""; }

   res = curl_easy_setopt(curl, CURLOPT_VERBOSE, true);

   // DEBUG: uncomment to check the contents of the FORM post
   // size_t total_size = 0;
   // curl_formget(formpost, &total_size, print_httppost_callback);

   res = curl_easy_perform(curl);
   if(res != CURLE_OK) { LOG_ERROR("Failed to POST data: " << curlErrorBuffer); returnedData = ""; }

   curl_easy_cleanup(curl);
   curl_formfree(formpost);

   LOG_DEBUG("Returned data from POST: " << returnedData);

   return returnedData;
}


std::string AtsHandler::postLocations(CURL *curl, std::string dataType, std::vector< char > &payload ) 
{
   CURLcode res;
   // no need to parse payload - it is already a JSON buffer
   
    
   struct curl_httppost* formpost=NULL;
   struct curl_httppost* lastptr=NULL;

   std::string url = config->getUrl(PLI_POST_LOCS);
   LOG_DEBUG("url: "+ url);
   res = curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
   if(res != CURLE_OK) { LOG_ERROR("Failed to set URL: " << curlErrorBuffer); return ""; }

   curl_formadd(&formpost, &lastptr,
                CURLFORM_COPYNAME, "json",
                CURLFORM_COPYCONTENTS, &payload[0], // conversion from std::vector<char> to char *
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
