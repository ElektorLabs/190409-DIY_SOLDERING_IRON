#include <Arduino.h>
#include "enums.h"
#include "serialcli.h"


uint8_t msgbuffer[64]={0,};
uint8_t msgbufferidx=0;
bool first_con = false;

void WriteTXPower(uint8_t power);
uint8_t GetTXPower( void );

typedef enum {
cmd_data_error=0,
cmd_data_get,
cmd_data_set,
cmd_data_clear,
cmd_data_help
} cmd_accesstype_t;


typedef enum {
    cmd_none = 0,
    cmd_setpoint,
    cmd_temperatur,
    cmd_ambienttemp,
    cmd_error,
    cmd_irontype,
    cmd_cnt
} cmd_command_t;

typedef enum{
    param_none=0,
    param_u8,
    param_i8,
    praam_u16,
    param_i16,
    param_u32,
    param_i32,
    param_string,
    param_keyword
} cmd_param_t;

typedef enum {
  KW_PARSE_ERROR=0,
  KW_ON,
  KW_OFF,
  KW_JBC,
  KW_HKO,
  KW_WRT,
  KW_CNT
}keyword_t;




typedef union {
        uint8_t u8;
        int8_t i8;
        uint16_t u16;
        int16_t i16;
        uint32_t u32;
        int32_t i32;
        float fl;
        char* ch;
        keyword_t kw; //keyword index for parameter like on or off
    }cmd_parameter_t;

typedef struct{
    cmd_accesstype_t cmd_dir;
    cmd_command_t command;
    cmd_parameter_t parameter;
    uint8_t param_len;
} serial_command_t;

cmd_param_t ParamLUT[ cmd_cnt ]={
 [cmd_none] = param_none,
 [cmd_setpoint] = praam_u16,
 [cmd_temperatur] = param_none,
 [cmd_ambienttemp] = param_none,
 [cmd_error] = param_none,
 [cmd_irontype] = param_keyword
};


/* This is a bit hacky and needs to be solved by functionpointer */
extern void command_if_update_setpoint( uint16_t newsetpoint );
extern uint16_t command_if_get_setpoint( void );
extern uint16_t command_if_get_temperature(void  );
extern uint8_t command_if_get_error( void );
extern void command_if_clear_error( void );
extern int16_t command_if_get_ambienttemp( void );
extern SolderingIronType_t command_if_get_irontype( void );
extern void command_if_set_irontype( SolderingIronType_t iron );






void ProcessCommand(  serial_command_t command );
bool ConvertHexStrToHexArray( char* keystr, uint8_t keylen, uint8_t* data, uint8_t datalen);

/**************************************************************************************************
 *    Function      : print_array
 *    Description   : will print an array of data as hex
 *    Input         : uint8_t * data , uint32_t len 
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void print_array(uint8_t * data , uint32_t len ){

  for(uint32_t i=0;i<len;i++){
           Serial.print("0x");
           Serial.print(*data++,HEX);
           Serial.print(" ");
  }
  Serial.println("");
}

/**********************************************************************************************************
                                void PrintErrorLocation()        
**********************************************************************************************************
 Function:    void PrintErrorLocation()
 Input:       uint8_t startidx, uint8_t current_index 
 Output:      None 
 Description: Show the location of a parsing error
**********************************************************************************************************/
void PrintErrorLocation(uint8_t startidx, uint8_t current_index  ){
          Serial.print(F(" Syntax error near:\""));
                        for(uint8_t c = startidx;c<current_index;c++){
                            if( msgbuffer[c]=='\n' ) {
                                Serial.print(F("[NEW LINE]"));
                            } else if ( msgbuffer[c]=='\r'){
                            Serial.print(F("[RETURN]"));
                            } else {
                                Serial.write( msgbuffer[c]);
                            }
                        }
                    Serial.println("\"");
}


/**********************************************************************************************************
                                void SerialConsolePrintWelcome()        
**********************************************************************************************************
 Function:    void SerialConsolePrintWelcome()
 Input:       None
 Output:      None 
 Description: Show the welcome banner
**********************************************************************************************************/
void SerialConsolePrintWelcome( void ){
  if(Serial){
        Serial.println(F("Elektor Solderingstation FW 1.4"));
        Serial.println(F("  on Hardware 190409           "));
        Serial.println(F("------------------------------------------------------------------"));
        Serial.print(F(">"));

  }
}


