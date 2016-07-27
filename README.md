# VentArrow-MkII

Updated code for VentArrow MKII system.  

Written by Ryan Bowers and Danny Toth July/August 2016

Code functions include XBee command support, a configurable failsafe cutdown timer, GPS support, SD support, and witty one-liners.

====================================================================================
XBee Commands!
====================================================================================
COMMAND|              FUNCTION               | NOTES 

-1     | Flight Start/Reset flight clock     | Logs action, returns nothing. Failsafe timer is 4 hours
  
01     | Open vent until further command     | Logs action, returns nothing

00     | Close Vent until further command    | Logs action, returns nothing

10     | Poll vent status                    | Returns string explaining status

11     | Poll for total open time in minutes | Returns string of open time in mins

02     | Add time to failsafe countdown      | Requires integer parameter, returns confirmation

12     | Poll remaining failsafe time        | Returns integer remaining failsafe time

13     | Check cutdown status                | Returns string explaining status

04     | Just extends cutdown arrow          | Logs action, returns nothing

05     | Just retracts cutdown arrow         | Logs action, returns nothing

1xxyy  | Open for xx minutes, yy seconds     | 5 digit input starting with 1. 2nd and 3rd are minutes, 4th and 5th are seconds

1xx    | Add xx minutes to failsafe          | 3 digit input starting with 1. 2nd and 3rd are minutes

-1xx   | Remove xx minutes from failsafe     | 3 digit input starting with -1. 2nd and 3rd are minutes




