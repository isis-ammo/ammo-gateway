package edu.vu.isis.ammo.gateway;

enum MessageScope {
    SCOPE_GLOBAL(0),
    SCOPE_LOCAL(1);

    private int value;
    private MessageScope(int value) {
	this.value = value;
    }

}