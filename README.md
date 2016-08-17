# VentArrow-MkII

Updated code for VentArrow MkII system.  

Written by Ryan Bowers and Danny Toth July/August 2016

Code functions include XBee command support, a configurable failsafe cutdown timer, GPS support, SD support, and witty one-liners.

====================================================================================
XBee Commands!
====================================================================================

Identifier: VA

COMMAND|              FUNCTION               | NOTES 

FS     | Flight Start/Reset flight clock     | Sent at start of flight to signal start time. Resets flight clock with reuse

VF	   | Poll raw ventFeed number			 | Returns the number of the current vent position from 0-1023 (actual minimum varies)
  
OV     | Open vent until further command     | Returns success/fail

CV     | Close Vent until further command    | Returns success/fail

CAL	   | Calibrate Vent						 | Causes vent to open and close for feedback calibration. Results saved but not sent

EXT    | Just extends cutdown arrow          | Returns success/fail. Intended for testing only

RET    | Just retracts cutdown arrow         | Returns success/fail

VS     | Poll vent status                    | Returns current vent state (Open/Closed)

TO     | Poll for total open time in minutes | Returns total time vent has been open in min:sec

TL     | Poll remaining failsafe time        | Returns time until auto cut in min:sec

GPS	   | Poll for current GPS data			 | Forces a new GPS string to be sent

LS	   | Get list of AutoVent alts and times | Returns numbered list of planned AutoVents. Use # in commands for editing AutoVents

AR	   | Get most recent ascent rate		 | Returns the last ascent rate calculated in ft/s. Updates every AutoVent

ELF    | Initiate Cutdown					 | Extends the arrow for 10 seconds, then retracts automatically and checks for burst

IBS    | Check cutdown status                | Returns success/fail based on altitude change over last 10 seconds

HBS	   | Set hasBurst to true				 | Send to tell system burst has occured if not detected automatically

OT*xyy | Open vent for *x min, yy sec		 | May contain any number of digits after OT, but hundreds digit and greater are minutes

AT*x   | Add *x minutes to failsafe          | Adds time before attempting automatic cutdown. May contain any number of digits

RT*x   | Remove *x minutes from failsafe	 | Removes time until automatic cutdown. May contain any number of digits

AC*x   | Set auto cut to *x,000ft			 | Set altitude to automatically attempt cutdown. May contain any number of digits

AL#*xyy| Set Auto # time of x min, y sec	 | AutoVent to set time is # (from LS). May contain any number of digits, 100s + are min

AA#*x  | Set target of *x,000ft for Auto x	 | AutoVent to set altitude is # (from LS). May contain any number of digits