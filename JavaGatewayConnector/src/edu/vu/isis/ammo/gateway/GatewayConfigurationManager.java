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

    private static GatewayConfigurationManager sharedInstance;

    private String gatewayAddress;
    private String gatewayInterface;
    private int gatewayPort;
}
