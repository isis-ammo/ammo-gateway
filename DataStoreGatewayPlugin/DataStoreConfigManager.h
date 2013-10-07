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
