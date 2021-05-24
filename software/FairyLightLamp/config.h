
// Enable debug prints, uncomment to watch the MySensors debug log.
//#define MY_DEBUG

// Comment to turnof Serial communication (saves space) comment it when you want to debug.
//#define MY_DISABLED_SERIAL

// Set the wait for ready to zero if you want the light the be operable even when the Gateway is off.
//#define MY_TRANSPORT_WAIT_READY_MS 0


// Enable and select radio type attached
#define MY_RADIO_NRF24
//#define MY_NODE_ID 21 // This is the development node ID. I think we should port it to the first protottype as well.

#include <SPI.h>
#include <MySensors.h>

#define MS_SketchName "FairyLight lantern"
#define MS_SketchVersion "1.0"

#define CHILD_ID_LIGHT 1

const uint8_t EEPROM_DIM_LEVEL_LAST = 1;
const uint8_t DimmerDefaultValue = 5;

MyMessage lightMsg( CHILD_ID_LIGHT, V_LIGHT );
MyMessage dimmerMsg( CHILD_ID_LIGHT, V_DIMMER );

// Definitions for the light level
const uint8_t MIN_BRIGHTNES = 1;
const uint8_t MAX_BRIGHTNESS = 15; // 15 level lamp means around 17 pwm steps per level (since we start at 10)

const uint8_t MIN_MYSENSORS_BRIGHTNES = 1;
const uint8_t MAX_MYSENSORS_BRIGHTNESS = 100; // 15 level lamp means around 17 pwm steps per level (since we start at 10)


// Variables for storing the current state of power, brightness and the controls.
bool    powerState = false; // false means light off, true means light on
uint8_t lightBrightness, newBrightness; // the brightness and the potential new brightness
long oldEncoderPosition, newEncoderPosition; // The last read encoder position and the potential new position
bool switchStateUpdated; // Indicates wether or not the state of the power switch has been changed,

const unsigned long HEART_BEAT_INTERVAL = 1800000; // We send a heart beat ruffly each half hour

/*
 Returns the given gateway brightness to the lamps brightness level.
 The lamp doesn't support 1-100 by design. Because it's really anoying having to turn a
 rotary encode 100 times to go from max to low or the other way arround. 15 brightness levels seem
 to be good enough. Which can all be changed by changing the constants in this file.
 */
uint8_t converFromMySensorsBrightness( int aBrightness ) {
  return map( aBrightness, MIN_MYSENSORS_BRIGHTNES, MAX_MYSENSORS_BRIGHTNESS, MIN_BRIGHTNES, MAX_BRIGHTNESS );
}

/*
 Returns the current light brightness converted to the gateway brightness (Domotiz supports 1-100)
 */
uint8_t getConvertedMySensorsBrightness() {
  return map( lightBrightness, MIN_BRIGHTNES, MAX_BRIGHTNESS, MIN_MYSENSORS_BRIGHTNES, MAX_MYSENSORS_BRIGHTNESS );
}

/*
 Sends the current brightness level to the gateway when e.g. the user changed the brightness manually.
 */
void sendBrightnessLevelToGateWay() {
  send( dimmerMsg.set( (int)getConvertedMySensorsBrightness() ) ); // Send the stored brightness value to the Gateway
}

/*
 Sends the current power state to the gateway. If the current power state is on (true), the current brightness level is also
 send, because domoticz will set the brightness to full when a dimmer is turned on. Don't know if this is by design (I use a very old
 Domoticz version) or if it's a bug. But it's certainly anoying. But resending the current brightness syncs the domoticz brightness
 level to what was stored on the lamp.
 */
void sendPowerstateToGateWay( ) {
  send( lightMsg.set( powerState == true ? "1" : "0" ) );

  if ( powerState ) {
    delay( 15 ); // Delay is for preventing ddos effect on Gateway and to give the antenna more time to recover. 
    sendBrightnessLevelToGateWay(); // Domotics sets the brightness to max when it turns on a dimmer. It doesn't go back to the previous,
                                     // So in this case we use the stored users brightness
  }
}
