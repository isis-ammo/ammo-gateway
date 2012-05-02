package edu.vu.isis.ammo.rmcastplugin;

import org.json.JSONException;
import org.json.JSONObject;
import org.json.JSONArray;
import org.json.JSONTokener;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.InputStream;
import java.io.FileInputStream;
import java.io.File;
import java.io.Reader;
import java.io.FileReader;
import java.io.FileNotFoundException;
import java.util.List;
import java.util.ArrayList;

class PluginConfigurationManager {
    public final static String CONFIG_DIRECTORY = "ammo-gateway";
    public final static String CONFIG_FILE = "RMCastPluginConfig.json";

    private static final Logger logger = LoggerFactory.getLogger(PluginConfigurationManager.class);

    public static PluginConfigurationManager getInstance() {
	return getInstance(CONFIG_FILE);
    }

    public static PluginConfigurationManager getInstance(String configFile) {
	if (sharedInstance == null) {
	    sharedInstance = new PluginConfigurationManager(configFile);
	}
	return sharedInstance;
    }

    public List<String> getMimeTypes() {
	return mimeTypes;
    }

    private PluginConfigurationManager( String configFile ) {
	mimeTypes = new ArrayList<String>();

	String fileName = findConfigFile(configFile);
	if (fileName != null) {
	    try {
		final JSONTokener tokener = new JSONTokener( new FileReader(fileName) );
		final JSONObject input = new JSONObject( tokener );
		if (input != null) {
		    if(input.has("MimeTypes")) {
			JSONArray jsonArray = input.getJSONArray("MimeTypes");
			for(int i=0; i<jsonArray.length(); i++)
			    mimeTypes.add( jsonArray.getString(i) );
		    } else {
			logger.error("<constructor>: MimeTypes is missing or wrong type (should be string array)");
		    }
      
		} else {
		    logger.error("<constructor> JSON parsing error in config file {}. using defaults", configFile);
		}
	    
	    } catch (JSONException jsx) {
		logger.error("Exception while parsing Plugin Configuration File: {}",
			jsx.getStackTrace() );
		jsx.printStackTrace();
	    } catch (FileNotFoundException fex) {
		logger.error("Exception while opening Plugin Configuration File: {}",
			fex.getStackTrace() );
		fex.printStackTrace();
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

    private static PluginConfigurationManager sharedInstance;

    private List<String> mimeTypes;
}