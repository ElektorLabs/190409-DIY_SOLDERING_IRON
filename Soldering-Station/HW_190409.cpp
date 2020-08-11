#include <EEPROM.h>
#include "HW_190409.h"

#define VOLTAVGSIZE ( 16 ) 

volatile uint16_t VoltAVG[VOLTAVGSIZE] = { 0xFF,};
void(*__cb_250us_Timer)(void) = nullptr;
FE_190409 *__cb_fe1ms_Timer = nullptr;

/**********************************************************************************************************
                                ISR TCB3_INT_vect        
**********************************************************************************************************
 Function:    ISR TCB3_INT_vect
 Input:       void* 
 Output:      None
 Discription: Interrupt with callback for 100us timing and callback
**********************************************************************************************************/
volatile uint32_t ticks=0;
ISR( TCB3_INT_vect ){
  static uint8_t divider = 0;
  //Inrease ticks
  ticks++;
  /* Clear flag */
  TCB3.INTFLAGS = TCB_CAPT_bm;
  if(__cb_250us_Timer != nullptr){
      __cb_250us_Timer();
      
  }
  if(divider<3){
    divider++;
  } else {
    divider = 0;
    if(__cb_fe1ms_Timer != nullptr){
      __cb_fe1ms_Timer->display_1ms_tick();  
    }
  }
}


/**********************************************************************************************************
                                void Setup()        
**********************************************************************************************************
 Function:    void Setup()
 Input:       void* 
 Output:      None
 Discription: Dose the init for the HW of the Station also sets the callback for the 1ms timer
**********************************************************************************************************/
void HW_190409::Setup( void (*cb_250us_Timer)(void) ){
    setup_gpio();
    Frontend.display_setup(); 

    SetSolderingIron(GetSolderingIron()); //This will restore confing from EEPROM
    //0.1ms timer with callback attached
    //This will be called in front of setup() and allow to filldle with timers
    cli();
    /* Select vanilla 16 bit periodic interrupt mode */
    TCB3.CTRLB = TCB_CNTMODE_INT_gc;

    /* TOP value for overflow every N clock cycles */
    TCB3.CCMP = ((TIME_TRACKING_TIMER_COUNT/4) - 1);

    /* Enable TCB interrupt */
    TCB3.INTCTRL |= TCB_CAPT_bm;

    /* Clock selection is F_CPU/N -- which is independent of TCA */
    TCB3.CTRLA = TCB_CLKSEL_CLKDIV1_gc; /* F_CPU */
    /* Enable & start */
    TCB3.CTRLA |= TCB_ENABLE_bm; /* Keep this last before enabling interrupts to ensure tracking as accurate as possible */

    __cb_250us_Timer = cb_250us_Timer;
    __cb_fe1ms_Timer = &Frontend;
 
    //This will reconfigure the pwm to 16bit mode
    
    //Every bit set to 1 will enable output on its pin
    PORTC.DIRSET = ( 1<< PIN0 );
        //Disable timer, keep devider
    TCA0.SPLIT.CTRLA = 0x00; //Reset default
    // PORTMUX setting for TCA -> outputs [2:5] point to PORTC pins [2:5]
    PORTMUX.TCAROUTEA  = PORTMUX_TCA0_PORTC_gc;
    // Disable split mode 
    TCA0.SPLIT.CTRLD =0x00;
    // Reset all Outputs
    TCA0.SINGLE.CTRLB = 0x00; //Set register to Restvalue
    //Timer is back to 16Bit normal mode

    /* disable event counting */
    TCA0.SINGLE.EVCTRL &= ~(TCA_SINGLE_CNTEI_bm); 
    
    TCA0.SINGLE.CTRLB = TCA_SINGLE_CMP0EN_bm            /* enable compare channel 0 */
                        | TCA_SINGLE_WGMODE_SINGLESLOPE_gc;    /* set dual-slope PWM mode */
    //We set 0% duty here 
    TCA0.SINGLE.PERBUF = 0x800; 
    TCA0.SINGLE.CMP0BUF = 0x000;
    //Enable Timer: PWM to ~10kHz , ~20MHz Clock
    TCA0.SINGLE.CTRLA =  TCA_SINGLE_CLKSEL_DIV1_gc        /* set clock source (sys_clk/4) */
                        | TCA_SINGLE_ENABLE_bm;           /* start timer */
                        
    sei();

    

}

