/*!
 * @file Notecard.h
 *
 * The note-arduino Arduino library for communicating with the
 * <a href="https://blues.io">Blues Wireless</a>
 * Notecard via serial or I2C.
 *
 * This library allows you to control a Notecard by writing an Arduino sketch in
 * C or C++. Your sketch may programmatically configure Notecard and send Notes
 * to <a href="https://notehub.io">Notehub.io</a>.
 *
 * This library is a wrapper around the
 * <a href="https://github.com/blues/note-c">note-c library</a>, which it
 * includes as a git submodule.
 *
 * Written by Ray Ozzie and Brandon Satrom for Blues Inc.
 *
 * MIT License. Use of this source code is governed by licenses granted by the
 * copyright holder including that found in the
 * <a href="https://github.com/blues/note-arduino/blob/master/LICENSE">LICENSE</a>
 * file.
 *
 */
#ifndef Notecard_h
#define Notecard_h

#include <stddef.h>
#include <stdint.h>
#ifndef MOCK
#include <Arduino.h>
#include <HardwareSerial.h>
#include <Wire.h>
#include <note-c/note.h>
#else
#include "mock/mock-arduino.hpp"
#include "mock/mock-parameters.hpp"
#endif

#ifdef ARDUINO_ARCH_ESP32

#ifdef B0
#undef B0
#endif
#define B0 21

#ifdef D5
#undef D5
#endif
#define D5 14

#ifdef D6
#undef D6
#endif
#define D6 32

#ifdef D9
#undef D9
#endif
#define D9 15

#ifdef D10
#undef D10
#endif
#define D10 33

#ifdef D11
#undef D11
#endif
#define D11 27

#ifdef D12
#undef D12
#endif
#define D12 12

#ifdef D13
#undef D13
#endif
#define D13 13

#else

#ifndef D5
#define D5 5
#endif

#ifndef D6
#define D6 6
#endif

#ifndef D9
#define D9 9
#endif

#ifndef D10
#define D10 10
#endif

#ifndef D11
#define D11 11
#endif

#ifndef D12
#define D12 12
#endif

#ifndef D13
#define D13 13
#endif

#endif

/**************************************************************************/
/*!
    @brief  Class that stores state and functions for interacting with the
    Blues Wireless Notecard.
*/
/**************************************************************************/
class Notecard
{
public:
    void begin(uint32_t i2cAddress = NOTE_I2C_ADDR_DEFAULT,
               uint32_t i2cMax = NOTE_I2C_MAX_DEFAULT,
               TwoWire &wirePort = Wire);
    void begin(HardwareSerial &serial, int speed = 9600);
    void setDebugOutputStream(Stream &dbgserial);
    void clearDebugOutputStream(void);
    void i2cTest(int Adjustment);
    J *newRequest(const char *request);
    bool sendRequest(J *req);
    J *requestAndResponse(J *req);
    void deleteResponse(J *rsp);
    void logDebug(const char *message);
    void logDebugf(const char *format, ...);
    bool debugSyncStatus(int pollFrequencyMs, int maxLevel);
    bool responseError(J *rsp);

private:
    static TwoWire *_i2cPort;
    static HardwareSerial *_notecardSerial;
    static int _notecardSerialSpeed;
    static Stream *_debugSerial;
    static bool _debugSerialInitialized;

    static bool noteSerialReset(void);
    static void noteSerialTransmit(uint8_t *text, size_t len, bool flush);
    static bool noteSerialAvailable(void);
    static char noteSerialReceive(void);
    static bool noteI2CReset(uint16_t DevAddress);
    static const char *noteI2CTransmit(uint16_t DevAddress, uint8_t* pBuffer, uint16_t Size);
    static const char *noteI2CReceive(uint16_t DevAddress, uint8_t* pBuffer, uint16_t Size, uint32_t *avail);
    static size_t debugSerialOutput(const char *message);
};

#endif
