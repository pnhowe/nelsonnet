Nelson Net
==========

Monitoring of Pond level and other things, so we can sleep at night


Install
=======

bridge
------

bridge.py watches a serial port that has a feather with the "reciever" code running, and loads the data into Graphite

external.py pulls other data points and loads them into Graphite


reciever
--------

Install to a Feather M0 FR9x LoRa Radio, attach the USB cable to the machine running bridge.py1


sender
------

make a copy of sender.ino and modify it for the data gathering.  Install to a Feather M0 FR9x LoRa Radio.

modifications to make:
modify "SENDER_NAME", this will be prefixed before the values when put in to Graphite
modify "values", change the lenth of the array to 1+ the number of values.
Make sure the last value is '{"\0", 0, 0}'.
