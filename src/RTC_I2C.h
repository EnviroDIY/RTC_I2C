/*****************************************************************************************
 *
 * This library provides a universal interface to many different RTCs making it possible
 * to use these RTCs in parallel, not caring about their particular features.
 *
 * License: MIT
 *
 *
 *****************************************************************************************/

#ifndef _RTC_I2C_H_
#define _RTC_I2C_H_

#include <Arduino.h>
// #include <TimeLib.h>
#include <Wire.h>
#include <time.h>
#include <EpochTime.h>


/// Number of clock/calendar bytes transferred by the common RTC implementation.
#define TIMEBYTES 7

/// Capabilities supported by an RTC implementation.
enum class RTCCapability : byte {
  RTC_CAP_32KHZ = 0x01,   ///< The RTC can generate a 32 kHz signal.
  RTC_CAP_1HZ = 0x02,     ///< The RTC can generate a 1 Hz signal.
  RTC_CAP_ALARM = 0x04,   ///< The RTC has alarm functionality.
  RTC_CAP_BATTERY = 0x08, ///< The RTC supports configuring its backup-battery switchover mode.
  RTC_CAP_OFFSET = 0x10,  ///< The RTC has an offset/calibration register.
  RTC_CAP_TEMP = 0x20,    ///< The RTC has a temperature sensor.
};

#ifndef SECONDS_IN_DAY
/// Number of seconds in a day.
#define SECONDS_IN_DAY 86400L
#endif

/// The mode for switchover of the backup battery.
enum class BatteryMode : int8_t {
  SWITCHING_DISABLED, ///< Battery switch-over function is disabled, only VDD is used
  LEVEL_SWITCHING, ///< Level Switching Mode (LSM). Switchover when VDD < threshold AND VBACKUP > threshold.  Use with a
                   ///< standard coin cell battery.
  DIRECT_SWITCHING ///< Direct Switching Mode (DSM). Switchover when VDD < VBACKUP.  Use when charging a rechargeable
                   ///< battery.
};

/**
 * @brief Selects how a clock calibration offset or trim is interpreted.
 *
 * See the specific RTC documentation for details on the correction frequencies that are considered to be coarse, fine,
 * or very fine.
 */
enum class OffsetMode : byte {
  COARSE_OFFSET = 0, ///< Apply the offset as a coarse correction.
  FINE_OFFSET = 1,   ///< Apply the offset as a fine correction.
  RAW_OFFSET = 2     ///< Treat the given offset as the exact value to write to the RTC's offset/calibration-register.
};

/**
 * @brief Selects the clock fields that must match for an alarm.
 *
 * The values are bit flags so `getSupportedAlarmMatches()` can return any supported combination as a byte mask.
 */
enum class AlarmMatch : byte {
  EVERY_SECOND = 0x01,       ///< Trigger every second.
  MATCH_SECONDS = 0x02,      ///< Match seconds, triggering every minute.
  MATCH_MINUTES = 0x04,      ///< Match minutes and seconds, triggering every hour.
  MATCH_HOURS = 0x08,        ///< Match hours, minutes, and seconds, triggering every day.
  MATCH_DAY_OF_WEEK = 0x10,  ///< Match weekday and time, triggering every week.
  MATCH_DAY_OF_MONTH = 0x20, ///< Match day of month and time, triggering every month.
  MATCH_DAY_OF_YEAR = 0x40,  ///< Match month, day of month, and time, triggering every year.
  MATCH_EXACT = 0x80,        ///< Match the full year, month, day, and time.
};

/**
 * @brief Calendar and time fields used to configure an alarm.
 *
 * RTC_I2C does not validate these values. The caller is responsible for supplying values accepted by the selected RTC.
 * When the requested match mode is supported, fields finer than an RTC's alarm resolution are truncated by the device
 * implementation. For example, a minute-resolution RTC ignores `second`, so 12:34:56 is configured as 12:34:00 and
 * may trigger up to 59 seconds early.
 */
struct AlarmTime {
  uint16_t year = 2000; ///< Full calendar year.
  byte month = 1;       ///< Calendar month, normally 1 through 12.
  byte day = 1;         ///< Day of the month, normally 1 through 31.
  byte weekday = 0;     ///< Day of the week, normally 0 through 6.
  byte hour = 0;        ///< Hour after midnight, normally 0 through 23.
  byte minute = 0;      ///< Minute after the hour, normally 0 through 59.
  byte second = 0;      ///< Second after the minute, normally 0 through 59.
};


