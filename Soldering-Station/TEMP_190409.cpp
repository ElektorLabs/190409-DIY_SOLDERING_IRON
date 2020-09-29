#include "TEMP_190409.h"


OneWire  ds(15);  // on pin 38/PF4 (a 4.7K resistor is necessary)
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
   OneWireStartConversation();
   
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
   
   //We need to substract 273.15° to get Celsius and also 25°C as all calculations will add 25°C as this is
   //is considered room temperatur makes -298.15 ~299---
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

  if(true == OneWireSensorFound){
    return GetLastOneWireTempKelvin();
  } else {
    //return GetOnchipTempKelvin(); //This is generally 7°C above ambient
    //we return simply 25°C
    return 298;
  }
}

int16_t TEMP_190409::GetOnchipTempKelvin(){
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
   //The value deliverd is about 7 degree about surrounding ....
   
}

bool TEMP_190409::OneWireStartConversation( void ){
  //We only start every 2 seconds a new conversation....

  byte i;

  byte present = 0;
  byte data[12];
  float celsius, fahrenheit;

  if( (true == OneWireFetchNewData ) && ( true == OneWireSensorFound) ) {
    if( ( last_onewirestrart+10000 )<millis() ){
        //Read new data.....
        present = ds.reset();
        ds.select(addr);    
        ds.write(0xBE);         // Read Scratchpad
      
        for ( i = 0; i < 9; i++) {           // we need 9 bytes
          data[i] = ds.read();
        }
      
        if( data[8] != OneWire::crc8(data, 8) ){
          //CRC error 
          return false;
        }
      
        // Convert the data to actual temperature
        // because the result is a 16 bit signed integer, it should
        // be stored to an "int16_t" type, which is always 16 bits
        // even when compiled on a 32 bit processor.
        int16_t raw = (data[1] << 8) | data[0];
        if (type_s) {
          raw = raw << 3; // 9 bit resolution default
          if (data[7] == 0x10) {
            // "count remain" gives full 12 bit resolution
            raw = (raw & 0xFFF0) + 12 - data[6];
          }
        } else {
          byte cfg = (data[4] & 0x60);
          // at lower res, the low bits are undefined, so let's zero them
          if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
          else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
          else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
          //// default is 12 bit resolution, 750 ms conversion time
        }
        celsius = (float)raw / 16.0;
        fahrenheit = celsius * 1.8 + 32.0;
        LastTempReadKelvin=(int16_t)celsius+273;
        OneWireFetchNewData=false;
      
    }
    
  } 

  
  if(  ( ( last_onewirestrart+20000 )<millis()  )  && ( true == OneWireSensorFound) ) {
    ds.reset_search();
    last_onewirestrart=millis();
        
        
              ds.reset();
              ds.select(addr);
              ds.write(0x44, 1);        // start conversion, with parasite power on at the end
              ds.reset_search();
              OneWireFetchNewData=true;
              return true;
            
            
   }

        
  
  return false;
  
  
  
  
}

int16_t TEMP_190409::GetLastOneWireTempKelvin( void ){

  return LastTempReadKelvin;
  
}


bool TEMP_190409::SearchOneWireSensor( void ){

    if(false == OneWireSensorFound ){
          while( ds.search(addr)) {
        
              Serial.print("ROM =");
              for( uint8_t i = 0; i < 8; i++) {
                Serial.write(' ');
                Serial.print(addr[i], HEX);
              }
            
              if (OneWire::crc8(addr, 7) != addr[7]) {
                  Serial.println("CRC is not valid!");
                  OneWireSensorFound=false;
                  return false;
              }
              Serial.println();
         
              // the first ROM byte indicates which chip
              switch (addr[0]) {
                case 0x10:
                  Serial.println("  Chip = DS18S20");  // or old DS1820
                  type_s = 1;
                  break;
                case 0x28:
                  Serial.println("  Chip = DS18B20");
                  type_s = 0;
                  break;
                case 0x22:
                  Serial.println("  Chip = DS1822");
                  type_s = 0;
                  break;
                default:
                  Serial.println("Device is not a DS18x20 family device.");
                  OneWireSensorFound=false;
                  return false;
                  
              } 
              OneWireSensorFound=true;
          }
    }
    return OneWireSensorFound;
}

uint16_t TEMP_190409::GetWireTempKelvin( void ){
        
        byte i;
        
        byte present = 0;
        byte data[12];
        float celsius, fahrenheit;

        ds.reset();
        ds.select(addr);
        ds.write(0x44, 1);        // start conversion, with parasite power on at the end
        ds.reset_search();

        delay(1000);


        present = ds.reset();
        ds.select(addr);    
        ds.write(0xBE);         // Read Scratchpad
      
        for ( i = 0; i < 9; i++) {           // we need 9 bytes
          data[i] = ds.read();
         
      
        }
       
       if(data[8] != OneWire::crc8(data, 8) ){
        Serial.println("OneWire CRC Error"); 
       }
       
        // Convert the data to actual temperature
        // because the result is a 16 bit signed integer, it should
        // be stored to an "int16_t" type, which is always 16 bits
        // even when compiled on a 32 bit processor.
        int16_t raw = (data[1] << 8) | data[0];
        if (type_s) {
          raw = raw << 3; // 9 bit resolution default
          if (data[7] == 0x10) {
            // "count remain" gives full 12 bit resolution
            raw = (raw & 0xFFF0) + 12 - data[6];
          }
        } else {
          byte cfg = (data[4] & 0x60);
          // at lower res, the low bits are undefined, so let's zero them
          if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
          else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
          else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
          //// default is 12 bit resolution, 750 ms conversion time
        }
        celsius = (float)raw / 16.0;
        fahrenheit = celsius * 1.8 + 32.0;
        LastTempReadKelvin=(int16_t)celsius+273; 
        return LastTempReadKelvin;
}
