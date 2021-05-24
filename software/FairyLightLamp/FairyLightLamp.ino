/*
   My Sensos Sketch for the fair ligh based lamp, with smooth transition animation,

   By Theo
   January 4th 2021

   Revision
   01-04-2021 - initial version

   Future notes for myself - for a future project that uses a dimmer:
   - Handling Domoticz messages:
      only adjust brightness. Don't listen to brightness 0
      when V_LIGHT is 1, send current brightness. Domoticz turns on with 100% but it doesn't report it to the node
   - Handling user input:
      send current brightness when the lamp is turned on after sending V_LIGHT 1 ( "1" is on and "0" is off )
      send new brightness after user changes it.
*/

#include <Bounce2.h>
#include <Encoder.h>
#include "ledAnimation.h"
#include "multiClick.h"
#include "config.h"

const uint8_t POWER_SWITCH_PIN = 7; // Interupt pin so the sketch can wake up
const uint8_t LEDSTRING_PIN = 5; // Interupt pin so the sketch can wake up
const uint8_t ENCODER_FIRST_PIN = 2; // The first pin of the encoder, we use an interrupt pin for faster reading
const uint8_t ENCODER_SECOND_PIN = 4; // The second pin of the encoder, no more interrups left, but we should be able to wake up with just one
const uint8_t ENCODER_INCREMENTS = 4; // the amount of increments per encoder position

// Hardware definitions
Encoder *encoder; // encoder( ENCODER_FIRST_PIN, ENCODER_SECOND_PIN );  // The rotary encoder

SoftDebouncedMultiClick *powerSwitch; //( POWER_SWITCH_PIN );

AnimationManager animations( LEDSTRING_PIN  );

unsigned long currentMillis; // Used to pass the millis() value to different function so we don't have to call it too many times.
                              // that way we get shorter loop durations.
unsigned long lastTimeHBSent;


// Initializing the sketch
void setup() {

  encoder = new Encoder( ENCODER_FIRST_PIN, ENCODER_SECOND_PIN );  // The rotary encoder
  powerSwitch = new SoftDebouncedMultiClick( POWER_SWITCH_PIN );

  // Setup led output pin doesn't need a pinMode we're using pwm
  turnLightsOff();

  uint8_t storedBrightness = loadState( EEPROM_DIM_LEVEL_LAST );
  lightBrightness = storedBrightness == 255 ? DimmerDefaultValue : storedBrightness;
  oldEncoderPosition = lightBrightness * ENCODER_INCREMENTS;

  assignBrightnessLevelToEncoder();

  powerSwitch->setKeyPressHandler( handlePowerSwitchPressed );
  powerSwitch->setKeyClickHandler( handlePowerSwitchClicked );

  lastTimeHBSent = millis() - HEART_BEAT_INTERVAL;
}

// We only respond to a long press when the lamp is on so we can give the user feedback that he/she can release the switch.
// We use the long press to store the current brightness. Which will always be used when turning the lamp manually on
void handlePowerSwitchPressed( KEY_SCAN_STATES type, uint8_t amount, SoftDebouncedMultiClick source ) {
  if ( type == KP_LONG_PRESS && powerState ) {
    saveState( EEPROM_DIM_LEVEL_LAST, lightBrightness );
    animations.startBoundaryReachedAnimation();
  }
}

// Handler for normal clicks. Regardless of the amount of clicks. We always toggle the power.
// In the future we can implement double clicks and even tripple clicks to change the mode when
// the lamp is on.
void handlePowerSwitchClicked( KEY_SCAN_STATES type, uint8_t amount, SoftDebouncedMultiClick source ) {
  if ( type == KP_SHORT_PRESS_SEQUENCE ) {
    toggleLightState();
  }
}

void loop() {
  currentMillis = millis();

  if ( currentMillis - lastTimeHBSent >= HEART_BEAT_INTERVAL ) {
    lastTimeHBSent = currentMillis;
    sendHeartbeat();
    Serial.println( "Heartbeat send" );    
  }
  
  animations.checkAnimation( currentMillis );
  powerSwitch->checkSwitch( currentMillis );

  checkEncoder();
}


/*                   Routines for handling changes to the power state of the lamp    */

/*
  Turns on the fairy light led string with an animated fade to the current brightness level.
*/
void turnLightsOn() {
  animations.fadeToBrightnessLevel( map( lightBrightness, MIN_BRIGHTNES, MAX_BRIGHTNESS, 10, 255  ) );
}

/*
  Turns off the fairly led string.
*/
void turnLightsOff() {
  animations.fadeToBrightnessLevel( 0 );
}

/*
  Assigns the given brightness as the new brightness, if and only if the new brightness differs from the current brightness.
*/
void setNewLightBrightness( uint8_t newLightBrightness ) {
  if ( lightBrightness != newLightBrightness ) {
    if ( !powerState ) {
      powerState = true;
    }

    lightBrightness = newLightBrightness;
    turnLightsOn();
  }
  else {
    if ( !powerState ) {
      powerState = true;
      turnLightsOn();
    }
  }
}

