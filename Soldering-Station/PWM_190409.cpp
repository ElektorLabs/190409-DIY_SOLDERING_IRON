#include "PWM_190409.h"


/**********************************************************************************************************
                                void On()        
**********************************************************************************************************
 Function:    void On()
 Input:       uint16_t value
 Output:      None
 Discription: Sets the PWM to the given value, is the value is zero, this turns the PWM off
**********************************************************************************************************/
void PWM_190409::On( uint16_t value ){
    if(value ==0)
    {
      Off();
    
    } else {
    
      if(value >= 0x800 ){
        value = 0x07FF;
      }
      TCA0.SINGLE.CMP0BUF = value&0x07FF; //Clip value to 11Bit
     
    }
}

/**********************************************************************************************************
                                void Off()        
**********************************************************************************************************
 Function:    void Off()
 Input:       None
 Output:      None
 Discription: Turns the PWM off
**********************************************************************************************************/
void PWM_190409::Off( void ) {

  //Disable PWM
  TCA0.SINGLE.CMP0BUF=0;

}  

/**********************************************************************************************************
                                uint16_t ReadPWMValue()        
**********************************************************************************************************
 Function:    uint16_t ReadPWMValue()     
 Input:       None
 Output:      uint16_t OCR3A
 Discription: Returns the RAW value ot the PWM
**********************************************************************************************************/  
uint16_t PWM_190409::ReadPWMValue( void ) {
    
    //Return current PWM value
    return TCA0.SINGLE.CMP0BUF;
    
}


    
    
