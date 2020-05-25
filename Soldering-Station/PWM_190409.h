#ifndef PWM_190409_H_
 #define PWM_190409_H_

 
 
 #include "Arduino.h"
 #include "enums.h"
 #include "pwm_hal.h"

 #define PWM_PIN        PC0
 
 
/**********************************************************************************************************
                               CLASS PWM_150500 
**********************************************************************************************************
 Baseclass:   SolderingStation_PWM
 Function:    Derrived class for 150500 PWM
 Input:       None
 Output:      None
 Discription: Handels the PWM of 150500
**********************************************************************************************************/
 class PWM_190409 : public SolderingStation_PWM {
        public:
        PWM_190409( ){ };
        void On( uint16_t value );
        void Off( void );
        uint16_t ReadPWMValue( void );
    };

#endif
