/* This is the class for RV3028 that can be used as part of the RTC_I2C library */

#ifndef _RTC_RV3028_H_
#define _RTC_RV3028_H_

#include <RTC_I2C.h>


/// 7-bit I2C slave address for the RV-3028.
#define RV3028_ADDRESS 0x52
/// Seconds register; the clock/calendar register sequence begins at address 0x00.
#define RV3028_CLOCKREG 0x00
/// Minutes Alarm register; referred to as Minutes Alarm in documentation (ADDRESS 0x07).
#define RV3028_ALARM 0x07
/// Status register; referred to as Status in documentation (ADDRESS 0x0E).
#define RV3028_STATUS 0x0E
/// Control 1 register; referred to as Control 1 in documentation (ADDRESS 0x0F).
#define RV3028_CONTROL 0x0F
/// EEPROM Clkout register; referred to as Clkout in documentation (ADDRESS 0x35).
#define RV3028_CLKOUT 0x35
/// EEPROM Backup register; referred to as Backup in documentation (ADDRESS 0x37), containing the BSM field.
#define RV3028_BSM 0x37
/// EEPROM Offset register; referred to as Offset in documentation (ADDRESS 0x36).
#define RV3028_OFFSET 0x36
/// EE Command register; referred to as EECMD in documentation (ADDRESS 0x27).
#define RV3028_EECMD 0x27
/// EE Data register; referred to as EEDATA in documentation (ADDRESS 0x26).
#define RV3028_EEDATA 0x26
/// EE Address register; referred to as EEADDR in documentation (ADDRESS 0x25).
#define RV3028_EEADDR 0x25

/// Weekday numbering used by the RTC: 0 through 6.
#define RV3028_WDAYBASE 0
/// The weekday register comes after the day-of-month register in the clock register sequence.
#define RV3028_WDAYFIRST true
/// No clock-register bit 7 must be forced when writing time.
#define RV3028_BIT7 0
/// Capabilities supported by the RV-3028 implementation.
#define RV3028_CAP (RTC_CAP_32KHZ | RTC_CAP_1HZ | RTC_CAP_ALARM | RTC_CAP_HOURLY_ALARM | RTC_CAP_OFFSET)


class RV3028 : public RTC_I2C {
 public:
  RV3028(void) {
    _i2caddr = RV3028_ADDRESS;
    _clockreg = RV3028_CLOCKREG;
    _wdaybase = RV3028_WDAYBASE;
    _wdayfirst = RV3028_WDAYFIRST;
    _capabilities = RV3028_CAP;
    _bit7set = RV3028_BIT7;
  };
  void init(BatteryMode mode = BatteryMode::LEVEL_SWITCHING) override;
  bool isValid(void) override;
  void setAlarm(byte minute, byte hour) override;
  void setAlarm(byte minute) override;
  bool senseAlarm(void) override;
  void clearAlarm(void) override;
  void enableAlarm(void) override;
  void disableAlarm(void) override;
  void enable32kHz(void) override;
  void disable32kHz(void) override;
  void enable1Hz(void) override;
  void disable1Hz(void) override;
  /**
   * @copydocs RTC_I2C::setOffset()
   *
   * This RTC has only one calibrated correction mode. Both `OffsetMode::FINE_OFFSET` and `OffsetMode::COARSE_OFFSET`
   * are treated identically.
   *
   * Negative values make the clock faster by 0.9537 ppm/LSB
   * The range of the internal parameter goes from -256 to +255.
   * This means that possible values for offset range from -243.2 ppm to +244.1 ppm.
   */
  void setOffset(int offset, OffsetMode mode = OffsetMode::FINE_OFFSET) override;
  unsigned int getOffset(void) override;
  String getManufacturer(void) override;
  String getModel(void) override;

 protected:
  void updateEEPROMByte(byte reg);
};
#endif

// cSpell:ignore EECMD EEDATA EEADDR
