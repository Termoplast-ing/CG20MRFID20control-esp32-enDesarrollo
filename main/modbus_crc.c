#include "modbus_crc.h"

uint16_t modbus_crc16(const uint8_t *buffer, uint16_t length) {
    uint16_t crc = 0xFFFF;

    for (uint16_t pos = 0; pos < length; pos++) {
        crc ^= (uint16_t)buffer[pos];
        for (int i = 0; i < 8; i++) {
            if ((crc & 0x0001) != 0) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}