/**********************************************************************************************************
                                fsmstate_t CheckLimits()        
**********************************************************************************************************
 Function:    fsmstate_t CheckLimits()    
 Input:       None
 Output:      fsmstate_t
 Discription: Returns 'fault' states
**********************************************************************************************************/
void HW_190409::setup_gpio( void ){

  //We will setup all unused IO-Pins to a given state to reduce power consumption

 pinMode(PIN_PC1, INPUT);           // set pin to input
 digitalWrite(PIN_PC1, HIGH);       // turn on pullup resistors

 pinMode(PIN_PC2, INPUT);           // set pin to input
 digitalWrite(PIN_PC2, HIGH);       // turn on pullup resistors

 pinMode(PIN_PC3, INPUT);           // set pin to input
 digitalWrite(PIN_PC3, HIGH);       // turn on pullup resistors

 pinMode(PIN_PC4, INPUT);           // set pin to input
 digitalWrite(PIN_PC4, HIGH);       // turn on pullup resistors

 pinMode(PIN_PC5, INPUT);           // set pin to input
 digitalWrite(PIN_PC5, HIGH);       // turn on pullup resistors

 pinMode(PIN_PD3, INPUT);           // set pin to input
 digitalWrite(PIN_PD3, HIGH);       // turn on pullup resistors

 pinMode(PIN_PD4, INPUT);           // set pin to input
 digitalWrite(PIN_PD4, HIGH);       // turn on pullup resistors

 pinMode(PIN_PD5, INPUT);           // set pin to input
 digitalWrite(PIN_PD5, HIGH);       // turn on pullup resistors

 pinMode(PIN_PD6, INPUT);           // set pin to input
 digitalWrite(PIN_PD6, HIGH);       // turn on pullup resistors

 pinMode(PIN_PD7, INPUT);           // set pin to input
 digitalWrite(PIN_PD7, HIGH);       // turn on pullup resistors

 pinMode(PIN_PF5, INPUT);           // set pin to input
 digitalWrite(PIN_PF5, HIGH);       // turn on pullup resistors

 pinMode(PIN_PF2, INPUT);           // set pin to input
 digitalWrite(PIN_PF2, HIGH);       // turn on pullup resistors

 pinMode(PIN_PF3, INPUT);           // set pin to input
 digitalWrite(PIN_PF3, HIGH);       // turn on pullup resistors


//Spare pins - currently not used
 pinMode(PIN_PE1, INPUT);           // set pin to input
 digitalWrite(PIN_PE1, HIGH);       // turn on pullup resistors

 pinMode(PIN_PE2, INPUT);           // set pin to input
 digitalWrite(PIN_PE2, HIGH);       // turn on pullup resistors

 pinMode(PIN_PE3, INPUT);           // set pin to input
 digitalWrite(PIN_PE3, HIGH);       // turn on pullup resistors

//UART2 - Currently not used
 pinMode(PIN_PF0, INPUT);           // set pin to input
 digitalWrite(PIN_PF0, HIGH);       // turn on pullup resistors

 pinMode(PIN_PF1, INPUT);           // set pin to input
 digitalWrite(PIN_PF1, HIGH);       // turn on pullup resistors


//UART0 - Currently not used
 pinMode(PIN_PA0, INPUT);           // set pin to input
 digitalWrite(PIN_PA0, HIGH);       // turn on pullup resistors

 pinMode(PIN_PA1, INPUT);           // set pin to input
 digitalWrite(PIN_PA1, HIGH);       // turn on pullup resistors

//PWM Pin

 pinMode(PIN_PC0, OUTPUT);          // set pin to output
 digitalWrite(PIN_PC0, LOW);        // set pin to low

//12/24V Switch 

 pinMode(PIN_PE0, OUTPUT);          // set pin to output
 digitalWrite(PIN_PE0, LOW);        // set pin to low for 12V

//Vin Monitor
 pinMode(PIN_PD2, INPUT);           // set pin to input
 digitalWrite(PIN_PD2, LOW);        // turn off pullup

//Temperature monitor K-Element
 pinMode(PIN_PD0, INPUT);           // set pin to input
 digitalWrite(PIN_PD0, LOW);        // turn off pullup
 
//Temperature monitor C-Element
 pinMode(PIN_PD1, INPUT);           // set pin to input
 digitalWrite(PIN_PD1, LOW);        // turn off pullup

//I2C Bus
 pinMode(PIN_PA2, INPUT);           // set pin to input
 digitalWrite(PIN_PA2, LOW);        // turn off pullup

 pinMode(PIN_PA3, INPUT);           // set pin to input
 digitalWrite(PIN_PA2, LOW);        // turn off pullup

//Rotray Encoder
 pinMode(PIN_PA4, INPUT);           // set pin to input
 digitalWrite(PIN_PA4, HIGH);        // turn on pullup

 pinMode(PIN_PA5, INPUT);           // set pin to input
 digitalWrite(PIN_PA5, HIGH);        // turn on pullup

 pinMode(PIN_PA6, INPUT);           // set pin to input
 digitalWrite(PIN_PA6, HIGH);        // turn on pullup

//We use PA7 as testouptut, sparepin on station
 pinMode(PIN_PA7, INPUT);           // set pin to input
 digitalWrite(PIN_PA7, HIGH);        // turn on pullup


}  



/**********************************************************************************************************
                                fsmstate_t CheckLimits()        
**********************************************************************************************************
 Function:    fsmstate_t CheckLimits()    
 Input:       None
 Output:      fsmstate_t
 Discription: Returns 'fault' states
**********************************************************************************************************/
fsmstate_t HW_190409::CheckLimits(){
  
  return NOFAULT;

}


