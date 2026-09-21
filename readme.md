# RTC_I2C

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)

[![Commits since latest](https://img.shields.io/github/commits-since/felias-fogg/RTC_I2C/latest)](https://github.com/felias-fogg/RTC_I2C/commits/master)

![Hit Counter](https://visitor-badge.laobi.icu/badge?page_id=felias-fogg_RTC_I2C)

## A universal API for I2C RTCs

This library provides a universal API to a number of I2C RTCs, which are described in one of my [blog articles](https://hinterm-ziel.de/index.php/2023/02/24/everything-you-wanted-to-know-about-real-time-clocks-but-were-afraid-to-ask/):

* [Analog Devices DS1307](https://www.analog.com/media/en/technical-documentation/data-sheets/DS1307.pdf)
* [Analog Devices DS1337](https://www.analog.com/media/en/technical-documentation/data-sheets/DS1337-DS1337C.pdf)
* [Analog Devices DS3231](https://www.analog.com/media/en/technical-documentation/data-sheets/DS3231.pdf)
* [Microchip MCP79410](https://www.microchip.com/en-us/product/mcp79410)
* [NXP Semiconductors PCF8523](https://www.nxp.com/docs/en/data-sheet/PCF8523.pdf)
* [NXP Semiconductors PCF8563](https://www.nxp.com/docs/en/data-sheet/PCF8563.pdf)
* [Ricoh RS5C372](https://www.datasheetarchive.com/?q=RS5C372A)
* [Micro Crystal RV-3028](https://www.microcrystal.com/en/news/news-detail/press-release-rv-3028-c7)
* [Micro Crystal RV-3032](https://www.microcrystal.com/fileadmin/Media/Products/RTC/Datasheet/RV-3032-C7.pdf)
* [Micro Crystal RV-8523](https://www.microcrystal.com/fileadmin/Media/Products/RTC/Datasheet/RV-8523-C3.pdf)
* [Epson RV-8803](https://www.microcrystal.com/en/products/real-time-clock-rtc-modules/rv-8803-c7)
* [DFRobot SD2405](https://www.digikey.com/htmldatasheets/production/2382573/0/0/1/dfr0469.pdf)

The interface is minimal, but is just enough to get the basic functionality. And it is provided by a base class. This means you can even use many RTCs in parallel without caring about what particular model you use. I tried to identify the least common denominator of the functionality. However, I wanted to have some kind of alarm, and being able to make use of an offset register and read the temperature, if possible. Unfortunately, one RTC does not have alarms at all, one RTC does not allow recurring alarms, a few RTCs do not have an offset register, and many do not possess the ability to read out the temperature.

In order to check at runtime what capabilities an RTC has, the method `getCapabilities()` will be useful (see table entry below). The list of capabilities and at which pins you can sense the different signals appears in the [RTC capabilities section](#capa).

In addition, one can, of course, extend each particular RTC class with methods catering for the particular RTC model, if one wishes to do so.

## Methods of the RTC class

| Name              | Parameters                                                                       | Comments                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           |
| ----------------- | -------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `begin`           | optional `TwoWire` instance                                                      | Needs to be called in the beginning in order to initialize the I2C bus. The optional parameter can be used if you want to pass a Wire instance different from the default one. It is checked whether the RTC can be addressed on the I2C bus. Returns `false` if unsuccessful, otherwise `true`.                                                                                                                                                                                                                                   |
| `init`            | optional `byte` switch-mode (default 1)                                          | Restores state of clock to configuration after power-up. It sets 24 hour mode, disables all clock outputs and alarms, and it will also start the clock (if not already running) and clear all invalid flags. If switch-mode=0, then no switching from regular power supply to backup is enabled, if switch-mode=1, then level switching is enabled, which draws more current than the next mode. If switch-mode=2, then direct switching is enabled. The latter should only be used when Vcc and VBat are significantly different. |
| `isValid`         | none                                                                             | Returns `true` if the clock is valid and running, otherwise `false`.                                                                                                                                                                                                                                                                                                                                                                                                                                                               |
| `setTime`         | Unix `time_t` value                                                              | Sets the RTC time from provided parameter. Will use the 24 hour format.                                                                                                                                                                                                                                                                                                                                                                                                                                                            |
| `setTime`         | `tm` record                                                                      | Sets the RTC time from a `tm` structure. The `tm_year` field is the number of years since 1900, and `tm_mon` uses zero-based month numbering (0-11, where 0 is January).                                                                                                                                                                                                                                                                                                                                                                                                 |
| `getTime`         | optional parameter `bool` blocking (default `false`)                             | Returns the current time of the RTC as a Unix `time_t` value. If `blocking=true`, then the method waits until the next second has just started.                                                                                                                                                                                                                                                                                                                                                                                    |
| `getTime`         | `&tm` record, optional parameter `bool` blocking (default `false`)               | Sets all fields of the record from the current time of the RTC. For blocking see above.                                                                                                                                                                                                                                                                                                                                                                                                                                            |
| `enable32kHz`     | none                                                                             | Enables output of clock pulses.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    |
| `disable32kHz`    | none                                                                             | Disables output of clock pulses.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   |
| `enable1Hz`       | none                                                                             | Enables output of 1 Hz signal.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     |
| `disable1Hz`      | none                                                                             | Disables 1 Hz output.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              |
| `setAlarm`        | `byte` minute [, `byte` hour]                                                    | Set an alarm time, where one has to specify either minute and hour or only minute. If the RTC has more than one alarm, the first one is used. Only DS1307 and RV-3028 using the Unix clock do not support alarms. The MCP79410 uses a specific future date when both minute and hour are supplied; `setAlarm(minute)` configures its recurring minute match. The RS5C372 does not support recurring hourly alarms.                                                                                                                 |
| `enableAlarm`     | none                                                                             | Enables the alarm interrupt.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       |
| `disableAlarm`    | none                                                                             | Disables alarm.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    |
| `senseAlarm`      | none                                                                             | Returns `true` if alarm has been raised.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           |
| `clearAlarm`      | none                                                                             | Clears alarm flag.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 |
| `setOffset`       | `int` value for 0.01 ppm correction steps and one optional `byte` mode parameter | Not all RTCs support trimming and they have different step sizes, ranging from about 0.1 ppm to 4.3 ppm. The function tries to approximate the requested value as closely as possible. DS1307, DS1337, and PCF8563 do not have an offset register. PCF8523 and RV-8523 offer different modes; with mode 2, the offset register can be set in the RTC's native representation.                                                                                                                                                      |
| `getOffset`       | none                                                                             | Returns contents of offset register verbatim (0 if no register).                                                                                                                                                                                                                                                                                                                                                                                                                                                                   |
| `getTemp`         | none                                                                             | Returns temperature as an integer value if the RTC has a user-accessible temperature sensor.                                                                                                                                                                                                                                                                                                                                                                                                                                       |
| `getRegister`     | `byte` register address                                                          | Returns contents of RTC register.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  |
| `setRegister`     | `byte` register address, `byte` value                                            | Sets RTC register to value.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        |
| `getCapabilities` | none                                                                             | Returns a byte with capability bits: Bit 0 = can output 32 kHz signal, bit 1 = can output 1 Hz signal, bit 2 = has alarm functionality, bit 3 = has an hourly recurring alarm, bit 4 = has an offset register, bit 5 = has a temperature sensor, bit 6 = uses the RS5C372-style register-addressing scheme.                                                                                                                                                                                                                        |

## Caveats

While this library is a great tool when you want to use different RTCs in parallel (there are not too many use cases for this, though, I guess), you should be aware that the RTCs often have similar I2C addresses. In order to avoid I2C address clashes, you need an I2C multiplexer or have to use different I2C buses. The former can be accomplished by a I2C multiplexer such as TCA9548A or PCA9548. The latter could be achieved by using different Wire instances, e.g., on an ESP32. If you want to do that, you need to pass the Wire instance as a parameter in the `begin` method call.

If you want to run just one RTC, the above is, of course, irrelevant. And if the functionality of the API is enough for you, you just found the right tool. As mentioned above, you can, of course, add all the features of your favorite RTC by adding more methods.

Finally, it should be pointed out that this library seems to be more memory hungry than [RTClib](https://github.com/adafruit/RTClib). This is probably caused by the dynamic binding of methods.

## Example sketches

There are the following example sketches:

`simple.ino`: Simple test sketch for setting and getting time. Change include file and class declaration.

`testall.ino`: Interactive sketch that can be used to test all functions of an RTC that are supported by their library. Simply change include file and class name. If you want to monitor the outputs (SQW, INT), you also need to define the constants `PIN1HZ`, `PIN32KHZ` and `PINALARM`. Which outputs to use can be seen in the next section.

`iterate.ino`: It defines an array of RTCs that can all be treated in the same way. This is demonstrated by iterating over them and printing out the time from each RTC. The current version is not executable because of I2C address clashes. In order to make that executable, you need also two I2C multiplexers that would need to be integrated into the code.

## <a name="capa"></a>RTC capabilities

Most RTCs supported by this library maintain time as separate calendar fields (seconds, minutes, hours, date, month, and year). The RV-3028 also provides a Unix-time counter that represents time as seconds since 1970-01-01 00:00:00 UTC.

The following tables list the hardware capabilities relevant to the common library API. A dash (`—`) means that the capability is not available or does not apply. The tables describe the RTC hardware; they do not imply that every hardware-specific feature is exposed by the common API.

### Capability bits

| RTC        | 32 kHz | 1 Hz  | Alarm | Hourly alarm | Offset | Temperature | Special register addressing |
| ---------- | :----: | :---: | :---: | :----------: | :----: | :---------: | :-------------------------: |
| DS1307     |  Yes   |  Yes  |   —   |      —       |   —    |      —      |              —              |
| DS1337     |  Yes   |  Yes  |  Yes  |     Yes      |   —    |      —      |              —              |
| DS3231S(N) |  Yes   |  Yes  |  Yes  |     Yes      |  Yes   |     Yes     |              —              |
| DS3231M    |  Yes   |  Yes  |  Yes  |     Yes      |  Yes   |     Yes     |              —              |
| MCP79410   |  Yes   |  Yes  |  Yes  |     Yes      |  Yes   |      —      |              —              |
| PCF8523    |  Yes   |  Yes  |  Yes  |     Yes      |  Yes   |      —      |              —              |
| PCF8563    |  Yes   |  Yes  |  Yes  |     Yes      |   —    |      —      |              —              |
| RS5C372    |  Yes   |  Yes  |  Yes  |      —       |  Yes   |      —      |             Yes             |
| RV-3028    |  Yes   |  Yes  |  Yes  |     Yes      |  Yes   |      —      |              —              |
| RV-3028U   |  Yes   |  Yes  |   —   |      —       |  Yes   |      —      |              —              |
| RV-3032    |  Yes   |  Yes  |  Yes  |     Yes      |  Yes   |     Yes     |              —              |
| RV-8523    |  Yes   |  Yes  |  Yes  |     Yes      |  Yes   |      —      |              —              |
| RV-8803    |  Yes   |  Yes  |  Yes  |     Yes      |  Yes   |      —      |              —              |
| SD2405     |  Yes   |  Yes  |  Yes  |     Yes      |  Yes   |      —      |              —              |

The `RTC_CAP_HOURLY_ALARM` bit describes the recurring hourly alarm supported by the common API. It is deliberately separate from the more general alarm capabilities described in the alarm table below.

### Timekeeping accuracy

| RTC        | Specified time accuracy                          | Typical oscillator frequency tolerance                            |
| ---------- | ------------------------------------------------ | ----------------------------------------------------------------- |
| DS1307     | —                                                | ±20 ppm<sup><a href="#f1">1)</a></sup>                            |
| DS1337     | —                                                | ±20 ppm<sup><a href="#f1">1)</a></sup>                            |
| DS3231S(N) | ±2 ppm (0 to +40 °C); ±3.5 ppm (-40 to +85 °C)   | —                                                                 |
| DS3231M    | ±5 ppm (-45 to +85 °C)                           | —                                                                 |
| MCP79410   | —                                                | ±20 ppm<sup><a href="#f1">1)</a></sup>                            |
| PCF8523    | —                                                | ±10 ppm or ±20 ppm<sup><a href="#f1">1)</a></sup>                 |
| PCF8563    | —                                                | ±20 ppm<sup><a href="#f1">1)</a></sup>                            |
| RS5C372    | ±1.5 ppm @ 25 °C<sup><a href="#f2">2)</a></sup>  | ±20 ppm<sup><a href="#f2">2)</a></sup>                            |
| RV-3028    | ±1 ppm @ 25 °C                                   | —                                                                 |
| RV-3032    | ±1.5 ppm (0 to +50 °C); ±2.5 ppm (-40 to +85 °C) | —                                                                 |
| RV-8523    | —                                                | ±10 ppm typ.; ±20 ppm max. @ 25 °C<sup><a href="#f1">1)</a></sup> |
| RV-8803    | ±1.5 ppm (0 to +50 °C); ±3 ppm (-40 to +85 °C)   | —                                                                 |
| SD2405     | —                                                | ±5 ppm<sup><a href="#f1">1)</a></sup>                             |

<a name="f1"></a><sup>1)</sup> The value shown is the oscillator/crystal frequency tolerance specified by the manufacturer. It is not necessarily a specification for the complete RTC's timekeeping accuracy.

<a name="f2"></a><sup>2)</sup> The RS5C372A can achieve approximately ±1.5 ppm timing precision at 25 °C after adjustment with its built-in time-trimmer. Ricoh also describes the use of a ±20 ppm crystal as an example; the ±20 ppm figure is the crystal's frequency tolerance, not the adjusted RTC's accuracy.

### Frequency adjustment / calibration

The library exposes frequency correction through `setOffset()` and `getOffset()` when the RTC has `RTC_CAP_OFFSET`.

| RTC        | Offset register | Library adjustment step (ppm) | Native/library mode notes                             |
| ---------- | :-------------: | ----------------------------: | ----------------------------------------------------- |
| DS1307     |        —        |                             — | No offset register                                    |
| DS1337     |        —        |                             — | No offset register                                    |
| DS3231S(N) |       Yes       |                          ≈0.1 | DS3231 offset register                                |
| DS3231M    |       Yes       |                          ≈0.1 | DS3231 offset register                                |
| MCP79410   |       Yes       |                            ≈1 | Offset register                                       |
| PCF8523    |       Yes       |                 4.34 or 4.069 | Two library modes; mode 2 uses native representation  |
| PCF8563    |        —        |                             — | No offset register                                    |
| RS5C372    |       Yes       |                         3.051 | Native offset representation                          |
| RV-3028    |       Yes       |                        0.9537 | Offset register                                       |
| RV-3028U   |       Yes       |                        0.9537 | Uses the same RV-3028 offset mechanism                |
| RV-3032    |       Yes       |                        0.2384 | Offset register                                       |
| RV-8523    |       Yes       |                 4.34 or 4.069 | Inherits the PCF8523 implementation and its two modes |
| RV-8803    |       Yes       |                        0.2384 | Offset register                                       |
| SD2405     |       Yes       |                         3.051 | Offset register                                       |

The library's `setOffset()` parameter is expressed in 0.01 ppm correction units, but the RTC hardware has coarser adjustment steps. The library therefore approximates the requested correction using the resolution available in each RTC.

### Temperature measurement

`getTemp()` is implemented as a capability-dependent operation. A value is available only for RTCs with the `RTC_CAP_TEMP` capability.

| RTC        | `getTemp()` | Temperature accuracy |
| ---------- | :---------: | -------------------: |
| DS1307     |      —      |                    — |
| DS1337     |      —      |                    — |
| DS3231S(N) |     Yes     |                ±3 °C |
| DS3231M    |     Yes     |                ±3 °C |
| MCP79410   |      —      |                    — |
| PCF8523    |      —      |                    — |
| PCF8563    |      —      |                    — |
| RS5C372    |      —      |                    — |
| RV-3028    |      —      |                    — |
| RV-3028U   |      —      |                    — |
| RV-3032    |     Yes     |                ±3 °C |
| RV-8523    |      —      |                    — |
| RV-8803    |      —      |                    — |
| SD2405     |      —      |                    — |

### I2C interface

The I2C address comes from the RTC implementation in this library. Maximum bus speed is the value listed for the supported device family.

| RTC        | I2C address | Maximum I2C speed (MHz) |
| ---------- | :---------: | ----------------------: |
| DS1307     |   `0x68`    |                     0.1 |
| DS1337     |   `0x68`    |                     0.4 |
| DS3231S(N) |   `0x68`    |                     0.4 |
| DS3231M    |   `0x68`    |                     0.4 |
| MCP79410   |   `0x6F`    |                     0.4 |
| PCF8523    |   `0x68`    |                     1.0 |
| PCF8563    |   `0x51`    |                     0.4 |
| RS5C372    |   `0x32`    |                     0.4 |
| RV-3028    |   `0x52`    |                     0.4 |
| RV-3028U   |   `0x52`    |                     0.4 |
| RV-3032    |   `0x51`    |                     0.4 |
| RV-8523    |   `0x68`    |                     1.0 |
| RV-8803    |   `0x32`    |                     0.4 |
| SD2405     |   `0x32`    |                     0.4 |

Several devices therefore share the same I2C address. This is the reason that multiple RTCs often require either different I2C buses or an I2C multiplexer.

The RS5C372 is additionally marked with `RTC_CAP_SREGADDR` because its register-addressing scheme is different from the other RTCs supported by the common base class.

### Time representation

The common API converts RTC calendar fields to and from Unix `time_t` and `tm` representations. The RV-3028 is unusual because the device also provides a hardware Unix-time counter.

| RTC        | Calendar time registers | Hardware Unix-time counter |
| ---------- | :---------------------: | :------------------------: |
| DS1307     |           Yes           |             —              |
| DS1337     |           Yes           |             —              |
| DS3231S(N) |           Yes           |             —              |
| DS3231M    |           Yes           |             —              |
| MCP79410   |           Yes           |             —              |
| PCF8523    |           Yes           |             —              |
| PCF8563    |           Yes           |             —              |
| RS5C372    |           Yes           |             —              |
| RV-3028    |           Yes           |            Yes             |
| RV-3028U   |            —            |            Yes             |
| RV-3032    |           Yes           |             —              |
| RV-8523    |           Yes           |             —              |
| RV-8803    |           Yes           |             —              |
| SD2405     |           Yes           |             —              |

The `RV-3028U` class uses the RV-3028 Unix-time counter for timekeeping and does not provide the alarm capability of the regular `RV-3028` class.

### 1 Hz and 32 kHz oscillator outputs

| RTC        | 1 Hz output                                                            | 32 kHz output                           |
| ---------- | ---------------------------------------------------------------------- | --------------------------------------- |
| DS1307     | `SQW/OUT`                                                              | `SQW/OUT`                               |
| DS1337     | `SQW/nINTB`                                                            | `SQW/nINTB`                             |
| DS3231S(N) | `nINT/SQW`<sup><a href="#f3">3)</a></sup>                              | `32kHz`<sup><a href="#f3">3)</a></sup>  |
| DS3231M    | `nINT/SQW`<sup><a href="#f3">3)</a></sup>                              | `32kHz`                                 |
| MCP79410   | `MFP`<sup><a href="#f4">4)</a></sup>                                   | `MFP`                                   |
| PCF8523    | `nINT1/CLKOUT`                                                         | `nINT1/CLKOUT`                          |
| PCF8563    | `CLKOUT`                                                               | `CLKOUT`                                |
| RS5C372    | `nINTRB`<sup><a href="#f4">4)</a></sup>                                | `nINTRB`                                |
| RV-3028    | `nINT`<sup><a href="#f3">3)</a></sup>                                  | `CLKOUT`                                |
| RV-3032    | `CLKOUT`<sup><a href="#f3">3)</a></sup>                                | `CLKOUT`                                |
| RV-8523    | `nINT1/CLKOUT`                                                         | `nINT1/CLKOUT`                          |
| RV-8803    | `CLKOUT`<sup><a href="#f3">3)</a></sup><sup><a href="#f5">5)</a></sup> | `CLKOUT`<sup><a href="#f5">5)</a></sup> |
| SD2405     | `nINT`                                                                 | `nINT`                                  |

