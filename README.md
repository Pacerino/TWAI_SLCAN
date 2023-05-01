# TWAI SLCAN

This is a quick and dirty implementation of SLCAN for the ESP32-WROOM-1 on [RejsaCAN v3.4](https://github.com/MagnusThome/RejsaCAN-ESP32).

This repo is based on the work of [mintynet/esp32-slcan](https://github.com/mintynet/esp32-slcan)

SLCAN/LAWICEL protocol definition can be found [here](http://www.can232.com/docs/canusb_manual.pdf)

## LED Explanation
| LED        | PIN | Explanation                                                                                                                   |
|------------|-----|-------------------------------------------------------------------------------------------------------------------------------|
| BLUE_LED   | 10  | The Blue LED indicates the status of the CAN Bus, Blink once means CAN 0 is open, Blink twice means the CAN 0 has been closed |
| YELLOW_LED | 11  | This LED indicates an error. I've implemented this to debug the code/show that an error occurred                          |

## Disclaimer
I'm not an C++ developer and therefore missing the knowledge. What im seeing from the serial console looks correct. Right now sending data from ESP32 to the CAN bus is not implemented! The TWAI driver runs in `TWAI_MODE_LISTEN_ONLY`.