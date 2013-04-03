package edu.vu.isis.ammo;

import edu.vu.isis.ammo.core.pb.AmmoMessages;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.Unpooled;
import io.netty.channel.ChannelHandlerContext;
import io.netty.handler.codec.ByteToMessageDecoder;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.nio.ByteOrder;
import java.util.zip.CRC32;

/**
 * Created with IntelliJ IDEA.
 * User: jwilliams
 * Date: 4/3/13
 * Time: 3:26 PM
 * To change this template use File | Settings | File Templates.
 */
public class AndroidMessageDecoder extends ByteToMessageDecoder {
    private static final Logger logger = LoggerFactory.getLogger(AndroidMessageDecoder.class);
    final int MAGIC_NUMBER = 0xfeedbeef;

    @Override
    protected Object decode(ChannelHandlerContext channelHandlerContext, ByteBuf in) throws Exception {
        if(in.readableBytes() < 20) {
            //haven't gotten a full header yet
            return null;
        }

        in.markReaderIndex();

        byte[] header = new byte[20];

        in.readBytes(header, 0, 20);

        ByteBuf headerBuf = Unpooled.wrappedBuffer(header);
        ByteBuf headerBufLittleEndian = headerBuf.order(ByteOrder.LITTLE_ENDIAN);

        int magicNumber = headerBufLittleEndian.readInt();
        int size = headerBufLittleEndian.readInt();
        byte priority = headerBufLittleEndian.readByte();
        byte error = headerBufLittleEndian.readByte();
        headerBufLittleEndian.readBytes(2); //two reserved bytes; not used
        int checksum = headerBufLittleEndian.readInt();
        int headerChecksum = headerBufLittleEndian.readInt();

        //verify header checksum
        CRC32 crc = new CRC32();
        crc.update(header, 0, 16);
        int expectedChecksum = (int) crc.getValue();
        if(magicNumber == MAGIC_NUMBER) {
            if(headerChecksum == expectedChecksum) {
                //check and see if we have enough data to continue
                if(in.readableBytes() < size) {
                    in.resetReaderIndex();
                    return null;
                }

                byte[] data = new byte[size];

                in.readBytes(data, 0, size);
                logger.debug("Read data");

                CRC32 dataCrc = new CRC32();
                dataCrc.update(data);
                int expectedDataChecksum = (int) dataCrc.getValue();

                if(checksum == expectedDataChecksum) {
                    AmmoMessages.MessageWrapper msg = AmmoMessages.MessageWrapper.parseFrom(data);
                    return msg;
                } else {
                    logger.error("Data checksum mismatch: {} != {} (expected)", expectedDataChecksum, checksum);
                    return null;
                }
            } else {
                logger.error("Header checksum mismatch: {} != {} (expected)", expectedChecksum, headerChecksum);
                return null;
            }
        } else {
            logger.error("Magic number mismatch: {} != {} (expected)", magicNumber, MAGIC_NUMBER);
            return null;
        }
    }
}
