# Adding a Super Mini Board Definition in PlatformIO 

Two files need to be added to define the Super Mini ESP32-C3 development board described in [First Look at the Super Mini ESP32-C3](https://sigmdel.ca/michel/ha/esp8266/super_mini_esp32c3_en.html). They are contained in this directory.

1. `super_mini_esp32c3.json` is the board definition file. It references the next file.

2. `pins_arduino.h` is the variant pin definition file.

They should be copied to the correct subdirectory of the `platformio` directory. The location of the latter depends on the operating system and where PlatformIO was initially installed. The *default* locations are as follows.

Linux: 
 1. `~/.platformio/platforms/espressif32/boards/super_mini_esp32c3.json`

 2. `~/.platformio/packages/framework-arduinoespressif32/variants/super_mini_esp32c3/pins_arduino.h`

In practice this means:   `home/<user>/.platformio/...`.

Windows:

 1.  `%HOMEPATH%\.platformio\platforms\espressif32\boards\super_mini_esp32c3.json`

 2.  `%HOMEPATH%\.platformio\packages/framework-arduinoespressif32/variants/super_mini_esp32c3/pins_arduino.h`

In practice this means: `C:\Users\<user>\.platformio\...`.


Note that in the second case, the `super_mini_esp32c3` directory in which `pins_arduino.h` is saved will have to be created.

**NOTE:**

The above will work, in the sense that the new board will show up in the Project Wizard. However, the two additional files will be deleted when the `espressif32` platform is updated or reinstalled.

There is a simple way to add a locally defined custom board definition to a project. See [Custom Embedded Boards](https://docs.platformio.org/en/latest/platforms/creating_board.html#installation) and [boards_dir](https://docs.platformio.org/en/latest/projectconf/sections/platformio/options/directory/boards_dir.html#projectconf-pio-boards-dir). However adding the variant `pins_arduino.h` file is not that easy. See [Custom board missing file: pins_arduino.h: No such file or directory](https://community.platformio.org/t/custom-board-missing-file-pins-arduino-h-no-such-file-or-directory/36622). 

Hopefully, a future release of the `espressif32` platform will include a correct Super Mini dev board definition and the problem will become moot.