<a name="f3"></a><sup>3)</sup> On these devices, the offset register influences the oscillator frequency directly.

<a name="f4"></a><sup>4)</sup> The offset register influences the frequency. However, the adjustments are made every 10, 20, or 60 seconds. This means one would need a large set of measurements when one wants to determine the true average frequency.

<a name="f5"></a><sup>5)</sup> The output cannot be disabled using software, but one has to pull down the `CLKOE` pin. On the SparkFun breakout board, this input is pulled to GND by a 100 kΩ resistor, i.e., one has to tie it to Vcc in order to enable CLKOUT.

### Alarm

The alarm table distinguishes three different concepts:

* **Single event** means that the hardware provides an explicit one-shot/future-event alarm mode.
* **Periodic** means that the hardware provides an explicit periodic alarm mode.
* **Every matching occurrence** means that an enabled calendar match can raise the alarm again when the programmed condition occurs again. This can provide recurring daily, hourly, weekly, or other calendar-based events without being a separate hardware "periodic mode".

The last column therefore does not mean that the common library automatically implements every possible recurrence exposed by the hardware.

| RTC        | Alarm output   | Alarm level | Number of alarms | Alarm matching fields                                                                                        |             Single event             |               Periodic               |      Every matching occurrence       |
| ---------- | -------------- | ----------- | ---------------: | ------------------------------------------------------------------------------------------------------------ | :----------------------------------: | :----------------------------------: | :----------------------------------: |
| DS1307     | —              | —           |                0 | —                                                                                                            |                  —                   |                  —                   |                  —                   |
| DS1337     | `SQW/nINTB`    | Active low  |                2 | Alarm 1: seconds, minutes, hours, day/date; Alarm 2: minutes, hours, day/date<sup><a href="#f6">6)</a></sup> |                  No                  |  Yes<sup><a href="#f6">6)</a></sup>  |                 Yes                  |
| DS3231S(N) | `nINT/SQW`     | Active low  |                2 | Alarm 1: seconds, minutes, hours, day/date; Alarm 2: minutes, hours, day/date<sup><a href="#f7">7)</a></sup> |                  No                  |  Yes<sup><a href="#f7">7)</a></sup>  |                 Yes                  |
| DS3231M    | `nINT/SQW`     | Active low  |                2 | Alarm 1: seconds, minutes, hours, day/date; Alarm 2: minutes, hours, day/date<sup><a href="#f7">7)</a></sup> |                  No                  |  Yes<sup><a href="#f7">7)</a></sup>  |                 Yes                  |
| MCP79410   | `MFP`          | Active low  |                2 | Minute, hour, day, weekday, month<sup><a href="#f8">8)</a></sup>                                             |  Yes<sup><a href="#f8">8)</a></sup>  |  Yes<sup><a href="#f8">8)</a></sup>  |                 Yes                  |
| PCF8523    | `nINT1/CLKOUT` | Active low  |                1 | Minute, hour, day, weekday<sup><a href="#f9">9)</a></sup>                                                    |                  No                  |                  No                  |  Yes<sup><a href="#f9">9)</a></sup>  |
| PCF8563    | `nINT`         | Active low  |                1 | Minute, hour, day, weekday<sup><a href="#f10">10)</a></sup>                                                  |                  No                  |                  No                  | Yes<sup><a href="#f10">10)</a></sup> |
| RS5C372    | `nINTRA`       | Active low  |                2 | Minute, hour, weekday<sup><a href="#f11">11)</a></sup>                                                       |                  No                  |                  No                  | Yes<sup><a href="#f11">11)</a></sup> |
| RV-3028    | `nINT`         | Active low  |                1 | Minute, hour, weekday/date<sup><a href="#f12">12)</a></sup>                                                  |                  No                  |                  No                  | Yes<sup><a href="#f12">12)</a></sup> |
| RV-3032    | `nINT`         | Active low  |                1 | Minute, hour, date<sup><a href="#f13">13)</a></sup>                                                          |                  No                  |                  No                  | Yes<sup><a href="#f13">13)</a></sup> |
| RV-8523    | `nINT1/CLKOUT` | Active low  |                1 | Minute, hour, day, weekday<sup><a href="#f14">14)</a></sup>                                                  |                  No                  |                  No                  | Yes<sup><a href="#f14">14)</a></sup> |
| RV-8803    | `nINT`         | Active low  |                2 | Minute, hour, weekday/date<sup><a href="#f15">15)</a></sup>                                                  |                  No                  |                  No                  | Yes<sup><a href="#f15">15)</a></sup> |
| SD2405     | `nINT`         | Active low  |                1 | Second, minute, hour, day, weekday, month, year<sup><a href="#f16">16)</a></sup>                             | Yes<sup><a href="#f16">16)</a></sup> | Yes<sup><a href="#f16">16)</a></sup> | Yes<sup><a href="#f16">16)</a></sup> |

