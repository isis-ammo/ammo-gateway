package edu.vu.isis.ammo;

import org.json.JSONException;
import org.json.JSONObject;
import org.json.JSONArray;
import org.json.JSONTokener;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.File;
import java.io.FileReader;
import java.io.FileNotFoundException;
import java.util.List;
import java.util.ArrayList;

class SecureGatewayPluginConfigurationManager {
    public final static String CONFIG_DIRECTORY = "ammo-gateway";
    public final static String CONFIG_FILE = "SecureGatewayPluginConfig.json";

    private static final Logger logger = LoggerFactory.getLogger(SecureGatewayPluginConfigurationManager.class);

    public static SecureGatewayPluginConfigurationManager getInstance() {
        return getInstance(CONFIG_FILE);
    }

    public static SecureGatewayPluginConfigurationManager getInstance(String configFile) {
        if (sharedInstance == null) {
            sharedInstance = new SecureGatewayPluginConfigurationManager(configFile);
        }
        return sharedInstance;
    }

    private SecureGatewayPluginConfigurationManager(String configFile) {
        listenAddress = "0.0.0.0";
        listenPort = 33290;
        clientAuthEnabled = true;

        String fileName = findConfigFile(configFile);
        if (fileName != null && !fileName.isEmpty()) {
            try {
                final JSONTokener tokener = new JSONTokener( new FileReader(fileName) );
                final JSONObject input = new JSONObject( tokener );
                if (input != null) {
                    if(input.has("ListenAddress")) {
                        listenAddress = input.getString("ListenAddress");
                    } else {
                        logger.error("<constructor>: ListenAddress is missing or wrong type (should be string)");
                    }

                    if(input.has("ListenPort")) {
                        listenPort = input.getInt("ListenPort");
                    } else {
                        logger.error("<constructor>: ListenPort is missing or wrong type (should be int)");
                    }

                    if(input.has("ClientAuthEnabled")) {
                        clientAuthEnabled = input.getBoolean("ClientAuthEnabled");
                    } else {
                        logger.error("<constructor>: ClientAuthEnabled is missing or wrong type (should be boolean)");
                    }

                    if(input.has("HeartbeatTimeout")) {
                        heartbeatTimeout = input.getInt("HeartbeatTimeout");
                    } else {
                        logger.error("<constructor>: HeartbeatTimeout is missing or wrong type (should be int)");
                    }


                } else {
                    logger.error("<constructor> JSON parsing error in config file {}. using defaults", configFile);
                }

            } catch (JSONException jsx) {
                logger.error("Exception while parsing Plugin Configuration File: {}", jsx);
                jsx.printStackTrace();
            } catch (FileNotFoundException fex) {
                logger.error("Exception while opening Plugin Configuration File: {}", fex);
                fex.printStackTrace();
            }

        }

        logConfiguration();
    }

    public void logConfiguration() {
        logger.info("Plugin configuration:");
        logger.info("    ListenAddress: {}", getListenAddress());
        logger.info("    ListenPort: {}", getListenPort());
        logger.info("    ClientAuthEnabled: {}", isClientAuthEnabled());
        logger.info("    HeartbeatTimeout: {}", getHeartbeatTimeout());
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

    private static SecureGatewayPluginConfigurationManager sharedInstance;

    private List<String> mimeTypes;

    String getListenAddress() {
        return listenAddress;
    }

    int getListenPort() {
        return listenPort;
    }

    boolean isClientAuthEnabled() {
        return clientAuthEnabled;
    }

    int getHeartbeatTimeout() {
        return heartbeatTimeout;
    }

    private String listenAddress;
    private int listenPort;
    private boolean clientAuthEnabled;
    private int heartbeatTimeout;
}