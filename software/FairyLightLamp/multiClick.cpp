#include "multiClick.h"

/**
 Constructs an instance of the SoftDebouncedMultiClick class.
 debouncer: a reference to the debouncer that is used to read the switch states
 */
SoftDebouncedMultiClick::SoftDebouncedMultiClick( Bounce* debouncer ) {
  this->debouncer = debouncer;
}

/**
 Constructs an instance of the SoftDebouncedMultiClick class.
 switchPin: the pun to witch the switch that is being monitored is connected.
 */
SoftDebouncedMultiClick::SoftDebouncedMultiClick( uint8_t switchPin ) {
  this->debouncer = new Bounce();
  this->debouncer->attach( switchPin, INPUT_PULLUP ); // Attach the debouncer to a pin with INPUT_PULLUP mode
  this->debouncer->interval( 25 );                    // Use a debounce interval of 25 milliseconds
}

/*
 Assignes the given handler as the keypress handler. 
 aHandler: the reference to the handler or NULL if the press doesn't need to be handled anymore.

 Note: Single normal clicks can not be detected, because we can not know wether this is a long or a normal press
 until the switch is either released or when it's pressed long enough to detect a long press.
 */
void SoftDebouncedMultiClick::setKeyPressHandler( multiClickEventHandler aHandler ) {
  this->keyPressHandler = aHandler;
}

/*
 Assignes the given handler as the key click handler. 
 aHandler: the reference to the handler or NULL if the press doesn't need to be handled anymore.
 */
void SoftDebouncedMultiClick::setKeyClickHandler( multiClickEventHandler aHandler ) {
  this->keyReleaseHandler = aHandler;
}

/**
 The state machine implementation. This sould be called in each main loop.
 currentMillis: the currentMills 
 */
void SoftDebouncedMultiClick::checkSwitch( unsigned long currentMillis ) {
  switchUpdated = debouncer->update();
  switchState = debouncer->read();

  switch ( scanState ) {
    case KP_SCANNING : {
        if ( switchUpdated ) {
          if ( switchState == KEY_PRESSED ) {
            keyPressedTS = currentMillis;
          }
          else {
            if ( currentMillis - keyPressedTS <= SHORT_KEY_PRESS_DURATION ) {
              setKeyScanningState( KP_SHORT_PRESS_SEQUENCE );
            }
            else {
              fireKeyReleasedEvent( KP_SHORT_PRESS_SEQUENCE, 1 );
            }
          }
        }
        else {
          if ( switchState == KEY_PRESSED ) {
            if ( currentMillis - keyPressedTS >= LONG_KEY_PRESS_DURATION ) {
              setKeyScanningState( KP_LONG_PRESS );
            }
          }
        }
        break;
      }
    case KP_LONG_PRESS: {
        if ( switchUpdated ) {
          if ( switchState == KEY_RELEASED ) { // Long press has ended
            setKeyScanningState( KP_SCANNING );
          }
        }
        break;
      }
    case KP_SHORT_PRESS_SEQUENCE: {
        if ( switchUpdated ) {
          if ( switchState == KEY_PRESSED ) {
            keyPressedTS = currentMillis;
            pressCount++;
            fireKeyPressedEvent( KP_SHORT_PRESS_SEQUENCE, pressCount );
          }
        }
        else {
          if ( switchState == KEY_RELEASED ) {
            if ( currentMillis - keyPressedTS > ( 2 * SHORT_KEY_PRESS_DURATION ) ) {
              setKeyScanningState( KP_SCANNING );
            }
          }
        }
        break;
      }
  }
}

/*
 Assigns the given state as the new state of the state maching. And fires events when needed.
 newState: the newState of the state machine.
 */
void SoftDebouncedMultiClick::setKeyScanningState( KEY_SCAN_STATES newState ) {
  if ( newState != scanState ) {
    switch ( newState ) {
      case KP_SCANNING: {
          if ( scanState == KP_LONG_PRESS ) {
            fireKeyReleasedEvent( KP_LONG_PRESS, 1 );
          }
          else if ( scanState == KP_SHORT_PRESS_SEQUENCE ) {
           fireKeyReleasedEvent( KP_SHORT_PRESS_SEQUENCE, pressCount );
          }
          break;
        }
      case KP_LONG_PRESS: {
          fireKeyPressedEvent( KP_LONG_PRESS, 1 );
          break;
        }
      case KP_SHORT_PRESS_SEQUENCE: {
          pressCount = 1;
          break;
        }
    }
    scanState = newState;
  }
}

/*
 Fires a key pressed (Meaning the switch is hold down) event.
 */
void SoftDebouncedMultiClick::fireKeyPressedEvent( KEY_SCAN_STATES eventType, uint8_t amount ) {
  if ( keyPressHandler != NULL ) {
    keyPressHandler( eventType, amount, this );
  }
}

/*
 Fires a key click (Meaning the switch is released) event.
 */
void SoftDebouncedMultiClick::fireKeyReleasedEvent( KEY_SCAN_STATES eventType, uint8_t amount ) {
  if ( keyReleaseHandler != NULL ) {
    keyReleaseHandler( eventType, amount, this );
  }
}

/*
 Determines wether the internal state machine is idle (true) or busy with scanning (false).
 Idles means that no key is down and no short press sequence is in progress. If the internal state is idle
 the arduino can be put in sleep mode, without missing any vital information regarding the clicks.
 */
bool SoftDebouncedMultiClick::isIdle( unsigned long currentMillis ) {
  return scanState == KP_SCANNING && switchState == KEY_RELEASED && ( currentMillis - keyPressedTS > SHORT_KEY_PRESS_DURATION );
}
