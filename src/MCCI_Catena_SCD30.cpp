/*

Module:	MCCI_Catena_SCD30.cpp

Function:
    Implementation code for MCCI Sensirion CO2 sensor library

Copyright and License:
    This file copyright (C) 2020 by

        MCCI Corporation
        3520 Krums Corners Road
        Ithaca, NY  14850

    See accompanying LICENSE file for copyright and license information.

Author:
    Terry Moore, MCCI Corporation	October 2020

*/

#include "MCCI_Catena_SCD30.h"

using namespace McciCatenaScd30;

bool cSCD30::begin()
    {
    // if no Wire is bound, fail.
    if (this->m_wire == nullptr)
        return this->setLastError(Error::NoWire);

    if (this->isRunning())
        return true;

    this->m_wire->begin();
    // assume it's in idle state.
    this->m_state = this->m_state == State::End ? State::Triggered : State::Initial;
    bool result = this->readProductInfo();
    if (result)
        {
        this->m_tReady = millis() + this->m_ProductInfo.MeasurementInterval * 1000;
        if (this->m_state == State::Initial)
            // allow an extra 500 ms in initial, then we'll force a start.
            this->m_tReady += 500;
        }
    else
        {
        // we failed to get the product info
        this->m_state = State::Uninitialized;
        }
    return result;
    }

void cSCD30::end()
    {
    if (this->isRunning())
        this->m_state = State::End;
    }

bool cSCD30::readProductInfo()
    {
    if (! this->checkRunning())
        return false;

    ProductInfo info;
    if (! this->readFirmwareVersion(info.FirmwareVersion))
        return false;

    if (! this->readMeasurementInterval(info.MeasurementInterval))
        return false;

    if (! this->readAutoSelfCalibration(info.fASC_status))
        return false;

    if (! this->readForcedRecalibrationValue(info.ForcedRecalibrationValue))
        return false;

    if (! this->readTemperatureOffset(info.TemperatureOffset))
        return false;

    if (! this->readAltitudeCompensation(info.AltitudeCompensation))
        return false;

    this->m_ProductInfo = info;
    return true;
    }

bool
cSCD30::readFirmwareVersion(
    std::uint16_t &version
    )
    {
    return this->readUint16(Command::ReadFirmwareVersion, version);
    }

bool
cSCD30::readMeasurementInterval(
    std::uint16_t &seconds
    )
    {
    return this->readUint16(Command::SetMeasurementInterval, seconds);
    }

bool
cSCD30::readAutoSelfCalibration(
    std::uint16_t &fEnabled
    )
    {
    return this->readUint16(Command::EnableAutoSelfCal, fEnabled);
    }

bool
cSCD30::readForcedRecalibrationValue(
    std::uint16_t &CO2ppm
    )
    {
    return this->readUint16(Command::SetForcedRecalibration, CO2ppm);
    }

bool
cSCD30::readTemperatureOffset(
    std::int16_t &offsetCentiCel
    )
    {
    std::uint16_t nonce;
    bool result;

    result = this->readUint16(Command::SetTemperatureOffset, nonce);

    // no need to check result; readUint16 sets nonce to zero on error.
    offsetCentiCel = (std::int16_t) nonce;

    return result;
    }

bool
cSCD30::readAltitudeCompensation(
    std::int16_t &meters
    )
    {
    std::uint16_t nonce;
    bool result;

    result = this->readUint16(Command::AltitudeCompensation, nonce);

    // no need to check result; readUint16 sets nonce to zero on error.
    meters = (std::int16_t) nonce;

    return result;
    }

bool
cSCD30::readDataReadyStatus(
    std::uint16_t &flag
    )
    {
    // Serial.print(">");
    return this->readUint16(Command::GetDataReady, flag);
    }

/*

Name:	cSCD30::readUint16()

Function:
    Read a uint16_t from an SCD30 sensor.

Definition:
    bool cSCD30::readUint16(
        cSCD30::Command cmd,
        std::uint16_t &value
        );

Description:
    The given command is issued to the sensor. If successful, the method
    delays 3 milliseconds, then reads 3 bytes, checking CRC. If further
    successful, the method interprets the value read as a big-endian
    16-bit value.

Returns:
    This function returns `true` if the value was successfully
    fetched. Otherwise, it returns `false` and sets the last error
    code appropriately.

Notes:
    If an error occurs, the value is set to zero.

*/

