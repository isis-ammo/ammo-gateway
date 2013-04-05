package edu.vu.isis.ammo;

import io.netty.channel.ChannelInitializer;
import io.netty.channel.ChannelPipeline;
import io.netty.channel.socket.SocketChannel;
import io.netty.handler.codec.LengthFieldBasedFrameDecoder;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.nio.ByteOrder;

/**
 * Created with IntelliJ IDEA.
 * User: jwilliams
 * Date: 4/3/13
 * Time: 3:16 PM
 * To change this template use File | Settings | File Templates.
 */
public class GatewayServerInitializer extends ChannelInitializer<SocketChannel> {
    private static final Logger logger = LoggerFactory.getLogger(GatewayServerInitializer.class);

    final int MAXIMUM_FRAME_LENGTH = Integer.MAX_VALUE;

    @Override
    protected void initChannel(SocketChannel socketChannel) throws Exception {
        ChannelPipeline pipeline = socketChannel.pipeline();

        pipeline.addLast("decoder", new AndroidMessageDecoder());
        pipeline.addLast("handler", new AndroidMessageHandler());

        pipeline.addLast("encoder", new AndroidMessageEncoder());
    }
}
