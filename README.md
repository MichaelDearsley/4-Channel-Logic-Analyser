# 4-Channel Arduino Logic Analyser

A simple 4-channel digital logic analyser built using an Arduino and Python. The project captures four digital signals at approximately 100 kHz, transfers the captured data to a computer over USB serial, displays the waveforms in Python, and provides UART decoding on Channel 1.

## Features

* 4 digital input channels
* Approximately 100 kHz sampling rate
* 1,000 samples per capture
* Python waveform visualisation using Matplotlib
* USB serial communication between Arduino and Python
* UART decoding on Channel 1
* UART test transmitter on Arduino
* Decoded UART messages displayed on the waveform
* Button-triggered test capture

## Hardware

* Arduino with a 16 MHz ATmega328P-compatible microcontroller
* Breadboard
* Push button
* Jumper wires

### Pin configuration

| Function         | Arduino pin |
| ---------------- | ----------: |
| Channel 1        |          D2 |
| Channel 2        |          D3 |
| Channel 3        |          D4 |
| Channel 4        |          D5 |
| Test button      |          D6 |
| UART transmitter |          D8 |

The digital inputs use the Arduino's internal pull-up resistors.

## How it works

The Arduino uses Timer 1 in CTC mode to generate an interrupt approximately every 10 microseconds. At each interrupt, the states of D2-D5 are captured simultaneously and stored as a 4-bit value in a 1,000-byte buffer.

The four channels are packed into a single value:

```text
CH4 CH3 CH2 CH1
 1   0   1   1
```

This allows all four digital channels to be captured at the same instant.

Once 1,000 samples have been collected, the Arduino sends the captured data to the Python program over USB serial at 115200 baud.

Python converts the received binary values back into the four individual channels and displays them as digital step waveforms.

## UART decoding

Channel 1 also supports UART decoding.

For testing, the Arduino generates a 9600-baud UART signal on D8. D8 is connected to Channel 1 (D2), allowing the analyser to capture its own UART transmission.

The UART transmitter uses Timer 1 to generate the signal. The test program sends:

```text
HELLO
```

Python detects the UART start bit, samples the eight data bits, reconstructs the byte and converts it into an ASCII character.

For example:

```text
01001000 → 72 → H
```

The decoded message is displayed above the waveform:

```text
UART: HELLO
```

## UART test setup

For the built-in UART test:

```text
Arduino D8 ───────► Arduino D2 (CH1)
Arduino GND ──────► common GND
```

The D6 button starts the test capture.

Channels 2-4 can be connected to other digital signals for observation. For unused inputs, appropriate pull-up/pull-down arrangements should be used to prevent floating inputs.

## Limitations

* Approximately 100 kHz sampling rate
* 1,000 samples per capture
* UART decoding currently configured for the tested UART format and baud rate
* Digital signals only
* No analogue voltage measurement
* No automatic SPI or I²C decoding
* Input voltage must remain within the safe limits of the Arduino being used

## Future improvements

* Adjustable sampling rate
* Adjustable UART baud rate
* Better UART framing/error detection
* SPI decoding
* I²C decoding
* Trigger configuration
* Longer capture buffers
* Frequency and duty-cycle measurements
* Improved graphical interface
* More efficient data transfer

## Project status

**Working prototype complete.**

https://github.com/user-attachments/assets/a227f25e-0a18-4238-bb18-60a180c8d443
