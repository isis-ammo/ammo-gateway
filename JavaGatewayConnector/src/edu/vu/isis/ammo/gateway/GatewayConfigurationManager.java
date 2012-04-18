package edu.vu.isis.ammo.gateway;

import org.json.JSONException;
import org.json.JSONObject;
import org.json.JSONTokener;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.InputStream;
import java.io.FileInputStream;
import java.io.File;
import java.io.Reader;
import java.io.FileReader;
import java.io.FileNotFoundException;

class GatewayConfigurationManager {
    public final static String CONFIG_DIRECTORY = "ammo-gateway";
    public final static String CONFIG_FILE = "GatewayConfig.json";

    private static final Logger logger = LoggerFactory.getLogger(GatewayConfigurationManager.class);

    public static GatewayConfigurationManager getInstance() {
	return getInstance(CONFIG_FILE);
    }

    public static GatewayConfigurationManager getInstance(String configFile) {
	if (sharedInstance == null) {
	    sharedInstance = new GatewayConfigurationManager(configFile);
	}
	return sharedInstance;
    }

    public String getGatewayAddress() {
	return gatewayAddress;
    }

    public String getGatewayInterface() {
	return gatewayInterface;
    }

    public int getGatewayPort() {
	return gatewayPort;
    }

    private GatewayConfigurationManager( String configFile ) {
	gatewayAddress = "127.0.0.1";
	gatewayInterface = "0.0.0.0";
	gatewayPort = 12475;

	String fileName = findConfigFile(configFile);
	if (fileName != null) {
	    try {
	    final JSONTokener tokener = new JSONTokener( new FileReader(fileName) );
	    final JSONObject input = new JSONObject( tokener );
	    if (input != null) {
		if(input.has("GatewayInterface")) {
		    gatewayInterface = input.getString("GatewayInterface");
		} else {
		    logger.error("<constructor>: GatewayInterface is missing or wrong type (should be string)");
		}
      
		if(input.has("GatewayAddress")) {
		    gatewayAddress = input.getString("GatewayAddress");
		} else {
		    logger.error("<constructor>: GatewayAddress is missing or wrong type (should be string)");
		}
      
		if(input.has("GatewayPort")) {
		    gatewayPort = input.getInt("GatewayPort");
		} else {
		    logger.error("<constructor>: GatewayPort is missing or wrong type (should be integer)");
		}
	    } else {
		logger.error("<constructor> JSON parsing error in config file {}. using defaults", configFile);
	    }
	    
	    } catch (JSONException jsx) {
		logger.error("Exception while parsing Gateway Configuration File: {}",
			     jsx.getMessage() );
	    } catch (FileNotFoundException fex) {
		logger.error("Exception while opening Gateway Configuration File: {}",
			     fex.getMessage() );
	    }
	    
	}
	    

    }

    private String findConfigFile( String configFile ) {
	String filePath = configFile;
	String home = System.getenv("HOME");
	if (home == null) home = new String("");
	String gatewayRoot = System.getenv("GATEWAY_ROOT");
	if (gatewayRoot == null) gatewayRoot = new String("");

	if (new File(filePath).exists() == false) {
	    filePath = home + "/." + CONFIG_DIRECTORY + "/" + CONFIG_FILE;
	    if (new File(filePath).exists() == false) {
		filePath = new String("/etc/") + CONFIG_DIRECTORY + "/" + CONFIG_FILE;
		if (new File(filePath).exists() == false) {
		    filePath = gatewayRoot + "/etc/" + CONFIG_FILE;
		    if (new File(filePath).exists() == false) {
			filePath = gatewayRoot + "/build/etc/" + CONFIG_FILE;
			if (new File(filePath).exists() == false) {
			    filePath = new String("../etc/") + CONFIG_FILE;
			    if (new File(filePath).exists() == false) {
				logger.error("findConfigFile: unable to find config file");
				return "";
			    }
			}
		    }
		}
	    }
	}

	logger.info("findConfigFile: using config file {}", filePath);
	return filePath;
    }

    private static GatewayConfigurationManager sharedInstance;

    private String gatewayAddress;
    private String gatewayInterface;
    private int gatewayPort;
}