<a name="f6"></a><sup>6)</sup> The DS1337 provides two independent alarms. Alarm 1 supports alarm rates from once per second through matching seconds, minutes, hours, and either date or weekday. Alarm 2 supports alarm rates from once per minute through matching minutes, hours, and either date or weekday. The alarm condition is evaluated as the selected calendar fields match. The hardware does not provide a separate one-shot mode; software can disable or clear an alarm after it fires if one event is desired.

<a name="f7"></a><sup>7)</sup> The DS3231 provides two independent alarms. Alarm 1 supports alarm rates from once per second through matching seconds, minutes, hours, and either date or weekday. Alarm 2 supports alarm rates from once per minute through matching minutes, hours, and either date or weekday. The alarm condition is evaluated as the selected calendar fields match. The hardware does not provide a separate one-shot mode; software can disable or clear an alarm after it fires if one event is desired. This applies to the DS3231S(N) and DS3231M implementations in this library.

<a name="f8"></a><sup>8)</sup> The MCP79410 provides two independent alarms. Each alarm can either generate an interrupt at a specific time in the future or generate a periodic interrupt every minute, hour, day, day of week, or month. The library's `setAlarm(minute, hour)` implementation uses the specific-future-time behavior and selects the next applicable date; `setAlarm(minute)` configures a recurring minute match.

