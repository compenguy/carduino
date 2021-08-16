#ifndef __bleserial_h__
#define __bleserial_h__

#include <ArduinoBLE.h>
class BLESerial {
    public:
        BLESerial() {};
        virtual ~BLESerial() {};

        int begin();
        void connect();
        int connected();
        int open();

        int writeValue(uint8_t value);

    private:
        BLECharacteristic bus;
};


#endif // __bleserial_h__