/// A generic RTC base class
class RTC_I2C {
 public:
  /**
   * @brief Initialize the I2C interface and establish communication with the RTC.
   *
   * If communication has already been established, subsequent calls return without
   * reinitializing the interface. Once the RTC responds, the device-specific `init()`
   * function is called.
   *
   * @param wi The I2C interface to use.
   * @param reset_configuration Whether to reset the RTC's operational configuration to the library defaults.
   * @param clear_flags Whether to clear any previously set error flags during initialization.  Optional with a default
   * of true.  In versions prior to 0.8.0, this was always true.
   * @return `true` if the RTC address is valid and the device responds on I2C; otherwise `false`.
   */
  bool begin(TwoWire *wi = &Wire, bool reset_configuration = true, bool clear_flags = true);

  /**
   * @brief Initialize device-specific RTC settings.
   * @param reset_configuration Whether to reset the RTC's operational configuration to the library defaults.  This
   * does not change the time, battery mode, calibration, or error flags.  In versions prior to 0.8.0, this was always
   * true.
   * @param clear_flags Whether to clear any previously set error flags during initialization.  Optional with a default
   * of true.  In versions prior to 0.8.0, this was always true.
   * @return `true` when the required I2C communication succeeds; otherwise `false`.
   */
  virtual bool init(bool reset_configuration = true, bool clear_flags = true) = 0;

  /**
   * @brief Set the backup-battery switchover mode.
   *
   * RTCs that do not support configurable battery switchover leave their configuration unchanged and return `false`.
   *
   * @param mode The battery switchover mode to configure.
   * @return `true` when the requested mode is supported and is already active or successfully configured; `false` if
   * the configuration is unsupported or no battery modes are supported.
   */
  virtual bool setBatteryMode(BatteryMode mode);

  /**
   * @brief Check whether the RTC is operating normally.
   * @return `true` when the RTC reports a valid operating state; otherwise `false`.
   */
  virtual bool isValid() = 0;

  /**
   * @brief Clear any set error or warning flags in the clock registers.
   *
   * @return `true` when the acknowledge operation succeeds or is a no-op; otherwise `false`.
   */
  virtual bool clearErrorFlags() { return true; }

  /**
   * @brief Set the RTC from a Unix timestamp.
   * @param unixTimestamp The Unix timestamp to write to the RTC.
   * @return `true` when the required I2C communication succeeds; otherwise `false`.
   */
  virtual bool setTime(timestamp_t unixTimestamp);

  /**
   * @brief Set the RTC from an `epochTime` object.
   * @param eTime The time value to write to the RTC.
   * @return `true` when the required I2C communication succeeds; otherwise `false`.
   */
  virtual bool setTime(epochTime eTime);

  /**
   * @brief Set the RTC from a `tm` calendar-time structure.
   *
   * The base implementation writes the seconds, minutes, hours, weekday, day,
   * month, and year fields using the register layout described by the derived RTC.
   *
   * @param timeParts The calendar time to write to the RTC.
   * @return `true` when the I2C write succeeds; otherwise `false`.
   */
  virtual bool setTime(tm timeParts);

  /**
   * @brief Read the RTC as a Unix timestamp.
   * @return The current RTC time as a Unix timestamp.
   */
  virtual timestamp_t getTime();

  /**
   * @brief Read the RTC and return a Unix timestamp when the complete operation succeeds.
   *
   * The default implementation reads `tm` calendar fields with `getTime(tm&)` and converts
   * them only after a successful I2C read.
   *
   * @param[out] unixTimestamp The Unix timestamp read from the RTC.
   * @return `true` when both the I2C read and conversion succeed; otherwise `false`.
   */
  virtual bool getTime(timestamp_t &unixTimestamp);

  /**
   * @brief Read the RTC into a `tm` calendar-time structure.
   * @param[out] timeParts The calendar-time structure to populate.
   * @return `true` when the register-address write and requested 7-byte read succeed; otherwise `false`.
   */
  virtual bool getTime(tm &timeParts);

  /**
   * @brief Enable the RTC's 32 kHz output; a no-op for RTCs that do not have a 32kHz output.
   * @return `true` when the operation succeeds or is a no-op; otherwise `false`.
   */
  virtual bool enable32kHz() { return true; }

  /**
   * @brief Disable the RTC's 32 kHz output; a no-op for RTCs that do not have a 32kHz output.
   * @return `true` when the operation succeeds or is a no-op; otherwise `false`.
   */
  virtual bool disable32kHz() { return true; }

