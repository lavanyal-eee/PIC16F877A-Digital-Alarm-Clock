
# Digital Alarm Clock using PIC16F877A

## Project Overview
This project implements a Digital Alarm Clock using the PIC16F877A microcontroller and Embedded C. It displays time and date on a 16x2 LCD and supports three configurable alarms.

## Hardware Components
- PIC16F877A Microcontroller
- 16x2 LCD Display
- Push Buttons
- Buzzer

## Software Tools
- MPLAB X IDE
- XC8 Compiler
- PICSimLab (for simulation, if used)

## Features
- Display current time and date
- Set time and date
- Configure and view three alarms
- Once, Daily, and Weekly alarm modes
- Enable and disable alarms
- Edit and delete alarms
- Buzzer alert when an alarm triggers
- Manual alarm stop using a button
- Automatic alarm stop
- LCD-based configuration menu
- Configuration idle timeout

## Source Files
- `main.c` - Main program and alarm logic
- `lcd.c` - LCD control functions
- `lcd.h` - LCD function declarations
- `config.h` - Configuration definitions
- `config_877A.h` - Microcontroller configuration definitions
- `atraso.h` - Delay function declarations
- `Makefile` - Build instructions
- `DigitalAlarmClock.X.zip` - Project archive

## Working Principle
The PIC16F877A manages the clock, calendar, alarm settings, button inputs, LCD display, and buzzer output. Users can configure the time, date, and alarms using the available buttons.

## Testing
The implemented features were tested, including time and date setting, alarm configuration, Once/Daily/Weekly modes, alarm enable/disable, alarm deletion, buzzer activation, manual stop, automatic stop, and configuration timeout.

## Author
Lavanya L.

