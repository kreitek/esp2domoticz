# esp2domoticz
ESP8266 communication with DomoticZ server amoung wifi network.

The sketch is able to update an sensor from ESP to DomoticZ(dht22 in the example).
The sketch is able to switch on/off from DomoticZ to ESP:

It's necessary define switch/button/light on DomoticZ and set it up with:

For your switching on "switches->edit(on your specific switch)->On action":
  http://<esp username>:<esp password>@<esp ip>/control?cmd=light,1
  
For your switching off "switches->edit(on your specific switch)->Off action":
  http://<esp username>:<esp password>@<esp ip>/control?cmd=light,0