<a name="f9"></a><sup>9)</sup> The PCF8523 provides one alarm function with independently selectable minute, hour, day, and weekday comparisons. When the alarm flag is cleared, it is set again when the time subsequently increments into another matching condition. It therefore provides recurring calendar matching rather than a separate one-shot or periodic-mode setting.

<a name="f10"></a><sup>10)</sup> The PCF8563 provides one alarm function with independently selectable minute, hour, day, and weekday comparisons. When the alarm flag is cleared, it is set again when the time subsequently increments into another matching condition. It therefore provides recurring calendar matching rather than a separate one-shot or periodic-mode setting.

<a name="f11"></a><sup>11)</sup> The RS5C372 provides two equivalent alarm register sets. Each compares the programmed minute, hour, and weekday values with the calendar counter. An enabled alarm generates an interrupt whenever the programmed calendar condition coincides with the current time. The separate periodic interrupt function is not part of the alarm function, and the library therefore does not advertise `RTC_CAP_HOURLY_ALARM` for this RTC.

<a name="f12"></a><sup>12)</sup> The RV-3028 provides one alarm function with independently selectable minute, hour, and weekday/date comparisons. The available combinations produce recurring events such as every minute, hourly, daily, or on selected weekdays/dates. There is no separate one-shot alarm mode. The `RV-3028U` library variant uses the Unix-time counter for timekeeping and does not support alarms.

<a name="f13"></a><sup>13)</sup> The RV-3032 provides one alarm function with independently selectable minute, hour, and date comparisons. The available combinations produce recurring events such as every minute, hourly, daily, or monthly. There is no separate one-shot alarm mode.

<a name="f14"></a><sup>14)</sup> The RV-8523 uses the PCF8523 implementation in this library. Its alarm function has independently selectable minute, hour, day, and weekday comparisons. The alarm is generated when the enabled calendar fields match; the alarm function does not provide a separate one-shot mode.

<a name="f15"></a><sup>15)</sup> The RV-8803 provides two alarm register sets. Each supports independently selectable minute, hour, and weekday/date comparisons. The available combinations produce recurring events such as every minute, hourly, daily, or on selected weekdays/dates. The separate periodic time-update and countdown-timer interrupts are not counted as additional alarms.

<a name="f16"></a><sup>16)</sup> The SD2405 provides one alarm function with second, minute, hour, weekday, day, month, and year matching. Its alarm interrupt has an explicit single-event mode and a periodic mode. The periodic mode continues to generate alarm interrupts until the alarm flag is reset. Its separate frequency and countdown interrupts are not counted as additional alarms.

<!-- cSpell:ignore capa -->
