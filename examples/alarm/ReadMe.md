# Alarm Interrupts<!--! {#example_alarm} -->

This example demonstrates receiving alarm interrupts from a DS3231 real-time clock.
It configures Alarm 1 to interrupt every second for 15 seconds and verifies that 15 interrupts were received.
It then configures Alarm 1 to interrupt at second zero of every minute, runs for two minutes, and verifies that two interrupts were received.
Finally, it reads the current clock time, sets an alarm for five seconds later, verifies that the alarm fires once, and repeats that test five times.

Connect the DS3231 `INT/SQW` output to digital pin 10 on the Arduino board.
The `INT/SQW` output is active low, and the example enables the Arduino pin's internal pull-up resistor.
Change `alarmInterruptPin` in the sketch if a different input pin is required.

On AVR boards, the example uses the EnableInterrupt library so the alarm can be connected to any pin supported by that library.
On other Arduino architectures, it uses the standard `attachInterrupt()` function.

The interrupt service routine increments a volatile counter.
Serial output and I2C communication are unsafe inside an interrupt service routine, so the foreground code clears the DS3231 alarm flag and prints `Clock alarm detected` for each interrupt.

_______

<!--! @section example_alarm_code The Complete Code -->

<!--! @include{lineno} alarm/alarm.ino -->
