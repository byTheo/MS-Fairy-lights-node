#include "ledAnimation.h"

/*
   Overall comment: the this-> operator isn't need in every place, the compiler fills it in
   but I use it so there's a visible distinct in what are class members or functions on what not
*/


//                              AnimationBase

/*
  Assignes the given animation listener as the listener for the animation. Assign NULL if you
  want to deregister the current listener.
*/
void AnimationBase::setAnimationListener( AnimationListener *aListener ) {
  this->listener = aListener;
}

/*
  Animations must call this method when the animation is finised so that
  the animation finished event can be received by the animation listener.
*/
void AnimationBase::fireAnimationFinished() {
  if ( this->listener != NULL ) {
    this->listener->onAnimationFinished( this ); // Not sure if it can reference the child reference
  }
}




//                              SmoothBrightnessTransistion

/*
  Create an instance of the SmoothBrightnessTransistion class.

  pwmPin: IO pin of the arduino to is used to show the animation.
*/
SmoothBrightnessTransistion::SmoothBrightnessTransistion( uint8_t pwmPin ) {
  this->pwmPin = pwmPin;
  this->currentLightLevel = 0.0;
  this->targetLightLevel = 0.0; // We are at the end of the animation
  this->animationStepDuration = 50;
  this->animationStarted = millis();
  this->stepIncrement = 0.0; // doesn't need to be initialized
}

/*
  method for checking and handling the current animation.

  currentMillis: the current millis of the micro controller.
*/
void SmoothBrightnessTransistion::checkAnimation( unsigned long currentMillis ) {
  if ( !this->isAnimationFinished() ) {
    if ( currentMillis - this->animationStarted >= animationStepDuration ) {
      this->animationStarted = currentMillis;
      this->currentLightLevel += this->stepIncrement;

      // Cap of current level if it exceeds the target level.
      if ( this->stepIncrement < 0 ) {
        if ( this->currentLightLevel < this->targetLightLevel ) {
          this->currentLightLevel = this->targetLightLevel;
        }
      }
      else {
        if ( this->currentLightLevel > this->targetLightLevel ) {
          this->currentLightLevel = this->targetLightLevel;
        }
      }
      analogWrite( this->pwmPin, (uint8_t)this->currentLightLevel );
    }
  }
}

/*
   Determines wether the animation is finished (true) or still running (false)
*/
bool SmoothBrightnessTransistion::isAnimationFinished() {
  return (uint8_t)this->currentLightLevel == (uint8_t)this->targetLightLevel;
}

/*
  (re)triggers the animation. The currentLevel will be transistioned to the given target level.
*/
void SmoothBrightnessTransistion::setLevel( uint8_t targetLevel ) {
  this->targetLightLevel = targetLevel;
  this->stepIncrement = ( (double)this->targetLightLevel - this->currentLightLevel ) / animationSteps;
  this->animationStarted = millis();
}

/*
  Returns the current brightness as a byte - internal it's a double.
*/
uint8_t SmoothBrightnessTransistion::getCurrentBrightnessLevel() {
  return (uint8_t)this->currentLightLevel;
}



//                              AnimationBase

/*
  Creates an instance of the OffBlinkAnimation class
  pwmPin    : the PWM pin on which the animations need to be shown
  amount    : the amount of off blinks that will be shown
  blinkDelay: the duration in ms between the on and off state
*/
OffBlinkAnimation::OffBlinkAnimation( uint8_t pwmPin, uint8_t amount, unsigned long blinkDelay ) {
  this->blinkAmount = amount;
  this->blinkDelay = blinkDelay;
  this->pwmPin = pwmPin;
  this->blinkState = true;
  this->blinkCounter = amount;
}

/*
  Starts a blink animation.
  brightnessLevel: the brightness level of the on state.
*/
void OffBlinkAnimation::startAnimation( uint8_t brightnessLevel ) {
  this->blinkCounter = 1; // We start with the blink cycle
  this->brightnessLevel = brightnessLevel;
  this->blinkState = false; // means off
  this->animationStart = millis();
  analogWrite( this->pwmPin, 0 ); // Show the first off
}

/*
  Checks and handles the animation. Must be called in the main loop
*/
void OffBlinkAnimation::checkAnimation( unsigned long currentMillis ) {
  if ( ( currentMillis - this->animationStart >= this->blinkDelay ) && !this->animationFinished()) {
    this->blinkState = !this->blinkState;

    if ( this->blinkState ) {
      analogWrite( this->pwmPin, this->brightnessLevel ); // Show the first off
    }
    else {
      analogWrite( this->pwmPin, 0 ); // Show the first off
      this->blinkCounter++;
    }
    if ( this->animationFinished() ) {
      this->fireAnimationFinished();
    }

    this->animationStart = currentMillis;
  }
}

/*
   Determins wether an animation is running (false) or if the animations is finished (true).
*/
bool OffBlinkAnimation::animationFinished() {
  return this->blinkCounter == this->blinkAmount && this->blinkState;
}



//                              AnimationBase

/*
  Creates an instance of the AnimationManager class
  pwmPin : the pwm pin to which the fairy light led string is connected (through a mosfet)
*/
AnimationManager::AnimationManager( uint8_t pwmPin ) {
  // our off blink animation is defined as turned 3 times of and end with the lights on
  // we assume the lights are on, because they are when you turn the rotary encoder to change
  // the brightness level.
  this->offBlinkAnimation = new OffBlinkAnimation( pwmPin, 3, 300 );
  this->smoothTransistionAnimation = new SmoothBrightnessTransistion( pwmPin );

  this->offBlinkAnimation->setAnimationListener( this );
  this->smoothTransistionAnimation->setAnimationListener( this );
}

/*
  Listens for animation finish events for the Animations managed by the class.
*/
void AnimationManager::onAnimationFinished( AnimationBase *source  ) {
  if ( source == this->offBlinkAnimation ) {
  }
  else if ( source == this->smoothTransistionAnimation ) {
  }
}

/*
  Starts a boundary reached animation, if and only if there's no boundary reached animation currently running.
*/
void AnimationManager::startBoundaryReachedAnimation() {
  if ( this->offBlinkAnimation->animationFinished() ) {
    this->offBlinkAnimation->startAnimation( this->smoothTransistionAnimation->getCurrentBrightnessLevel() );
  }
}

/*
  determines wether the animation is finised (true) or if an animation is running (false).
*/
bool AnimationManager::animationFinished() {
  bool result = this->offBlinkAnimation->animationFinished() && this->smoothTransistionAnimation->isAnimationFinished();
  return result;
}

/*
  Checks and handles animations. Must be call from the main loop for each cycle.
*/
void AnimationManager::checkAnimation( unsigned long currentMillis ) {
  if ( !this->offBlinkAnimation->animationFinished() ) {
    this->offBlinkAnimation->checkAnimation( currentMillis );
  }
  else if ( !this->smoothTransistionAnimation->isAnimationFinished() ) {
    this->smoothTransistionAnimation->checkAnimation( currentMillis );
  }
}

/*
  Transistions the fairy light lef string to the given target brightness level. Starting brightness
  is the current brightness.
*/
void AnimationManager::fadeToBrightnessLevel( uint8_t targetLevel ) {
  this->smoothTransistionAnimation->setLevel( targetLevel );
}
