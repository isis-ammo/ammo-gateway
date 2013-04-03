
package edu.vu.isis.ammo;

import io.netty.bootstrap.ServerBootstrap;
import io.netty.channel.EventLoopGroup;
import io.netty.channel.nio.NioEventLoopGroup;
import io.netty.channel.socket.nio.NioServerSocketChannel;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

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

            b.bind(33289).sync().channel().closeFuture().sync();

        } finally {
            bossGroup.shutdown();
            workerGroup.shutdown();
        }
    }
}
