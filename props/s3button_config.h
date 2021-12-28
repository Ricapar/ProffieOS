// Sabertrio Proffie Saber - Button Configuration
#ifndef PROPS_SABER_SABERTRIO_BUTTONS_H
#define PROPS_SABER_SABERTRIO_BUTTONS_H

#include "prop_base.h"

#undef PROP_TYPE
#define PROP_TYPE SaberSabertrioButtons


EFFECT(battery); // for EFFECT_BATTERY_LEVEL

// The Saber class is for the implementation of the basic states and actions of the saber.
class SaberSabertrioButtons : public PropBase {
public:
SaberSabertrioButtons() : PropBase() {}
  const char* name() override { return "SaberSabertrioButtons"; }

  void Loop() override {
    PropBase::Loop();
    DetectSwing();
  }

  bool Event2(enum BUTTON button, EVENT event, uint32_t modifiers) override {
    switch (EVENTID(button, event, modifiers)) {
      case EVENTID(BUTTON_POWER, EVENT_PRESSED, MODE_ON):
      case EVENTID(BUTTON_AUX, EVENT_PRESSED, MODE_ON):
        if (accel_.x < -0.15) {
          pointing_down_ = true;
        } else {
          pointing_down_ = false;
        }
        return true;
		
// S3 - Blade ON
      case EVENTID(BUTTON_POWER, EVENT_LATCH_ON, MODE_OFF):
      case EVENTID(BUTTON_AUX, EVENT_LATCH_ON, MODE_OFF):
      case EVENTID(BUTTON_AUX2, EVENT_LATCH_ON, MODE_OFF):
      case EVENTID(BUTTON_POWER, EVENT_CLICK_SHORT, MODE_OFF):
        aux_on_ = false;
        On();
        return true;	


// S3 - Blade OFF
      case EVENTID(BUTTON_POWER, EVENT_HELD_MEDIUM, MODE_ON):
#ifndef DISABLE_COLOR_CHANGE
		if (SaberBase::GetColorChangeMode() == SaberBase::COLOR_CHANGE_MODE_NONE) 
			{
				Off();        
				swing_blast_ = false;
			}
#endif			
        return true;
		
	
// S3 - Mute
  case EVENTID(BUTTON_POWER, EVENT_CLICK_SHORT, MODE_OFF | BUTTON_AUX):
    if (SetMute(true)) {
      unmute_on_deactivation_ = true;
      On();
    }
    return true;

		
// S3 - Next Soundfont
      case EVENTID(BUTTON_AUX, EVENT_HELD_LONG, MODE_OFF):
#ifdef DUAL_POWER_BUTTONS
        aux_on_ = true;
        On();
#else
        next_preset();
#endif
        return true;
		
// S3 - Previous Soundfont
      case EVENTID(BUTTON_POWER, EVENT_HELD_LONG, MODE_OFF):
#ifdef DUAL_POWER_BUTTONS
        aux_on_ = true;
        On();
#else
        previous_preset();
#endif
        return true;	


// S3 - Blaster Block + Confirm selected blade color in color change mode, with aux switch 
		case EVENTID(BUTTON_AUX, EVENT_CLICK_SHORT, MODE_ON):
        if (!SaberBase::Lockup()) 
		{
			swing_blast_ = false;
#ifndef DISABLE_COLOR_CHANGE
          if (SaberBase::GetColorChangeMode() != SaberBase::COLOR_CHANGE_MODE_NONE) 
		  {

            ToggleColorChangeMode();
            return true;
          }
#endif

         DoBlast();
        }
        
        return true;
		
		
		  
// S3 - Confirm selected blade color in color change mode, with power switch 		  
		case EVENTID(BUTTON_POWER, EVENT_CLICK_SHORT, MODE_ON):
		
#ifndef DISABLE_COLOR_CHANGE
          if (SaberBase::GetColorChangeMode() != SaberBase::COLOR_CHANGE_MODE_NONE) {			 
            ToggleColorChangeMode();
		  	swing_blast_ = false;
           
		  }
#endif					
				
			return true;
				
				
// S3 - Lightning Block
      case EVENTID(BUTTON_POWER, EVENT_SECOND_HELD, MODE_ON):
        SaberBase::SetLockup(SaberBase::LOCKUP_LIGHTNING_BLOCK);
        swing_blast_ = false;
        SaberBase::DoBeginLockup();
        return true;



// S3 - Lockup & Drag
      case EVENTID(BUTTON_NONE, EVENT_CLASH, MODE_ON | BUTTON_AUX):

        if (!SaberBase::Lockup()) {
          if (pointing_down_) {
            SaberBase::SetLockup(SaberBase::LOCKUP_DRAG);
          } else {
            SaberBase::SetLockup(SaberBase::LOCKUP_NORMAL);
          }
          swing_blast_ = false;
          SaberBase::DoBeginLockup();
          return true;
        }
        break;
		
		
// S3 - Battery Indicator	
	 case EVENTID(BUTTON_AUX, EVENT_CLICK_SHORT, MODE_OFF):
        SaberBase::DoEffect(EFFECT_BATTERY_LEVEL, 0);
        return true;
				
		
// S3 - Melt Lockup
      case EVENTID(BUTTON_NONE, EVENT_STAB, MODE_ON | BUTTON_AUX):
        if (!SaberBase::Lockup()) {
          SaberBase::SetLockup(SaberBase::LOCKUP_MELT);
          swing_blast_ = false;
          SaberBase::DoBeginLockup();
          return true;
        }
        break;
		

// S3 - Toggle color change mode
#ifndef DISABLE_COLOR_CHANGE
	  case EVENTID(BUTTON_POWER, EVENT_CLICK_SHORT, MODE_ON | BUTTON_AUX):
	    ToggleColorChangeMode();
		break;
#endif

	
// S3 - Force
      case EVENTID(BUTTON_AUX, EVENT_HELD_MEDIUM, MODE_ON):
        if (!SaberBase::Lockup()) 
		{
#ifndef DISABLE_COLOR_CHANGE
			if (SaberBase::GetColorChangeMode() == SaberBase::COLOR_CHANGE_MODE_NONE) 
			{
				SaberBase::DoForce();
				swing_blast_ = false;
			}
#endif	
		}
        return true;
		

#ifdef BLADE_DETECT_PIN
      case EVENTID(BUTTON_BLADE_DETECT, EVENT_LATCH_ON, MODE_ANY_BUTTON | MODE_ON):
      case EVENTID(BUTTON_BLADE_DETECT, EVENT_LATCH_ON, MODE_ANY_BUTTON | MODE_OFF):
        // Might need to do something cleaner, but let's try this for now.
        blade_detected_ = true;
        FindBladeAgain();
        SaberBase::DoBladeDetect(true);
        return true;

      case EVENTID(BUTTON_BLADE_DETECT, EVENT_LATCH_OFF, MODE_ANY_BUTTON | MODE_ON):
      case EVENTID(BUTTON_BLADE_DETECT, EVENT_LATCH_OFF, MODE_ANY_BUTTON | MODE_OFF):
        // Might need to do something cleaner, but let's try this for now.
        blade_detected_ = false;
        FindBladeAgain();
        SaberBase::DoBladeDetect(false);
        return true;
#endif

      case EVENTID(BUTTON_AUX2, EVENT_CLICK_SHORT, MODE_OFF):
#ifdef DUAL_POWER_BUTTONS
        next_preset();
#else
        previous_preset();
#endif
        return true;

// Events that needs to be handled regardless of what other buttons are pressed.
      case EVENTID(BUTTON_POWER, EVENT_RELEASED, MODE_ANY_BUTTON | MODE_ON):
      case EVENTID(BUTTON_AUX, EVENT_RELEASED, MODE_ANY_BUTTON | MODE_ON):
        if (SaberBase::Lockup()) {
          SaberBase::DoEndLockup();
          SaberBase::SetLockup(SaberBase::LOCKUP_NONE);
          swing_blast_ = false;
          return true;
        }
    }
    return false;
  }
  
  void SB_Effect(EffectType effect, float location) override {
    switch (effect) {
     
      case EFFECT_BATTERY_LEVEL:
        if (SFX_battery) {
          hybrid_font.PlayCommon(&SFX_battery);
        } else {
          beeper.Beep(0.5, 3000);
        }
        return;
      
     
    }
  }

private:
  bool aux_on_ = true;
  bool pointing_down_ = false;
  bool swing_blast_ = false;
};

	#endif

	
		