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

package edu.vu.isis.ammo.mcastplugin;

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
    public final static String CONFIG_FILE = "MCastPluginConfig.json";

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
    
    public int getTtl() {
        return ttl;
    }

    private PluginConfigurationManager( String configFile ) {
	mimeTypes = new ArrayList<String>();
	ttl = 7;

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
		    
		    if(input.has("TTL")) {
                ttl = input.getInt("TTL");
            } else {
                logger.error("<constructor>: TTL is missing or wrong type (should be int)");
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
	final String os = System.getProperty("os.name").toLowerCase();
	String filePath;

	if (os.indexOf("win") >= 0) {
	    filePath = findConfigFileWindows(configFile);
	}
	else {
	    filePath = findConfigFileLinux(configFile);
	}

	logger.info("findConfigFile: using config file {}", filePath);
	return filePath;
    }

    /**
     * Searches for the gateway config file.  Search order:
     *   1) The current working directory
     *   2) ~/.ammo-gateway/
     *   3) /etc/ammo-gateway/
     *   Fallback locations (don't rely on these; they may change or disappear in a
     *   future release.  Gateway installation should put the config file into
     *   a location that's searched by default):
     *   4) $GATEWAY_ROOT/etc
     *   5) $GATEWAY_ROOT/build/etc
     *   6) ../etc
     */
    private String findConfigFileLinux( String configFile ) {
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

	return filePath;
    }

    /**
     * Searches for the gateway config file.  Search order:
     *   1) The current working directory
     *   2) The user's configuration directory (Roaming appdata directory/ammo-gateway)
     *   3) The all users configuration directory (i.e. C:\ProgramData\ammo-gateway on Vista/7)
     *   Fallback locations (don't rely on these; they may change or disappear in a
     *   future release.  Gateway installation should put the config file into
     *   a location that's searched by default):
     *   4) $GATEWAY_ROOT/etc
     *   5) $GATEWAY_ROOT/build/etc
     *   6) ../etc
     */
    private String findConfigFileWindows( String configFile ) {
	String filePath = configFile;
	String userConfigPath = System.getenv("APPDATA");
	if (userConfigPath == null) userConfigPath = new String("");
	String systemConfigPath = System.getenv("PROGRAMDATA");
	if (systemConfigPath == null) systemConfigPath = new String("");
	String gatewayRoot = System.getenv("GATEWAY_ROOT");
	if (gatewayRoot == null) gatewayRoot = new String("");

	if (new File(filePath).exists() == false) {
	    filePath = userConfigPath + "/" + CONFIG_DIRECTORY + "/" + CONFIG_FILE;
	    if (new File(filePath).exists() == false) {
		filePath = systemConfigPath + "/" + CONFIG_DIRECTORY + "/" + CONFIG_FILE;
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

	return filePath;
    }

    private static PluginConfigurationManager sharedInstance;

    private List<String> mimeTypes;
    private int ttl;
}
