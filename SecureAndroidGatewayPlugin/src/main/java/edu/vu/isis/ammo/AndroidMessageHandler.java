package edu.vu.isis.ammo;

import edu.vu.isis.ammo.core.pb.AmmoMessages;
import io.netty.channel.*;
import io.netty.handler.ssl.SslHandler;
import io.netty.util.concurrent.Future;
import io.netty.util.concurrent.GenericFutureListener;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import javax.net.ssl.SSLSession;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.cert.Certificate;
import java.security.cert.CertificateEncodingException;
import java.security.cert.X509Certificate;
import java.util.HashMap;
import java.util.Map;

/**
 * Created with IntelliJ IDEA.
 * User: jwilliams
 * Date: 4/3/13
 * Time: 4:29 PM
 * To change this template use File | Settings | File Templates.
 */
public class AndroidMessageHandler extends ChannelInboundMessageHandlerAdapter<AmmoMessages.MessageWrapper> {
    private static final Logger logger = LoggerFactory.getLogger(AndroidMessageHandler.class);

    Map<Channel, GatewayConnectionHandler> gatewayConnectionMap;

    public AndroidMessageHandler() {
        gatewayConnectionMap = new HashMap<Channel, GatewayConnectionHandler>();
    }

    @Override
    public void messageReceived(ChannelHandlerContext channelHandlerContext, AmmoMessages.MessageWrapper messageWrapper) throws Exception {
        //To change body of implemented methods use File | Settings | File Templates.
        logger.error("{} Received message: {}", this.hashCode(), messageWrapper);
        gatewayConnectionMap.get(channelHandlerContext.channel()).processMessage(messageWrapper);
    }

    @Override
    public void channelActive(final ChannelHandlerContext ctx) throws Exception {
        super.channelActive(ctx);

        ctx.pipeline().get(SslHandler.class).handshakeFuture().addListener(new GenericFutureListener<Future<Channel>>() {
            @Override
            public void operationComplete(Future<Channel> future) throws Exception {
                SecureGatewayPluginConfigurationManager config = SecureGatewayPluginConfigurationManager.getInstance();

                //TODO: Verify that handshake succeeded
                if(future.isSuccess()) {
                    SSLSession session = ctx.pipeline().get(SslHandler.class).engine().getSession();

                    logger.info("SSL handshake complete; session established");
                    logger.info("  Protocol: {}", session.getProtocol());
                    logger.info("  Cipher suite: {}", session.getCipherSuite());

                    if(config.isClientAuthEnabled()) {
                        Certificate[] certs = session.getPeerCertificates();
                        if(certs.length > 0 && certs[0] instanceof X509Certificate) {
                            X509Certificate peerCert = (X509Certificate) certs[0];
                            logger.info("  Connected peer: DN: {}", peerCert.getSubjectX500Principal());
                            logger.info("    Serial: {}", peerCert.getSerialNumber());
                            logger.info("    Fingerprint: {}", getPrintableCertFingerprint(peerCert));

                            deviceConnected(ctx);
                        } else if(certs.length == 0) {
                            logger.error("No certificates in peer certificate chain");
                        } else {
                            logger.error("Client authenticated with something that's not an X.509 certificate ({})", certs[0].getType());
                        }
                    } else {
                        deviceConnected(ctx);
                    }
                } else {
                    logger.warn("SSL handshake failure");
                }
            }
        });
    }

    private void deviceConnected(ChannelHandlerContext ctx) {
        logger.info("{} {} connected", this.hashCode(), ctx.channel().remoteAddress());

        //create new gateway connector for this connection
        Channel c = ctx.channel();
        GatewayConnectionHandler connectionHandler = new GatewayConnectionHandler(c);

        gatewayConnectionMap.put(c, connectionHandler);
    }

    @Override
    public void channelInactive(ChannelHandlerContext ctx) throws Exception {
        super.channelInactive(ctx); 
        logger.info("{} {} disconnected", this.hashCode(), ctx.channel().remoteAddress());

        GatewayConnectionHandler h = gatewayConnectionMap.remove(ctx.channel());
        if(h != null) {
          h.clientDisconnected();
        }
    }

    @Override
    public void exceptionCaught(ChannelHandlerContext ctx, Throwable cause) throws Exception {
        super.exceptionCaught(ctx, cause);    //To change body of overridden methods use File | Settings | File Templates.
    }

    private static String getPrintableCertFingerprint(X509Certificate cert) throws NoSuchAlgorithmException, CertificateEncodingException {
        MessageDigest md = MessageDigest.getInstance("SHA1");
        md.update( cert.getEncoded() );
        byte[] fp = md.digest();
        return bytesToHex(fp);
    }

    private static String bytesToHex(byte[] bytes) {
        final char[] hexArray = {'0','1','2','3','4','5','6','7','8','9','a', 'b','c','d','e','f'};
        char[] hexChars = new char[bytes.length * 2];
        int v;
        for ( int j = 0; j < bytes.length; j++ ) {
            v = bytes[j] & 0xFF;
            hexChars[j * 2] = hexArray[v >>> 4];
            hexChars[j * 2 + 1] = hexArray[v & 0x0F];
        }
        return new String(hexChars);
    }
}
