#include "LocationStore.h"

#include <string>

#include "ace/ACE.h"
#include "ace/OS_NS_string.h"

LocationStoreReceiver::LocationStoreReceiver (void)
  : filename_ ("loc_file_")
{
  ACE_TCHAR day_and_time[35];
  ACE::timestamp (day_and_time, sizeof day_and_time / sizeof (ACE_TCHAR));
  filename_ += day_and_time;
	
  out_.open (filename_.c_str ());
}

void LocationStoreReceiver::onConnect (GatewayConnector *sender)
{
}

void LocationStoreReceiver::onDisconnect (GatewayConnector *sender)
{
}

void LocationStoreReceiver::onDataReceived (GatewayConnector * /* sender */,
										    std::string uri,
										    std::string mimeType,
										    std::vector<char> & /* data */)
{
  out_ << uri << endl
	   << mimeType << endl;
	
  cout << "Got data." << endl;
  cout << "  URI: " << uri << endl;
  cout << "  Mime type: " << mimeType << endl;
}
