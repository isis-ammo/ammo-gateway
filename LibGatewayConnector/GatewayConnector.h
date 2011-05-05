/**
* @mainpage
* 
* This library provides the interface from a gateway plugin to the core gateway.
* Client plugins should create at least one GatewayConnector object, then
* subclass GatewayConnectorDelegate and any listener classes (i.e. 
* DataPushReceiverListener) that they need.
*/

#ifndef GATEWAY_CONNECTOR_H
#define GATEWAY_CONNECTOR_H

#include <string>
#include <vector>
#include <map>
#include "ace/Connector.h"
#include "ace/SOCK_Connector.h"

#include "LibGatewayConnector_Export.h"

#include "GatewayConfigurationManager.h"
#include "GatewayServiceHandler.h"

#include "Enumerations.h"

namespace ammo {
  namespace gateway {
    class GatewayConnectorDelegate;
    class DataPushReceiverListener;
    class PullRequestReceiverListener;
    class PullResponseReceiverListener;
    
    class PushData {
    public:
      std::string uri;
      std::string mimeType;
      std::vector<char> data;
      std::string originUsername;
      
      friend std::ostream& operator<<(std::ostream &os, const ammo::gateway::PushData &pushData) {
        os << "URI: " << pushData.uri << " type: " << pushData.mimeType;
        return os;
      }
    };
    
    class PullRequest {
    public:
      std::string requestUid;
      std::string pluginId;
      std::string mimeType;
      std::string query;
      std::string projection;
      unsigned int maxResults;
      unsigned int startFromCount;
      bool liveQuery;
      
      friend std::ostream& operator<<(std::ostream &os, const ammo::gateway::PullRequest &pullReq) {
        os << "Pull << " << pullReq.requestUid << " from " << pullReq.pluginId << " for type " << pullReq.mimeType << " query: " << pullReq.query;
        return os;
      }
    };
    
    class PullResponse {
    public:
      std::string requestUid;
      std::string pluginId;
      std::string mimeType;
      std::string uri;
      std::vector<char> data;
      
      friend std::ostream& operator<<(std::ostream &os, const ammo::gateway::PullResponse &resp) {
        os << "Response to " << resp.pluginId << " for request " << resp.requestUid << " for type " << resp.requestUid;
        return os;
      }
    };
    
    /**
    * This class is used to connect a gateway plugin to the core gateway.  Each 
    * plugin should use at least one instance of this class; a plugin may create
    * more than one (establish more than one connection to the core gateway) if
    * needed (see the AndroidGatewayPlugin, which creates a new GatewayConnector
    * for each connected device; this aids in tracking requests and subscriptions
    * for multiple devices).
    */
    class LibGatewayConnector_Export GatewayConnector {
    public:
      /**
      * Creates a new GatewayConnector with the given GatewayConnectorDelegate and
      * establishes a connection to the gateway core.
      *
      * @param delegate A GatewayConnectorDelegate object to be used by this
      *                 GatewayConnector instance.  May be NULL (no delegate methods
      *                 will be called).
      * @param configfile The optional configuration file path
      * @param loggerName The logger name that should be used by this GatewayConnector.
      */
      GatewayConnector(GatewayConnectorDelegate *delegate);
      GatewayConnector(GatewayConnectorDelegate *delegate, std::string configfile);
    
      /**
      * Destroys a GatewayConnector.
      */
      ~GatewayConnector();
      
      //General connection negotiation and bookkeeping
      /**
      * Associates a device with this GatewayConnector in the gateway core.  This
      * method should only be needed by device connector plugins (e.g. the Android
      * Gateway Plugin).
      * 
      * @todo Authentication is not yet implemented in the gateway--  this method
      *       will send an authentication request, but the gateway will always
      *       will always return 'success' (and this method will always return
      *       true).  We should actually perform authentication here (pending more
      *       information from the security people).
      * 
      * @param device The unique ID of the device connecting to the gateway
      * @param user The unique ID of the user associated with the device connecting
      *             to the gateway.
      * @param key The authentication key/password for this user and device. Content
      *            of this parameter is still to be determined.
      * 
      * @return true if authentication was successful; false if authentication
      *         failed.
      */
      bool associateDevice(std::string device, std::string user, std::string key);
      
      //--Data-Push support methods--
    