/**********************************************************************************************************
                                void SerialCommandShowHelp()        
**********************************************************************************************************
 Function:    void SerialCommandShowHelp()
 Input:       None
 Output:      None 
 Description: Show the help
**********************************************************************************************************/
void SerialCommandShowHelp( void ){
    Serial.println(F("Supported commands"));
    Serial.println(F("------------------------------------------------------------------"));
    Serial.println(F("set/get setpoint [xxx] -> This will set or get the setpoint"));
    Serial.println(F("set/get irontype [xxx] -> This will set or get the irontype"));
    Serial.println(F("                  WRT = Weller RT Tip ( 12V / Type K )  "));
    Serial.println(F("                  HKO = HAKKO Tip ( 24V / Type C )  "));
    Serial.println(F("                  JBC = JBC Tip ( 24V / Type K )  "));
    Serial.println(F("get temperature        -> This will report the current temp."));
    Serial.println(F("get ambienttemp        -> This will report the ambient temp."));
    Serial.println(F("clear error            -> If an error is shown this will clear it"));
    Serial.println(F("help                   -> This will show this help"));
    Serial.println(F("-------------------------------------------------------------------"));
}




/**********************************************************************************************************
                                void SerialConsoleParseInput()        
**********************************************************************************************************
 Function:    void SerialConsoleParseInput()
 Input:       None
 Output:      serial_command_t 
 Description: Command parser, returns a struct with the parsed result
**********************************************************************************************************/
serial_command_t SerialConsoleParseInput( void ){
    serial_command_t command;
    command.cmd_dir = cmd_data_error;
    command.command = cmd_none;
    command.parameter.u32=0;
    if(msgbufferidx>sizeof(msgbuffer)){
        Serial.println( F("Buffer overflow") );
        for(uint8_t i=0;i<sizeof(msgbuffer);i++){
           msgbuffer[i]=0;
        }
        msgbufferidx=0;
    } else {
        uint8_t startidx = 0;
        uint8_t endidx = 0;
        for(uint8_t i=0;i<msgbufferidx;i++){
            if( ( msgbuffer[i]==' ') || (msgbuffer[i]=='\n') || (msgbuffer[i]=='\r') ){
                /* The first token has ended, we accept only 'set', 'get' and 'help' */
                if(strncmp_P((const char*)&msgbuffer[startidx] ,PSTR("set"), i)==0){
                    command.cmd_dir = cmd_data_set;
                } else if(strncmp_P((const char*)&msgbuffer[startidx] ,PSTR("get"), i)==0){
                    command.cmd_dir = cmd_data_get;
                } else if(strncmp_P((const char*)&msgbuffer[startidx] ,PSTR("help"), i)==0){
                    command.cmd_dir = cmd_data_help;
                } else if (strncmp_P((const char*)&msgbuffer[startidx] ,PSTR("clear"), i)==0){
                     command.cmd_dir = cmd_data_clear;
                } else {
                    command.cmd_dir = cmd_data_error;
                    PrintErrorLocation(startidx,i);
                }
                endidx=i;
                break;
            } else {
              endidx=i;
            }
        } 

        startidx = endidx+1;
        if( (startidx<msgbufferidx) && (msgbuffer[endidx]!='\n' ) &&  (msgbuffer[endidx]!='\r') )  {
          
        } else {
            /* Parsing is done */
            return command;
        }

        for(uint8_t i=startidx;i<msgbufferidx;i++){
            if( ( msgbuffer[i]==' ') || (msgbuffer[i]=='\n') || (msgbuffer[i]=='\r') ){
                /* The second token has ended, we accept only 'setpoint' , 'temperature' and 'error' */
                       uint8_t tokenlenght = i - startidx;
                if(strncmp_P(&msgbuffer[startidx] ,PSTR("setpoint"), (tokenlenght) )==0){
                    command.command=cmd_setpoint;
                } else if(strncmp_P(&msgbuffer[startidx] ,PSTR("temperature"), (tokenlenght) )==0){
                    command.command=cmd_temperatur;
                 } else if(strncmp_P(&msgbuffer[startidx] ,PSTR("ambienttemp"), (tokenlenght) )==0){
                    command.command=cmd_ambienttemp;
                } else if(strncmp_P(&msgbuffer[startidx] ,PSTR("error"), (tokenlenght) )==0){
                    command.command=cmd_error;
                } else if(strncmp_P(&msgbuffer[startidx] ,PSTR("irontype"), (tokenlenght) )==0){
                    command.command=cmd_irontype;
                } else {
                    command.command = cmd_none;
                        PrintErrorLocation(startidx,i);
                }
                endidx=i;
                break;
                
            } else {
                endidx=i;
            }
        }

         startidx = endidx+1;
        if( (startidx<msgbufferidx) && (msgbuffer[endidx]!='\n' ) &&  (msgbuffer[endidx]!='\r') )  {
          //We have some elements left, check if this is wanted
          if((ParamLUT[command.command]!=param_none)){
              //We can move on
          } else {
              //we have a syntax problem here
              command.command = cmd_none;
              PrintErrorLocation(startidx,msgbufferidx);
          }
        } else {
            /* Parsing is done */
            return command;
        }
        /*  
            last is to parse the parameter here 
            we currently strings and integer ( u32 ) but need to decide what we must do
        */
        
        if( (ParamLUT[command.command]!=param_string) && (ParamLUT[command.command]!=param_keyword) ){
          uint32_t value = 0;
          command.param_len=0;
          for(uint8_t i=startidx;i<msgbufferidx;i++){
              
              if( (msgbuffer[i]>='0') && (msgbuffer[i]<='9') ){
                  value = value * 10;
                  value = value + ( msgbuffer[i] -'0');
              } else {
                  if( ( msgbuffer[i]=='\n') || (msgbuffer[i]=='\r') ){
                      command.param_len = i;
                       command.parameter.u32 = value;
                      return command;
                  } else if( (msgbuffer[i]<31) || ( msgbuffer[i]>=127)){
                       /* Parsing error */
                  command.cmd_dir = cmd_data_error;
                  command.command = cmd_none;
                  command.parameter.u32=0;
                  Serial.print(" Syntax error near:\"");
                  PrintErrorLocation(startidx,i);
                    
                  } else {
                    //Next char
                  }
              }                
          }
        } else if( (ParamLUT[command.command]==param_string) || (ParamLUT[command.command]==param_keyword) ){
          //This will be "simple" we look if we end with \n or \r and use the rest as
          //string for command processing
          command.parameter.ch=(char*)&msgbuffer[startidx]; //String start in Buffer
          uint8_t StringLen = 0;
          for(uint8_t i=startidx;i<msgbufferidx;i++){
             
                if( ( msgbuffer[i]=='\n') || (msgbuffer[i]=='\r') ){
                      //If we expect a param_sting we are done
                    if(ParamLUT[command.command]==param_string){
                        command.param_len=StringLen;
                        return command;
                    } else {
                          //We need to ckeck for supported keywords
                        if(strncmp_P((const char*)&msgbuffer[startidx] ,PSTR("ON"), (StringLen) )==0){
                                command.parameter.kw =(keyword_t)( KW_ON );
                                 return command;
                        } else if(strncmp_P((const char*)&msgbuffer[startidx] ,PSTR("OFF"), (StringLen) )==0){
                                command.parameter.kw =(keyword_t)( KW_OFF );
                                 return command;
                        } else if(strncmp_P((const char*)&msgbuffer[startidx] ,PSTR("JBC"), (StringLen) )==0){
                                command.parameter.kw =(keyword_t)( KW_JBC );
                                 return command;
                        } else if(strncmp_P((const char*)&msgbuffer[startidx] ,PSTR("HKO"), (StringLen) )==0){
                                command.parameter.kw =(keyword_t)( KW_HKO );
                                 return command;
                        } else if(strncmp_P((const char*)&msgbuffer[startidx] ,PSTR("WRT"), (StringLen) )==0){
                                command.parameter.kw =(keyword_t)( KW_WRT );
                                 return command;
                        } else {
                            //We need to signal an parsing error
                            Serial.print("'");
                            for( uint8_t i=0;i<StringLen;i++){
                                Serial.print((char)(msgbuffer[startidx+i]) );
                            }
                            Serial.println("' not recognized as keyword" );
                            command.parameter.u32 =(uint32_t)( KW_PARSE_ERROR );
                            return command;
                        }
                    }
                }
                StringLen++;
            }
            /* Parsing error */
            command.cmd_dir = cmd_data_error;
            command.command = cmd_none;
            command.parameter.u32=0;
            Serial.print("Syntax error near:\"");
            PrintErrorLocation(startidx,msgbufferidx);
          
          
        } else {
          
        }
        
        //We need to grab the end of the input and build a kind of param out of it
        //Also we may need to do the error handling here 

        return command;

    }
    return command;

}


