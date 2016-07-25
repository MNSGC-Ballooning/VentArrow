/*  This code is designed to run on a Teensy 3.2 on board the VentArrow MkII.
 *  Currently supports use of GPS, SD datalogging, and xBee communications.
 *  Diff Pressure sensor to be added later.
 *  Can be used as both an autonomous system or commanded from ground. */

/*  By: Ryan Bowers, with assistance from Danny Toth
 *  Based off VentArrow MkI code by Christopher Gosch */


/*  Teensy pin connections:
 *   -------------------------------------------------------------------------------------------------------------------------
 *   Component                    | Pins used             | Notes
 *   
 *   Adafruit Ultimate GPS        | D0-1 (RX/TX1)         | Red LED connects to fix pin on GPS
 *   Series 2 xBee                | D7-8 (RX/TX3)         |
 *   Sparkfun SD Breakout         | D10-13 (CS,DO,DI,SCK1)|
 *   Power LED (Green)            | D23                   | Turns on at startup - off indicates loss of power or critical error
 *   Data LED (Yellow)            | D22                   | Flashes when writing to SD card
 *   Vent Actuator                | D16-17, D18 (A4)      | Set D16 high to open, D17 high to close. D18/A4 is feedback
 *   Arrow Actuator               | D19-20, D21 (A7)      | Set D19 high to exend, D20 high to retract. D21/A7 is feedback
 *   -------------------------------------------------------------------------------------------------------------------------
 */
