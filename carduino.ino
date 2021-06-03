// ArduinoBLE - Version: Latest
#include <ArduinoBLE.h>

// WiFiNINA - Version: Latest
#include <WiFiNINA.h>

// Arduino_LSM6DSOX - Version: Latest
#include <Arduino_LSM6DSOX.h>

//const char BLE_SVC_UUID[] = "0000FFE0-0000-1000-8000-00805F9B34FB";
//const char BLE_SVC_UUID_SHORT[] = "ffe0";
const char BLE_SVC_UUID_SHORT[] = "fff0";
//const char BLE_CHR_UUID[] = "0000FFE1-0000-1000-8000-00805F9B34FB";
const char BLE_CHR_UUID_SHORT[] = "ffe1";

BLEDevice car;

// The wifinina functions send the relevant commands to the nina module over SPI
void wifinina_pin_mode(NinaPin pin, PinMode mode) {
	WiFiDrv::pinMode(static_cast<uint8_t>(pin), static_cast<uint8_t>(mode));
}

void wifinina_digital_write(NinaPin pin, PinStatus value) {
	WiFiDrv::digitalWrite(static_cast<uint8_t>(pin), static_cast<uint8_t>(value));
}

void setup() {
	wifinina_pin_mode(LEDR, OUTPUT);
	wifinina_pin_mode(LEDG, OUTPUT);
	wifinina_pin_mode(LEDB, OUTPUT);

	// NOTE: wifinina gpio access to LEDs is active-low
	wifinina_digital_write(LEDR, LOW);
	wifinina_digital_write(LEDB, HIGH);
	wifinina_digital_write(LEDG, HIGH);

	Serial.begin(9600);
	while (!Serial);

	// begin initialization
	if (!BLE.begin()) {
		Serial.println("starting BLE failed!");

		while (1);
	}
	Serial.print("BLE interface initialized with address ");
	Serial.println(BLE.address());

	if (!IMU.begin()) {
		Serial.println("starting IMU failed!");

		while (1);
	}

	wifinina_digital_write(LEDR, HIGH);
	wifinina_digital_write(LEDB, LOW);

	Serial.println("BLE Central registering device callbacks");

	// assign event handlers for peripheral discovery, connection, and disconnection
	BLE.setEventHandler(BLEDiscovered, bleCentralDiscoverHandler);
	BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
	BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);

	Serial.println("Scanning for supported device(s)");

	// start scanning for peripherals with duplicates
	BLE.scanForUuid(BLE_SVC_UUID_SHORT, false);

	Serial.println("Setup completed.");
}

void loop() {
	float x = 0.0f, y = 0.0f, z = 0.0f;

	while (car && car.connected()) {
		wifinina_digital_write(LEDB, HIGH);
		wifinina_digital_write(LEDG, LOW);

		Serial.println("Connected to remote device.");
		if (!car.discoverAttributes()) {
			Serial.println("Attribute discovery failed.");
			car.disconnect();
			break;
		}
		BLECharacteristic bleSerial = car.characteristic(BLE_CHR_UUID_SHORT);
		if (!bleSerial || !bleSerial.canWrite()) {
			Serial.println("Error attaching to the serial characteristic of the remote device.");
			car.disconnect();
			break;
		}
		while (car.connected()) {
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
	if (car) {
		wifinina_digital_write(LEDB, LOW);
		wifinina_digital_write(LEDG, HIGH);
	} else {
		Serial.println("Sleeping while waiting for a connection...");
		delay(1000);
		BLE.scanForUuid(BLE_SVC_UUID_SHORT, false);
	}
}

void bleCentralDiscoverHandler(BLEDevice peripheral) {
	Serial.println("---------------------------");
	Serial.println("Compatible peripheral found");
	Serial.print("Name: ");
	Serial.println(peripheral.localName());
	Serial.print("Address: ");
	Serial.println(peripheral.address());
	Serial.print("Service UUID: ");
	Serial.println(peripheral.advertisedServiceUuid());
	if (peripheral.connect()) {
		Serial.println("Connected.");
		car = peripheral;
	} else {
		Serial.println("Connection failed.");
	}
}

void blePeripheralConnectHandler(BLEDevice central) {
	Serial.println("Connected!");
	BLE.stopScan();
}

void blePeripheralDisconnectHandler(BLEDevice central) {
	Serial.println("Disconnected!");
	BLE.scanForUuid(BLE_SVC_UUID_SHORT, false);
}