  /**
   * @brief Enable the RTC's 1 Hz output; a no-op for RTCs that do not have a 1Hz output.
   * @return `true` when the operation succeeds or is a no-op; otherwise `false`.
   */
  virtual bool enable1Hz() { return true; }

  /**
   * @brief Disable the RTC's 1 Hz output; a no-op for RTCs that do not have a 1Hz output.
   * @return `true` when the operation succeeds or is a no-op; otherwise `false`.
   */
  virtual bool disable1Hz() { return true; }

  /**
   * @brief Configure an alarm using the fields selected by `match`.
   *
   * This function performs no date or time validation. The caller is responsible for supplying values accepted by the
   * selected RTC. For supported match modes, implementations truncate fields finer than the hardware alarm resolution;
   * they do not round upward. Unsupported match modes return `false` and are not replaced by a less-specific mode.
   *
   * @param match The fields that must match.
   * @param alarm The requested alarm fields.
   * @return `true` when the alarm registers are configured; `false` when the match mode is unsupported or communication
   * fails.
   */
  virtual bool setAlarm(AlarmMatch match, const AlarmTime &alarm);

  /**
   * @brief Configure an exact date-and-time alarm.
   * @param year The full calendar year.
   * @param month The calendar month.
   * @param day The day of the month.
   * @param hour The hour after midnight.
   * @param minute The minute after the hour.
   * @param second The second after the minute.
   * @return The result of `setAlarm(AlarmMatch::MATCH_EXACT, alarm)`.
   */
  bool setExactAlarm(uint16_t year = 2000, byte month = 1, byte day = 1, byte hour = 0, byte minute = 0,
                     byte second = 0);
  /**
   * @brief Configure a yearly alarm matching month, day, and time.
   * @param month The calendar month.
   * @param day The day of the month.
   * @param hour The hour after midnight.
   * @param minute The minute after the hour.
   * @param second The second after the minute.
   * @return The result of `setAlarm(AlarmMatch::MATCH_DAY_OF_YEAR, alarm)`.
   */
  bool setYearlyAlarm(byte month = 1, byte day = 1, byte hour = 0, byte minute = 0, byte second = 0);
  /**
   * @brief Configure a monthly alarm matching day and time.
   * @param day The day of the month.
   * @param hour The hour after midnight.
   * @param minute The minute after the hour.
   * @param second The second after the minute.
   * @return The result of `setAlarm(AlarmMatch::MATCH_DAY_OF_MONTH, alarm)`.
   */
  bool setMonthlyAlarm(byte day = 1, byte hour = 0, byte minute = 0, byte second = 0);
  /**
   * @brief Configure a weekly alarm matching weekday and time.
   * @param weekday The day of the week.
   * @param hour The hour after midnight.
   * @param minute The minute after the hour.
   * @param second The second after the minute.
   * @return The result of `setAlarm(AlarmMatch::MATCH_DAY_OF_WEEK, alarm)`.
   */
  bool setWeeklyAlarm(byte weekday = 0, byte hour = 0, byte minute = 0, byte second = 0);
  /**
   * @brief Configure a daily alarm matching time.
   * @param hour The hour after midnight.
   * @param minute The minute after the hour.
   * @param second The second after the minute.
   * @return The result of `setAlarm(AlarmMatch::MATCH_HOURS, alarm)`.
   */
  bool setDailyAlarm(byte hour = 0, byte minute = 0, byte second = 0);
  /**
   * @brief Configure an hourly alarm matching minute and second.
   * @param minute The minute after the hour.
   * @param second The second after the minute.
   * @return The result of `setAlarm(AlarmMatch::MATCH_MINUTES, alarm)`.
   */
  bool setHourlyAlarm(byte minute = 0, byte second = 0);
  /**
   * @brief Configure an alarm every minute matching the requested second.
   * @param second The second after the minute.
   * @return The result of `setAlarm(AlarmMatch::MATCH_SECONDS, alarm)`.
   */
  bool setMinuteAlarm(byte second = 0);
  /**
   * @brief Configure an alarm every second.
   * @return The result of `setAlarm(AlarmMatch::EVERY_SECOND, alarm)`.
   */
  bool setSecondAlarm();

  /**
   * @brief Enable the RTC alarm interrupt or alarm function; a no-op for RTCs that do not have an alarm.
   * @return `true` when the operation succeeds or is a no-op; otherwise `false`.
   */
  virtual bool enableAlarm() { return true; }

