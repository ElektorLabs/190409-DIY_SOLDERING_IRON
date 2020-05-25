#ifndef VIN_190409_H_
 #define VIN_190409_H_ 
 
 /* includes for this class */
 #include "Arduino.h"
 #include "enums.h"

#define VIN_SWITCH ( 30 )
/**********************************************************************************************************
                               CLASS TEMP_150500 
**********************************************************************************************************
 Baseclass:   N/A
 Function:    Reads the IRON Temperatur
 Input:       None
 Output:      None
 Discription: Handels the temperatur read of 150500
**********************************************************************************************************/
class VIN_190409 {
    
public: 
    VIN_190409(){};
    uint16_t Read( uint8_t avg_cnt);
    uint16_t GetLastValue( void ) ;
    void SetConfiguredVIN( VIN_TARGETVOLTAGE_t tgv );
    VIN_TARGETVOLTAGE_t GetConfiguredVIN( void );
 
private:
    uint16_t LastReadVIN=0;

};
#endif