#include "log.h"

#include "json/reader.h"
#include "json/value.h"

#include "PassGatewayReceiver.h"
#include "PassConfigurationManager.h"

#include "soap/soapPASSPortBindingProxy.h"

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
  
  // Hard-coded for now, maybe later from the config file or elsewhere.
  item_data.item_USCOREdata_USCOREclassification =
    _pass__item_USCOREdata_item_USCOREdata_USCOREclassification__UNCLASSIFIED;
    
  Json::Value root;
  Json::Reader reader;
  
  if (!reader.parse (pushData.data, root))
    {
      LOG_ERROR ("JSON parsing error: "
                 << reader.getFormatedErrorMessages ());
                 
      return;
    }
    
  soap_dom_element position_report (0, "", "PositionReport");

  soap_dom_element unit (0, "", "Unit");
  
  // ID, Name, URN, SymbolCode and Service are all children of Unit.
  soap_dom_element id (0, "", "ID", root["userid"].asCString ());
  unit.add (id);
  
  soap_dom_element name (0, "", "Name", root["name"].asCString ());
  unit.add (name);
  
  soap_dom_element urn (0, "", "URN", "");
  unit.add (urn);
  
  soap_dom_element symbol_code (0, "", "SymbolCode", "");
  unit.add (symbol_code);
  
  soap_dom_element service (0, "", "Service", "");
  unit.add (service);
  
  position_report.add (unit);
  
  soap_dom_element latitude (0, "", "Latitude", root["lat"].asCString ());
  position_report.add (latitude);
  
  soap_dom_element longitude (0, "", "Longitude", root["lon"].asCString ());
  position_report.add (longitude);
  
  soap_dom_element report_date (0, "", "ReportDate", root["created"].asCString ());
  position_report.add (report_date);
  
  soap_dom_element elevation (0, "", "Elevation", "0.0");
  position_report.add (elevation);
  
  item_data.__any.add (position_report);

  pass__item item;
  
  // Generate a timestamp in the format yyyy-mm-ddThh-mm-ss.usecZ
  timeval tv;
  tm *timeinfo;
  const size_t len = 30;
  char fmt[len];
  char buf[len];
  gettimeofday (&tv, 0);
  timeinfo = localtime (&tv.tv_sec);
  strftime (fmt, len, "%Y-%m-%dT%H:%M:%S.%%06uZ", timeinfo);
  snprintf (buf, len, fmt, tv.tv_usec);
  
  // Message id = plugin id + "__" + timestamp
  std::string item_id (cfg_mgr_->getPassPluginId ());
  item_id += "__";
  item_id += buf;
  
  item.item_USCOREid = item_id;
  
  item.item_USCOREdtg = time (0);
  item.item_USCOREdata = &item_data;
  item.item_USCORETTL_USCOREsecs = "1000";
  
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
      proxy.soap_print_fault (stderr);
      LOG_DEBUG ("Published error status: " << status.status_USCOREcode);
      
      if (status.status_USCOREphrase != 0)
        {
          LOG_DEBUG ("   (" << *(status.status_USCOREphrase) << ")");
        }
    }
  else
    {
      LOG_DEBUG ("Published: " << status.status_USCOREcode);
      
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



