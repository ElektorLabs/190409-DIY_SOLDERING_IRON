#include "TEMP_190409.h"



TEMP_190409::TEMP_190409( void ){
    analogReference(VDD); 
};


void TEMP_190409::SetThermoType( ThermoeElementType_t tp){
   ElementType = tp;
}

ThermoeElementType_t TEMP_190409::GetThermoType( void ){
   return ElementType;
}

/**********************************************************************************************************
                                uint16_t Read( uint8_t )        
**********************************************************************************************************
 Function:    uint16_t Read( uint8_t )     
 Input:       uint16_t avg_cnt
 Output:      uint16_t Temp in °C
 Discription: Reads the Temp in °C
**********************************************************************************************************/
uint16_t TEMP_190409::Read( uint8_t avg_cnt){
    
    uint16_t temperature=999;
    uint16_t samples=0;
    uint16_t ADCvoltage =0;  

   //At this point the Temperatur will be compensated to 25°C Junction Point temperatur due to the code involved 
   //We need to messure the current chip temperatur and compensate for it
   uint16_t AmbienetTempKelvin = GetAmbientTemperaturKelvin();
   if(AmbienetTempKelvin > 399 ){
      //We have more than 125°C thats wrong
      //We shoudl give a warning about it
      AmbienetTempKelvin = 399;
   }

   if(AmbienetTempKelvin <  233 ){
      //We have more than -40°C thats wrong
      //We shoudl give a warning about it
      AmbienetTempKelvin = 233;
   }
   
   //We need to substract 273.15° to get Celsius
   int16_t OffSetDegree =  AmbienetTempKelvin  - 299 ;

   


   switch(ElementType){
   
      case Type_K:{
               for(uint8_t i =0;i< avg_cnt;i++)
               {
                  samples += analogRead(A0);
               }
               samples = samples / avg_cnt;
               ADCvoltage = map(samples,0,1023,0,5000);

            if(ADCvoltage>2000){
               temperature=999;
            } else if( (ADCvoltage>=1550) && (ADCvoltage < 2000) ){
            temperature = map(ADCvoltage, 1550, 2000,460,630);
            } else if(ADCvoltage >= 885 && ADCvoltage < 1555){//1555 max voltage value (mV)
               temperature = map(ADCvoltage, 885, 1560,290,460); //ADCvoltage =  voltage value (mV), 885 = lower limit(mV), 1555 = higher limit(mv), 290 = lower limit(degree), 460 = higher limit(degree)  
            } else if(ADCvoltage >=  205&& ADCvoltage < 885) {
               temperature = map(ADCvoltage, 205, 885,100,289); 
            } else if (ADCvoltage >= 58 && ADCvoltage <205){
               temperature = map(ADCvoltage,58,205,50,99);
            } else if(ADCvoltage < 58) {
               temperature = map(ADCvoltage,0,58,25,49);  //25 considered as room temperature.
            } else {
               /* This will considerd as internal fault */
            }
      } break;

      case Type_C:{
            for(uint8_t i =0;i< avg_cnt;i++)
            {
               samples += analogRead(A1);
            }
            samples = samples / avg_cnt;
            ADCvoltage = map(samples,0,1023,0,5000);

            if(ADCvoltage>2000){
               temperature=999;
            } else if( (ADCvoltage>=1550) && (ADCvoltage < 2000) ){
            temperature = map(ADCvoltage, 1550, 2000,460,630);
            } else if(ADCvoltage >= 885 && ADCvoltage < 1555){//1555 max voltage value (mV)
               temperature = map(ADCvoltage, 885, 1560,290,460); //ADCvoltage =  voltage value (mV), 885 = lower limit(mV), 1555 = higher limit(mv), 290 = lower limit(degree), 460 = higher limit(degree)  
            } else if(ADCvoltage >=  205&& ADCvoltage < 885) {
               temperature = map(ADCvoltage, 205, 885,100,289); 
            } else if (ADCvoltage >= 58 && ADCvoltage <205){
               temperature = map(ADCvoltage,58,205,50,99);
            } else if(ADCvoltage < 58) {
               temperature = map(ADCvoltage,0,58,25,49);  //25 considered as room temperature.
            } else {
               /* This will considerd as internal fault */
            }

      } break;

      default:{
               //This is a config problem and shall not happen at all
               temperature=999;
               
      } break;
   }
      

   //We have calculated with 25°C or 298.15°K 
   if((  temperature - OffSetDegree) > 0 ){
      if(temperature<999){
         LastReadTemp = ( temperature - OffSetDegree );
         } else {
          LastReadTemp =  temperature; 
         }
   } else {
      LastReadTemp =0;
   }
   
   return LastReadTemp;
}

/**********************************************************************************************************
                                uint16_t GetLastValue( void )        
**********************************************************************************************************
 Function:    uint16_t GetLastValue( void )     
 Input:       None
 Output:      uint16_t Temp in °C
 Discription: Returns the last read the Temp in °C
**********************************************************************************************************/
uint16_t TEMP_190409::GetLastValue( void ){
    return LastReadTemp;
}

int16_t TEMP_190409::GetAmbientTemperaturKelvin( void ){
   
   uint8_t RegTempCTRLC = ADC0.CTRLC;
   uint8_t RegTempCTRLD = ADC0.CTRLD;
   uint8_t RegTempSAMPLECTRL = ADC0.SAMPCTRL; 
   uint8_t RegTempMUXPOS = ADC0.MUXPOS;

   uint8_t RegTempVREFCrtA =  VREF.CTRLA;
   uint8_t RegTempVREFCrtB =  VREF.CTRLB;

   VREF.CTRLA = 0x10;
   VREF.CTRLB = 0x02;

   ADC0.CTRLC = 0x40 | 0x00 | 0x04;
   ADC0.CTRLD = ADC0.CTRLD | 0x40 ; //32us * 0.625MHz = 20 , setting 32
   ADC0.SAMPCTRL = 24;
   ADC0.MUXPOS = 0x1E;

   /* Start conversion */
	ADC0.COMMAND = ADC_STCONV_bm;

	/* Wait for result ready */
	while(!(ADC0.INTFLAGS & ADC_RESRDY_bm));

	/* Save state */
	uint8_t status = SREG;
	cli();
   uint8_t low, high;
	/* Read result */
	low = ADC0.RESL;
	high = ADC0.RESH;

	/* Restore state */
	SREG = status;

   ADC0.CTRLC = RegTempCTRLC;
   ADC0.CTRLD = RegTempCTRLD;
   ADC0.SAMPCTRL = RegTempSAMPLECTRL;
   ADC0.MUXPOS = RegTempMUXPOS;

    VREF.CTRLA = RegTempVREFCrtA;
    VREF.CTRLB = RegTempVREFCrtB;
   
   int8_t sigrow_offset = SIGROW.TEMPSENSE1; // Read signed value from signature row
   uint8_t sigrow_gain = SIGROW.TEMPSENSE0;  // Read unsigned value from signature row
   uint16_t adc_reading = (high << 8) | low; // ADC conversion result with 1.1 V internal reference 
   uint32_t temp = adc_reading - sigrow_offset;temp *= sigrow_gain; // Result might overflow 16 bit variable (10bit+8bit)
   temp += 0x80; // Add 1/2 to get correct rounding on division below
   temp >>= 8; // Divide result to get Kelvin
   uint16_t temperature_in_K = temp;
   return temperature_in_K;
   
}