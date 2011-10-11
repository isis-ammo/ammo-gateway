#include "log.h"
#include <sstream>

#include "json/reader.h"
#include "json/value.h"

#include "PassGatewayReceiver.h"
#include "PassConfigurationManager.h"

#include "soap/soapPASSPortBindingProxy.h"

using namespace std;
using namespace ammo::gateway;

PassGatewayReceiver::PassGatewayReceiver (void)
  : cfg_mgr_ (0)
{
}

PassGatewayReceiver::~PassGatewayReceiver (void)
{
}

void
PassGatewayReceiver::onConnect (GatewayConnector * /* sender */)
{
}

void
PassGatewayReceiver::onDisconnect (GatewayConnector * /* sender */)
{
}

void
PassGatewayReceiver::onPushDataReceived (GatewayConnector * /* sender */,
                                         PushData &pushData)
{
  LOG_TRACE ("PASS data push received");

  _pass__publish pass_publish;

  pass_publish.topic = cfg_mgr_->getPassTopic ();
  
  pass_publish.publisher_USCOREid = cfg_mgr_->getPassPublisherId ();
  
  pass__item_USCOREdata item_data;
  
  // Hard-coded for now, maybe later set from the config file.
  item_data.item_USCOREdata_USCOREclassification =
    _pass__item_USCOREdata_item_USCOREdata_USCOREclassification__UNCLASSIFIED;
    
  Json::Value root;
  Json::Reader reader;

  LOG_DEBUG ("JSON data: " << pushData.data);
  
  if (!reader.parse (pushData.data, root))
    {
      LOG_ERROR ("JSON parsing error: "
                 << reader.getFormatedErrorMessages ());
                 
      return;
    }
    
  if (root["lid"].isString ())
    {
      // A match here means we already pushed this data to the gateway.
      if (root["lid"].asString () == cfg_mgr_->getPassPluginSubscriberTag ())
        {
          LOG_DEBUG ("onPushDataReceived: skipping message that was a "
                     "previous gateway push " << root["lid"].asString ());
          return;
        }
    }
    
  soap_dom_element position_report (0, "", "PositionReport");

  soap_dom_element unit (0, "", "Unit");
  
  // Are there values in the config file for ID, URN, SymbolCode
  // and Service matched with the location message's 'name' field?
  // If so, this call will store them for access below.
  bool unit_config = cfg_mgr_->checkName (root["name"].asString ());
  
  // ID, Name, URN, SymbolCode and Service are all children of Unit.
  
  std::string id_str =
    (unit_config ? cfg_mgr_->getId () : root["userid"].asString ());
  soap_dom_element id (0, "", "ID", id_str.c_str ());
  unit.add (id);
  
  soap_dom_element name (0, "", "Name", root["name"].asCString ());
  unit.add (name);
  
  std::string urn_str =
    (unit_config ? cfg_mgr_->getUrn () : root["userid"].asString ());
  soap_dom_element urn (0, "", "URN", urn_str.c_str ());
  unit.add (urn);
  
  std::string sc_str =
    (unit_config ? cfg_mgr_->getSymbolCode () : cfg_mgr_->getDefaultSymbolCode ());
  soap_dom_element symbol_code (0, "", "SymbolCode", sc_str.c_str ());
  unit.add (symbol_code);
  
  std::string sv_str =
    (unit_config ? cfg_mgr_->getService () : cfg_mgr_->getDefaultService ());
  soap_dom_element service (0, "", "Service", sv_str.c_str ());
  unit.add (service);
  
  LOG_DEBUG ("ID = " << id_str);
  LOG_DEBUG ("URN = " << urn_str);
  LOG_DEBUG ("SymbolCode = " << sc_str);
  LOG_DEBUG ("Service = " << sv_str);
  
  position_report.add (unit);

  const char *lat = root["lat"].asCString ();
  const char *lon = root["lon"].asCString ();
  double llat = (double) strtol (lat, 0, 10) / 1.0e6;
  double llon = (double) strtol (lon, 0, 10) / 1.0e6;

  ostringstream lats, lons;
  lats << llat;
  lons << llon;

  soap_dom_element latitude (0, "", "Latitude", lats.str ().c_str ());
  position_report.add (latitude);
  
  soap_dom_element longitude (0, "", "Longitude", lons.str ().c_str ());
  position_report.add (longitude);
  
  const char *reportDate = root["created"].asCString ();
  long long t = strtoll (reportDate, 0, 10);
  time_t rptTime = t / 1000ll;

  const size_t len = 30;
  char fmt[len];
  strftime (fmt, len, "%Y-%m-%dT%H:%M:%SZ", gmtime (&rptTime));  

  soap_dom_element report_date (0, "", "ReportDate", fmt);
  position_report.add (report_date);
  
  soap_dom_element elevation (0, "", "Elevation", "0.0");
  position_report.add (elevation);
  
  item_data.__any.add (position_report);

  pass__item item;
  
  // Generate a timestamp in the format yyyy-mm-ddThh-mm-ss.usecZ
  // timeval tv;
  // tm *timeinfo;
  // gettimeofday (&tv, 0);
  // timeinfo = localtime (&tv.tv_sec);
  // strftime (fmt, len, "%Y-%m-%dT%H:%M:%S.%%06uZ", timeinfo);
  // snprintf (buf, len, fmt, tv.tv_usec);
  
  // Message id = plugin id + "__" + timestamp
  std::string item_id (cfg_mgr_->getPassPluginId ());
  item_id += "__";
  item_id += fmt;		// use the timestamp from the message as item id.
  
  item.item_USCOREid = item_id;
  
  item.item_USCOREdtg = time (0);
  item.item_USCOREdata = &item_data;
  item.item_USCORETTL_USCOREsecs = "300"; // 5 min
  
  pass_publish.item.push_back (&item);
  
  pass__status status;
  
  LOG_DEBUG ("Publishing...");
  
  PASSPortBindingProxy proxy (SOAP_XML_DEFAULTNS, SOAP_XML_DEFAULTNS);
  
  // At least until we have a valid SSL certificate on the server.
  proxy.ssl_flags = SOAP_SSL_SKIP_HOST_CHECK;
  
  proxy.soap_endpoint = cfg_mgr_->getPassServerAddress ().c_str ();
  
  int result = proxy.publish (&pass_publish, &status);
  
  if (result != SOAP_OK)
    {
      LOG_DEBUG ("Publish failed - SOAP error code = " << result);
      proxy.soap_print_fault (stderr);
      LOG_DEBUG ("Published error status: " << status.status_USCOREcode);
      
      if (status.status_USCOREphrase != 0)
        {
          LOG_DEBUG ("   (" << *(status.status_USCOREphrase) << ")");
        }
    }
  else
    {
      LOG_DEBUG ("Publish succeeded");
      
      if (status.status_USCOREphrase != 0)
        {
          LOG_DEBUG ("   (" << *(status.status_USCOREphrase) << ")");
        }
    }
}

void
PassGatewayReceiver::set_cfg_mgr (PassConfigurationManager * const cfg_mgr)
{
  cfg_mgr_ = cfg_mgr;
}



