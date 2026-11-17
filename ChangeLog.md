# ChangeLog<!--! {#change_log} -->

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/) and its stricter, better defined, brother [Common Changelog](https://common-changelog.org/).

This project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

<!-- @tableofcontents{XML:1,HTML:1} -->

<!--! @m_footernavigation -->

<!--! @if GITHUB -->

- [ChangeLog](#changelog)
  - [Unreleased](#unreleased)
  - [0.8.0 (2026-11-14)](#080-2026-11-14)
  - [0.7.0](#070)
  - [V0.6.0 (22.3.2023)](#v060-2232023)
  - [V0.5.2 (8.3.2023)](#v052-832023)
  - [V0.5.1 (7.3.2023)](#v051-732023)
  - [V0.5.0 (7.3.2023)](#v050-732023)
  - [V0.4.1 (6.3.2023)](#v041-632023)
  - [V0.4.0 (5.3.2023)](#v040-532023)
  - [V0.3.2 (4.3.2023)](#v032-432023)
  - [V0.3.1 (4.3.2023)](#v031-432023)
  - [V0.3.0 (4.3.2023)](#v030-432023)
  - [V0.2.5 (3.3.2023)](#v025-332023)
  - [V0.2.4 (3.3.2023)](#v024-332023)
  - [V0.2.3 (2.3.2023)](#v023-232023)
  - [V0.2.2 (2.3.2023)](#v022-232023)
  - [V0.2.1 (28.2.2023)](#v021-2822023)
  - [V0.2.0 (28.2.2023)](#v020-2822023)
  - [V0.1.1 (26.2.2023)](#v011-2622023)
  - [V0.1.0 (25.2.2023)](#v010-2522023)

<!--! @endif -->

***

## [Unreleased]

### Changed

### Added

### Removed

### Fixed

***

## [0.8.0] (2026-11-14)

### Changed

- **Breaking:** Replaced the `RTC_CAP_*` macros with the scoped `RTCCapability` enum and made `getCapabilities()` non-virtual.
- Moved device-specific I2C transaction flags out of the public capability mask and into an internal quirks mask.
- **Potentially breaking:** Moved backup-battery switchover configuration from `begin()` and `init()` to the separate `setBatteryMode()` function.
- Added independent `reset_configuration` and `clear_flags` options to `begin()` and `init()`.
  The `reset_configuration` option controls whether RTC-specific operational control registers are set to library defaults without changing the time, battery mode, or calibration.
  The `clear_flags` option controls whether power-loss, oscillator-stop, and voltage-low validity evidence is cleared during initialization.
  Both options default to `true` to preserve the previous initialization behavior unless the caller opts out.
- Centralized shared alarm and validity behavior in the DS-family and PCF-family parent classes.
- Redesigned alarm configuration around one virtual `setAlarm(AlarmMatch, const AlarmTime&)` implementation point returning a simple success/failure result.
  Alarm inputs are passed to the hardware without date/time validation; supported coarser-resolution modes truncate finer fields and may therefore trigger early.
- Increased minimum required EpochTime to 1.3.0.

### Added

- Added `supports(RTCCapability)` and `supports(AlarmMatch)` capability queries and the `RTCCapability::RTC_CAP_BATTERY` capability.
- Added success-returning `getTime(timestamp_t&)` and `getTemperature(float&)` APIs.
- Added `AlarmMatch`, `AlarmTime`, `getSupportedAlarmMatches()`, and non-virtual convenience functions from
  `setSecondAlarm()` through `setExactAlarm()`; each convenience function delegates to `setAlarm()`.
- Added recurring alarm support only for the match modes each RTC can represent; unsupported modes return `false`.
- Added non-blocking DS3231 forced temperature conversion and native fake-I2C regression tests for time, alarm, validity, temperature, and EEPROM paths.
- Added example usage and documentation for the new alarm and time APIs.
- Added `RTCCapability::RTC_CAP_COUNTDOWN`, `CountdownTimerFrequency`, and periodic countdown timer configuration, enable, disable, sense, and clear APIs for the PCF8523, PCF8563, RV-3028, RV-3028-U, RV-3032, RV-8523, RV-8803, and SD2405.

### Removed

- **BREAKING:** Removed the redundant `RTC_CAP_HOURLY_ALARM` capability.
- **BREAKING**: Removed the legacy alarm setters and overloads and the upward-rounding `roundAlarmToMinute()` helper.

### Fixed

- Preserved DS3231 quarter-degree temperature resolution and decoded the RV-3032 signed 12-bit temperature at one-sixteenth-degree resolution.
- Preserved the RV-8803 repeated-read workaround in success-returning timestamp reads and propagated I2C failures through new alarm and temperature paths.
- Avoided unchanged RV-3032 EEPROM-backed configuration writes.
- Made RV-3028 and RV-3032 EEPROM retries persist the requested value without allowing automatic refresh to replace the RAM mirror first.
- Avoided stopping running PCF-family clocks when their oscillator-stop or voltage-low flag is already clear.
- Synchronized the RV-3028-U calendar and Unix counters when setting time so inherited alarms use the same time base.

***

## [0.7.0]

### Changed

- Replaced Paul Stoffregen's Time library with the EnviroDIY EpochTime library and the standard `time.h` `tm` structure.
- Renamed the base `RTC` class to `RTC_I2C` to avoid conflicts with platform core definitions.
- Changed initialization, time-setting, alarm, output, offset, and register operations to return success or failure.
- Added strongly typed battery-switching and offset-mode enums, and made `begin()` perform device initialization.
- Expanded support from AVR-only to all Arduino architectures.
- Updated and documented the examples, public API, and RTC capability tables.

### Added

- Added functions to get each clock's manufacturer, model, combined make and
  model, and I2C address.
- Added timestamp and `epochTime` overloads plus portable time-conversion
  helpers.
- Added documentation and example-build workflows, Doxygen configuration, and
  ReadMe files for each example.

### Removed

- Removed the blocking-read option from `getTime()`.

### Fixed

- Corrected weekday and calendar-time conversions, including behavior on mbed
  and Raspberry Pi Pico platforms.
- Corrected RTC manufacturer names and propagated I2C communication failures
  through the public API.

***

## V0.6.0 (22.3.2023)

- added: new method getOffset(): returns contents of offset
  register verbatim (i.e., no interpretation as a ppm value)

***

## V0.5.2 (8.3.2023)

- fixed: enable battery switch over (VBATEN) for MCP79410 when setting
         the time
- fixed: implemented setting of time for MCP79410 as described in the
         data sheet, i.e., stop oscillator, set time, start oscillator

***

## V0.5.1 (7.3.2023)

- added hourly recurring alarm (DS1307 and SD2405 do not support them)
- added hourly alarms to testall.ino
- added skipping to 28.2.2100, 23:59:45

***

## V0.5.0 (7.3.2023)

- integrated the strange register addressing  (RTC_CAP_SREGADDR) into the base class, so now it is totally transparent to everybody using the library.

***

## V0.4.1 (6.3.2023)

- fixed in testall: register number when 'setting' must also be massaged when RTC_CAP_SREGADDR.
- added also OFFSET constants for all RTCs that do not have any
- added some explanations read.md about that and about the new example iterate.ino

***

## V0.4.0 (5.3.2023)

- added mode=2 for setOffset in order to store the offset value in the
  native format

***

## V0.3.2 (4.3.2023)

- added SD2405
- added possible extra '1' bits in clock register for some RTCs by new var `_bit7set`
- moved constructor into header file

***

## V0.3.1 (4.3.2023)

- added MCP79140
- fixed masking of unused bits in the clock register

***

## V0.3.0 (4.3.2023)

- added update of registers from RAM to EEPROM in RV-3032
- fixed problem with switch-over confusion
- added 3028
- added 3028U (using the Unix counter for timekeeping)
- added timeouts in all loops where we wait for a condition
- renamed library and classes to make them more readable
- new example: iterate

***

## V0.2.5 (3.3.2023)

- added RV-3032
- added 'mode' parameter to init, which can be used to select the battery switch-over mode

***

## V0.2.4 (3.3.2023)

- added optional parameter 'blocking' to getTime, which is false by default;
  if true, then we wait with the return until a fresh second has started.
- added RV-8803
- _wdaybase can now also be 2, which means that the day of the week is
  0-based, but it is now the bit position (for RV-8803)
- changed handling of _wdayfirst in setTime and getTime so that wday-encoding/decoding now
  happens just once
- added section of RTC capabilities in readme

***

## V0.2.3 (2.3.2023)

- checked with new PCF8563 module.
This one works flawlessly!
The Waveshare one is apparently broken.
- added RV-8523 (well, it is simply a synonym for PCF8523)
- added getTemp method

***

## V0.2.2 (2.3.2023)

- added support for RS5C372
- added a new "capability": strange register address: SREGADDR used for RS5C372, who wants to see the register address in the upper nibble

***

## V0.2.1 (28.2.2023)

- added support for PCF8563

***

## V0.2.0 (28.2.2023)

- added new methods: enableXXHz, disableXXHz, alarm methods, and a getCapabilities method
- thrown out setAlarm with only minutes since not all RTCs support that, so only hour with minute is supported
- added support for DS1337, DS3231, PCF8523
- added example testall, which can be used as a unit test for the different modules

***

## V0.1.1 (26.2.2023)

- renamed from univRTC to I2CRTC
- framework works with DS1307 example

***

## V0.1.0 (25.2.2023)

- first stub with an idea for the API

[Unreleased]: https://github.com/EnviroDIY/RTC_I2C/compare/v0.8.0...HEAD
[0.8.0]: https://github.com/EnviroDIY/RTC_I2C/releases/tag/v0.8.0
[0.7.0]: https://github.com/EnviroDIY/RTC_I2C/releases/tag/v0.7.0

<!--! @tableofcontents{HTML:1} -->

<!--! @m_footernavigation -->

<!-- cspell:words I2CRTC SREGADDR Waveshare wday pico -->