/**********************************************************************************************************
                                void SerialConsoleProcess()        
**********************************************************************************************************
 Function:    void SerialConsoleProcess()
 Input:       None
 Output:      None 
 Description: Processes incomming serial data
**********************************************************************************************************/
bool SerialConsoleProcess( void ){
    bool InputProcessed = false;
    if(Serial){
        if(first_con == false){
            SerialConsolePrintWelcome();
            first_con=true;
        }
        /* Grab data from interface */
        while ( ( Serial ) && (Serial.available() > 0)) {
                InputProcessed = true;
                // read the incoming byte:
                uint8_t data = Serial.read();
                if( (data!='\r' ) && (data!='\n')){
                    Serial.write(data);
                }
            
                if(msgbufferidx<sizeof(msgbuffer) ){
                    msgbuffer[msgbufferidx]=data;
                    msgbufferidx++;
                } else {
                    /* we have an overflow */
                    msgbufferidx=sizeof(msgbuffer);
                }

                if( (data=='\r' ) || (data=='\n')){   /* Time to parse data */
                    Serial.print(F("\n\r"));
                    if( msgbufferidx == 0){
                       /* No Data in the buffer */ 
                    } else {
                        /* we need to parse the buffer */
                        serial_command_t command = SerialConsoleParseInput();
                        ProcessCommand(command);
                       
                        /* we clear the buffer */

                        for (uint8_t i=0;i<sizeof(msgbuffer);i++){
                            msgbuffer[i]=0;
                        }
                        msgbufferidx = 0;
                        Serial.print(F("\r\n>"));

                    }
                }

        }     
    } else {
        first_con=false;
    }


    return InputProcessed;


}


