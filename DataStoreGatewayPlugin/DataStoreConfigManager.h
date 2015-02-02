/* Copyright (c) 2010-2015 Vanderbilt University
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef DATASTORE_CONFIG_MANAGER_H
#define DATASTORE_CONFIG_MANAGER_H

#include "json/reader.h"
#include "json/value.h"

class DataStoreReceiver;

namespace ammo
{
  namespace gateway
  {
    class GatewayConnector;
  }
}

class DataStoreConfigManager
{
public:
  static
  DataStoreConfigManager *getInstance (
	  DataStoreReceiver *receiver = 0,
	  ammo::gateway::GatewayConnector *connector = 0);
	  
	const std::string &getEventMimeType (void) const;
	void setEventMimeType (const std::string &val);
	
	const std::string &getMediaMimeType (void) const;
	void setMediaMimeType (const std::string &val);
	
	const std::string &getSMSMimeType (void) const;
	void setSMSMimeType (const std::string &val);
	
	const std::string &getReportMimeType (void) const;
	void setReportMimeType (const std::string &val);
	
	const std::string &getLocationsMimeType (void) const;
	void setLocationsMimeType (const std::string &val);
	
	const std::string &getPrivateContactsMimeType (void) const;
	void setPrivateContactsMimeType (const std::string &val);
	
	const std::string &getChatMimeType (void) const;
	void setChatMimeType (const std::string &val);
	
	const std::string &getChatMediaMimeType (void) const;
	void setChatMediaMimeType (const std::string &val);
	
private:
  DataStoreConfigManager (
	  DataStoreReceiver *receiver,
	  ammo::gateway::GatewayConnector *connector);
	
  std::string findConfigFile (void);
	
  static DataStoreConfigManager *sharedInstance;
	
private:
  Json::Value root_;
	
  DataStoreReceiver *receiver_;
  ammo::gateway::GatewayConnector *connector_;
  
  std::string event_mime_type_;
  std::string media_mime_type_;
  std::string sms_mime_type_;
  std::string report_mime_type_;
  std::string locations_mime_type_;
  std::string private_contacts_mime_type_;
  std::string chat_mime_type_;
  std::string chat_media_mime_type_;
};

#endif // DATASTORE_CONFIG_MANAGER_H
