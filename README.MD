SMS-A-SKETCH
============

This is the source code for the SMS-A-Sketch demo I made for the smspower.org coding competition 2021.
[1]: https://www.smspower.org/Homebrew/SMSASketch-SMS

SMS-A-Sketch is a simple demo where you can use two paddle controllers to draw on the screen, etch-a-sketch style. Not everyone has a paddle controller, let alone two of them... So other kinds of controllers are supported:

  * Standard controllers. Press 1 to enable/disable drawing, 2 to save, and pause to "shake" the sms-a-sketch screen to erase.
  * Sega Mega Mouse (MK-1645): Hold the left button to draw, the middle button to "shake", and the mouse "start button" (the recessed button) to save.
  * Graphics Board (select it by pressing PAUSE at the diagnostic screen)
  * Sports Pad (select it by pressing PAUSE at the diagnostic screen)
  * Light Phaser (select it by pressing PAUSE at the diagnostic screen)

I had been thinking of doing a drawing demo using two paddle controllers ever since I wrote code to support paddles in my Donkey.SMS game. I also wanted to learn how tiles are made (bit planes, etc) for another project (run-time tile combining for a sokoban clone) so figuring out how to draw pixels seemed like a good exercise.

I got a bit carried away and added an intro with music, scalable text using a simple vector font, moving knobs the while writing/drawing stuff...
Tested on real hardware using a Japanese SMS and a US Megadrive.

Disclaimer: The code quality is low, this was just an experiment. If something does not seem to make sense well... it probably does not. Don't look too hard for a reason why I did this or that.

