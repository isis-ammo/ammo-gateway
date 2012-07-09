package edu.vu.isis.ammo.gateway;

public enum PushStatus {
    PUSH_RECEIVED(0),
	PUSH_SUCCESS(1),
	PUSH_FAIL(2),
	PUSH_REJECTED(3);

    private int value;
    private PushStatus(int value) {
	this.value = value;
    }

}