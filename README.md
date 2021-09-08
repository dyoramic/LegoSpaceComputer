# Lego Space Computer
![lego_space_computer](https://user-images.githubusercontent.com/90341720/132551425-b4114dd0-1e21-4fd4-8c4c-eedace00033a.jpg)
The classic Lego space console/computer has always been one of my favorite bricks. I decided to make a working one for myself and shared it via [YouTube](https://youtu.be/Y-henu_-17A). Some people who saw the video have asked me to share the code, so here it is.
### Warning
**This is junk code that I wrote quickly for myself and never intended to share. On my best day I'm a pretty poor programmer and this was far from my best day. You would probably be faster writing the whole thing from scratch rather than starting from this code. There is pretty much 0 error handling in this code and multiple bugs!**

### Parts
I used the follwing parts:
- ESP32S development board!
- [Adafruit 1.5" OLED breakout board](https://www.adafruit.com/product/1431)
- 3 Push switches

The case was 3D printed but I don't intend to share the STLs at this point.

### Code
In order to use the code you will need to load the images in the data file onto the flash memory of the ESP32. If you're using the Arduino IDE you can use the [FileSystem Uploader](https://randomnerdtutorials.com/install-esp32-filesystem-uploader-arduino-ide/) plugin to do this. 

You will also need fill in your WiFi network name (line 11) and password (line 12). The time zone for the clock can be changed on line 81.
