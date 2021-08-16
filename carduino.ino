// ArduinoBLE - Version: Latest
// WiFiNINA - Version: Latest
#include <WiFiNINA.h>
#include <utility/wifi_drv.h>

// Arduino_LSM6DSOX - Version: Latest
#include <Arduino_LSM6DSOX.h>

#include "bleserial.h"

BLESerial car;

// The wifinina functions send the relevant commands to the nina module over SPI
void wifinina_pin_mode(NinaPin pin, PinMode mode) {
	WiFiDrv::pinMode(pin.get(), (uint8_t)mode);
}

void wifinina_digital_write(NinaPin pin, PinStatus value) {
	WiFiDrv::digitalWrite(pin.get(), (uint8_t)value);
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
	// It's ok if we can't initialize serial - it's only needed for debug logging
	//while (!Serial);

	// begin initialization
	if (!car.begin()) {
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

	Serial.println("Scanning for supported device(s)");

	// start scanning for peripherals with duplicates
	car.connect();

	Serial.println("Setup completed.");
}

void loop() {
	float x = 0.0f, y = 0.0f, z = 0.0f;

	while (car.connected()) {
		uint32_t count = 0;
		wifinina_digital_write(LEDB, HIGH);
		wifinina_digital_write(LEDG, LOW);

		Serial.println("Connected to remote device.");
        if (!car.open()) {
			Serial.println("Error attaching to the serial characteristic of the remote device.");
			break;
		}
		while (car.connected()) {
			if (IMU.gyroscopeAvailable()) {
				float dx, dy, dz;
				char dxbuf[8], xbuf[8], dybuf[8], ybuf[8], dzbuf[8], zbuf[8];
				char logbuf[56];
				IMU.readGyroscope(dx, dy, dz);
				x += dx;
				y += dy;
				z += dz;
				if ((count % 25) == 0) {
					snprintf(logbuf, sizeof(logbuf), "%7s %7s %7s %7s %7s %7s", "dx", "x", "dy", "y", "dz", "z");
					Serial.println(logbuf);
				}
				snprintf(dxbuf, sizeof(dxbuf), "%+02d.%03d", (int)dx, abs(((int)(dx*100))%100));
				snprintf(dybuf, sizeof(dybuf), "%+02d.%03d", (int)dy, abs(((int)(dy*100))%100));
				snprintf(dzbuf, sizeof(dzbuf), "%+02d.%03d", (int)dz, abs(((int)(dz*100))%100));
				snprintf(xbuf, sizeof(xbuf), "%+04d.%01d", (int)x, abs(((int)(x*10))%10));
				snprintf(ybuf, sizeof(ybuf), "%+04d.%01d", (int)y, abs(((int)(y*10))%10));
				snprintf(zbuf, sizeof(zbuf), "%+04d.%01d", (int)z, abs(((int)(z*10))%10));
				snprintf(logbuf, sizeof(logbuf), "%7s %7s %7s %7s %7s %7s", dxbuf, xbuf, dybuf, ybuf, dzbuf, zbuf);
				Serial.println(logbuf);
				translateInput(x, dx, y, dy, z, dz);
			}
			count++;
		}
	}
	if (car.connected()) {
		wifinina_digital_write(LEDB, LOW);
		wifinina_digital_write(LEDG, HIGH);
	} else {
		Serial.println("Sleeping while waiting for a connection...");
		delay(1000);
        car.connect();
	}
}

void translateInput(float &x, float &dx, float &y, float &dy, float &z, float &dz) {
	// TODO: translate axial rotation into a vehicle command
	// e.g. 0
	car.writeValue((uint8_t)'f');
	// TODO: add a button or some kind of signal to reset orientation, to counteract drift
	// accelerometer is capable of detecting motion gestures - figure out how to make it respond to a
	// 'tap' or 'shake' gesture.
	// Additionally, calculate the axis along which 1G acceleration is being experienced
	// that way is down to the controller, which can help 'zero' non-z rotation,
	// for which no absolute reference exists
}
