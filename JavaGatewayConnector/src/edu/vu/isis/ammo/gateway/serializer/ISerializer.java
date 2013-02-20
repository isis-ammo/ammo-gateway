package edu.vu.isis.ammo.gateway.serializer;

import java.io.IOException;
import java.util.List;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import edu.vu.isis.ammo.gateway.util.DeserializedMessage;
import edu.vu.isis.ammo.gateway.util.FieldType;

public interface ISerializer {
    /**
     * Serializes a content item.
     * 
     * @param item The item to be serialized
     * @return the serialized content
     * @throws IOException 
     */
    public byte[] serialize(IContentItem item) throws IOException;
    
    /**
     * Deserializes a content item.
     * 
     * @param data The data to deserialize from.
     * @param fieldNames an ordered list of field names
     * @param dataTypes an ordered list of data types for the fields in fieldNames
     * @return a DeserializedMessage object containing the deserialized data
     */
    public DeserializedMessage deserialize(byte[] data, List<String> fieldNames, List<FieldType> dataTypes);
}
