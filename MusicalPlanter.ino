/***************************************************
 * This sketch was created for Bompas&Parr
 *
 * It uses a capacitive sensor to control the volume of an mp3
 * file being played on the Adafruit VS1053 Codec Breakout shield.
 *
 * https://www.adafruit.com/products/1381
 *
 * Adapted by Luke Sturgeon for Bompass&Parr.
 * Written by Limor Fried/Ladyada for Adafruit Industries.
 * BSD license, all text above must be included in any redistribution
 ****************************************************/

// include SPI, MP3 and SD libraries
#include <SPI.h>
#include <Adafruit_VS1053.h>
#include <SD.h>

// include CapacitiveSensor library
#include <CapacitiveSensor.h>


// These are the pins used for the breakout example
#define BREAKOUT_RESET  9      // VS1053 reset pin (output)
#define BREAKOUT_CS     10     // VS1053 chip select pin (output)
#define BREAKOUT_DCS    8      // VS1053 Data/command select pin (output)
// These are the pins used for the music maker shield
#define SHIELD_RESET  -1      // VS1053 reset pin (unused!)
#define SHIELD_CS     7      // VS1053 chip select pin (output)
#define SHIELD_DCS    6      // VS1053 Data/command select pin (output)

// These are common pins between breakout and shield
#define CARDCS 4     // Card chip select pin
// DREQ should be an Int pin, see http://arduino.cc/en/Reference/attachInterrupt
#define DREQ 3       // VS1053 Data request, ideally an Interrupt pin


Adafruit_VS1053_FilePlayer musicPlayer =
  Adafruit_VS1053_FilePlayer(SHIELD_RESET, SHIELD_CS, SHIELD_DCS, DREQ, CARDCS);


// These are the pins used for the capacitive sensor
CapacitiveSensor capSensor = CapacitiveSensor(8, 9);
int capSensorValue = 0;
int capThreshold = 1500;
int currentVolume = 100;
int loudVolume = 0;
int quietVolume = 100;


////

void setup() {
  Serial.begin(9600);
  Serial.println("Adafruit VS1053 Library Test");

  // initialise the music player
  if (! musicPlayer.begin()) { // initialise the music player
    Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
    while (1);
  }
  Serial.println(F("VS1053 found"));

  musicPlayer.sineTest(0x44, 500);    // Make a tone to indicate VS1053 is working

  if (!SD.begin(CARDCS)) {
    Serial.println(F("SD failed, or not present"));
    while (1);  // don't do anything more
  }
  Serial.println("SD OK!");

  // Set volume for left, right channels. lower numbers == louder volume!
  musicPlayer.setVolume(currentVolume, currentVolume);

  // This option uses a pin interrupt. No timers required! But DREQ
  // must be on an interrupt pin. For Uno/Duemilanove/Diecimilla
  // that's Digital #2 or #3
  // See http://arduino.cc/en/Reference/attachInterrupt for other pins
  // *** This method is preferred
  if (! musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT))
    Serial.println(F("DREQ pin is not an interrupt pin"));

  // stop the sensor from auto calibrating
  //  capSensor.set_CS_AutocaL_Millis(0xFFFFFFFF);
}

void loop() {

  // Start playing a file, then we can do stuff while waiting for it to finish
  if (! musicPlayer.startPlayingFile("track001.mp3")) {
    Serial.println("Could not open file track001.mp3");
    while (1);
  }

  Serial.println(F("Started playing"));

  while (musicPlayer.playingMusic) {
    // file is now playing in the 'background' so now's a good time
    // to do something else like handling LEDs or buttons :)

    // check for sensor
    capSensorValue = capSensor.capacitiveSensor(64);

    if (capSensorValue > capThreshold) {
      // is touching
      if (currentVolume > loudVolume) {
        // make soud louder by making volume value lower!
        currentVolume -= 20;
        // limit the end
        if (currentVolume < loudVolume) {
          currentVolume = loudVolume;
        }
        Serial.println("vol++");
        musicPlayer.setVolume(currentVolume, currentVolume);
      }
    } else {
      // not touching - lower Volume
      if (currentVolume < quietVolume) {
        // make sounder quiet by making volume value higher!
        currentVolume += 1;
        // limit the end
        if (currentVolume > quietVolume) {
          currentVolume = quietVolume;
        }
        Serial.println("vol--");
        musicPlayer.setVolume(currentVolume, currentVolume);
      }
    }

    Serial.print(capSensorValue);
    Serial.print("\t");
    Serial.println(capThreshold);

    delay(250);
  }

  Serial.println("Done playing music. Will automatically repeat.");
}