      //Sender-side
      /**
      * Pushes a piece of data (with a particular URI and type) to the gateway.
      * 
      * Data pushed with this method can be received by listeners registered with
      * registerDataInterest.  All listeners registered for the type specified by
      * mimeType will receive this piece of data.
      * 
      * @param uri The URI of this piece of data.  This URI should be a universally
      *            unique identifier for the object being pushed (no two distinct
      *            pieces of data should have the same URI).
      * @param mimeType The MIME type of this piece of data.  This MIME type is used
      *                 to determine which other gateway plugins will receive this
      *                 pushed data.
      * @param data The data to be pushed to the gateway.  Represented as a C++
      *             string; can contain any arbitrary binary data (the gateway will
      *             accept strings with bytes invalid in C-strings such as embedded
      *             nulls).
      * 
      * @return true if the operation succeeded; false if the operation failed.
      */
      bool pushData(std::string uri, std::string mimeType, const std::string &data, MessageScope scope = SCOPE_GLOBAL);
    
      /**
      * Requests data from a gateway plugin or device (which claims it can handle a
      * pull request of a particular type).
      *
      * @param requestUid A unique identifier for this pull request.  It will be
      *                   returned with each item returned by the pull request.
      * @param pluginId   The unique identifier for this plugin or connected device.
      *                   For devices, should be the same as the device ID used by
      *                   associateDevice.
      * @param mimeType   The data type to request.  mimeType is used to determine
      *                   which plugin or plugins a request is routed to.
      * @param query      The query for this pull request.  Its format is defined
      *                   by the plugin handling the request.
      * @param projection 
      * @param maxResults The maximum number of results to return from this pull
      *                   request.
      * @param startFromCount An offset specifying the result to begin returning
      *                       from (when a subset of results is returned).
      * @param liveQuery  Specifies a live query--  results are returned continuously
      *                   as they become available.  The exact behavior of this
      *                   option is defined by the plugin handling the request.
      *
      * @return true if the operation succeeded; false if the operation failed.
      */
      bool pullRequest(std::string requestUid, std::string pluginId,
           std::string mimeType, std::string query,
           std::string projection, unsigned int maxResults,
           unsigned int startFromCount, bool liveQuery);
    
      /** 
      * Sends a response to a pull request.  Used by plugins with a registered
      * pull request handler.
      *
      * @param requestUid The unique identifier for this pull request, as specified
      *                   in the initial request.
      * @param pluginId   The unique identifier of the device to send the response
      *                   to; must match the identifier from the initial request
      *                   or data will not be routed correctly.
      * @param mimeType   The data type of the data in this response.
      * @param uri        The URI of the data in this response.
      * @param data       The data to be sent to the requestor.
      *
      * @return true if the operation succeeded; false if the operation failed.
      */
      bool pullResponse(std::string requestUid, std::string pluginId,
            std::string mimeType, std::string uri,
            std::vector<char>& data);
    
    
      //Receiver-side
      /**
      * Registers interest in pushed data of type mime_type.  Data will be pushed
      * to the DataPushReceiverListener specified by listener.
      *
      * @param mime_type The type of data to listen for.
      * @param listener  The DataPushReceiverListener object which will be called
      *                  when data is available.
      *
      * @return true if the operation succeeded; false if the operation failed.
      */
      bool registerDataInterest(std::string mime_type, DataPushReceiverListener *listener, MessageScope scope = SCOPE_GLOBAL);
      
      /**
      * Unregisters interest in pushed data of type mime_type.  Will unregister all
      * listeners associated with this plugin if this plugin has registered multiple
      * listeners for the same type.
      * 
      * @param mime_type The type of data to unregister interest in.
      * 
      * @return true if the operation succeeded; false if the operation failed.
      */
      bool unregisterDataInterest(std::string mime_type, MessageScope scope = SCOPE_GLOBAL);
      
      /**
      * Registers a pull request handler for the specified data type.
      * 
      * @param mime_type The type of data to listen for pull requests for.
      * @param listener  The PullRequestReceiverListener object which will be called
      *                  when a new pull request is received.
      *
      * @return true if the operation succeeded; false if the operation failed.
      */
      bool registerPullInterest(std::string mime_type, PullRequestReceiverListener *listener);
      
      /**
      * Unregisters interest in pull requests for the specified data type.  Will
      * unregister all listeners associated with this plugin if this plugin has 
      * registered multiple listeners for the same type.
      * 
      * @param mime_type The type of data to unregister interest in.
      * 
      * @return true if the operation succeeded; false if the operation failed.
      */
      bool unregisterPullInterest(std::string mime_type);
      
