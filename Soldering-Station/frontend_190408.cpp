
#include "frontend_190409.h"
#include "TM1637.h"

TM1637 tm1637(TM1637_CLK, TM1637_DIO);

/**********************************************************************************************************
                                void display_1ms_tick()        
**********************************************************************************************************
 Function:    void display_setup()
 Input:       None
 Output:      None
 Discription: To be called every 1ms for internal timing
**********************************************************************************************************/
void FE_190409::display_1ms_tick( void ){
    if(display_timout>0){
        display_timout--;
    }


}
/**********************************************************************************************************
                                void display_setup()        
**********************************************************************************************************
 Function:    void display_setup()
 Input:       None
 Output:      None
 Discription: Display setup
**********************************************************************************************************/
void FE_190409::display_setup( void ){

    /* This is for 190409 only */
    pinMode(TM1637_CLK, OUTPUT);
    pinMode(TM1637_DIO, OUTPUT);
    tm1637.init();
    tm1637.set(BRIGHT_TYPICAL);//BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;
    float num = 8888;
    // Without specifying decimal pointt it displays int
    tm1637.displayNum(num);    // 8888
    
}

/**********************************************************************************************************
                                void display_welcome_logo()        
**********************************************************************************************************
 Function:    void display_welcome_logo()
 Input:       None
 Output:      None
 Discription: Shows the welcome logo
**********************************************************************************************************/
void FE_190409::display_welcome_logo( void ) {
    tm1637.displayStr("F1_4");
}

/**********************************************************************************************************
                                void display_title()        
**********************************************************************************************************
 Function:    void display_title()
 Input:       None
 Output:      None
 Discription: Shows the welcome title
**********************************************************************************************************/
void FE_190409::display_title( void ){
        tm1637.displayStr("boot");

}

/**********************************************************************************************************
                                void display_title()        
**********************************************************************************************************
 Function:    void display_show_config()
 Input:       None
 Output:      None
 Discription: Shows the ConF title
**********************************************************************************************************/
void FE_190409::display_show_config( void  ){
    tm1637.displayStr("CONF");
    
}

/**********************************************************************************************************
                                void display_title()        
**********************************************************************************************************
 Function:    void display_show_current_ironmode()
 Input:       None
 Output:      None
 Discription: Shows current configured IronMode
**********************************************************************************************************/
void FE_190409::display_show_current_ironmode( SolderingIronType_t Iron){
    switch( Iron){
        case IRON_HAKKO_FX8801:{
            tm1637.displayStr("0 C1");
        }break;

        case IRON_JBC_T245A:{
            tm1637.displayStr("0 C2");
        }break;

        case IRON_WELLER_RT:{
            tm1637.displayStr("0 C3");
        }break;

        default:{
            tm1637.displayStr("0 C8");
        }break;

    }
}

/**********************************************************************************************************
                                void display_invert()        
**********************************************************************************************************
 Function:    void display_invert()
 Input:       None
 Output:      None
 Discription: inverts the display
**********************************************************************************************************/
void FE_190409::display_invert( bool invert ) {

}

/**********************************************************************************************************
                                void display_dim()        
**********************************************************************************************************
 Function:    void display_dim()
 Input:       bool dim
 Output:      None
 Discription: Dims the display if parmaeter is true, otherwyse restores brightness
**********************************************************************************************************/
void FE_190409::display_dim(bool dim ){

    
    
}

/**********************************************************************************************************
                                void display_show_Temperatur()        
**********************************************************************************************************
 Function:    void display_show_Temperatur()
 Input:       uint16_t dispTemperature, uint8_t HeatPwr_Percent, uint16_t setpoint, fsmstate_t state, uint16_t timestamp
 Output:      None
 Discription: Shows the Mainscreen with temperatur
**********************************************************************************************************/
void FE_190409::display_show_Temperatur(uint16_t dispTemperature, uint8_t HeatPwr_Percent, uint16_t setpoint, fsmstate_t state, uint16_t timestamp ){
    static uint16_t current_setpoint;
    static float dispval = 0;
    
    if(setpoint != current_setpoint){
        //We display for at least 2 ticks the new setpoint
         current_setpoint = setpoint;
         display_timout = 2000;
         tm1637.set(BRIGHTEST);//BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;
         dispval = setpoint;
         
    } 
    
    if(display_timout > 0){
            //We do nothing right now
    } else {
            dispval = dispTemperature;
            tm1637.set(BRIGHT_TYPICAL);//BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;
    }
    tm1637.displayNum(dispval);
 



}

/**********************************************************************************************************
                                void display_show_TempError()        
**********************************************************************************************************
 Function:    void display_show_TempError()
 Input:       uint8_t ErrNo
 Output:      None
 Discription: Shows the temperaturerror screen
**********************************************************************************************************/
void FE_190409::display_show_TempError(uint8_t ErrNo){
    char err_str[] = "E-0 ";
    if(ErrNo<10){
        err_str[3]=48+ErrNo;
    } else {
        uint8_t ten = ErrNo / 10;
        uint8_t sgl = ErrNo - ( 10 * ten );
        err_str[2]=48+ten;
        err_str[3]=48+sgl;

        
    }
    tm1637.displayStr(err_str);

}

/**********************************************************************************************************
                                void display_show_TempError()        
**********************************************************************************************************
 Function:    void display_show_TempError()
 Input:       uint8_t ErrNo
 Output:      None
 Discription: Shows the temperaturerror screen
**********************************************************************************************************/
void FE_190409::display_show_Undervoltage(uint16_t Vin){

  tm1637.displayStr("UVLO");

}

/**********************************************************************************************************
                                void display_show_sleep()        
**********************************************************************************************************
 Function:    void display_show_sleep()
 Input:       uint16_t timestamp
 Output:      None
 Discription: Displays the sleep animaton, depending on the timestamp
**********************************************************************************************************/
void FE_190409::display_show_sleep( uint16_t timestamp){

  tm1637.displayStr("----");
}