/**********************************************************************************************************
                                fsmstate_t CheckLimits()        
**********************************************************************************************************
 Function:    fsmstate_t CheckLimits()    
 Input:       None
 Output:      fsmstate_t
 Discription: Returns 'fault' states
**********************************************************************************************************/
void HW_190409::SetPWM( uint16_t PWM_Value){
 
    if(PWM_Value>=MAX_PWM_LIMIT){
      PWM_Value = MAX_PWM_LIMIT;
    }
    PWM.On(PWM_Value);

}

/**********************************************************************************************************
                                void ShowUndervoltage()        
**********************************************************************************************************
 Function:    void ShowUndervoltage()    
 Input:       None
 Output:      None
 Discription: Show the undervoltage screen 
**********************************************************************************************************/
void HW_190409::ShowUndervoltage(){
  //Not supported
}


/**********************************************************************************************************
                                void SetSolderingIron()        
**********************************************************************************************************
 Function:    void SetSolderingIron()    
 Input:       SolderingIronType_t
 Output:      None
 Discription: Sets the Iron and configures VON accordingly
**********************************************************************************************************/
void HW_190409::SetSolderingIron(SolderingIronType_t Iron ){

    switch( Iron ){
      case IRON_HAKKO_FX8801:{
        //24V, Type-C
        Temp.SetThermoType(Type_C);
        Vin.SetConfiguredVIN(VIN_24V);
      } break;

      case IRON_JBC_T245A:{
        //24V, Type-K
        Temp.SetThermoType(Type_K);
        Vin.SetConfiguredVIN(VIN_24V);
      } break;

      case IRON_WELLER_RT:{
        //12V, Type-K
        Temp.SetThermoType(Type_K);
        Vin.SetConfiguredVIN(VIN_12V);

      }break;

      default:{
        Temp.SetThermoType(Type_K);
        Vin.SetConfiguredVIN(VIN_12V);
      }break;
    }
    write_StoreIronConfig(Iron);
}


/**********************************************************************************************************
                                void GetSolderingIron()        
**********************************************************************************************************
 Function:    void GetSolderingIron()    
 Input:       None
 Output:      SolderingIronType_t
 Discription: Get the currently configured iron form the station
**********************************************************************************************************/
SolderingIronType_t HW_190409::GetSolderingIron( void ){
  return read_StoreIronConfig();
}


/*************************************************************************************************************
 *                                          write_StoreTemperature()
 *************************************************************************************************************
 Function:    write_StoreTemperature()
 Input:       Nones
 Output:      uint16_t 
 Description: Read previous saved temperature from EEPROM,
              If no previous value saved then set temperature to 50 degrees
 *************************************************************************************************************/    
void HW_190409::write_StoreIronConfig(SolderingIronType_t IronType)
{
            uint8_t mode = (uint8_t)(IronType);
            EEPROM.update(7, ( mode ) );
            EEPROM.update(8, (~mode) );
}


/*************************************************************************************************************
 *                                          read_StoreTemperature()
 *************************************************************************************************************
 Function:    read_StoreTemperature()
 Input:       None
 Output:      uint16_t 
 Description: Read previous saved temperature from EEPROM,
              If no previous value saved then set temperature to 50 degrees
 *************************************************************************************************************/    
SolderingIronType_t HW_190409::read_StoreIronConfig( void )
{
  uint8_t mode=0;
  uint8_t moden=0;

   mode = EEPROM.read(7);
   moden= EEPROM.read(8);
   moden = ~moden;

   if( mode != moden ){
    mode=IRON_WELLER_RT;
    write_StoreIronConfig(IRON_WELLER_RT);
   }
   return (SolderingIronType_t)(mode);
   
}

/*************************************************************************************************************
 *                                          read_StoreTemperature()
 *************************************************************************************************************
 Function:    read_StoreTemperature()
 Input:       None
 Output:      uint16_t 
 Description: Read previous saved temperature from EEPROM,
              If no previous value saved then set temperature to 50 degrees
 *************************************************************************************************************/    
uint16_t HW_190409::read_StoreTemperature()
{
   uint16_t saved_Temperature_1=0;

   saved_Temperature_1 = EEPROM.read(2);
   saved_Temperature_1 = saved_Temperature_1 << 8;
   saved_Temperature_1 = saved_Temperature_1 | EEPROM.read(3);
  
   if( ( saved_Temperature_1 > MAX_TEMP) || ( saved_Temperature_1 < MIN_TEMP) ){
    saved_Temperature_1= MIN_TEMP ;
   }
   return saved_Temperature_1;
   
}

/*************************************************************************************************************
 *                                          write_StoreTemperature()
 *************************************************************************************************************
 Function:    write_StoreTemperature()
 Input:       Nones
 Output:      uint16_t 
 Description: Read previous saved temperature from EEPROM,
              If no previous value saved then set temperature to 50 degrees
 *************************************************************************************************************/    
void HW_190409::write_StoreTemperature(uint16_t tempValue)
{
            EEPROM.update(2, ( ( tempValue & 0xFF00 ) >> 8 ) );
            EEPROM.update(3, ( ( tempValue & 0x00FF ) >> 0 ) );
}