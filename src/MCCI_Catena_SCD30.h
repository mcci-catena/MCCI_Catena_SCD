/*

Module: MCCI_Catena_SCD30.h

Function:
    Definitions for the Catena library for the Sensirion SDP sensor family.

Copyright and License:
    See accompanying LICENSE file.

Author:
    Terry Moore, MCCI Corporation   October 2020

*/

#ifndef _MCCI_CATENA_SCD30_H_
# define _MCCI_CATENA_SCD30_H_
# pragma once

#include <cstdint>
#include <Wire.h>

namespace McciCatenaScd30 {

/// create a version number for comparison
static constexpr std::uint32_t
makeVersion(
    std::uint8_t major, std::uint8_t minor, std::uint8_t patch, std::uint8_t local = 0
    )
    {
    return ((std::uint32_t)major << 24) | ((std::uint32_t)minor << 16) | ((std::uint32_t)patch << 8) | (std::uint32_t)local;
    }

/// extract major number from version
static constexpr std::uint8_t
getMajor(std::uint32_t v)
    {
    return std::uint8_t(v >> 24u);
    }

/// extract minor number from version
static constexpr std::uint8_t
getMinor(std::uint32_t v)
    {
    return std::uint8_t(v >> 16u);
    }

/// extract patch number from version
static constexpr std::uint8_t
getPatch(std::uint32_t v)
    {
    return std::uint8_t(v >> 8u);
    }

/// extract local number from version
static constexpr std::uint8_t
getLocal(std::uint32_t v)
    {
    return std::uint8_t(v);
    }

/// version of library, for use by clients in static_asserts
static constexpr std::uint32_t kVersion = makeVersion(0,2,0);

class cSCD30
    {
private:
    static constexpr bool kfDebug = false;

public:
    // the address type:
    enum class Address : std::int8_t
        {
        Error = -1,
        SCD30 = 0x61,
        };

    // the type for pin assignments, in case the ready pin is used
    using Pin_t = std::int8_t;

    // constructor
    cSCD30(TwoWire &wire, Address Address = Address::SCD30, Pin_t pinReady = -1)
        : m_wire(&wire)
        , m_address(Address)
        , m_pinReady(pinReady)
        {}

    // neither copyable nor movable
    cSCD30(const cSCD30&) = delete;
    cSCD30& operator=(const cSCD30&) = delete;
    cSCD30(const cSCD30&&) = delete;
    cSCD30& operator=(const cSCD30&&) = delete;

    // measurements, as a collection.
    struct Measurement
        {
        float CO2ppm;
        float Temperature;
        float RelativeHumidity;
        };

    // product ID info
    struct ProductInfo
        {
        std::uint16_t   FirmwareVersion;        /// Version of SCD30 firmware
        std::uint16_t   MeasurementInterval;    /// Measurement interval in seconds. Can range from 2 to 1800.
        std::uint16_t   fASC_status;            /// Flag: is ASC (automatic self-calibration) enabled?
        std::uint16_t   ForcedRecalibrationValue; /// Forced recalibration value
        std::int16_t    TemperatureOffset;      /// temperature offset in 0.01-degrees Celsius
        std::int16_t    AltitudeCompensation;   /// altitude compensation in meters
        };

    // the I2C commands
    enum class Command : std::uint16_t
        {
        // sorted in ascending numerical order.
        StartContinuousMeasurement              =   0x0036,     // 1.4.1; takes 16-bit arg (pressure)
        StopContinuosMeasurement                =   0x0104,     // 1.4.2; no argument
        GetDataReady                            =   0x0202,     // 1.4.4; no argument
        ReadMeasurement                         =   0x0300,     // 1.4.5; no argument (returns 6*3 bytes: CO2, T, RH)
        SetMeasurementInterval                  =   0x4600,     // 1.4.3; set/get measurement interval; takes 16 bit arg (seconds)
        AltitudeCompensation                    =   0x5102,     // 1.4.8; 16-bit argument (meters above sea level)
        SetForcedRecalibration                  =   0x5204,     // 1.4.6; 16-bit CO2 concentration (ppm)
        EnableAutoSelfCal                       =   0x5306,     // 1.4.6; 16-bit bool (enable/disable)
        SetTemperatureOffset                    =   0x5403,     // 1.4.7; 16-bit temperature in 0.01 deg C
        ReadFirmwareVersion                     =   0xD100,     // 1.4.9; no argument (returns 3 bytes)
        SoftReset                               =   0xD304,     // 1.4.10; no argument
        };

    // the errors
    enum class Error : std::uint8_t
        {
        Success = 0,
        NoWire,
        CommandWriteFailed,
        CommandWriteBufferFailed,
        InternalInvalidParameter,
        I2cReadShort,
        I2cReadRequest,
        I2cReadLong,
        Busy,
        NotMeasuring,
        Crc,
        Uninitialized,
        InvalidParameter,
        InternalInvalidState,
        SensorUpdateFailed,
        };

    static constexpr std::uint32_t kCommandRecoveryMs = 20; // from Sensirion sample code.
    static constexpr std::uint32_t kReadDelayMs = 3;    // delay after write to read.

