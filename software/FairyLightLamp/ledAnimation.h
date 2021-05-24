#ifndef LED_ANIMATION_H
#define LED_ANIMATION_H

#include <Arduino.h>

/*
  Library for handling fading and blinking anymotiond on a PWM pin. This libraru was developped to
  fade and blink a fairy light led string driven by a 2n7000 mosfet on a PWM pin.

  Author: By Theo
  Created: January 6th 2021

  The library contains classes for the implementation for the following animations:
  - SmoothBrightnessTransistion: Class for smooth transistions between different brightness levels.
                                 it translates from the current brightness level to the target brightness level
                                 in 10 steps of a 50ms duration. Whenever a new target brightness level is set
                                 the animation is adjusted to the new situation. Providing smooth transistions with
                                 a static total duration (10 * 50ms = 500ms) for a beter user experience.
   - OffBlinkAnimation         : Class for an off blink animation. The amount of times the lamp is turned of and
                                 the duration for eacht on and off can be provided when the class is instanciated.
                                 You could use this as an OnBlinkAnimation, but bare in mind that it always starts with
                                 on off blink. This is because this class was implemented to provide feedback
                                 when the user tries to exceeds the brightness limits (minimum and maximum) of the
                                 fairy light led strings lamp. If this is unwanted behavior and you want the opposite -
                                 meaning x on blinks, it is better to write a new class,

  All of the above animation classes are wrapped in the AnimationManager class. This class handles the correct
  animation according to the implement animation hierarchie, which is if an off blink is requested this animation is handled
  first. If a smooth transistion animation is requested it is handled of no off blink animation is active. This is so that we
  can provide the user with exceeding brightness level and if the user changes the brightness level when the off animation is
  playing, the transition to the new brightnes is handled afterwards. This way wwe provide the user with a better overall
  experience.

  Revision history:
    06-01-2021 Initial version.
*/


// const that defines the amount of steps for a smooth transistion animations.
const uint8_t animationSteps = 10;

/*
  Pre declare listener class so we can implement the circular reference between the AnimationListern and the AnimationBase classes
*/
class AnimationListener;

/*
  Base class for animation, so that listener can subscribe for animition finished events.
*/
class AnimationBase {
  public:
    void setAnimationListener( AnimationListener *aListener );
  protected:
    void fireAnimationFinished();
  private:
    AnimationListener* listener = NULL;
};

/*
  Classes that want to subscribe to Animation changes should inherint the AnimationListener class.

  note: Currently not used by the AnimationManager class. But could be used so that the sketch can go into
       sleep mode when all animations have ended.
*/
class AnimationListener {
  public:
    virtual void onAnimationFinished( AnimationBase* source );
};


/*
  Class for providing smooth transisitions between different light brightness levels. See
  cpp file for the documentation.
*/
class SmoothBrightnessTransistion : public AnimationBase {
  public:
    SmoothBrightnessTransistion( uint8_t pwmPin );

    void checkAnimation( unsigned long currentMillis );
    void setLevel( uint8_t targetLevel );

    uint8_t getCurrentBrightnessLevel();

    bool isAnimationFinished();
  private:
    uint8_t       pwmPin;
    double        currentLightLevel;
    double        targetLightLevel;
    double        stepIncrement; // The increment amount. Can be negative

    // members for the animation (step) duration
    unsigned long animationStarted;
    unsigned long animationStepDuration;
};

/*
  Class for handling off blink animations. Dee cpp file for documentation.
*/
class OffBlinkAnimation : public AnimationBase {
  public:
    OffBlinkAnimation( uint8_t pwmPin, uint8_t amount, unsigned long blinkDelay ); // AnimationListener *animationListener

    void startAnimation( uint8_t brightnessLevel );
    void checkAnimation( unsigned long currentMillis );
    bool animationFinished();
  protected:
  private:
    uint8_t       pwmPin;
    uint8_t       blinkAmount;
    uint8_t       blinkCounter;
    uint8_t       brightnessLevel;
    bool          blinkState;

    unsigned long blinkDelay;
    unsigned long animationStart;

    AnimationListener *animationListener;
};

/*
 Class the implements hierarchy for the supported animations. See cpp file for documentation.
  The boundary reeached animation has a higher hierachy than the fade to brightness level animation.
 */
class AnimationManager : public AnimationListener {
  public:
    AnimationManager( uint8_t pwmPin );
    void onAnimationFinished( AnimationBase* source  );

    void startBoundaryReachedAnimation();
    void fadeToBrightnessLevel( uint8_t targetLevel );


    bool animationFinished();
    void checkAnimation( unsigned long currentMillis );
  private:
    OffBlinkAnimation           *offBlinkAnimation;
    SmoothBrightnessTransistion *smoothTransistionAnimation;
};


#endif
