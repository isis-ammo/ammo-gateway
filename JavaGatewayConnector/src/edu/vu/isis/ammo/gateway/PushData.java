package edu.vu.isis.ammo.gateway;

import java.util.*;

public class PushData {
    public PushData() {
	uri = null;
	mimeType = null;
	encoding = new String("json");
	data = null;
	originUserName = null;
	scope = MessageScope.SCOPE_GLOBAL;
    }


    public String uri;                   ///< The URI of this piece of data.  This URI should be a universally
                                        ///  unique identifier for the object being pushed (no two pieces of
                                        ///  data should have the same URI).
    public String mimeType;             ///< The MIME type of this piece of data.  This MIME type is used to
                                        ///  determine which other gateway plugins will receive this pushed
                                        ///  data.
    public String encoding;             ///< The encoding of the data in this message (optional; defaults
                                        ///  to "json" if not specified).
    public byte[] data;                 ///< The data to be pushed to the gateway.  Represented as a C++ string,
                                        ///  but can contain any arbitrary binary data (the gateway will 
                                        ///  accept strings with bytes invalid in C-strings such as embedded
                                        ///  nulls).
    public String originUserName;       ///< The username of the user who generated this data.  May be
                                        ///  overwritten by the gateway in some cases.  Optional.
    public MessageScope scope;          ///< The scope of this object (determines how many gateways to send
                                        ///  this object to in a multiple gateway configuration).  Optional,
                                        ///  will default to SCOPE_GLOBAL.
    
    @Override public String toString() {
        return new String("Push URI: " + uri + " type: " + mimeType);
    }
}