  /**
   * @brief Disable the RTC alarm interrupt or alarm function; a no-op for RTCs that do not have an alarm.
   * @return `true` when the operation succeeds or is a no-op; otherwise `false`.
   */
  virtual bool disableAlarm() { return true; }

  /**
   * @brief Check whether the RTC alarm condition is asserted.
   * @return `true` if the RTC reports an asserted alarm; otherwise `false`.
   */
  virtual bool senseAlarm() { return false; }

  /**
   * @brief Clear the RTC alarm condition; a no-op for RTCs that do not have an alarm.
   * @return `true` when the operation succeeds or is a no-op; otherwise `false`.
   */
  virtual bool clearAlarm() { return true; }

  /**
   * @brief Apply a calibration offset to the RTC.
   *
   * The interpretation of `offset` is selected by `mode`. The base implementation
   * does nothing for RTCs without a calibration register.
   *
   * @param offset The calibration correction to apply.
   * @param mode Selects whether the correction is interpreted as a coarse, fine, or raw offset.
   * @return `true` when the I2C write succeeds or the operation is a no-op; otherwise `false`.
   */
  virtual bool setOffset(__attribute__((unused)) int offset,
                         __attribute__((unused)) OffsetMode mode = OffsetMode::FINE_OFFSET) {
    return true;
  }

  /**
   * @brief Read the RTC calibration offset or zero for RTCs without a calibration register.
   * @return The calibration offset in the RTC-specific library representation.
   */
  virtual unsigned int getOffset() { return 0; }

  /**
   * @brief Read the RTC's temperature measurement or `-128` for RTCs without a temperature sensor.
   * @return The RTC temperature, or `-128` when no temperature sensor is available.
   */
  virtual int getTemp() {
    float temperatureC;
    if (!getTemperature(temperatureC)) return -128;
    int integerTemperature = (int)temperatureC;
    if (temperatureC < (float)integerTemperature) --integerTemperature;
    return integerTemperature;
  }

  /**
   * @brief Read the RTC's temperature in degrees Celsius.
   *
   * This API separates communication success from valid negative temperatures.
   *
   * @param[out] temperatureC The measured temperature in degrees Celsius.
   * @return `true` when the temperature register read succeeds; otherwise `false`.
   */
  virtual bool getTemperature(__attribute__((unused)) float &temperatureC) { return false; }

  /**
   * @brief Start a nonblocking RTC temperature conversion when supported.
   * @return `true` when the conversion command succeeds or a conversion is already in progress; otherwise `false`.
   */
  virtual bool startTemperatureConversion() { return false; }

  /**
   * @brief Get the RTC manufacturer name.
   * @return The manufacturer name.
   */
  virtual String getManufacturer() = 0;

  /**
   * @brief Get the RTC model name.
   * @return The model name.
   */
  virtual String getModel() = 0;

  /**
   * @brief Get the RTC manufacturer and model as one string.
   * @return The manufacturer and model separated by a space.
   */
  virtual String getMakeModel();

  /**
   * @brief Write one RTC register.
   *
   * The register address is adjusted automatically for RTCs that use the library's
   * special register-address format.
   *
   * @param reg The RTC register address.
   * @param val The byte value to write.
   * @return `true` when the I2C transmission succeeds; otherwise `false`.
   */
  virtual bool setRegister(byte reg, byte val);

  /**
   * @brief Read one RTC register.
   *
   * The register address is adjusted automatically for RTCs that use the library's
   * special register-address format. A failed I2C transaction returns `0xFF`.
   *
   * @param reg The RTC register address.
   * @return The value read from the register, or `0xFF` if the I2C transaction fails.
   */
  virtual byte getRegister(byte reg);

  /**
   * @brief Get the capability flags supported by this RTC.
   *
   * The returned value is a bitmask composed of `RTCCapability` values.
   *
   * @return The RTC capability bitmask.
   */
  byte getCapabilities() const { return _capabilities; }

  /**
   * @brief Get the alarm match modes supported by this RTC.
   * @return A bitmask composed of `AlarmMatch` values.
   */
  byte getSupportedAlarmMatches() const { return _supportedAlarmMatches; }

  /**
   * @brief Check whether the RTC supports a capability.
   * @param capability The capability to check.
   * @return `true` when the capability is supported; otherwise `false`.
   */
  bool supports(RTCCapability capability) const { return (_capabilities & static_cast<byte>(capability)) != 0; }