    // state of the measurement enging
    enum class State : std::uint8_t
        {
        Uninitialized,      /// this->begin() has never succeeded.
        End,                /// this->begin() succeeded, followed by this->end()
        Initial,            /// initial after begin [indeterminate]
        Idle,               /// idle (not measuring)
        Triggered,          /// continuous measurement running, no data available.
        Ready,              /// continuous measurement running, data availble.
        };

private:
    // this is internal -- centralize it but require that clients call the
    // public method (which centralizes the strings and the search)
    static constexpr const char * const m_szErrorMessages =
        "Success\0"
        "NoWire\0"
        "CommandWriteFailed\0"
        "CommandWriteBufferFailed\0"
        "InternalInvalidParameter\0"
        "I2cReadShort\0"
        "I2cReadRequest\0"
        "I2cReadLong\0"
        "Busy\0"
        "NotMeasuring\0"
        "Crc\0"
        "Uninitialized\0"
        "InvalidParmaeter\0"
        "InternalInvalidState\0"
        ;

    // this is internal -- centralize it but require that clients call the
    // public method (which centralizes the strings and the search)
    static constexpr const char * const m_szStateNames =
        "Uninitialized" "\0"
        "End"           "\0"
        "Initial"       "\0"
        "Idle"          "\0"
        "Triggered"     "\0"
        "Ready"         "\0"
        ;

public:
    // the public methods
    bool begin();
    void end();
    bool startContinuousMeasurement()
        {
        return startContinuousMeasurementCommon(0);
        }
    bool startContinuousMeasurement(std::uint16_t pressure_mBar)
        {
        if (pressure_mBar < 700 || pressure_mBar > 1400)
            return setLastError(Error::InvalidParameter);
        return startContinuousMeasurementCommon(pressure_mBar);
        }
    bool stopMeasurement();
    bool setMeasurementInterval(std::uint16_t interval);
    bool queryReady(bool &fCommError);
    bool readMeasurement();
    bool activateAutomaticSelfCalbration(bool fEnableIfTrue);
    bool setForcedRecalibrationValue(std::uint16_t CO2ppm);
    bool setTemperatureOffset(std::int16_t offset_centidegrees_C);
    bool setAltitudeCompensation(std::int16_t meters);
    bool softReset();
    float getTemperature() const { return this->m_Measurement.Temperature; }
    float getCO2ppm() const { return this->m_Measurement.CO2ppm; }
    float getRelativeHumidity() const { return this->m_Measurement.CO2ppm; }
    Measurement getMeasurement() const { return this->m_Measurement; }
    // return cached copy of product information structure.
    ProductInfo getInfo() const { return this->m_ProductInfo; }
    // return cached copy of measurement interval, in ms.
    std::uint16_t getMeasurementInterval() const { return this->m_ProductInfo.MeasurementInterval; }
    Error getLastError() const
        {
        return this->m_lastError;
        }
    bool setLastError(Error e)
        {
        this->m_lastError = e;
        return e == Error::Success;
        }
    static const char *getErrorName(Error e);
    const char *getLastErrorName() const
        {
        return getErrorName(this->m_lastError);
        }
    static const char *getStateName(State s);
    const char *getCurrentStateName() const
        {
        return getStateName(this->getState());
        }
    bool readProductInfo();
    bool isRunning() const
        {
        return this->m_state > State::End;
        }
    State getState() const { return this->m_state; }
    // return millis to next measurement ready, or 0 if it should be ready now.
    std::uint32_t getMsToNextMeasurement() const
        {
        auto const now = millis();
        if (std::int32_t(this->m_tReady - now) < 0)
            return 0;
        else
            return this->m_tReady - now;
        }

protected:
    bool startContinuousMeasurementCommon(std::uint16_t param);
    bool writeCommand(Command c);
    bool writeCommand(Command c, std::uint16_t param);
    bool writeCommandBuffer(const std::uint8_t *pBuffer, size_t nBuffer);
    bool readResponse(std::uint8_t *buf, size_t nBuf);
    bool readFirmwareVersion(std::uint16_t &version);
    bool readMeasurementInterval(std::uint16_t &interval);
    bool readAutoSelfCalibration(std::uint16_t &flag);
    bool readForcedRecalibrationValue(std::uint16_t &CO2ppm);
    bool readTemperatureOffset(std::int16_t &offsetCentiCel);
    bool readAltitudeCompensation(std::int16_t &meters);
    bool readDataReadyStatus(std::uint16_t &flag);
    bool readUint16(Command c, std::uint16_t &value);
    static std::uint8_t crc(const std::uint8_t *buf, size_t nBuf, std::uint8_t crc8 = 0xFF);
    bool crc_multi(const std::uint8_t *buf, size_t nBuf);
    std::int8_t getAddress() const
        { return static_cast<std::int8_t>(this->m_address); }
    bool checkRunning()
        {
        if (! this->isRunning())
            return this->setLastError(Error::Uninitialized);
        else
            return true;
        }

// following are arranged for alignment.
private:
    Measurement m_Measurement;      /// most recent measurement
    TwoWire *m_wire;                /// pointer to bus to be used for this device
    std::uint32_t m_tReady;         /// estimated time next measurement will be ready (millis)
    ProductInfo m_ProductInfo;      /// product information read from device
    Address m_address;              /// I2C address to be used
    Pin_t m_pinReady;               /// alert pin, or -1 if none.
    Error m_lastError;              /// last error.
    State m_state                   /// current state
        { State::Uninitialized };   // initially not yet started.

    static constexpr std::uint16_t getUint16BE(const std::uint8_t *p)
        {
        return (p[0] << 8) + p[1];
        }
    static constexpr std::int16_t getInt16BE(const std::uint8_t *p)
        {
        return std::int16_t((p[0] << 8) + p[1]);
        }
    static float getFloat32BE(const std::uint8_t *p);
   };

} // namespace McciCatenaScd30

#endif // _MCCI_CATENA_SCD30_H_
