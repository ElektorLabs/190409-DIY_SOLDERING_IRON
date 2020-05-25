#include "enums.h"
#include "VIN_190409.h"

/**********************************************************************************************************
                                void Read()        
**********************************************************************************************************
 Function:    Read
 Input:       uint16_t avg_cnt
 Output:      uint16_t ( Voltage in mV ) 
 Discription: Reads the inputvoltage 
**********************************************************************************************************/
uint16_t VIN_190409::Read( uint8_t avg_cnt){
    
    uint16_t voltage=0;
    uint16_t samples=0;  
    for(uint8_t i =0;i< avg_cnt;i++)
    {
      samples += analogRead(A2);
    } 
    samples = samples / avg_cnt;
    /* 
        we have a 10k / 2k2 divider in front 
        this results in vin = vadc / 0.2125984252
        with vadc = value *5 / 1024
        we have 4.8875mV per Bit
        
        correct would be  adc_value * 4.8875 *4.7037 = vin_mV
    */
   voltage = samples * 23 ; 
   LastReadVIN=voltage;
   return voltage;
}

/**********************************************************************************************************
                                void GetLastValue()        
**********************************************************************************************************
 Function:    GetLastValue
 Input:       none
 Output:      uint16_t ( Voltage in mV ) 
 Discription: Returns the last sampeled inputvoltage 
**********************************************************************************************************/
uint16_t VIN_190409::GetLastValue( void ){
    return LastReadVIN;
}


/**********************************************************************************************************
                                void SetConfiguredVIN()        
**********************************************************************************************************
 Function:    SetConfiguredVIN
 Input:       VIN_TARGETVOLTAGE_t
 Output:      none
 Discription: Sets the input voltage configuration 
**********************************************************************************************************/
void VIN_190409::SetConfiguredVIN( VIN_TARGETVOLTAGE_t tgv ){

     switch(tgv){
         
         case VIN_12V:{
             digitalWrite(VIN_SWITCH,LOW);
         } break;

         case VIN_24V:{
            digitalWrite(VIN_SWITCH,HIGH);
         } break;

         default:{
             //We fall back to 12V
              digitalWrite(VIN_SWITCH,LOW);
         } break;

     }
 }

/**********************************************************************************************************
                                void GetConfiguredVIN()        
**********************************************************************************************************
 Function:    GetConfiguredVIN
 Input:       none
 Output:      VIN_TARGETVOLTAGE_t  
 Discription: Returns the input voltage configuration 
**********************************************************************************************************/
VIN_TARGETVOLTAGE_t VIN_190409::GetConfiguredVIN( void ){
    if(digitalRead(VIN_SWITCH)!=0){
        return VIN_24V;
    } else {    
        return VIN_12V;
    }
}