  /**
   * @brief Check whether the RTC supports an alarm match mode.
   * @param match The alarm match mode to check.
   * @return `true` when the alarm match mode is supported; otherwise `false`.
   */
  bool supports(AlarmMatch match) const { return (_supportedAlarmMatches & static_cast<byte>(match)) != 0; }

  /**
   * @brief Get the RTC's I2C address.
   * @return The 7-bit I2C address configured for this RTC.
   */
  virtual uint8_t getAddress() { return _i2caddr; }

 protected:
  /// Internal I2C behavior flags used by RTC implementations.
  enum RTCQuirk : byte {
    QUIRK_SREGADDR = 0x01,         ///< The register address is placed in the upper nibble.
    QUIRK_STOP_BEFORE_READ = 0x02, ///< A STOP is required before reading the addressed register.
  };

  /**
   * @brief Read one RTC register and report I2C failures separately from the byte value.
   * @param reg The RTC register address.
   * @param[out] val The value read from the register when the transaction succeeds.
   * @return `true` when the complete I2C transaction succeeds; otherwise `false`.
   *
   * This differs from `getRegister()` in that it returns a bool based on success and requires a reference to store the
   * read value.
   */
  bool readRegister(byte reg, byte &val);

  /**
   * @brief Convert a packed BCD byte to a binary value.
   * @param val The packed BCD value.
   * @return The corresponding binary value.
   */
  static byte bcd2bin(byte val) __attribute__((weak)) { return val - 6 * (val >> 4); }

  /**
   * @brief Convert a binary value to a packed BCD byte.
   * @param val The binary value to convert.
   * @return The corresponding packed BCD value.
   */
  static byte bin2bcd(byte val) __attribute__((weak)) { return val + 6 * (val / 10); }

  /**
   * @brief Decode a weekday bit field into the `tm` weekday numbering.
   * @param bits The RTC weekday bit field.
   * @return The decoded weekday in the `tm` range 0 through 6.
   */
  static byte decodewday(byte bits);

  /// Internal reference to the I2C address
  byte _i2caddr = 0;
  /// Internal reference to the I2C bus object
  TwoWire *_wire = NULL;
  /// Flag indicating whether the RTC has been successfully started
  bool _started = false;
  /// where clock reg starts
  byte _clockreg;
  /// base of weekday counting, either 0 (days range from 0-6) or 1 (days range from 1-7)
  byte _wdaybase;
  /// true when weekday comes before day in clock register
  bool _wdayfirst;
  /// lists all capabilities of this RTC
  byte _capabilities = 0;
  /// bitmask of supported AlarmMatch values
  byte _supportedAlarmMatches = 0;
  /// bitmask of internal I2C behavior flags
  byte _quirks = 0;
  /// if the 7th bit in a byte of the clock register must be set (bit 0=sec, bit 1=min, ...)
  byte _bit7set;
};

/// Base class for Alarms for DS-family RTCs
class DSAlarm : public RTC_I2C {
 public:
  bool clearErrorFlags() override;
  bool setAlarm(AlarmMatch match, const AlarmTime &alarm) override;

  /**
   * @brief Enable the DS-family alarm.
   * @return `true` if the alarm was successfully enabled; otherwise `false`.
   */
  bool enableAlarm() override;

  /**
   * @brief Disable the DS-family alarm.
   * @return `true` if the alarm was successfully disabled; otherwise `false`.
   */
  bool disableAlarm() override;

  /**
   * @brief Check the DS-family alarm status flag.
   *
   * @return `true` if the alarm status bit is set; otherwise `false`.
   */
  bool senseAlarm() override;

  /**
   * @brief Clear the DS-family alarm status flag.
   * @return `true` if the alarm status flag was successfully cleared; otherwise `false`.
   */
  bool clearAlarm() override;
};

/// Base class for Alarms for PCF-family RTCs
class PCFAlarm : public RTC_I2C {
 public:
  bool clearErrorFlags() override;
  bool setAlarm(AlarmMatch match, const AlarmTime &alarm) override;

  /**
   * @brief Check the PCF-family alarm status flag.
   *
   * @return `true` if the alarm status bit is set; otherwise `false`.
   */
  bool senseAlarm() override;

  /**
   * @brief Clear the PCF-family alarm status flag.
   * @return `true` if the alarm status flag was successfully cleared; otherwise `false`.
   */
  bool clearAlarm() override;
};
#endif
