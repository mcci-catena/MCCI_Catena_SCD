/*

Module: scd30_simple.ino

Function:
        Simple example for SCD30 CO2 sensors.

Copyright and License:
        See accompanying LICENSE file.

Author:
        Terry Moore, MCCI Corporation   October 2020

*/

#include <MCCI_Catena_SCD30.h>

#include <Arduino.h>
#include <Wire.h>
#include <cstdint>

/****************************************************************************\
|
|   Manifest constants & typedefs.
|
\****************************************************************************/

using namespace McciCatenaScd30;

/****************************************************************************\
|
|   Read-only data.
|
\****************************************************************************/

#if defined(MCCI_CATENA_4801)
static constexpr bool k4801 = true;
#else
static constexpr bool k4801 = false;
#endif

/****************************************************************************\
|
|   Variables.
|
\****************************************************************************/

bool fLed;
cSCD30 gScd30 {Wire};

/****************************************************************************\
|
|   Code.
|
\****************************************************************************/

void printFailure(const char *pMessage)
    {
    for (;;)
        {
        Serial.print(pMessage);
        Serial.print(", state: ");
        Serial.print(unsigned(gScd30.getState()));
        Serial.print(", error: ");
        Serial.print(gScd30.getLastErrorName());
        Serial.print("(");
        Serial.print(std::uint8_t(gScd30.getLastError()));
        Serial.println(")");
        delay(2000);
        }
    }

void setup()
    {
    Serial.begin(115200);

    // wait for USB to be attached.
    while (! Serial)
        yield();

    Serial.println("SDP Simple Test");

    if (k4801)
        {
        /* turn on boost regulator */
        pinMode(D5, OUTPUT);
        digitalWrite(D5, 1);
    
        /* power up sensor */
        pinMode(D11, OUTPUT);
        digitalWrite(D11, 1);

        /* power up other i2c sensors */
        pinMode(D10, OUTPUT);
        digitalWrite(D10, 1);

        Serial.print("Reset reason: ");
        Serial.println(RCC->CSR >> 24, HEX);
        RCC->CSR |= RCC_CSR_RMVF;
        }

    /* bring up the LED */
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, 1);
    fLed = true;

    // let message get out.
    delay(1000);

    if (! gScd30.begin())
        {
        printFailure("gScd30.begin() failed");
        }

    Serial.print("Found sensor: firmware version ");
    auto const info = gScd30.getInfo();
    Serial.print(unsigned(info.FirmwareVersion / 256));
    Serial.print(".");
    Serial.print(unsigned(info.FirmwareVersion & 0xFFu));
    Serial.println(".");
    Serial.print("  Automatic Sensor Calibration: "); Serial.println(info.fASC_status);
    Serial.print("  Sample interval:              "); Serial.print(info.MeasurementInterval); Serial.println(" secs");
    Serial.print("  Forced Recalibration:         "); Serial.print(info.ForcedRecalibrationValue); Serial.println(" ppm");
    Serial.print("  Temperature Offset(delta-F):  "); Serial.println(info.TemperatureOffset / 100.0f * 9.0f / 5.0f);
    Serial.print("  Altitude (meters):            "); Serial.println(info.AltitudeCompensation);
    }

void loop()
    {
    // toggle the LED
    auto const secs = millis() / 1000u;
    fLed = secs & 1;
    digitalWrite(LED_BUILTIN, fLed);

    // wait for measurement to complete.
    bool fError;
    if (! gScd30.queryReady(fError))
        {
        if (fError)
            printFailure("queryReady() failed");

        return;
        }

    // get the results.
    if (! gScd30.readMeasurement())
        {
        printFailure("readMeasurement() failed");
        }

    // display the results by fetching then displaying.
    auto m = gScd30.getMeasurement();

    Serial.print("T(F)=");
    Serial.print(m.Temperature * 1.8f + 32.0f);
    Serial.print("  Humidity=");
    Serial.print(m.RelativeHumidity);
    Serial.print("%  Dewpoint(F)=");
    Serial.print(dewpoint(m.Temperature, m.RelativeHumidity) * 1.8f + 32.0f);
    Serial.print("  CO2 concentration=");
    Serial.print(m.CO2ppm);
    Serial.println(" ppm");
    }

float dewpoint(float t, float rh)
    {
    float const c1 = 243.04f;
    float const c2 = 17.625f;
    float h = rh / 100.0f;

    if (h < 0.01f)
        h = 0.01f;
    else if (h > 1.0f)
        h = 1.0f;

    float const lnh = logf(h);
    float const tpc1 = t + c1;
    float const txc2 = t * c2;
    float const txc2_tpc1 = txc2 / tpc1;

    return c1 * (lnh + txc2_tpc1) / (c2 - lnh - txc2_tpc1);
    }