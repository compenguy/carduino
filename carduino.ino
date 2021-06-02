// ArduinoBLE - Version: 1.2.1
#include <ArduinoBLE.h>

// Arduino_LSM6DSOX - Version: Latest
#include <Arduino_LSM6DSOX.h>

const char BLE_SVC_UUID[] = "0000FFE0-0000-1000-8000-00805F9B34FB";
//const char BLE_SVC_UUID_SHORT[] = "ffe0";
const char BLE_CHR_UUID[] = "0000FFE1-0000-1000-8000-00805F9B34FB";
//const char BLE_CHR_UUID_SHORT[] = "ffe1";

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");

    while (1);
  }
  if (!IMU.begin()) {
    Serial.println("starting IMU failed!");

    while (1);
  }

  Serial.println("BLE Central scan callback");

  // assign event handlers for peripheral discovery, connection, and disconnection
  BLE.setEventHandler(BLEDiscovered, bleCentralDiscoverHandler);
  BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
  BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);

  // start scanning for peripherals with duplicates
  BLE.scanForUuid(BLE_SVC_UUID, false);
}

void loop() {
  BLEDevice peripheral = BLE.available();
  float x = 0.0f, y = 0.0f, z = 0.0f;

  while (peripheral && peripheral.connected()) {
    BLECharacteristic bleSerial = peripheral.characteristic(BLE_CHR_UUID);
    if (!bleSerial || !bleSerial.canWrite()) {
      Serial.println("Error attaching to the serial characteristic of the remote device.");
      peripheral.disconnect();
      break;
    }
    while (peripheral.connected()) {
      if (IMU.gyroscopeAvailable()) {
        float dx, dy, dz;
        IMU.readGyroscope(dx, dy, dz);
        x += dx;
        y += dy;
        z += dz;
        // TODO: translate axial rotation into a vehicle command
        // e.g. 0
        bleSerial.writeValue((uint8_t)'f');
        // TODO: add a button or some kind of signal to reset orientation, to counteract drift
        // accelerometer is capable of detecting motion gestures - figure out how to make it respond to a
        // 'tap' or 'shake' gesture.
        // Additionally, calculate the axis along which 1G acceleration is being experienced
        // that way is down to the controller, which can help 'zero' non-z rotation,
        // for which no absolute reference exists
      }
    }
  }
}

void bleCentralDiscoverHandler(BLEDevice central) {
  // discovered a peripheral
  Serial.println("Discovered a supported peripheral");
  Serial.println("-----------------------");
  BLEDevice peripheral = BLE.available();

  if (!peripheral.connect()) {
    Serial.println("Connection failed.");
    return;
  }

  Serial.println("Success!");
  if (!peripheral.discoverAttributes()) {
    Serial.println("Failed to discover device attributes. Disconnecting...");
    peripheral.disconnect();
    return;
  }

  BLECharacteristic serialCharacteristic = peripheral.characteristic(BLE_CHR_UUID);
  if (!serialCharacteristic) {
    Serial.println("Failed to locate required device characteristic. Disconnecting...");
    peripheral.disconnect();
    return;
  }

}

void blePeripheralConnectHandler(BLEDevice central) {
  // central connected event handler
  Serial.print("Connected event, central: ");
  Serial.println(central.address());

  BLE.stopScan();
}

void blePeripheralDisconnectHandler(BLEDevice central) {
  // central disconnected event handler
  Serial.print("Disconnected event, central: ");
  Serial.println(central.address());

  BLE.scanForUuid(BLE_SVC_UUID, false);
}
