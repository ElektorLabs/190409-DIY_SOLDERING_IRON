#ifndef HW_190409_H_
 #define HW_190409_H_
 

#include "enums.h"
 /* This is for 190409 */
#include "frontend_190409.h"
#include "PWM_190409.h"
#include "TEMP_190409.h"
#include "VIN_190409.h"
#include "CURRENT_190409.h"
#include "pindefinitions.h"


/*
#define ROTARY_TERM_A ( 9 )
#define ROTARY_TERM_B ( 6 )
*/


#define ROTARY_TERM_A ( 5 )
#define ROTARY_TERM_B ( 6 )

#define ROTARY_BTN ( 4 )

/* ROTARY_CW_LEVEL can be HIGH or LOW */
#define ROTARY_CW_LEVEL ( LOW ) 

#define MAX_TEMP       ( 450 )
#define MIN_TEMP       ( 50 )

#define MAX_PWM        2048
#define MAX_PWM_LIMIT  2048
#define SAFE_PWM_VALUE 2048
#define CURRENT_NO_ADJ 

#define CURRENT_LIMIT ( 1500 )

#define ADC_AVG        8

/* This is specific for 190409 */
#define VIN_MIN_MV     (7800)

/**********************************************************************************************************
                               CLASS HW_190409 
**********************************************************************************************************
 Baseclass:   SolderingStation Hardware
 Function:    Base for the HW Platform
 Input:       None
 Output:      None
 Discription: Basic class for the Hardware
**********************************************************************************************************/
class HW_190409 {
    public:
        HW_190409(){};
        FE_190409 Frontend;
        PWM_190409 PWM;
        TEMP_190409 Temp;
        CURRENT_190409 IronCurrent;
        VIN_190409 Vin; 
        fsmstate_t CheckLimits( void );
        void Setup( void (*cb_250us_Timer)(void) );
        void SetPWM( uint16_t PWM_Value);
        void ShowUndervoltage();
        void SetSolderingIron(SolderingIronType_t Iron );
        SolderingIronType_t GetSolderingIron( void );
        uint16_t read_StoreTemperature( void );
        void write_StoreTemperature(uint16_t tempValue);
                
    private:
      
      void setup_gpio( void );    
      void write_StoreIronConfig(SolderingIronType_t IronType);
      SolderingIronType_t read_StoreIronConfig( void );
};

#endif
