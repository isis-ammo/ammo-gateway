package edu.vu.isis.ammo;

import edu.vu.isis.ammo.core.pb.AmmoMessages;
import io.netty.channel.ChannelHandlerContext;
import io.netty.channel.ChannelInboundMessageHandlerAdapter;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * Created with IntelliJ IDEA.
 * User: jwilliams
 * Date: 4/3/13
 * Time: 4:29 PM
 * To change this template use File | Settings | File Templates.
 */
public class AndroidMessageHandler extends ChannelInboundMessageHandlerAdapter<AmmoMessages.MessageWrapper> {
    private static final Logger logger = LoggerFactory.getLogger(AndroidMessageHandler.class);

    @Override
    public void messageReceived(ChannelHandlerContext channelHandlerContext, AmmoMessages.MessageWrapper messageWrapper) throws Exception {
        //To change body of implemented methods use File | Settings | File Templates.
        logger.error("{} Received message: {}", this.hashCode(), messageWrapper);
    }

    @Override
    public void channelActive(ChannelHandlerContext ctx) throws Exception {
        super.channelActive(ctx);
        logger.info("{} {} connected", this.hashCode(), ctx.channel().remoteAddress());
    }

    @Override
    public void exceptionCaught(ChannelHandlerContext ctx, Throwable cause) throws Exception {
        super.exceptionCaught(ctx, cause);    //To change body of overridden methods use File | Settings | File Templates.
    }
}
