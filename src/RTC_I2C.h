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


#define TIMEBYTES 7
#define RTC_CAP_32KHZ 0x01        // can generate 32 kHz signal
#define RTC_CAP_1HZ 0x02          // can generate 1 Hz signal
#define RTC_CAP_ALARM 0x04        // has alarm functionality to set hour and minute
#define RTC_CAP_HOURLY_ALARM 0x08 // can raise an alarm every hour
#define RTC_CAP_OFFSET 0x10       // has an offset register
#define RTC_CAP_TEMP 0x20         // has a temperature sensor
#define RTC_CAP_SREGADDR 0x40     // uses a strange format for register addresses (upper nibble)

#ifndef SECONDS_IN_DAY
/// @brief The number of seconds in a day
#define SECONDS_IN_DAY 86400L
#endif

enum class BatteryMode : int8_t {
  DISABLED, ///< Battery switch-over function is disabled, only VDD is used
  LSM, ///< Level Switching Mode (LSM). Switchover when VDD < threshold AND VBACKUP > threshold.  Use with a standard coin cell battery.
  DSM  ///< Direct Switching Mode (DSM). Switchover when VDD < VBACKUP.  Use when charging a rechargeable battery.
};

/// @brief Selects how a clock calibration offset is interpreted.
enum class OffsetMode : byte {
  EVERY_TWO_HOURS = 0, ///< Apply the correction every two hours (coarse correction mode).
  EVERY_MINUTE = 1,    ///< Apply the correction every minute (fine correction mode).
  RAW = 2              ///< Treat the offset as the RTC's raw calibration-register value.
};


