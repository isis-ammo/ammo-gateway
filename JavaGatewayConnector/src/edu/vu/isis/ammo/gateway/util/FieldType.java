package edu.vu.isis.ammo.gateway.util;

import java.util.HashMap;

public enum FieldType {
    /** no value specified */
    NULL(0),
    /** true or false */
    BOOL(1),
    /** Binary large object but not a file */
    BLOB(2),
    /** an approximation to a real number */
    FLOAT(3),
    /** either positive or negative integer */
    INTEGER(4),
    /** when an integer is not big enough */
    LONG(5),
    /** text string */
    TEXT(6),
    /** prefer over float */
    REAL(7),
    /** a foreign key reference */
    FK(8),
    /** a globally unique identifier */
    GUID(9),
    /** a single item from a set */
    EXCLUSIVE(10),
    /** a subset */
    INCLUSIVE(11),
    /** an integer timestamp in milliseconds */
    TIMESTAMP(12),
    /** a two byte integer */
    SHORT(13),
    /** the contents of a file, a named blob */
    FILE(14);

    private final int code;

    private FieldType(int code) {
        this.code = code;
    }

    //TODO: if needed, reimplement this using desktop Java classes
    /*private static final SparseArray<FieldType> codemap = new SparseArray<FieldType>();
    static {
        for (FieldType t : FieldType.values()) {
            FieldType.codemap.put(t.code, t);
        }
    }*/

    private static final HashMap<String, FieldType> stringMap = new HashMap<String, FieldType>();
    static {
        stringMap.put("NULL", NULL);
        stringMap.put("BOOL", BOOL);
        stringMap.put("BLOB", BLOB);
        stringMap.put("FLOAT", FLOAT);
        stringMap.put("INTEGER", INTEGER);
        stringMap.put("LONG", LONG);
        stringMap.put("TEXT", TEXT);
        stringMap.put("REAL", REAL);
        stringMap.put("FK", FK);
        stringMap.put("GUID", GUID);
        stringMap.put("EXCLUSIVE", EXCLUSIVE);
        stringMap.put("INCLUSIVE", INCLUSIVE);
        stringMap.put("TIMESTAMP", TIMESTAMP);
        stringMap.put("SHORT", SHORT);
        stringMap.put("FILE", FILE);
    }

    public int toCode() {
        return this.code;
    }

    //TODO: if needed, reimplement this using desktop Java classes
    /*public static FieldType fromCode(final int code) {
        return FieldType.codemap.get(code);
    }*/

    public static FieldType fromContractString(String dtype) {
        return stringMap.get(dtype);
    }
}
