package edu.vu.isis.ammo.gateway;

public enum MessagePriority {
    PRIORITY_AUTH(127),
	PRIORITY_CTRL(112),
	PRIORITY_FLASH(96),
	PRIORITY_URGENT(64),
	PRIORITY_IMPORTANT(32),
	PRIORITY_NORMAL(0),
	PRIORITY_BACKGROUND(-32);

    private int value;
    private MessagePriority(int value) {
	this.value = value;
    }
    
    public int getValue() {
	return this.value;
    }
}