/* A generic RTC base class */
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
   * @param mode The battery switchover mode to configure during initialization.
   * @return `true` if the RTC address is valid and the device responds on I2C; otherwise `false`.
   */
  bool begin(TwoWire *wi = &Wire, BatteryMode mode = BatteryMode::LSM);

  /**
   * @brief Initialize device-specific RTC settings.
   *
   * Each concrete RTC class must implement this function to configure the device
   * after I2C communication has been established.
   *
   * @param mode The battery switchover mode to configure.
   */
  virtual void init(BatteryMode mode = BatteryMode::LSM) = 0;

  /**
   * @brief Check whether the RTC is operating normally.
   *
   * Each concrete RTC class must implement this function using whatever validity
   * or oscillator-status information the device provides.
   *
   * @return `true` when the RTC reports a valid operating state; otherwise `false`.
   */
  virtual bool isValid(void) = 0;

  /**
   * @brief Set the RTC from a Unix timestamp.
   *
   * The timestamp is converted to calendar fields and then passed to the calendar-
   * based `setTime()` implementation.
   *
   * @param unixTimestamp The Unix timestamp to write to the RTC.
   */
  virtual void setTime(timestamp_t unixTimestamp);

  /**
   * @brief Set the RTC from an `epochTime` object.
   *
   * The object's timestamp is extracted and passed to the Unix-timestamp overload.
   *
   * @param eTime The time value to write to the RTC.
   */
  virtual void setTime(epochTime eTime);

  /**
   * @brief Set the RTC from a `tm` calendar-time structure.
   *
   * The base implementation writes the seconds, minutes, hours, weekday, day,
   * month, and year fields using the register layout described by the derived RTC.
   * The supported RTCs use years relative to 2000 for their year register.
   *
   * @param timeParts The calendar time to write to the RTC.
   */
  virtual void setTime(tm timeParts);

  /**
   * @brief Read the RTC as a Unix timestamp.
   *
   * The calendar time is read using the `tm` overload and converted to the library's
   * epoch representation. When `blocking` is true, the read waits for the RTC to
   * advance to the next second before reading the complete time.
   *
   * @param blocking If `true`, wait for the next second boundary before reading.
   * @return The current RTC time as a Unix timestamp.
   */
  virtual timestamp_t getTime(bool blocking = false);

  /**
   * @brief Read the RTC into a `tm` calendar-time structure.
   *
   * The base implementation reads the seven clock registers and converts the RTC's
   * register encoding into the standard `tm` field conventions. When `blocking` is
   * true, the read waits for the RTC to advance to the next second before reading.
   *
   * @param[out] timeParts The calendar-time structure to populate.
   * @param blocking If `true`, wait for the next second boundary before reading.
   */
  virtual void getTime(tm &timeParts, bool blocking = false);

  /**
   * @brief Enable the RTC's 32 kHz output.
   *
   * The base implementation is a no-op for RTCs that do not provide this feature.
   */
  virtual void enable32kHz(void) {};

  /**
   * @brief Disable the RTC's 32 kHz output.
   *
   * The base implementation is a no-op for RTCs that do not provide this feature.
   */
  virtual void disable32kHz(void) {};

  /**
   * @brief Enable the RTC's 1 Hz output.
   *
   * The base implementation is a no-op for RTCs that do not provide this feature.
   */
  virtual void enable1Hz(void) {};

  /**
   * @brief Disable the RTC's 1 Hz output.
   *
   * The base implementation is a no-op for RTCs that do not provide this feature.
   */
  virtual void disable1Hz(void) {};

  /**
   * @brief Configure an alarm that matches a specific hour and minute.
   *
   * The base implementation is a no-op for RTCs that do not provide this alarm
   * interface. Derived classes may interpret the alarm according to their hardware.
   *
   * @param minute The minute at which the alarm should match.
   * @param hour The hour at which the alarm should match.
   */
  virtual void setAlarm(__attribute__((unused)) byte minute, __attribute__((unused)) byte hour) {};

  /**
   * @brief Configure an alarm that matches a specific minute.
   *
   * The base implementation is a no-op for RTCs that do not provide this alarm
   * interface. Derived classes may interpret the alarm as a periodic hourly alarm.
   *
   * @param minute The minute at which the alarm should match.
   */
  virtual void setAlarm(__attribute__((unused)) byte minute) {};

  /**
   * @brief Enable the RTC alarm interrupt or alarm function.
   *
   * The base implementation is a no-op for RTCs without an alarm.
   */
  virtual void enableAlarm(void) {};

  /**
   * @brief Disable the RTC alarm interrupt or alarm function.
   *
   * The base implementation is a no-op for RTCs without an alarm.
   */
  virtual void disableAlarm(void) {};

  /**
   * @brief Check whether the RTC alarm condition is asserted.
   *
   * The base implementation reports no alarm condition.
   *
   * @return `true` if the RTC reports an asserted alarm; otherwise `false`.
   */
  virtual bool senseAlarm(void) { return false; };

  /**
   * @brief Clear the RTC alarm condition.
   *
   * The base implementation is a no-op for RTCs without an alarm.
   */
  virtual void clearAlarm(void) {};

  /**
   * @brief Apply a calibration offset to the RTC.
   *
   * The interpretation of `offset` is selected by `mode`. The base implementation
   * does nothing for RTCs without a calibration register.
   *
   * @param offset The calibration correction to apply.
   * @param mode Selects whether the correction is interpreted as a coarse, fine, or raw offset.
   */
  virtual void setOffset(__attribute__((unused)) int offset,
                         __attribute__((unused)) OffsetMode mode = OffsetMode::EVERY_MINUTE) {};

  /**
   * @brief Read the RTC calibration offset.
   *
   * The base implementation returns zero for RTCs without a calibration register.
   *
   * @return The calibration offset in the RTC-specific library representation.
   */
  virtual unsigned int getOffset(void) { return 0; };

  /**
   * @brief Read the RTC's temperature measurement.
   *
   * The base implementation returns the sentinel value `-128` for RTCs without a
   * temperature sensor.
   *
   * @return The RTC temperature, or `-128` when no temperature sensor is available.
   */
  virtual int getTemp(void) { return -128; };

  /**
   * @brief Get the RTC manufacturer name.
   *
   * Each concrete RTC class must provide its manufacturer name.
   *
   * @return The manufacturer name.
   */
  virtual String getManufacturer(void) = 0;

  /**
   * @brief Get the RTC model name.
   *
   * Each concrete RTC class must provide its model name.
   *
   * @return The model name.
   */
  virtual String getModel(void) = 0;

  /**
   * @brief Get the RTC manufacturer and model as one string.
   *
   * The result is formed by concatenating the manufacturer name, a space, and the
   * model name returned by the corresponding virtual functions.
   *
   * @return The manufacturer and model separated by a space.
   */
  virtual String getMakeModel(void);

  /**
   * @brief Write one RTC register.
   *
   * The register address is adjusted automatically for RTCs that use the library's
   * special register-address format.
   *
   * @param reg The RTC register address.
   * @param val The byte value to write.
   */
  virtual void setRegister(byte reg, byte val);

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
   * The returned value is a bitmask composed of the `RTC_CAP_*` constants.
   *
   * @return The RTC capability bitmask.
   */
  virtual byte getCapabilities(void) { return _capabilities; };

  /**
   * @brief Get the RTC's I2C address.
   *
   * @return The 7-bit I2C address configured for this RTC.
   */
  virtual uint8_t getAddress() { return _i2caddr; }

 protected:
  /**
   * @brief Convert a packed BCD byte to a binary value.
   *
   * @param val The packed BCD value.
   * @return The corresponding binary value.
   */
  static byte bcd2bin(byte val) __attribute__((weak)) { return val - 6 * (val >> 4); }

  /**
   * @brief Convert a binary value to a packed BCD byte.
   *
   * @param val The binary value to convert.
   * @return The corresponding packed BCD value.
   */
  static byte bin2bcd(byte val) __attribute__((weak)) { return val + 6 * (val / 10); }

  /**
   * @brief Decode a one-hot weekday bit field into the `tm` weekday numbering.
   *
   * The first set bit is interpreted as weekday zero, with bits seven through one
   * corresponding to weekdays six through zero respectively.
   *
   * @param bits The RTC weekday bit field.
   * @return The decoded weekday in the `tm` range 0 through 6.
   */
  static byte decodewday(byte bits);
  byte _i2caddr = 0;
  TwoWire *_wire = NULL;
  bool _started = false;
  byte _clockreg;     // where clock reg starts
  byte _wdaybase;     // base of weekday counting, either 0 (days range from 0-6) or 1 (days range from 1-7)
  bool _wdayfirst;    // true when weekday comes before day in clock register
  byte _capabilities; // lists all capabilities of this RTC
  byte _bit7set;      // if the 7th bit in a byte of the clock register must be set (bit 0=sec, bit 1=min, ...)
};

