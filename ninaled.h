#ifndef __ninaled_h__
#define __ninaled_h__

#include <WiFiNINA.h>
#include <nina_pins.h>
#include <utility/wifi_drv.h>

class NinaLed {
	public:
		NinaLed(NinaPin pin);
		virtual ~NinaLed() {};

		virtual void on();
		virtual void off();

	private:
		NinaPin led_pin;
		void set_pin_mode(PinMode mode);
		void set_pin_status(PinStatus value);
};

#endif // __ninaled_h__