bool
cSCD30::readUint16(
    cSCD30::Command cmd,
    std::uint16_t &value
    )
    {
    bool result;
    std::uint8_t buf[3];

    result = this->checkRunning();

    if (result)
        result = this->writeCommand(cmd);

    if (result)
        {
        delay(3);

        result = this->readResponse(buf, sizeof(buf));
        }

    if (result)
        {
        value = this->getUint16BE(buf);
        }
    else
        {
        value = 0;
        }

    return result;
    }

bool cSCD30::setMeasurementInterval(std::uint16_t interval)
    {
    if (interval < 2)
        return this->setLastError(Error::InvalidParameter);

    bool result = this->checkRunning();
    if (result)
        {
        result = this->writeCommand(Command::SetMeasurementInterval, interval);
        }
    if (result)
        {
        std::uint16_t nonce;
        result = this->readMeasurementInterval(nonce);
        if (result)
            this->m_ProductInfo.MeasurementInterval = nonce;
        }
    return result;
    }

bool cSCD30::activateAutomaticSelfCalbration(bool fEnableIfTrue)
    {
    bool result = this->checkRunning();
    if (result)
        {
        result = this->writeCommand(Command::EnableAutoSelfCal, fEnableIfTrue);
        }
    if (result)
        {
        std::uint16_t nonce;
        result = this->readAutoSelfCalibration(nonce);
        if (result)
            this->m_ProductInfo.fASC_status = nonce;
        }
    return result;
    }

bool cSCD30::writeCommand(cSCD30::Command command)
    {
    const std::uint8_t cbuf[2] = { std::uint8_t(std::uint16_t(command) >> 8), std::uint8_t(command) };

    return this->writeCommandBuffer(cbuf, sizeof(cbuf));
    }

bool cSCD30::writeCommand(cSCD30::Command command, std::uint16_t param)
    {
    std::uint8_t cbuf[5] =
        {
        std::uint8_t(std::uint16_t(command) >> 8), std::uint8_t(command),
        std::uint8_t(std::uint16_t(param) >> 8), std::uint8_t(param),
        /* crc */ 0
        };

    cbuf[4] = this->crc(&cbuf[2], 2);
    return this->writeCommandBuffer(cbuf, sizeof(cbuf));
    }

bool cSCD30::writeCommandBuffer(const std::uint8_t *pBuffer, size_t nBuffer)
    {
    this->m_wire->beginTransmission(std::uint8_t(this->m_address));
    if (this->m_wire->write(pBuffer, nBuffer) != nBuffer)
        return this->setLastError(Error::CommandWriteBufferFailed);
    if (this->m_wire->endTransmission() != 0)
        return this->setLastError(Error::CommandWriteFailed);

    return true;
    }

bool cSCD30::readResponse(std::uint8_t *buf, size_t nBuf)
    {
    bool ok;
    unsigned nResult;
    const std::int8_t addr = this->getAddress();
    uint8_t nReadFrom;

    if (buf == nullptr || nBuf > 30 || addr < 0 ||
        ((011111111111u >> nBuf) & 1u) == 0 /* not a multiple of 3 */)
        {
        return this->setLastError(Error::InternalInvalidParameter);
        }

    nReadFrom = this->m_wire->requestFrom(std::uint8_t(addr), /* bytes */ std::uint8_t(nBuf));

    if (nReadFrom != nBuf)
        {
        return this->setLastError(Error::I2cReadRequest);
        }
    nResult = this->m_wire->available();
    if (nResult > nBuf)
        return this->setLastError(Error::I2cReadLong);

    for (unsigned i = 0; i < nResult; ++i)
        buf[i] = this->m_wire->read();

    if (nResult != nBuf)
        {
        return this->setLastError(Error::I2cReadShort);
        }

    // finally, check the CRC on each 3-byte tuple
    return this->crc_multi(buf, nBuf);
    }

bool cSCD30::startContinuousMeasurementCommon(std::uint16_t param)
    {
    if (! this->checkRunning())
        return false;

    bool result = this->writeCommand(Command::StartContinuousMeasurement);

    if (result)
        {
        this->m_state = State::Triggered;
        this->m_tReady = millis() + this->m_ProductInfo.MeasurementInterval * 1000;
        }

    return result;
    }

