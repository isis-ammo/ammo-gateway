package edu.vu.isis.ammo.gateway.serializer;

import java.io.IOException;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.io.File;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import edu.vu.isis.ammo.gateway.util.ContentValues;

import edu.vu.isis.ammo.gateway.util.ContractStore;
import edu.vu.isis.ammo.gateway.util.Encoding;
import edu.vu.isis.ammo.gateway.util.FieldType;

public class ContentValuesContentItem implements IContentItem {
    static final Logger logger = LoggerFactory.getLogger("dist.serializer.cursoritem");
    
    private final ContentValues cv;
    private final Map<String, FieldType> fieldTypes;
    private String[] serialOrder;
    private Set<String> serialSet;
    
    public ContentValuesContentItem(ContentValues cv, ContractStore.Relation relation, Encoding encoding) {
        this.cv = cv;
        
        fieldTypes = new HashMap<String, FieldType>(cv.size());
        
        int i = 0;
        
        for (ContractStore.Field f : relation.getFields()) {
            fieldTypes.put(f.getName().getSnake(), FieldType.fromContractString(f.getDtype()));
        }
        
        //Get the message object for this encoding, if it exists
        boolean foundEncoding = false;
        for(ContractStore.Message m : relation.getMessages()) {
            if(m.getEncoding().equals(encoding.name())) {
                foundEncoding = true;
                serialOrder = new String[m.getFields().size()];
                serialSet = new HashSet<String>();
                
                for(ContractStore.MessageFieldRef f : m.getFields()) {
                    serialOrder[i] = f.getName().getSnake();
                    serialSet.add(f.getName().getSnake());
                    i++;
                    
                    //Serialized types can be overridden on a per-encoding basis
                    if(!f.getType().equals("")) {
                        fieldTypes.put(f.getName().getSnake(), FieldType.fromContractString(f.getType()));
                    }
                }
            }
        }
        
        if(!foundEncoding) {
            //if we didn't find an encoding-specific message, we fall back to serializing all fields,
            //in the order they appear in the contract
            serialSet = new HashSet<String>();
            serialOrder = new String[relation.getFields().size()];
            for (ContractStore.Field f : relation.getFields()) {
                serialOrder[i] = f.getName().getSnake();
                i++;
                serialSet.add(f.getName().getSnake());
            }
        }
    }

    @Override
    public void close() {
        //don't have anything to close
    }

    @Override
    public Set<String> keySet() {
        return serialSet;
    }
    
    public String[] getOrderedKeys() {
        return serialOrder;
    }

    @Override
    public FieldType getTypeForKey(String key) {
        return fieldTypes.get(key);
    }

    @Override
    public Object get(String key) {
        return cv.get(key);
    }

    @Override
    public Boolean getAsBoolean(String key) {
        return cv.getAsBoolean(key);
    }

    @Override
    public Byte getAsByte(String key) {
        return cv.getAsByte(key);
    }

    @Override
    public byte[] getAsByteArray(String key) {
        return cv.getAsByteArray(key);
    }

    @Override
    public Double getAsDouble(String key) {
        return cv.getAsDouble(key);
    }

    @Override
    public Float getAsFloat(String key) {
        return cv.getAsFloat(key);
    }

    @Override
    public Integer getAsInteger(String key) {
        return cv.getAsInteger(key);
    }

    @Override
    public Long getAsLong(String key) {
        return cv.getAsLong(key);
    }

    @Override
    public Short getAsShort(String key) {
        return cv.getAsShort(key);
    }

    @Override
    public String getAsString(String key) {
        return cv.getAsString(key);
    }

    @Override
    public File getFile(String field) throws IOException {
        // TODO Auto-generated method stub
        return null;
    }

}
