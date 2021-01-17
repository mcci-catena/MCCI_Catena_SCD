# MCCI Catena&reg; Arduino Library for Sensirion Carbon Dioxide Sensors

This library provides a simple interface to Sensirion SCD30 carbon dioxide (CO2) sensors. Although we tested this using the MCCI Catena 4801, there are no dependencies on MCCI hardware; this should work equally well with Adafruit breakout boards, etc. The LoRaWAN&reg; example uses the MCCI [Catena-Arduino-Platform](https://github.com/mcci-catena/Catena-Arduino-Platform) library, but the library itself does not.

See [Meta](#meta) for information on getting sensors from MCCI.

<!-- TOC depthFrom:2 updateOnSave:true -->

- [Installation](#installation)
	- [Installing Manually With Zip](#installing-manually-with-zip)
	- [Installing with the IDE](#installing-with-the-ide)
- [Using the Library](#using-the-library)
	- [Header file](#header-file)
	- [Namespaces](#namespaces)
	- [Declare Sensor Objects](#declare-sensor-objects)
	- [Preparing for use](#preparing-for-use)
	- [Start measurements](#start-measurements)
	- [Poll results](#poll-results)
	- [Read measurement results](#read-measurement-results)
	- [Get most recent data](#get-most-recent-data)
	- [Disable continuous measurements](#disable-continuous-measurements)
	- [Set Measurement Interval](#set-measurement-interval)
	- [Enable Automatic Self-Calibration (ASC)](#enable-automatic-self-calibration-asc)
	- [Shutdown sensor (for external power down)](#shutdown-sensor-for-external-power-down)
- [Use with Catena 4801 M301](#use-with-catena-4801-m301)
- [Meta](#meta)
	- [Sensors from MCCI](#sensors-from-mcci)
	- [License](#license)
	- [Support Open Source Hardware and Software](#support-open-source-hardware-and-software)
	- [Trademarks](#trademarks)

<!-- /TOC -->

## Installation

### Installing Manually With Zip

- Download the latest ZIP version from the [release page](https://github.com/mcci-catena/MCCI_Catena_SCD30/releases)
- Either unzip manually into your Arduino/Libraries directory, or use the IDE and select `Sketch > Include Library > Add .ZIP Library`.

### Installing with the IDE

- This library is published as an official Arduino library. So you can install from the IDE using `Sketch > Include Library > Manage Libraries...` to open the library manager, then search for `MCCI` and select this library from the list.

## Using the Library

### Header file

```c++
#include <MCCI_Catena_SCD30.h>
```

### Namespaces

```c++
using namespace McciCatenaScd30;
```

### Declare Sensor Objects

```c++
// declare an instance of a sensor using TwoWire interface Wire,
// the specified address, and the specified digitial I/O for the RDY pin.
// The only defined address is:
//    cSCD30::Address::SDP30 (0x61)
// If RDY pin is not used, supply -1 as the default.
// The default for the interface is &Wire, and the default i2c_address is SDP3x_A.

cSCD30 myScd(&Wire, cSCD30::Address::SDP30, -1);
```

You need to declare one `cSCD30` instance for each sensor. (Because all SCD30s have the same I2C address, you must either use multiple `TwoWire` busses or must implement some form of multiplexing.)

### Preparing for use

Use the `begin()` method to prepare for use.

```c++
bool cSCD30::begin();
```

It returns `true` for success, `false` for failure. This

### Start measurements

```c++
bool cSCD30::startContinuousMeasurements();
bool cSCD30::startContinuousMeasurements(std::uint16_t pressure_mBar);
```

Start the sensor (possibly after stopping), and set the ambient pressure compensation

### Poll results

```c++
bool cSCD30::queryReady(bool &fHardError);
```

After starting continuous measurements, you must delay for it to become available. `queryReady()` will return `true` once a measurement is ready.

To be really safe, if this returns `false` when using this to exit a busy loop, you should `fHardError` or check the last error code. If `fHardError` is `true`, or if the last error code is not `cSCD30::Error::Busy`, then a measurement is not in progress, and the loop will never exit.

### Read measurement results

```c++
bool cSCD30::readMeasurmeent();
```

This succeeds only if `queryReady()` has returned true. It reads the measurement from the sensor into internal buffers and returns `true` if all is successful. CRCs are checked.

If it fails, the last data in internal buffers is not changed.

### Get most recent data

```c++
// return temperature in degrees Celsius.
float cSCD30::getTemperature() const;
// return CO2 concentration in parts per million.
float cSCD30::getCO2ppm() const;
// return relative humidity in percent from 0.0 to 100.0.
float cSCD30::getRelativeHumidity() const;
// return Measurement, containing Temperature, CO2ppm, and RelativeHumidity.
cSCD30::Measurement getMeasurement() const;
```

### Disable continuous measurements

```c++
bool cSCD30::stopMeasurement();
```

This routine disables continuous measurements at the sensor. You must [start measurements](#start-measurements) before you can read data again. The SCD30 stores this setting in non-volatile memory, and so this is retained through reboot.

Returns `true` for success, `false` and sets last error for failure.

### Set Measurement Interval

```c++
bool cSCD30::setMeasurementInterval(std::uint16_t interval);
```

Sets interval and updates the product info database.

Returns `true` for success, `false` and sets last error for failure.

### Enable Automatic Self-Calibration (ASC)

```c++
bool cSCD30::activateAutomaticSelfCalbration(bool fEnableIfTrue);
```

Enables/disables automatic self-calibration ("ASC") and updates the product info database. Remember, per Sensirion data sheets, if ASC is enabled, the sensor must be exposed to fresh (400 ppm CO2) air for at least an hour a day for seven days, and must be powered continually during that time.

Returns `true` for success, `false` and sets last error for failure.

### Shutdown sensor (for external power down)

```c++
void cSCD30::end();
```

This routine shuts down the library (for example, if you're powering down the sensor).
You must call `cSCD30::begin()` before using the sensor again.

## Use with Catena 4801 M301

The Catena 4801 M301 is a modified Catena 4801, with I2C brought to JP2 (and a LPWAN radio, of course).

The configuration looks like this:

   | 4801 Pin |  Label | Grove Pin   | Cable Color
   |:--------:|:------:|:-----------:|---------------
   |   JP2-1  |  +VDD  |      3      | Red
   |   JP2-2  |   SCL  |      1      | Yellow
   |   JP2-3  |   SDA  |      2      | White
   |   JP2-4  |   GND  |      4      | Black

See the example sketch, [`examples/scd30_lorawan`](examples/scd30_lorawan/README.md), for a complete example that transmits data via the network.

## Meta

### Sensors from MCCI

MCCI offers a complete test kit, the MCCI Catena 4801 M321, consisting of an SCD30-based sensor module SeeedSense, a cable, and an [MCCI Catena 4801](https://mcci.io/catena4801) M301 I2C/Modbus to LoRaWAN board, mounted on a polycarbonate base with a Lithium Manganese Dioxide battery for remote CO2 sensing without external power. We also offer accessories and enclosures to help you get started quickly, along with technical support.

If you can't find the Catena 4801 kit on the store, please contact MCCI directly, or ask a question at [portal.mcci.com](https://portal.mcci.com).

### License

This repository is released under the [MIT](./LICENSE) license. Commercial licenses are also available from MCCI Corporation.

### Support Open Source Hardware and Software

MCCI invests time and resources providing this open source code, please support MCCI and open-source hardware by purchasing products from MCCI, Adafruit and other open-source hardware/software vendors!

For information about MCCI's products, please visit [store.mcci.com](https://store.mcci.com/).

### Trademarks

MCCI and MCCI Catena are registered trademarks of MCCI Corporation. All other marks are the property of their respective owners.
