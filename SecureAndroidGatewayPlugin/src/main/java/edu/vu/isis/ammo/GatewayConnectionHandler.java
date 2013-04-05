package edu.vu.isis.ammo;

import edu.vu.isis.ammo.core.pb.AmmoMessages;
import edu.vu.isis.ammo.gateway.*;
import io.netty.channel.Channel;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * Created with IntelliJ IDEA.
 * User: jwilliams
 * Date: 4/4/13
 * Time: 11:14 AM
 * To change this template use File | Settings | File Templates.
 */
public class GatewayConnectionHandler implements GatewayConnectorDelegate, DataPushReceiverListener, PullRequestReceiverListener, PullResponseReceiverListener {
    private static final Logger logger = LoggerFactory.getLogger(GatewayConnectionHandler.class);

    GatewayConnector connector;
    Channel channel;

    String deviceId;
    boolean deviceIdAuthenticated;

    public GatewayConnectionHandler(Channel channel) {
        this.channel = channel;
        this.connector = new GatewayConnector(this);
        deviceId = "";
        deviceIdAuthenticated = true;
    }

    public void clientDisconnected() {
        this.connector.close();
    }

    public void processMessage(AmmoMessages.MessageWrapper msg) {
        switch (msg.getType()) {

            case DATA_MESSAGE: {
                logger.debug("Received data message");

                AmmoMessages.DataMessage dataMessage = msg.getDataMessage();

                MessageScope scope;
                if(dataMessage.getScope() == AmmoMessages.MessageScope.LOCAL) {
                    scope = MessageScope.SCOPE_LOCAL;
                } else {
                    scope = MessageScope.SCOPE_GLOBAL;
                }

                PushData pushData = new PushData();
                pushData.uri = dataMessage.getUri();
                pushData.mimeType = dataMessage.getMimeType();
                pushData.data = dataMessage.getData().toByteArray();
                pushData.scope = scope;
                pushData.encoding = dataMessage.getEncoding();
                pushData.originUserName = dataMessage.getUserId();
                pushData.originDevice = dataMessage.getOriginDevice();
                pushData.priority = msg.getMessagePriority();
                pushData.ackThresholds.deviceDelivered = dataMessage.getThresholds().getDeviceDelivered();
                pushData.ackThresholds.pluginDelivered = dataMessage.getThresholds().getPluginDelivered();

                connector.pushData(pushData);

                if(msg.getDataMessage().getThresholds().getAndroidPluginReceived() == true) {
                    //send acknowledgement back to device

                    AmmoMessages.MessageWrapper.Builder ackMsgBuilder = AmmoMessages.MessageWrapper.newBuilder();
                    AmmoMessages.PushAcknowledgement.Builder ack = ackMsgBuilder.getPushAcknowledgementBuilder();
                    ack.setUri(dataMessage.getUri());
                    ack.setDestinationDevice((dataMessage.getOriginDevice()));
                    ack.setDestinationUser(dataMessage.getUserId());
                    AmmoMessages.AcknowledgementThresholds.Builder thresholds = ack.getThresholdBuilder();
                    thresholds.setDeviceDelivered(false);
                    thresholds.setPluginDelivered(false);
                    thresholds.setAndroidPluginReceived(true);
                    ack.setStatus(AmmoMessages.PushAcknowledgement.PushStatus.RECEIVED);
                    ackMsgBuilder.setType(AmmoMessages.MessageWrapper.MessageType.PUSH_ACKNOWLEDGEMENT);
                    ackMsgBuilder.setMessagePriority(pushData.priority);

                    logger.debug(("Sending push acknowledgement to device"));

                    channel.write(ackMsgBuilder.build());
                }
                break;
            }
            case AUTHENTICATION_MESSAGE: {
                logger.debug("Received authentication message");

                AmmoMessages.AuthenticationMessage authMessage = msg.getAuthenticationMessage();
                this.connector.associateDevice(authMessage.getDeviceId(), authMessage.getUserId(), authMessage.getUserKey());

                deviceId = authMessage.getDeviceId();

                break;
            }
            case SUBSCRIBE_MESSAGE: {
                logger.debug("Received subscribe message");
                AmmoMessages.SubscribeMessage subscribeMessage = msg.getSubscribeMessage();

                MessageScope scope;
                if(subscribeMessage.getScope() == AmmoMessages.MessageScope.LOCAL) {
                    scope = MessageScope.SCOPE_LOCAL;
                } else {
                    scope = MessageScope.SCOPE_GLOBAL;
                }

                connector.registerDataInterest(subscribeMessage.getMimeType(), this, scope);

                break;
            }
            case PUSH_ACKNOWLEDGEMENT: {
                AmmoMessages.PushAcknowledgement ackMsg = msg.getPushAcknowledgement();
                PushAcknowledgement pushAck = new PushAcknowledgement();

                pushAck.uid = ackMsg.getUri();
                pushAck.destinationDevice = ackMsg.getDestinationDevice();
                pushAck.acknowledgingDevice = ackMsg.getAcknowledgingDevice();
                pushAck.acknowledgingUser = ackMsg.getAcknowledgingUser();
                pushAck.destinationUser = ackMsg.getDestinationUser();
                pushAck.deviceDelivered = ackMsg.getThreshold().getDeviceDelivered();
                pushAck.pluginDelivered = ackMsg.getThreshold().getPluginDelivered();

                switch(ackMsg.getStatus()) {
                    case RECEIVED:
                        pushAck.status = PushStatus.PUSH_RECEIVED;
                        break;
                    case SUCCESS:
                        pushAck.status = PushStatus.PUSH_SUCCESS;
                        break;
                    case FAIL:
                        pushAck.status = PushStatus.PUSH_FAIL;
                        break;
                    case REJECTED:
                        pushAck.status = PushStatus.PUSH_REJECTED;
                        break;
                }

                connector.pushAcknowledgement(pushAck);
                break;
            }
            case PULL_REQUEST: {
                logger.debug("Received pull request");

                if(deviceIdAuthenticated) {
                    AmmoMessages.PullRequest pullRequest = msg.getPullRequest();

                    connector.registerPullResponseInterest(pullRequest.getMimeType(), this);

                    MessageScope scope;
                    if(pullRequest.getScope() == AmmoMessages.MessageScope.LOCAL) {
                        scope = MessageScope.SCOPE_LOCAL;
                    } else {
                        scope = MessageScope.SCOPE_GLOBAL;
                    }

                    PullRequest req = new PullRequest();
                    req.requestUid = pullRequest.getRequestUid();
                    req.pluginId = this.deviceId;
                    req.mimeType = pullRequest.getMimeType();
                    req.query = pullRequest.getQuery();
                    req.projection = pullRequest.getProjection();
                    req.maxResults = pullRequest.getMaxResults();
                    req.startFromCount = pullRequest.getStartFromCount();
                    req.liveQuery = pullRequest.getLiveQuery();
                    req.scope = scope;
                    req.priority = msg.getMessagePriority();
                    connector.pullRequest(req);
                }
                break;
            }
            case HEARTBEAT: {
                logger.debug("Received heartbeat from device");
                AmmoMessages.Heartbeat heartbeat = msg.getHeartbeat();

                AmmoMessages.MessageWrapper.Builder heartbeatAck = AmmoMessages.MessageWrapper.newBuilder();
                AmmoMessages.Heartbeat.Builder ack = heartbeatAck.getHeartbeatBuilder();

                ack.setSequenceNumber(heartbeat.getSequenceNumber());
                heartbeatAck.setType(AmmoMessages.MessageWrapper.MessageType.HEARTBEAT);
                heartbeatAck.setMessagePriority(MessagePriority.PRIORITY_CTRL.getValue());

                logger.debug("Sending heartbeat ack to device");
                channel.write(heartbeatAck);
                break;
            }
            case UNSUBSCRIBE_MESSAGE: {
                logger.debug("Received unsubscribe message");
                AmmoMessages.UnsubscribeMessage unsubscribeMessage = msg.getUnubscribeMessage();

                MessageScope scope;
                if(unsubscribeMessage.getScope() == AmmoMessages.MessageScope.LOCAL) {
                    scope = MessageScope.SCOPE_LOCAL;
                } else {
                    scope = MessageScope.SCOPE_GLOBAL;
                }

                connector.unregisterDataInterest(unsubscribeMessage.getMimeType(), scope);
                break;
            }
            default: {
                break;
            }
        }
    }


    @Override
    public void onPushDataReceived(GatewayConnector gatewayConnector, PushData pushData) {
        //To change body of implemented methods use File | Settings | File Templates.
    }

    @Override
    public void onConnect(GatewayConnector gatewayConnector) {
        logger.debug("Connected to gateway");
    }

    @Override
    public void onDisconnect(GatewayConnector gatewayConnector) {
        logger.debug("Disconnected from gateway");
    }

    @Override
    public void onAuthenticationResponse(GatewayConnector gatewayConnector, boolean result) {
        logger.debug("Delegate: onAuthenticationResponse");
        if(result == true) {
            deviceIdAuthenticated = true;
        }
    }

    @Override
    public void onPushAcknowledgementReceived(GatewayConnector gatewayConnector, PushAcknowledgement pushAcknowledgement) {
        //To change body of implemented methods use File | Settings | File Templates.
    }

    @Override
    public void onPullRequestReceived(GatewayConnector gatewayConnector, PullRequest pullRequest) {
        //To change body of implemented methods use File | Settings | File Templates.
    }

    @Override
    public void onPullResponseReceived(GatewayConnector gatewayConnector, PullResponse pullResponse) {
        //To change body of implemented methods use File | Settings | File Templates.
    }
}