/**********************************************************************************************************
                                void ProcessCommand()        
**********************************************************************************************************
 Function:    void SerialConsoleProcess()
 Input:       None
 Output:      None 
 Description: Processes decoded command
**********************************************************************************************************/
void ProcessCommand(  serial_command_t command ){
                         switch( command.cmd_dir ){
                            case cmd_data_help:{
                                SerialCommandShowHelp();
                            } break;

                            case cmd_data_set:{

                                switch ( command.command ){
                                    
                                    case cmd_setpoint:{
                                     if(command.parameter.u32<=UINT16_MAX){
                                            command_if_update_setpoint(command.parameter.u32);
                                            Serial.print(F("Setpoint:"));
                                            Serial.print(command_if_get_setpoint());  
                                            Serial.print(F("\n\rOK\n\r")); 
                                      }
                                    } break;

                                    case cmd_irontype:{
                                        switch(command.parameter.kw){
                                            case KW_HKO:{
                                                 command_if_set_irontype(IRON_HAKKO_FX8801);
                                                 Serial.print(F("\n\rOK\n\r"));
                                            }break;

                                            case KW_JBC:{
                                                command_if_set_irontype(IRON_JBC_T245A);
                                                 Serial.print(F("\n\rOK\n\r")); 
                                            }break;

                                            case KW_WRT:{
                                                command_if_set_irontype(IRON_WELLER_RT);
                                                 Serial.print(F("\n\rOK\n\r")); 
                                            }break;

                                            default:{
                                                Serial.println("Keyword not supported");
                                                Serial.print(F("\n\rERR\n\r")); 
                                            }
                                        }
                                    }break;

                                    default:{
                                        Serial.println("Not supported");
                                    }break;
                                }
                       
                            }break;

                            case cmd_data_get:{

                                  switch ( command.command ){
                                    
                                    case cmd_setpoint:{
                                    Serial.print(F("Setpoint:"));
                                    Serial.print(command_if_get_setpoint());  
                                    Serial.print(F("\n\rOK\n\r")); 
                                    } break;

                                    case cmd_temperatur:{
                                    Serial.print(F("Temperatur:"));
                                    Serial.print(command_if_get_temperature());  
                                    Serial.print(F("\n\rOK\n\r")); 
                                    } break;

                                    case cmd_ambienttemp:{
                                    Serial.print(F("Ambient Temperatur:"));
                                    Serial.print(command_if_get_ambienttemp());  
                                    Serial.print(F("\n\rOK\n\r"));  
                                    } break;

                                    case cmd_error:{
                                    Serial.print(F("Error:"));
                                    Serial.print(command_if_get_error());  
                                    Serial.print(F("\n\rOK\n\r")); 

                                    case cmd_irontype:{
                                    Serial.print(F("Irontype:"));
                                    switch(command_if_get_irontype() ){
                                        case IRON_WELLER_RT:{
                                            Serial.print(F("Weller RT")); 
                                        }break;

                                        case IRON_HAKKO_FX8801:{
                                            Serial.print(F("HAKKKO FX8801"));
                                        }break;

                                        case IRON_JBC_T245A:{
                                            Serial.print(F("JBC T245A"));
                                        }break;

                                        default:{
                                            Serial.print(F("UNKNOWN"));
                                        }break;
                                    }
                                    Serial.print(F("\n\rOK\n\r")); 
                                    }break;

                                    } break;

                                    default:{
                                        Serial.println("Not supported");
                                    }
                                }

                            } break;

                            case cmd_data_clear:{

                                switch ( command.command ){
                                    
                                    case cmd_error:{
                                        command_if_clear_error();
                                        Serial.println("OK");
                                    } break;

                                    default:{
                                        Serial.println("Not supported");
                                    }
                                }
                          
                            }break;

                            default:{

                            } break;
                        }

}

