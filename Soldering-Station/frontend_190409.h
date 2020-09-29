#ifndef FRONTEND_190409_H_
 #define FRONTEND_190409_H_ 
 
/*
This frontend requiers: https://github.com/Seeed-Studio/Grove_4Digital_Display to be installed from the arduino IDE
*/

 /* includes for this class */
 #include "Arduino.h"
 #include "enums.h"
 #include "Frontend.h"
 


/* PIN definitions for the display */ 
#define TM1637_CLK     3
#define TM1637_DIO     2

/**********************************************************************************************************
                               CLASS FE_150500 
**********************************************************************************************************
 Baseclass:   SolderingStation_FE
 Function:    Derrived class for 150500 frontend
 Input:       None
 Output:      None
 Discription: Handels the frontend of 150500
**********************************************************************************************************/
class FE_190409 : public SolderingStation_FE {
    
public: 
    FE_190409(){}
    void display_setup( void );
    void display_welcome_logo( void );
    void display_title( void );
    void display_invert( bool invert );
    void display_dim(bool dim );
    void display_show_Temperatur(  uint16_t dispTemperature, uint8_t HeatPwr_Percent, uint16_t setpoint, fsmstate_t state, uint16_t timestamp );
    void display_show_TempError( uint8_t ErrNo );
    void display_show_Undervoltage(uint16_t Vin);
    void display_show_sleep( uint16_t timestamp );
    void display_show_config( void  );
    void display_show_current_ironmode( SolderingIronType_t Iron);
    void display_1ms_tick( void );

private:
    volatile uint16_t display_timout =0;
    volatile uint8_t display_heat_blink=0;


};
#endif
