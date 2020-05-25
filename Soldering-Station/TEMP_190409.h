#ifndef TEMP_190409_H_
 #define TEMP_190409_H_ 
 
 /* includes for this class */
 #include "Arduino.h"
 #include "enums.h"


/**********************************************************************************************************
                               CLASS TEMP_190409 
**********************************************************************************************************
 Baseclass:   N/A
 Function:    Reads the IRON Temperatur
 Input:       None
 Output:      None
 Discription: Handels the temperatur read of 150500
**********************************************************************************************************/
class TEMP_190409 {
    
public: 
    TEMP_190409();
    void SetThermoType( ThermoeElementType_t tp);
    ThermoeElementType_t GetThermoType( void );
    uint16_t Read( uint8_t avg_cnt);
    uint16_t GetLastValue( void ) ;
    int16_t GetAmbientTemperaturKelvin( void );
    
private:
    uint16_t LastReadTemp=999;
    ThermoeElementType_t ElementType = Type_K;
   

};
#endif
