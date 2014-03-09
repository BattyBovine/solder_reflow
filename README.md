Solder Reflow
=============

Software for the ATmega168PA which controls a home-made solder reflow oven.



Pin layout
==========

PD2 - Door switch (detects when door is opened)
PD3 - SSR (controls heating elements)
PD5 - Rotary encoder B
PD6 - Rotary encoder A
PD7 - Enter/Cancel button
PC0 - Thermocouple measurement (receives output from AD8495 chip)



Bill Of Materials (incomplete)
==============================

- ATmega168PA or ATmega328P
- HD44780-compatible 20x4 character LCD display with I2C backpack
- K-type thermocouple
- AD8495 thermocouple amplifier
- Rotary encoder with push-button switch
- SSR-25DA or similar solid-state relay
- 16MHz crystal oscillator
- 22pF ceramic capacitor (x2)
- 4.7kOhm resistor (x2)
