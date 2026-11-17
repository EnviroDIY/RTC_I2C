# Alarm Interrupts<!--! {#example_alarm} -->

This example demonstrates receiving periodic countdown and calendar alarm interrupts from an RV3032 real-time clock.
It first configures a two-second periodic countdown alarm and verifies that ten interrupts are received.
It then runs each calendar alarm test supported by the selected RTC.

Connect the RTC's `INT` output to selected digital pin on the Arduino board.
The `INT` output is active low, and the example enables the Arduino pin's internal pull-up resistor.
Change `alarmInterruptPin` in the sketch if a different input pin is required.

On AVR boards, the example uses the EnableInterrupt library so the alarm can be connected to any pin supported by that library.
On other Arduino architectures, it uses the standard `attachInterrupt()` function.

The interrupt service routine increments a volatile counter.
Serial output and I2C communication are unsafe inside an interrupt service routine, so the foreground code clears the active countdown or calendar alarm flag and reports each interrupt.

_______

<!--! @section example_alarm_code The Complete Code -->

<!--! @include{lineno} alarm/alarm.ino -->
