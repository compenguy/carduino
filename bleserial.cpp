#include "bleserial.h"

//const char BLE_SVC_UUID[] = "0000FFE0-0000-1000-8000-00805F9B34FB";
//const char BLE_SVC_UUID_SHORT[] = "ffe0";
const char BLE_SVC_UUID_SHORT[] = "fff0";
//const char BLE_CHR_UUID[] = "0000FFE1-0000-1000-8000-00805F9B34FB";
const char BLE_CHR_UUID_SHORT[] = "ffe1";

// Yes, this is a hack.  If I intended to make this class general-purpose, I'd find
// a way to un-hack this and make is a class member
BLEDevice remote;

void bleCentralDiscoverHandler(BLEDevice peripheral) {
    //Serial.println("---------------------------");
    //Serial.println("Compatible peripheral found");
    //Serial.print("Name: ");
    //Serial.println(peripheral.localName());
    //Serial.print("Address: ");
    //Serial.println(peripheral.address());
    //Serial.print("Service UUID: ");
    //Serial.println(peripheral.advertisedServiceUuid());
    if (peripheral.connect()) {
        //Serial.println("Connected.");
        remote = peripheral;
    } else {
        //Serial.println("Connection failed.");
    }
}

void blePeripheralConnectHandler(BLEDevice central) {
    //Serial.println("Connected!");
    BLE.stopScan();
}

void blePeripheralDisconnectHandler(BLEDevice central) {
    //Serial.println("Disconnected!");
    BLE.scanForUuid(BLE_SVC_UUID_SHORT, false);
}

int BLESerial::begin() {
    int ret = BLE.begin();

    // assign event handlers for peripheral discovery, connection, and disconnection
    BLE.setEventHandler(BLEDiscovered, bleCentralDiscoverHandler);
    BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
    BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);

    return ret;
}

void BLESerial::connect() {
    // start scanning for peripherals with duplicates
    BLE.scanForUuid(BLE_SVC_UUID_SHORT, false);
}

int BLESerial::connected() {
    if (!remote) {
        return false;
    }
    if (!remote.connected()) {
        return false;
    }
    if (!remote.discoverAttributes()) {
        remote.disconnect();
        return false;
    }

    return true;
}

int BLESerial::open() {
    if (!connected()) {
        return false;
    }
    if (!bus) {
        bus = remote.characteristic(BLE_CHR_UUID_SHORT);
    }
    if (!bus) {
        remote.disconnect();
        return false;
    }
    if (!bus.canWrite()) {
        remote.disconnect();
        return false;
    }
    return true;
}

int BLESerial::writeValue(uint8_t value) {
    if (!open()) {
        return false;
    }
    return bus.writeValue(value);
}
