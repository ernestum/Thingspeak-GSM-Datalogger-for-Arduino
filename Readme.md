This is code that watches two binary sensors and sends out updates to thingspeak using an Arduino GSM shield.
It talks to the GSM modem in plain AT commands without the need for the officital GSM library which is a bit crappy.
It is designed to be very reliable (adding lots of delays and retransmission logic) at sending out those updates, not fast!

If you want to do something similar, I recommend using the ThingspeakConnection.h file because the whole project is actually designed around a shield that allows to cut all power away from the GSM shield to save power. The ThingspeakConnection.h file contains only the interesting communication features without all that clutter!
