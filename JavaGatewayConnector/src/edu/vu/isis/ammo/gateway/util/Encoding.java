package edu.vu.isis.ammo.gateway.util;

/**
 * Encoding is an indicator to the distributor as to how to encode/decode
 * requests. It is a wrapper around the encoding type. The custom encoding
 * is passed to the application specific coder. The name is primarily for
 * the application specific coder.
 */
public class Encoding extends Hashable {
    public enum Type {
        /** The encoding is to be as short as reasonable */
        TERSE,
        /** use the json encoding coupled with key/value blobs */
        JSON,
        /** defer the encoding to the application service */
        CUSTOM;
    }

    final private Type type;

    final private String name;

    /**
     * name is used for custom encoding, otherwise it is unnecessary.
     */
    public String name() {
        return this.name;
    }

    private Encoding(String name, Type type) {
        this.type = type;
        this.name = name;
    }

    private Encoding(Type type) {
        this.type = type;
        switch (type) {
            case JSON:
                this.name = "json";
                break;
            case TERSE:
                this.name = "terse";
                break;
            case CUSTOM:
                this.name = null;
                break;
            default:
                this.name = null;
                break;
        }
    }

    static final Encoding JSON = new Encoding(Type.JSON);
    static final Encoding TERSE = new Encoding(Type.TERSE);
    static final Encoding CUSTOM = new Encoding(Type.CUSTOM);
    static final Encoding DEFAULT = new Encoding(Type.JSON);

    public static Encoding newInstance(Type type) {
        return new Encoding(type);
    }

    public static Encoding newInstance(String typeName) {

        if (typeName.equalsIgnoreCase("json")) {
            return new Encoding(typeName, Encoding.Type.JSON);
        }
        if (typeName.equalsIgnoreCase("terse")) {
            return new Encoding(typeName, Encoding.Type.TERSE);
        }
        return new Encoding(typeName, Encoding.Type.CUSTOM);
    }

    public Type getType() {
        return this.type;
    }

    public String getPayloadSuffix() {
        switch (this.type) {
            case TERSE:
                return "";
            case JSON:
            case CUSTOM:
            default:
                return "_serial/";
        }
    }

    @Override
    public String toString() {
        return new StringBuilder().append('[').append(type.name())
                .append(':').append(this.name()).append(']').toString();
    }

    public static Encoding getInstanceByName(String encoding) {
        for (final Type type : Encoding.Type.values()) {
            if (!type.name().equalsIgnoreCase(encoding))
                continue;
            return new Encoding(type);
        }
        return null;
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj)
            return true;
        if (!(obj instanceof Encoding))
            return false;
        final Encoding that = (Encoding) obj;
        if (this.type != that.type)
            return false;
        if (HashBuilder.differ(this.name, that.name))
            return false;
        return true;
    }
}