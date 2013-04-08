
package edu.vu.isis.ammo;

import io.netty.bootstrap.ServerBootstrap;
import io.netty.channel.EventLoopGroup;
import io.netty.channel.nio.NioEventLoopGroup;
import io.netty.channel.socket.nio.NioServerSocketChannel;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * To run for testing:
 *
 *   1) Generate keys:  keytool -genkey -keystore mySrvKeystore -keyalg RSA
 *   2) Run at command line:  java -Djavax.net.ssl.keyStore=mySrvKeystore -Djavax.net.ssl.keyStorePassword=123456 -jar secure-gateway-plugin-1.7.2-SNAPSHOT.jar
 */

public class SecureGatewayPluginMain {
    private static final Logger logger = LoggerFactory.getLogger(SecureGatewayPluginMain.class);

    public static void main(String[] args) throws Exception {
        logger.error("Hello world");

        EventLoopGroup bossGroup = new NioEventLoopGroup();
        EventLoopGroup workerGroup = new NioEventLoopGroup();

        try {
            ServerBootstrap b = new ServerBootstrap();
            b.group(bossGroup, workerGroup)
                    .channel(NioServerSocketChannel.class)
                    .childHandler(new GatewayServerInitializer());

            b.bind(33290).sync().channel().closeFuture().sync();

        } finally {
            bossGroup.shutdown();
            workerGroup.shutdown();
        }
    }
}
