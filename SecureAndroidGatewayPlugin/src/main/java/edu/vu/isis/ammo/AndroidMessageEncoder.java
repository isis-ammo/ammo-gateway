package edu.vu.isis.ammo;

import edu.vu.isis.ammo.core.pb.AmmoMessages;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.Unpooled;
import io.netty.channel.ChannelHandlerContext;
import io.netty.handler.codec.MessageToByteEncoder;

import java.nio.ByteOrder;
import java.util.zip.CRC32;

/**
 * Created with IntelliJ IDEA.
 * User: jwilliams
 * Date: 4/5/13
 * Time: 2:36 PM
 * To change this template use File | Settings | File Templates.
 */
public class AndroidMessageEncoder extends MessageToByteEncoder<AmmoMessages.MessageWrapper> {
    @Override
    protected void encode(ChannelHandlerContext channelHandlerContext, AmmoMessages.MessageWrapper messageWrapper, ByteBuf byteBuf) throws Exception {
        byte[] messageData = messageWrapper.toByteArray();

        //generate the header
        ByteBuf headerBuf = Unpooled.buffer(20);
        ByteBuf headerBufLittleEndian = headerBuf.order(ByteOrder.LITTLE_ENDIAN);
        headerBufLittleEndian.writeInt(AndroidMessageDecoder.MAGIC_NUMBER);       //Magic number
        headerBufLittleEndian.writeInt(messageData.length);                       //Data length (not including header)
        headerBufLittleEndian.writeByte(messageWrapper.getMessagePriority());     //Message priority
        headerBufLittleEndian.writeByte(0);                                       //Error code
        headerBufLittleEndian.writeByte(0);                                       //Reserved byte
        headerBufLittleEndian.writeByte(0);                                       //Reserved byte

        CRC32 dataCrc = new CRC32();
        dataCrc.update(messageData);
        headerBufLittleEndian.writeInt((int) dataCrc.getValue());                 //Data CRC32

        CRC32 headerCrc = new CRC32();
        headerCrc.update(headerBufLittleEndian.array(), 0, 16);
        headerBufLittleEndian.writeInt((int) headerCrc.getValue());               //Header CRC32 (all 16 bytes of the header, not including itself)

        //send the header, then send the data
        byteBuf.writeBytes(headerBufLittleEndian);
        byteBuf.writeBytes(messageData);
    }
}
