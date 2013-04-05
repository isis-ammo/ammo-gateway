package edu.vu.isis.ammo;

import com.google.protobuf.ByteString;
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
public class GatewayConnectionHandler implements GatewayConnectorDelegate, DataPushReceiverListener, PullResponseReceiverListener {
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
                AmmoMessages.UnsubscribeMessage unsubscribeMessage = msg.getUnsubscribeMessage();

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
        logger.debug("Sending subscribed data to device");

        AmmoMessages.MessageWrapper.Builder msg = AmmoMessages.MessageWrapper.newBuilder();
        AmmoMessages.DataMessage.Builder dataMsg = msg.getDataMessageBuilder();
        dataMsg.setUri(pushData.uri);
        dataMsg.setMimeType(pushData.mimeType);
        dataMsg.setEncoding(pushData.encoding);
        dataMsg.setData(ByteString.copyFrom(pushData.data));
        dataMsg.setOriginDevice(pushData.originDevice);
        dataMsg.setUserId(pushData.originUserName);

        dataMsg.getThresholdsBuilder().setDeviceDelivered(pushData.ackThresholds.deviceDelivered);
        dataMsg.getThresholdsBuilder().setPluginDelivered(pushData.ackThresholds.pluginDelivered);
        dataMsg.getThresholdsBuilder().setAndroidPluginReceived(false);

        msg.setType(AmmoMessages.MessageWrapper.MessageType.DATA_MESSAGE);
        msg.setMessagePriority(pushData.priority);

        logger.debug("Sending Data Push message to connected device");
        channel.write(msg.build());
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

        AmmoMessages.MessageWrapper.Builder msg = AmmoMessages.MessageWrapper.newBuilder();
        msg.setType(AmmoMessages.MessageWrapper.MessageType.AUTHENTICATION_RESULT);
        msg.setMessagePriority(MessagePriority.PRIORITY_AUTH.getValue());
        msg.getAuthenticationResultBuilder().setResult(result ? AmmoMessages.AuthenticationResult.Status.SUCCESS : AmmoMessages.AuthenticationResult.Status.FAILED);
        channel.write(msg.build());
    }

    @Override
    public void onPushAcknowledgementReceived(GatewayConnector gatewayConnector, PushAcknowledgement ack) {
        logger.debug("Received push acknowledgement from gateway");

        AmmoMessages.MessageWrapper.Builder msg = AmmoMessages.MessageWrapper.newBuilder();
        AmmoMessages.PushAcknowledgement.Builder ackMsg = msg.getPushAcknowledgementBuilder();

        ackMsg.setUri(ack.uid);
        ackMsg.setDestinationDevice(ack.destinationDevice);
        ackMsg.setAcknowledgingDevice(ack.acknowledgingDevice);
        ackMsg.setDestinationUser(ack.destinationUser);
        ackMsg.setAcknowledgingUser(ack.acknowledgingUser);

        AmmoMessages.AcknowledgementThresholds.Builder thresholds = ackMsg.getThresholdBuilder();
        thresholds.setDeviceDelivered(ack.deviceDelivered);
        thresholds.setPluginDelivered(ack.pluginDelivered);
        thresholds.setAndroidPluginReceived(false);

        AmmoMessages.PushAcknowledgement.PushStatus status = AmmoMessages.PushAcknowledgement.PushStatus.RECEIVED;

        switch (ack.status) {
            case PUSH_RECEIVED:
                status = AmmoMessages.PushAcknowledgement.PushStatus.RECEIVED;
                break;
            case PUSH_SUCCESS:
                status = AmmoMessages.PushAcknowledgement.PushStatus.SUCCESS;
                break;
            case PUSH_FAIL:
                status = AmmoMessages.PushAcknowledgement.PushStatus.FAIL;
                break;
            case PUSH_REJECTED:
                status = AmmoMessages.PushAcknowledgement.PushStatus.REJECTED;
                break;
        }

        ackMsg.setStatus(status);

        msg.setType(AmmoMessages.MessageWrapper.MessageType.PUSH_ACKNOWLEDGEMENT);
        msg.setMessagePriority(MessagePriority.PRIORITY_CTRL.getValue());

        logger.debug("Sending push acknowledgement to device");
        channel.write(msg.build());
    }

    @Override
    public void onPullResponseReceived(GatewayConnector gatewayConnector, PullResponse pullResponse) {
        logger.debug("Received pull response from gateway");

        AmmoMessages.MessageWrapper.Builder msg = AmmoMessages.MessageWrapper.newBuilder();
        AmmoMessages.PullResponse.Builder pullMsg = msg.getPullResponseBuilder();

        pullMsg.setRequestUid(pullResponse.requestUid);
        pullMsg.setPluginId(pullResponse.pluginId);
        pullMsg.setMimeType(pullResponse.mimeType);
        pullMsg.setUri(pullResponse.uri);
        pullMsg.setEncoding(pullResponse.encoding);
        pullMsg.setData(ByteString.copyFrom(pullResponse.data));

        msg.setType(AmmoMessages.MessageWrapper.MessageType.PULL_RESPONSE);
        msg.setMessagePriority(pullResponse.priority);

        logger.debug("Sending pull response to device");
        channel.write(msg.build());
    }
}
