# VentArrow-MkII

Updated code for VentArrow MkII system.  

Written by Ryan Bowers and Danny Toth July/August 2016

Code functions include XBee command support, a configurable failsafe cutdown timer, GPS support, SD support, and witty one-liners.

====================================================================================
XBee Commands!
====================================================================================
COMMAND|              FUNCTION               | NOTES 

-1     | Flight Start/Reset flight clock     | Sent at start of flight to signal start time. Resets flight clock with reuse
  
0      | Open vent until further command     | Returns success/fail

1      | Close Vent until further command    | Returns success/fail

4      | Just extends cutdown arrow          | Returns success/fail. Intended for testing only

5      | Just retracts cutdown arrow         | Returns success/fail

10     | Poll vent status                    | Returns current vent state (Open/Closed)

11     | Poll for total open time in minutes | Returns total time vent has been open in min:sec

12     | Poll remaining failsafe time        | Returns time until auto cut in min:sec

42     | Initiate Cutdown					 | Extends the arrow for 10 seconds, then retracts automatically and checks for burst

43     | Check cutdown status                | Returns success/fail based on altitude change over last 10 seconds

1xxyy  | Open vent for xx min, yy sec		 | 5 digit input starting with 1. 2nd and 3rd are minutes, 4th and 5th are seconds

1xx    | Add xx minutes to failsafe          | 3 digit input starting with 1. 2nd and 3rd are minutes

-1xx   | Remove xx minutes from failsafe     | 3 digit input starting with -1. 2nd and 3rd are minutes




