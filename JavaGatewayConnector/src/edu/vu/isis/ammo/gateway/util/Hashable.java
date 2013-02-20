package edu.vu.isis.ammo.gateway.util;

import java.util.concurrent.atomic.AtomicBoolean;

public abstract class Hashable {
    protected volatile int hashcode = 0;
    protected AtomicBoolean dirtyHashcode = new AtomicBoolean(true);
}