/********************************************************************************************************
 *          bool VerfiyHexString(char* keystr, uint8_t keylen)       
**********************************************************************************************************
 Function:    VerfiyHexString
 Input:       char* keystr, uint8_t keylen, uint8_t* data, uint8_t datalen
 Output:      bool
 Description: Checks if a valid HEX-String is given
**********************************************************************************************************/
bool VerfiyHexString(char* keystr, uint8_t keylen){
    //We need to check if every char is 0-9 and a-f 
    //and if it is lower case we must transfer it to upper
    bool error = false;
    for(uint8_t i=0 ; i<keylen; i++){
        if( ( ( keystr[i] >= '0' ) && ( keystr[i] <='9' ) ) ||
            ( ( keystr[i] >='a' ) && ( keystr[i] <= 'f' ) ) ||
            ( ( keystr[i] >='A' ) && ( keystr[i] <= 'F' ) ) ){
                if( ( keystr[i] >='a' ) && ( keystr[i] <= 'f' ) ){
                    keystr[i]=keystr[i]-32;
                }
            } else {
                //This is a not valid char in the key
                error=true;
                Serial.print(F('['));
                Serial.print(keystr[i]);
                Serial.print(F(']'));
            }
    }
    if(true == error){
        Serial.println( F(" :Key contains error"));
    } else {
        //String is valid
    }
    return error;
}


/********************************************************************************************************
 *           bool HexStrToHexArray(char* keystr, uint8_t keylen, uint8_t* data, uint8_t datalen )       
**********************************************************************************************************
 Function:    HexStrToHexArray
 Input:       char* keystr, uint8_t keylen, uint8_t* data, uint8_t datalen
 Output:      bool
 Description: Converts an HEX-String input to HEX Array
**********************************************************************************************************/
bool HexStrToHexArray(char* keystr, uint8_t keylen, uint8_t* data, uint8_t datalen ){
    if(keylen != datalen*2 ){
        Serial.println(F("HexSring lenght not a multiple by 2 of datalen"));
        return true;
    } 

    for(uint8_t i=0;i<keylen;i++){
        if(keystr[i*2]<='9'){
            data[i]= (( keystr[i*2]-48 )<<4)&0xF0;
        } else {
            data[i] = (( keystr[i*2]-55 )<<4)&0xF0;
        }
        
        if(keystr[(i*2)+1]<='9'){
            data[i]= data[i] | ( ( keystr[(i*2)+1]-48 ) & 0x0F);
        } else {
            data[i]= data[i] | ( ( keystr[(i*2)+1]-55 ) & 0x0F);
        }
    }
    return false;
}

/********************************************************************************************************
 *     bool ConvertHexStrToHexArray( char* keystr, uint8_t keylen, uint8_t* data, uint8_t datalen)      
**********************************************************************************************************
 Function:    ConvertHexStrToHexArray
 Input:       char* keystr, uint8_t keylen, uint8_t* data, uint8_t datalen
 Output:      bool
 Description: Converts a HEX String to a HEX array and return ture if no error
**********************************************************************************************************/
bool ConvertHexStrToHexArray( char* keystr, uint8_t keylen, uint8_t* data, uint8_t datalen){
    bool error = false;
    error = VerfiyHexString( keystr, keylen );

    if(false == error ){
        error = HexStrToHexArray( keystr, keylen, data, datalen );
        if(true == error){
           Serial.println(F("HexSring conversion errors")); 
        }
    } else {
        Serial.println(F("HexSring contains errors"));
    } 
    return (!error);
}