class DSAlarm : public RTC_I2C {
 public:
  /**
   * @brief Configure a DS-family alarm for a specific hour and minute.
   *
   * The alarm matches the requested hour and minute and ignores the day by using
   * the alarm's always-match day setting.
   *
   * @param minute The minute at which the alarm should match.
   * @param hour The hour at which the alarm should match.
   */
  void setAlarm(byte minute, byte hour) override;

  /**
   * @brief Configure a DS-family alarm for a specific minute of every hour.
   *
   * The hour and day fields are configured as always-match fields.
   *
   * @param minute The minute at which the alarm should match.
   */
  void setAlarm(byte minute) override;

  /**
   * @brief Enable the DS-family alarm.
   */
  void enableAlarm(void) override;

  /**
   * @brief Disable the DS-family alarm.
   */
  void disableAlarm(void) override;

  /**
   * @brief Check the DS-family alarm status flag.
   *
   * @return `true` if the alarm status bit is set; otherwise `false`.
   */
  bool senseAlarm(void) override;

  /**
   * @brief Clear the DS-family alarm status flag.
   */
  void clearAlarm(void) override;
};

class PCFAlarm : public RTC_I2C {
 public:
  /**
   * @brief Configure a PCF-family alarm for a specific hour and minute.
   *
   * The day and weekday fields are configured as always-match fields.
   *
   * @param minute The minute at which the alarm should match.
   * @param hour The hour at which the alarm should match.
   */
  void setAlarm(byte minute, byte hour) override;

  /**
   * @brief Configure a PCF-family alarm for a specific minute of every hour.
   *
   * The hour, day, and weekday fields are configured as always-match fields.
   *
   * @param minute The minute at which the alarm should match.
   */
  void setAlarm(byte minute) override;

  /**
   * @brief Check the PCF-family alarm status flag.
   *
   * @return `true` if the alarm status bit is set; otherwise `false`.
   */
  bool senseAlarm(void) override;

  /**
   * @brief Clear the PCF-family alarm status flag.
   */
  void clearAlarm(void) override;
};
#endif
