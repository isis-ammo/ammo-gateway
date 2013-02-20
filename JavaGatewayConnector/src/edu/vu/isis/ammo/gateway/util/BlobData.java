package edu.vu.isis.ammo.gateway.util;

import java.util.Arrays;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;


public class BlobData {
    private static final Logger logger = LoggerFactory.getLogger(BlobData.class);
    /**
     * The presence of the BLOB_MARKER_FIELD as the first byte in the footer for
     * a blob data section indicates where the blob should be placed in the
     * content provider.
     */
    public static final byte BLOB_MARKER_FIELD = (byte) 0xff;

    /**
     * The blob type enum makes a distinction between blobs which are small and
     * those which are large. The basic idea is delivery and storage will be
     * performed differently depending upon the relative size of the blob.
     */
    public enum BlobTypeEnum {
        /** blob too large to send via binder */
        LARGE,
        /** blob sufficiently small to send via binder */
        SMALL;

        /**
         * The difficulty here is that the blob may have trailing null bytes.
         * e.g. fieldName = [data], fieldNameBlob= [[100, 97, 116, 97]], blob =
         * [[100, 97, 116, 97, 0]] These should result it a match.
         */
        public static BlobTypeEnum infer(String fieldName, byte[] blob) {

            final byte[] fieldNameBlob;
            try {
                fieldNameBlob = fieldName.getBytes("UTF-8");
            } catch (java.io.UnsupportedEncodingException ex) {
                return SMALL;
            }

            logger.trace("processing blob fieldName = [{}], fieldNameBlob= [{}], blob = [{}]", new Object[]{fieldName, fieldNameBlob, blob});

            if (blob == null)
                return LARGE;
            if (blob.length < 1)
                return LARGE;

            if (fieldNameBlob.length == blob.length)
                return Arrays.equals(blob, fieldNameBlob) ? LARGE : SMALL;

            if (fieldNameBlob.length > blob.length)
                return SMALL;

            int i;
            for (i = 0; i < fieldNameBlob.length; i++) {
                if (fieldNameBlob[i] != blob[i])
                    return SMALL;
            }
            for (; i < blob.length; i++) {
                if (blob[i] != (byte) 0)
                    return SMALL;
            }
            return LARGE;
        }
    }

    public final BlobTypeEnum blobType;
    public final byte[] blob;

    public BlobData(BlobTypeEnum blobType, byte[] blob) {
        this.blobType = blobType;
        this.blob = blob;
    }
}