#ifndef MULTI_CLICK_H
#define MULTI_CLICK_H

#include <Arduino.h>
#include <Bounce2.h>

/*
  Library for adding mutli click and long click detection to the Bounce2 library.

  Author: By Theo
  Created: January 7th 2021

  See multiClick.cpp file for documentation

  Disclaimer:
    Adding multi clicks to a switch to give the switch more function doesn't always improve the User Experience.

    The duration of single clicks within a duration has to be short. So that it doesn't take too long before you can
    responed to the end of a click sequence after the last click. A double click can be done by the majority of the
    humans. But tripple clicks or above or very hard, because you want to keep the time between clicks low.

    Also from a cognitive view it's hard for some people to remember different functions of one switch. So be mindful about
    this. And don't add ling sequences. Supporting tripple and long clicks on one switch means the switch supports 4 functions.
    Which can be hard for people to remember.

    If possible provide feedback to the user when the long press duration has ended. So the user knows when he or she can release the switch.


    We use a byte as a click counter. This means that after 255 clicks the value flips back to zero. But that's acceptable.

    Currently this library uses Bounce2 to do a soft debounce. While this is an awesome library, there's one disadvantage. It
    blocks the sketch while updating the state. This means when you have many switches, who's state have to be checked each loop,
    the sort press won't work or doesn't work well.
    On the Arduino site a hardware debouncer is posted with using a pull up resisitor. I find it hard to believe this will work much
    better. But it looks promising, because it's a non blocking soft debouncer. The code is also very small so we can add it into this library
    which gives us 1 less dependency. the reason why I'm hesitating is that you can miss changes when there's a blocking code (let's sat 10ms) in
    the loop during which a bounce happens. But the question is how bad is this opposed to the blocking behavior of Bounce2.
*/


/*
  Constants used by the multiclick library
*/
const unsigned long LONG_KEY_PRESS_DURATION = 1500; // It takes 1.5 seconds before we see a key press as a long key press
const unsigned long SHORT_KEY_PRESS_DURATION = 170; // A press (the duration between 2 key clicks) which makes ik count as a short
// press. Consecutive short presses will return the sum of clicks (like double or tripple
// clicks) The duration is a bit longer than the apple ear plug buttons. Because for me that
// period is too short. The library supports unlimited sequences. But experience wise I don't
// think you should implement more than double clicks. Sometimes it's better to add more switches
// You can use port extenders if you run out of ports. Also you can use diodes so the buttons
// can share a single interrupt pins.
// Constants for making the code better readable.
const uint8_t KEY_PRESSED = LOW;
const uint8_t KEY_RELEASED = HIGH;

// Pre declare classes
class SoftDebouncedMultiClick;


/*
  The different states of the multiclick's internal state machine.
*/
typedef enum { KP_SCANNING, KP_SHORT_PRESS_SEQUENCE, KP_LONG_PRESS  } KEY_SCAN_STATES;

/*
  Blue print for mutli click event handlers. First argument is the state, the 2nd is the amount (long press will always be 1)
*/
typedef void (*multiClickEventHandler)( KEY_SCAN_STATES, uint8_t, SoftDebouncedMultiClick );

/**
  Definition of the class, private aatributes are documented here
  The method documentation can be found in the multiClick.cpp file.
*/
class SoftDebouncedMultiClick {
  public:
    SoftDebouncedMultiClick( Bounce* debouncer );
    SoftDebouncedMultiClick( uint8_t switchPin );

    void setKeyPressHandler( multiClickEventHandler aHandler );
    void setKeyClickHandler( multiClickEventHandler aHandler );

    void checkSwitch( unsigned long currentMillis );
    bool isIdle(  unsigned long currentMillis );
  protected:
    void setKeyScanningState( KEY_SCAN_STATES newState );
    void fireKeyPressedEvent( KEY_SCAN_STATES eventType, uint8_t amount );
    void fireKeyReleasedEvent( KEY_SCAN_STATES eventType, uint8_t amount );
  private:
    Bounce*         debouncer;               // Reference to the Bounce object, used to detect changes in the switch state (software blocking debounce)
    bool            switchUpdated = false;   // Indicates wether the debouncer has been updated since the last chech
    uint8_t         switchState = LOW;       // stores the current state of the switch
    KEY_SCAN_STATES scanState = KP_SCANNING; // Stores the current state of the state machine
    unsigned long   keyPressedTS;            // Stores the last ts when the switch was pressed
    uint8_t         pressCount = 1;          // the amount of presses in the current sequence if short/normal clickes

    multiClickEventHandler keyPressHandler = NULL;   // Stores a reference to the keyPressHandler method
    multiClickEventHandler keyReleaseHandler = NULL; // Stores a reference to the keyReleaseHandler method
};

#endif
