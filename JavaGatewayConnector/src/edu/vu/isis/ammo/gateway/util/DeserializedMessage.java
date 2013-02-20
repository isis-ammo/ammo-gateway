package edu.vu.isis.ammo.gateway.util;

import java.util.Map;
import java.util.HashMap;

public class DeserializedMessage {
    public ContentValues cv;
    public Map<String, BlobData> blobs;

    public DeserializedMessage() {
        cv = new ContentValues();
        blobs = new HashMap<String, BlobData>();
    }
}