bool cSCD30::queryReady(bool &fError)
    {
    if (! checkRunning())
        {
        fError = true;
        return false;
        }

    if (this->m_state == State::Ready)
        {
        fError = false;
        return true;
        }

    if (this->m_state == State::Idle)
        {
        fError = true;
        return this->setLastError(Error::NotMeasuring);
        }

    if ((std::int32_t)(millis() - this->m_tReady) < 0)
        {
        fError = false;
        return this->setLastError(Error::Busy);
        }

    if (this->m_state == State::Triggered ||
        this->m_state == State::Initial)
        {
        std::uint16_t flag;

        if (! this->readDataReadyStatus(flag))
            {
            // could not read data-ready status
            fError = true;
            this->m_tReady = millis() + 1000;
            return false;
            }

        if (flag)
            {
            // observe that there's data available
            this->m_state = State::Ready;
            fError = false;
            return true;
            }
        else if (this->m_state == State::Initial)
            {
            // send the start command
            if (! this->startContinuousMeasurement())
                {
                // start command failed.
                fError = true;
                this->m_tReady = millis() + 1000;
                return false;
                }

            // need to wait.
            fError = false; // no error
            return this->setLastError(Error::Busy);   // but not ready.
            }
        else
            {
            fError = false;
            this->m_tReady = millis() + 100;
            return this->setLastError(Error::Busy);   // but not ready.
            }
        }

    // we've observed data
    if (this->m_state == State::Ready)
        {
        fError = false;
        return true;
        }

    // internal error
    fError = true;
    return this->setLastError(Error::InternalInvalidState);
    }

bool cSCD30::readMeasurement()
    {
    bool fNonce;
    if (! this->queryReady(fNonce))
        return false;

    if (! this->writeCommand(Command::ReadMeasurement))
        return false;

    this->m_state = State::Triggered;
    this->m_tReady = millis() + this->m_ProductInfo.MeasurementInterval * 1000;
    delay(3);

    std::uint8_t measurementBuffer[(4 + 2) * 3];
    bool result = this->readResponse(measurementBuffer, sizeof(measurementBuffer));

    if (result)
        {
        Measurement m;
        m.CO2ppm = getFloat32BE(&measurementBuffer[0]);
        m.Temperature = getFloat32BE(&measurementBuffer[6]);
        m.RelativeHumidity = getFloat32BE(&measurementBuffer[12]);

        this->m_Measurement = m;
        }

    return result;
    }

std::uint8_t cSCD30::crc(const std::uint8_t * buf, size_t nBuf, std::uint8_t crc8)
    {
    /* see cSHT3x CRC-8-Calc.md for a little info on this */
    static const std::uint8_t crcTable[16] =
        {
        0x00, 0x31, 0x62, 0x53, 0xc4, 0xf5, 0xa6, 0x97,
        0xb9, 0x88, 0xdb, 0xea, 0x7d, 0x4c, 0x1f, 0x2e,
        };

    for (size_t i = nBuf; i > 0; --i, ++buf)
        {
        uint8_t b, p;

        // calculate first nibble
        b = *buf;
        p = (b ^ crc8) >> 4;
        crc8 = (crc8 << 4) ^ crcTable[p];

        // calculate second nibble
        // this could be written as:
        //      b <<= 4;
        //      p = (b ^ crc8) >> 4;
        // but it's more effective as:
        p = ((crc8 >> 4) ^ b) & 0xF;
        crc8 = (crc8 << 4) ^ crcTable[p];
        }

    return crc8;
    }

bool cSCD30::crc_multi(const std::uint8_t *buf, size_t nbuf)
    {
    if (buf == nullptr)
        return this->setLastError(Error::InternalInvalidParameter);

    for (; nbuf >= 3; buf += 3, nbuf -= 3)
        {
        if (this->crc(buf, 2) != buf[2])
            return this->setLastError(Error::Crc);
        }

    if (nbuf != 0)
        return this->setLastError(Error::InternalInvalidParameter);

    return true;
    }

const char * cSCD30::getErrorName(cSCD30::Error e)
    {
    auto p = m_szErrorMessages;

    // iterate based on error index.
    for (unsigned eIndex = unsigned(e); eIndex > 0; --eIndex)
        {
        // stop when we get to empty string
        if (*p == '\0')
            break;
        // otherwise, skip this one.
        p += strlen(p) + 1;
        }

    // if we have a valid string, return it
    if (*p != '\0')
        return p;
    // otherwise indicate that the input wasn't valid.
    else
        return "<<unknown>>";
    }

float cSCD30::getFloat32BE(const std::uint8_t *p)
    {
    union
        {
        std::uint32_t uint;
        float f;
        } v;

    // collect the 32-bit value, skipping CRC
    v.uint = (p[0] << 24) | (p[1] << 16) | (p[3] << 8) | (p[4] << 0);

    // filter out NAN.
    if ((v.uint & 0x7F800000u) == 0x7F800000u)
        return 0.0f;            // map NANs to zero.

    // eliminate denormalized numbers
    if ((v.uint & 0x7F800000u) == 0u)
        v.uint &= 0xFF800000u;  // map denorms to +/- zero.

    // return the value interpreted as a float.
    return v.f;
    }
