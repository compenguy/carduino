#include "ninaled.h"

// The wifinina functions send the relevant commands to the nina module over SPI
NinaLed::NinaLed(NinaPin pin) : led_pin(pin) {
	this->set_pin_mode(OUTPUT);
	// Default LED to off
	this->off();
}

void NinaLed::on() {
	// NOTE: wifinina gpio access to LEDs is active-low
	this->set_pin_status(LOW);
}

void NinaLed::off() {
	// NOTE: wifinina gpio access to LEDs is active-low
	this->set_pin_status(HIGH);
}

void NinaLed::set_pin_mode(PinMode mode) {
	WiFiDrv::pinMode(this->led_pin.get(), (uint8_t)mode);
}

void NinaLed::set_pin_status(PinStatus value) {
	WiFiDrv::digitalWrite(this->led_pin.get(), (uint8_t)value);
}