/*
  Tries to assign to given state as the current state.
  If possible it will change the state accordingly.
*/
void setLightState( bool newState ) {
  if ( newState != powerState ) {
    powerState = newState;
    if ( powerState ) {
      turnLightsOn();
    }
    else {
      turnLightsOff();
    }
  }
}

/*
  Toggles the state of the lamp and sends the states to the gateway.
*/
void toggleLightState() {
  setLightState( !powerState );
  sendPowerstateToGateWay();
}


/*                Rotary encoder code    */

/*
  Scans for changes made by the user to the rotary encoder. And handles them accordingly.
*/
void checkEncoder() {
  newEncoderPosition = encoder->read();

  if ( newEncoderPosition != oldEncoderPosition ) {
    oldEncoderPosition = newEncoderPosition;
    if ( oldEncoderPosition % ENCODER_INCREMENTS == 0 ) {

      newBrightness = oldEncoderPosition / ENCODER_INCREMENTS;
      if ( newBrightness != lightBrightness ) {
        if ( newBrightness < MIN_BRIGHTNES ) {
          oldEncoderPosition = MIN_BRIGHTNES * ENCODER_INCREMENTS;
          assignBrightnessLevelToEncoder();
          // signal the user that the min brightness level has been reached
          animations.startBoundaryReachedAnimation();
        }
        else if ( newBrightness > MAX_BRIGHTNESS ) {
          oldEncoderPosition = MAX_BRIGHTNESS * ENCODER_INCREMENTS;
          assignBrightnessLevelToEncoder();

          // signal the user that the max brightness level has been reached
          animations.startBoundaryReachedAnimation();
        }
        else {
          if ( !powerState ) {
            // don't adjust brightness
            oldEncoderPosition = lightBrightness * ENCODER_INCREMENTS;
            assignBrightnessLevelToEncoder();
            setLightState( true ); // turn on the led

            sendPowerstateToGateWay();
          }
          else {
            setNewLightBrightness( oldEncoderPosition / ENCODER_INCREMENTS );
            sendBrightnessLevelToGateWay();
          }
        }
      }
    }
  }
}

void assignBrightnessLevelToEncoder() {
  encoder->write( lightBrightness * ENCODER_INCREMENTS  );
}



/*            MySensors specific code    */


/*
   Prestent the MySensors node to the Gateway. This includes the sketch name and -version as well as the childs of the node
*/
void presentation() {
  sendSketchInfo( MS_SketchName, MS_SketchVersion );
  delay( 50 ); // Let's not DDOS the Gateway
  present(  CHILD_ID_LIGHT, S_DIMMER );
  delay( 50 );
  send( dimmerMsg.set( (int)map( lightBrightness, MIN_BRIGHTNES, MAX_BRIGHTNESS, 1, 100 ) ) ); // Send the stored brightness value to the Gateway
  delay( 50 ); // When the node is closer to the gateway it doesn't receive it's current state, which I can not explain. But using delays works
              // A best practice would be to use random delays, chosen once at the start of the method. The delay would be 40 - 80ms
              // if all nodes do this it will give them all time to connect to the gateway after a power out. The Sonoff once had a
              // power out in production, and when the came back on all the sonoff Devices where literally DDOS-ing the Sonoff servers whilst
              // thrying to connect to it.
  request( CHILD_ID_LIGHT, V_LIGHT );
}

/*
  Event handler for handling message received from the gateway.
*/
void receive( const MyMessage &message ) {
//  Serial.print( "Received message for node id " ); Serial.println( getNodeId() );
  // We only accept messages for this node
  if (message.destination == getNodeId()) {
    if ( message.type == V_DIMMER ) {
      int dimvalue = atoi( message.data );
      if ( dimvalue  > 0 && dimvalue <= 100 ) {
        setNewLightBrightness( converFromMySensorsBrightness( dimvalue ) );
        assignBrightnessLevelToEncoder();
      }
    }
    else if ( message.type == V_LIGHT ) {
      int value = atoi( message.data );
      setLightState( value == 1 );
      if ( value == 1 ) {
        delay( 10 );
        sendBrightnessLevelToGateWay();
      }
    }
  }
}

/*
  THis method is called before any hardware. In here we can add the power saving code.
*/
void before() {
  // Initialize all out pins written to low. So that all unused pins will not use any power
  for ( uint8_t cnt = 0; cnt <= 19; cnt++ ) {
    if ( cnt == 8 ) { // Don't pull the spi pins low. They're used by MySensors
      cnt = 13;
      return;
    }
    pinMode( cnt, OUTPUT );
    digitalWrite( cnt, LOW );
  }

  // disable ADC
  ADCSRA = 0;  
}