      /**
      * Registers a listener to be called when data is received as a response from a
      * pull request.  Should be called in conjunction with pullRequest when
      * initiating a pull request; does not have to be called more than once if more
      * than one pull request will be made for the same mime type.
      * 
      * @param mime_type The type of data to listen for pull responses for.
      * @param listener  The PullResponseReceiverListener object which will be
      *                  called when a new pull response is received.
      * 
      * @return true if the operation succeeded; false if the operation failed.
      */
      bool registerPullResponseInterest(std::string mime_type, PullResponseReceiverListener *listener);
      
      /**
      * Unregisters interest in pull responses for the specified data type.  Will
      * unresgister all listeners associated with this plugin if this plugin has
      * registered multiple listeners for the same data type.
      * 
      * @param mime_type The type of data to unregister interest in.
      * 
      * @return true if the operation succeeded; false if the operation failed.
      */
      bool unregisterPullResponseInterest(std::string mime_type);
      
    private:
      void init(GatewayConnectorDelegate *delegate, ammo::gateway::internal::GatewayConfigurationManager *config); 
    
      void onAssociateResultReceived(const ammo::gateway::protocol::AssociateResult &msg);
      void onPushDataReceived(const ammo::gateway::protocol::PushData &msg);
      void onPullRequestReceived(const ammo::gateway::protocol::PullRequest &msg);
      void onPullResponseReceived(const ammo::gateway::protocol::PullResponse &msg);
      
      GatewayConnectorDelegate *delegate;
      std::map<std::string, DataPushReceiverListener *> receiverListeners;
      std::map<std::string, PullRequestReceiverListener *> pullRequestListeners;
      std::map<std::string, PullResponseReceiverListener *> pullResponseListeners;
    
      ACE_Connector<ammo::gateway::internal::GatewayServiceHandler, ACE_SOCK_Connector> *connector;
      ammo::gateway::internal::GatewayServiceHandler *handler;
      
      bool connected;
      
      friend class ammo::gateway::internal::GatewayServiceHandler;
    };
    
    /**
    * The delegate object for a GatewayConnector.  Plugins implement the methods in
    * this class to implement behaviors associated with lifecycle events of the
    * gateway connector, such as connection and disconnection.
    */
    class LibGatewayConnector_Export GatewayConnectorDelegate {
    public:
      /**
      * Called when the GatewayConnector connects to the gateway core.
      * 
      * @param sender The GatewayConnector object that connected to the gateway.
      */
      virtual void onConnect(GatewayConnector *sender) = 0;
      
      /**
      * Called when the GatewayConnector disconnects from the gateway core.
      * 
      * @param sender The GatewayConnector object that disconnect from the gateway.
      */
      virtual void onDisconnect(GatewayConnector *sender) = 0;
      
      /**
      * Optional delegate method called after an authentication message is processed
      * by the gateway.  GatewayConnectorDelegate subclasses do not have to
      * implement this method if it's not needed (associateDevice is not being 
      * called); the default implementation does nothing.
      * 
      * @param sender The GatewayConnector which received the authentication result.
      * @param result true if authentication succeeded; false if authentication
      *               failed.
      */
      virtual void onAuthenticationResponse(GatewayConnector *sender, bool result);
    };
    
    /**
    * Listener class for pushed data.  Plugins subclass this class to receive data
    * pushed with pushData.
    */
    class LibGatewayConnector_Export DataPushReceiverListener {
    public:
      virtual void onPushDataReceived(GatewayConnector *sender, ammo::gateway::PushData &pushData) = 0;
    };
    
    /**
    * Listener class for pull requests.  Plugins subclass this class to receive
    * pull requests sent with pullRequest.
    */
    class LibGatewayConnector_Export PullRequestReceiverListener {
    public:
      /**
      * Called when a pull request is received by the gateway for the registered
      * data type.  A plugin's implementation of this method should call
      * pullResponse at least once to send the requested data. 
      */
      virtual void onPullRequestReceived(GatewayConnector *sender, ammo::gateway::PullRequest &pullReq) = 0;
    };
    
    /**
    * Listener class for pull responses.  Plugins subclass this class to receive
    * responses from pull requests.
    */
    class LibGatewayConnector_Export PullResponseReceiverListener {
    public:
      virtual void onPullResponseReceived(GatewayConnector *sender, PullResponse &response) = 0;
    };

  }
}




#endif        //  #ifndef GATEWAY_CONNECTOR_H

