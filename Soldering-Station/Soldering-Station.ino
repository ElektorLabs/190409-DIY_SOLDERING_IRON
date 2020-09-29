/*********************************************************************************************************************
                   Project Name : Soldering Station 2020
**********************************************************************************************************************
MCU used     : Atmega4809
Descriptoon  : A simple soldering station is designed with an intension to make a compact version of a temperature
               control unit for the weller, Hakko or JBC tip. This soldering device can be used to heat up the soldering bit to a 
               temperature ranging from 50 – 450 degree Celsius and also to check the temperature of the soldering bit.
version       : 1.0 ( 11.08.2020 )
***********************************************************************************************************************/

/* Librarys requiered:
 OneWire 2.3.5 by Paul Stoffregen
 Grove 4-Digit Display 1.0 by Seeed Studio

 Boardpackage requiered:
 MegaCoreX ( https://github.com/MCUdude/MegaCoreX ) 
  
*/

#include <Arduino.h>
#include "enums.h"

 #include "HW_190409.h"
 #include "serialcli.h"
 HW_190409 Station;

#define POWERSAVE_TIMEOUT ( 10 * 60 )
#define DETLA_REG ( 50 )

/* everything that is volatile and need to be accessed accross functions */
volatile uint16_t timestamp=0;
volatile uint16_t setpoint =50;
volatile uint16_t powerSave_C =0;  
volatile fsmstate_t state=WELCOME_LOGO;
volatile uint8_t HeatPwr_Percent=0;
volatile uint16_t avg_power=0;
volatile uint16_t last_avg=0;
volatile uint8_t deltacount=0;
volatile uint8_t ErrNo=0;
volatile uint16_t LastActiveTemp=0;
volatile uint8_t RotaryEncoderLocked=0;
volatile uint8_t RotaryEncoderMenuEna=0;
volatile uint16_t delay_ms = 0;
volatile uint16_t btn_press_time=0;
volatile uint8_t adjustPWM_Running=0;
volatile uint16_t Ticks=0;
volatile uint8_t clear_error=0;

int32_t current_PWM;
uint16_t display_Temp =0;
uint16_t temperature_Refresh =0;                       
bool powerSave_F = false;
bool powerStandby_F = false;

/* function prototypes */
void display_Sleep( void );
void rotary_EncoderEnable(void);
void rotary_EncoderDisable( void );
void pwm_Adjust(void);
void Timer_250us_Callback( void );
void powerSave_TimerReset( void );
void faultMonitor( uint8_t heatpwr_percent, uint16_t temperature , uint16_t target);
void set_delay(uint16_t ms);
void ProcessRotaryEncoderInput( RotaryEncoderEvent_t event );



/**********************************************************************************************************
                                void setup()        
**********************************************************************************************************
 Function:    void setup()
 Input:       None
 Output:      None 
 Description: This function is for device initialisation.
**********************************************************************************************************/
void setup()
{  
   
    setpoint = Station.read_StoreTemperature();                 //Read previous saved temperature from EEPROM
    Station.Setup(Timer_250us_Callback);                  //Hardwaresetup for the system                                  
    Serial.begin(115200);
    rotary_EncoderEnable();                             //Pin Change interrupt setting
    powerSave_TimerReset();                             //Reset timoutcounter
    Station.PWM.On(0);                                  //PWM On ( Zero Power ) 
    state=WELCOME_LOGO;                                 // The next state the FSM in the main loop will enter 

}

/**********************************************************************************************************
                                void loop()        
**********************************************************************************************************
 Function:    void loop()
 Input:       None
 Output:      NOne 
 Description: This function call repeately. Houses the FSM and the statelogic for standby and faults 
**********************************************************************************************************/
void loop() 
{
      
        SerialConsoleProcess();
        Station.Temp.OneWireStartConversation();
        Station.Temp.GetLastOneWireTempKelvin();
        /* This check can also return states and we can react to it */
       
        switch(state){ 
         /*************************************************************
                               STATE WELCOME_LOGO       
          *************************************************************/
         case WELCOME_LOGO:{
           Station.Frontend.display_welcome_logo();
               if(true == Station.Temp.SearchOneWireSensor() ){
                Serial.println("Read Onewire Sensor");
                uint16_t Temp = Station.Temp.GetWireTempKelvin();
               }else{
                Serial.println("No Sensor found");
              }
           set_delay(2000);                        /* delay for the next state */
           state = WELCOME_LOGO_WAIT; 
        } break;

        /*************************************************************
                              STATE WELCOME_LOGO_WAIT       
        *************************************************************/
        case WELCOME_LOGO_WAIT:{
          if(0==delay_ms){      
            state = WELCOME_TITLE;                  /* the next state the fsm will enter */
          } else if( digitalRead( ROTARY_BTN ) == LOW ){         /* if the user pressed the button we will enter config mode */
            state= CONIFG_MODE_WELCOME;
          }
          
        } break;

        /*************************************************************
                              STATE WELCOME_TITLE       
        *************************************************************/
        case WELCOME_TITLE:{
           Station.Frontend.display_title();
           set_delay(2000);                        /* delay for the next state */
           state = WELCOME_TITLE_WAIT; 
        } break;

        /*************************************************************
                              STATE WELCOME_TITLE_WAIT       
        *************************************************************/
        case WELCOME_TITLE_WAIT:{
          if( (0==delay_ms) || ( digitalRead( ROTARY_BTN ) == LOW ) ){ /* if the user pressed the button we skip the intro */
            state=NOFAULT;
          }
        } break;

        case CONIFG_MODE_WELCOME:{
          //We display that we entered config mode
          set_delay(2000);                        /* delay for the next state */
          state = CONIFG_MODE_WELCOME_WAIT; 
          Station.Frontend.display_show_config();
          rotary_EncoderDisable(); //We don't want the tempaeratur to be changed...
          rotary_EncoderMenuEnable();
        }break;
        
        case CONIFG_MODE_WELCOME_WAIT:{
          //We wait a few seconds
          if( (0==delay_ms)  ){ /* if the user pressed the button we skip the intro */
            state=CONIFG_MODE;
          }
        }break;

        case CONIFG_MODE:{
            //This is the config mode, here we need to tweak a bit the station interface and some menu items...
            //We will be able to configure the irontype for the Station
            // WRT, JBC or HKO
            Station.SetPWM(0); //No Power applied....
            //To switch the config we will just respond to button presses
            Station.Frontend.display_show_current_ironmode(Station.GetSolderingIron());



        } break;

        /*************************************************************
                              STATE NOFAULT       
        *************************************************************/
        case NOFAULT: {
          clear_error=0;
          Station.Frontend.display_show_Temperatur(display_Temp,HeatPwr_Percent,setpoint,state,timestamp);
          Station.write_StoreTemperature(setpoint);        //store "set temperature" in eeprom
          pwm_Adjust();
          Station.CheckLimits();   
          /* avg_pwr for powersave */
          avg_power= avg_power*15+HeatPwr_Percent;
          avg_power = avg_power/16;
          /* due to rounding errors this is not accurate but serves the purpose */
          if( avg_power != (last_avg) ) { /* We are awake */
            if(deltacount<255){
              deltacount++;
            } 
          } else {
            deltacount=0;
          }

           if( (deltacount>7) || (100 == HeatPwr_Percent ) || ( ( btn_press_time>100 ) && ( btn_press_time < 2000 ) ) ) { /* if we are used we prevent sleeping */
              powerSave_TimerReset();
           } else if( powerSave_F == true){ /* we need to go in powersave */
                powerSave_TimerReset();
                state = POWERSAVE;
           } 

          if(btn_press_time>5000){
                current_PWM=0;
                state = SLEEP;
                rotary_EncoderDisable();              /* we disable rotary encoder  */
                LastActiveTemp=setpoint;
               
          }
           
          last_avg=avg_power;
         
        } break;

       /*************************************************************
                              STATE TEMPSENS_FAIL       
        *************************************************************/   
        case TEMPSENS_FAIL:{   /* this is given by the fault monitor    */
          PWM_Off();   /* we turn off the pwm                   */
          Station.Frontend.display_show_TempError(ErrNo); 
          state = WAIT;
        } break;

        /*************************************************************
                              STATE TEMPSENS_FAIL       
        *************************************************************/   
        case UNDERVOLTAGE:{
            PWM_Off();
            Station.ShowUndervoltage();
          
        } break;        
        /*************************************************************
                              STATE WAIT       
        *************************************************************/   
        case WAIT:{
            if(timestamp&0x0001){ /* we invert the display once a second */
              Station.Frontend.display_invert(1);//display.invertDisplay(1);
            } else {
              Station.Frontend.display_invert(0);//display.invertDisplay(0);
            }
           if( (clear_error != 0) || ( digitalRead( ROTARY_BTN ) == LOW ) )  /* if the user pressed the button we try to recover */
           {
            Station.Frontend.display_invert(0); //display.invertDisplay(0);
            clear_error=0;
            state= RECOVER;
           }
           
        } break;


        /*************************************************************
                              STATE RECOVER       
        *************************************************************/   
        case RECOVER:{
          /* Recovery time my be displayed
           *  
           */
           Station.Frontend.display_show_Temperatur(display_Temp,HeatPwr_Percent,setpoint,state,timestamp);    //display temperature on oled, let the PWR string blink
           
        } break;

 
        /*************************************************************
                              STATE POWERSAVE       
        *************************************************************/           
        case POWERSAVE:{                        /* powersave state, will be entered after 15 minutes */
          rotary_EncoderDisable();              /* we disable rotary encoder  */
          Station.Frontend.display_show_Temperatur(display_Temp,HeatPwr_Percent,setpoint,state,timestamp);       
          LastActiveTemp=setpoint;                 /* we save the current setpoint */
          if(setpoint>100){                        /* we switch to 100 degree if the temp is currently higher */
            setpoint = 100;
          }
          state= POWERSAVE_WAIT;
        } break;

  
        /*************************************************************
                              STATE POWERSAVE_WAIT       
        *************************************************************/           
        case POWERSAVE_WAIT:{
          /* If we stay for more than 5 minutes here we will completly power down */
          Station.Frontend.display_show_Temperatur(display_Temp,HeatPwr_Percent,setpoint,state,timestamp);          // power save mode after 15 min
          pwm_Adjust();
          Station.CheckLimits();
          if( ( digitalRead( ROTARY_BTN ) == LOW ) && ( btn_press_time < 1000 ))
          {
            setpoint = LastActiveTemp;   /* we restore the temperatur */
            rotary_EncoderEnable();  /* we setup the encoder again */
            state= NOFAULT;           /* we switch to opperating state */
          } else if( true == powerSave_F ){   
              powerSave_TimerReset();    /* we reset the timer */
              state = SLEEP;          /* and we go to sleep */
          }
           
        } break;

       /*************************************************************
                              STATE SLEEP       
        *************************************************************/           
        case SLEEP:{
          PWM_Off();
          Station.Frontend.display_dim(true);//display.dim(true); //Dim the display, if supported 
          display_Sleep();   //Show the sleep screen
          setpoint=0;           //We set the target to 0 degree, seem safe
          HeatPwr_Percent=0;
          state = SLEEP_WAIT; 
          
        } break;

 
     /*************************************************************
                              STATE SLEEP_WAIT       
     *************************************************************/                  
        case SLEEP_WAIT:{
          display_Sleep();
          if( ( digitalRead( ROTARY_BTN ) == LOW ) && ( btn_press_time < 1000 ) ) 
           {
              state= NOFAULT;
              setpoint = LastActiveTemp;
             
              powerSave_TimerReset();    /* we reset the timer */
              rotary_EncoderEnable();
             Station.Frontend.display_dim(false);//display.dim(false);
           }
        } break;

    /*************************************************************
                              STATE default       
     *************************************************************/                      
        default:{
          
        } break;
      }
}


SolderingIronType_t command_if_get_irontype( void ){
    return Station.GetSolderingIron();
}

void command_if_set_irontype( SolderingIronType_t iron ){
  Station.SetSolderingIron(iron);
}

/*************************************************************************************************************
 *                                          command_if_update_setpoint()
 *************************************************************************************************************
 Function:    command_if_update_setpoint()
 Input:       None 
 Output:      None
 Description: Sets the setpoint from a command interface
 *************************************************************************************************************/   
void command_if_update_setpoint( uint16_t newsetpoint )
{
  /* saintiy check */
  if( (newsetpoint >= MIN_TEMP) && ( newsetpoint<= MAX_TEMP)){
  
      /* depending on the state we need to decide if we write to the setpoint or LastActiveTemp */
      switch( state ){
        case SLEEP:
        case POWERSAVE:{
            LastActiveTemp = newsetpoint;
        } break;

        default:{
            setpoint = newsetpoint;
        } break;
      }
  }

}

/*************************************************************************************************************
 *                                          command_if_get_setpoint()
 *************************************************************************************************************
 Function:    command_if_get_setpoint()
 Input:       None 
 Output:      None
 Description: returns the setpoint to a command inteface
 *************************************************************************************************************/   
uint16_t command_if_get_setpoint( void ){
     uint16_t value = 0;
     /* depending on the state we need to decide if we to read the setpoint or LastActiveTemp */
      switch( state ){
        case SLEEP:
        case POWERSAVE:{
            value = LastActiveTemp ;
        } break;

        default:{
          value = setpoint;
        } break;
      }
      return value;
}

/*************************************************************************************************************
 *                                          command_if_get_temperature()
 *************************************************************************************************************
 Function:    command_if_get_temperature()
 Input:       None 
 Output:      None
 Description: returns the current temperatur to a command interface
 *************************************************************************************************************/   
uint16_t command_if_get_temperature( void ){
  return display_Temp;

}


int16_t command_if_get_onchiptemp( void ){
  int16_t Temp = Station.Temp.GetOnchipTempKelvin();
  Temp = Temp-273;
  return Temp;
}


int16_t command_if_get_onewiretemp( void ){
  int16_t Temp = Station.Temp.GetLastOneWireTempKelvin();
  Temp = Temp-273;
  return Temp;
}

/*************************************************************************************************************
 *                                          command_if_get_error()
 *************************************************************************************************************
 Function:    command_if_get_error()
 Input:       None 
 Output:      None
 Description: returns the current error to the command interface
 *************************************************************************************************************/   
uint8_t command_if_get_error( void ){
  switch(state){
    case WAIT:
    case TEMPSENS_FAIL:{
      return 0x80 + ErrNo;
    }break;

    case UNDERVOLTAGE:{
      return 0x40;
    }break;

    default:{
      return 0;
    } break;
  }
}

/*************************************************************************************************************
 *                                          command_if_clear_error()
 *************************************************************************************************************
 Function:    command_if_clear_error()
 Input:       None 
 Output:      None
 Description: clears the error form a command interface
 *************************************************************************************************************/   
void command_if_clear_error( void ){
  clear_error++;
}

/*************************************************************************************************************
 *                                          set_delay()
 *************************************************************************************************************
 Function:    set_delay()
 Input:       None 
 Output:      None
 Description: Set the delaytimer to a given value 
 *************************************************************************************************************/    
void set_delay( uint16_t ms){
  while(delay_ms!=ms){
    delay_ms=ms;
  }
 
}

/*************************************************************************************************************
 *                                          rotary_EncoderSetting()
 *************************************************************************************************************
 Function:    rotary_EncoderSetting()
 Input:       None 
 Output:      None
 Description: Configure pin change interrupts for rotary encoder pin 
 *************************************************************************************************************/    
void rotary_EncoderEnable()
{   
    RotaryEncoderLocked=0;
}

/*************************************************************************************************************
 *                                          rotary_EncoderSetting()
 *************************************************************************************************************
 Function:    rotary_EncoderDisable()
 Input:       None 
 Output:      None
 Description: Disable pin change interrupts for rotary encoder pin 
 *************************************************************************************************************/    
void rotary_EncoderDisable()
{   
    RotaryEncoderLocked=1;
}

/*************************************************************************************************************
 *                                          rotary_EncoderSetting()
 *************************************************************************************************************
 Function:    rotary_EncoderSetting()
 Input:       None 
 Output:      None
 Description: Configure pin change interrupts for rotary encoder pin 
 *************************************************************************************************************/    
void rotary_EncoderMenuEnable()
{   
    RotaryEncoderMenuEna=1;
}

/*************************************************************************************************************
 *                                          rotary_EncoderSetting()
 *************************************************************************************************************
 Function:    rotary_EncoderDisable()
 Input:       None 
 Output:      None
 Description: Disable pin change interrupts for rotary encoder pin 
 *************************************************************************************************************/    
void rotary_EncoderMenuDisable()
{   
    RotaryEncoderMenuEna=0;
}



/**********************************************************************************************************
                                void read_Current()        
**********************************************************************************************************
 Function:    void PWM_Off()
 Input:       None
 Output:      None
 Discription: Sets the PWM to zero Output ( Off ) 
**********************************************************************************************************/
void PWM_Off()
{
  cli();
    adjustPWM_Running=1;
    current_PWM=0;
    adjustPWM_Running=0;
  sei();
}  




/**********************************************************************************************************
                                void pwm_Adjust(void)        
**********************************************************************************************************
 Function:    void pwm_Adjust(void)
 Input:       None
 Output:      uint16_t 
 Description: Pwm adjustment
 Remarks: Addition of a FSM would remove the delay(10)
**********************************************************************************************************/
void pwm_Adjust(void)
{
    static uint16_t LastRun=0;
    uint16_t call_delta=0;
    int16_t temp_Diff=0;
    uint16_t temperature=999;
    
    if(LastRun>Ticks){
      call_delta=UINT16_MAX-LastRun+Ticks;
    } else {
      call_delta=Ticks-LastRun;
    }
    //We will adjust every 50ms the PWM value ( 20Hz )
    if(call_delta< DETLA_REG ){
      _NOP();
    } else {
      LastRun=Ticks;
      cli();
      adjustPWM_Running=1;
      sei();
      Station.PWM.Off();                          //switch off heater         
      delay(10);                                  //wait for some time (to get low pass filter in steady state)
      temperature = Station.Temp.Read(ADC_AVG);
      
      if(setpoint > temperature)
      {
        temp_Diff = (int32_t)setpoint - (int32_t)temperature;
        if(temp_Diff < 3)
        {   
          current_PWM +=(int32_t)5;
        } else {
          current_PWM = (int32_t)temp_Diff  * (int32_t)setpoint/ (int32_t)6 ;
        }
        if(current_PWM > MAX_PWM_LIMIT){   
           current_PWM = MAX_PWM_LIMIT;
        }
      } 
      else if(setpoint < temperature)
      {
          temp_Diff = (int32_t)temperature - (int32_t)setpoint;
          if(temp_Diff > 2)
          {
            current_PWM = 0;
          } else {
            if(current_PWM>0){
              if( ((int32_t)current_PWM/(int32_t)7) > 0){              
                current_PWM -= (int32_t)current_PWM/(int32_t)7;
                if( ((int32_t)current_PWM%(int32_t)7) > 3){
                  if(current_PWM>0){
                    current_PWM--;
                  }
                }
              } else {
                current_PWM--;
              }
            }
          }
          if(current_PWM < 0) { 
             current_PWM = 0;
          }
      } else {
        current_PWM = current_PWM;  
      }
      //This is the putput from the Heat Regulation
      //We also check if we run into any current limit

      //Currentlimit not supported here....

      //After the current limit is applied we we will set the new value
      HeatPwr_Percent= ((current_PWM*100) / MAX_PWM_LIMIT );
      Station.SetPWM(current_PWM);
      adjustPWM_Running=0;
    }   
    
}



/*************************************************************************************************************
 *                                          powerSave_TimerReset()
 *************************************************************************************************************
 Function:    powerSave_TimerReset
 Input:       None 
 Output:      None
 Description: Reset the timer for powersave
 *************************************************************************************************************/ 
void powerSave_TimerReset(){

  while( powerSave_C != 0 ){
    powerSave_C =0;
  }
  powerSave_F=false;      /* we reset the powersave request */
}


/*************************************************************************************************************
 *                                          display_Sleep()
 *************************************************************************************************************
 Function:    display_Sleep()
 Input:       None 
 Output:      None
 Description: This will show the screensaver
 *************************************************************************************************************/    
void display_Sleep()
{
      PWM_Off();     //switch PWM off until rotary switch is pressed
      /* Update display */
      Station.Frontend.display_show_sleep(timestamp);
      /* End of update */
      Station.Temp.Read(ADC_AVG); /* we now can safe read the temperatur */
      /* We refesh the temperatur for fault detection */
} 

/**********************************************************************************************************
                                Timer1_Callback        
**********************************************************************************************************
 Function:    Timer_1ms_Callback  
 Input:       None
 Output:      None 
 Description: Handels the fault monitor, powersave and the rotary encoder, updates the display temp
**********************************************************************************************************/
void Timer_250us_Callback( void )
{
   static uint16_t onesecond_prescaler=0;
   static uint16_t _100ms_prescaler=0;
   static uint8_t input_a_buffer=0;
   static uint16_t calldelta=0;
   static uint8_t ms_prescaler=0;
   
   //This will generate 1ms Ticks for us
   if(ms_prescaler>=3){
    ms_prescaler=0;
    Ticks++;
    if(delay_ms>0){
      delay_ms--;
    }

    //This generates 1 second ticks 
    if(onesecond_prescaler>1000){
       onesecond_prescaler=0;
       timestamp++;
       display_Temp = Station.Temp.GetLastValue();
       if(powerSave_C == POWERSAVE_TIMEOUT )
       {
            powerSave_F = true;   
       }else if ( powerSave_C > POWERSAVE_TIMEOUT ) {
          _NOP();
       } else {
          powerSave_C++;
       }
       faultMonitor( HeatPwr_Percent, Station.Temp.GetLastValue(),setpoint);
   } else {
    onesecond_prescaler++;
   }

   //This will gernate 100ms ticks
   if(_100ms_prescaler>=10){
    _100ms_prescaler=0;
  
   } else {
    _100ms_prescaler++;
   }
  } else {
    ms_prescaler++;
  }

  if(  digitalRead( ROTARY_BTN ) == LOW ){
    if(btn_press_time==0){
      ProcessRotaryEncoderInput(RE_BTN_PRESS_BEGIN);  
    }
    if(btn_press_time<UINT16_MAX){
      btn_press_time++;
    }
  } else {
    //We use this to generate Button press events
    if(btn_press_time>200){
      //We generate the button pressed events
      ProcessRotaryEncoderInput(RE_BTN_PRESS_END);
    }
    btn_press_time=0;
  }

   /* We limit the buffer to 4bit / 4 sampels */ 
   input_a_buffer=input_a_buffer<<1;
   input_a_buffer=input_a_buffer&0x0F;
   
   if(LOW==digitalRead( ROTARY_TERM_B )){  
    input_a_buffer=input_a_buffer|0x00;
   } else {
    input_a_buffer=input_a_buffer|0x01;
   }

   if( (0x03==input_a_buffer) ){ /* Rising edge */
  /*  We ssume terminal B as stable
   *  if it is LOW now we moved CW else
   *  CCW
   */
    if( ROTARY_CW_LEVEL == digitalRead( ROTARY_TERM_A )){
       if(calldelta < 10) {
         /* We do nothing and ignore it, may bounced */
       } else if(calldelta<50){
        ProcessRotaryEncoderInput(RE_MOVE_LEFT_FAST);
      } else if ( calldelta < 250 ){
        ProcessRotaryEncoderInput(RE_MOVE_LEFT_MID);
      } else {
       ProcessRotaryEncoderInput(RE_MOVE_LEFT_SLOW); 
      }
      calldelta=0;
         
    
    } else {
       if(calldelta < 10) {
         /* We do nothing and ignore it, may bounced */
       } else if(calldelta<50){
         ProcessRotaryEncoderInput(RE_MOVE_RIGHT_FAST);
        
      } else if ( calldelta < 250 ){
        ProcessRotaryEncoderInput(RE_MOVE_RIGHT_MID);
      } else {
        ProcessRotaryEncoderInput(RE_MOVE_RIGHT_SLOW);
      }
      calldelta=0;
    }
    
  }
  /* This is to prevent an interger overflow */
  if(calldelta<UINT16_MAX){
    calldelta++;
  }
}
   
  


void ProcessRotaryEncoderInput( RotaryEncoderEvent_t event ){
//We got an event from the rotary encoder and may have multiple
//things that like to get informed about it....
if(0==RotaryEncoderLocked) {
    switch(event){
      case RE_MOVE_LEFT_SLOW:{
        if(setpoint> MIN_TEMP){
            setpoint--;    
        } 
      }break;

      case RE_MOVE_LEFT_MID:{
          if(setpoint>( MIN_TEMP + 2 ) ){
            setpoint-=2;    
          } else {
            setpoint=50;
          }
      } break;

    case RE_MOVE_LEFT_FAST:{
          if(setpoint>( MIN_TEMP + 5 ) ){
            setpoint-=5;    
          } else {
            setpoint=50;
          }     
      } break;

      case RE_MOVE_RIGHT_SLOW:{
        if(setpoint<MAX_TEMP){
            setpoint++;    
          } 
      } break;

      case RE_MOVE_RIGHT_MID:{
          if(setpoint<(MAX_TEMP-2)){
            setpoint+=2;    
          } else {
            setpoint=MAX_TEMP;
          }
      } break;

      case RE_MOVE_RIGHT_FAST:{
          if(setpoint<(MAX_TEMP-5)){
            setpoint+=5;    
          } else {
            setpoint=MAX_TEMP;
          }     
      } break;

      default:{

      }break;

    }
    /* sainity check */
      if(setpoint<MIN_TEMP){
            setpoint=MIN_TEMP;
      } else if( setpoint>MAX_TEMP){
            setpoint=MAX_TEMP;
  }

 } else {
   //For menu interaction we need to add a second set of functiosn to handle input...
   if(1==RotaryEncoderMenuEna){
    switch(event){
      case RE_MOVE_LEFT_SLOW:{
        //We will change the type...
        uint8_t mode = (uint8_t)Station.GetSolderingIron();
        mode--;
        if(mode<=(uint8_t)IRON_UNKNOWN){
          mode=(uint8_t)IRON_HAKKO_FX8801;
        }
        if(mode>=(uint8_t)IRON_CNT){
          mode=((uint8_t)(IRON_CNT)-1);
        }
        Station.SetSolderingIron(mode);
      }break;

      case RE_MOVE_RIGHT_SLOW:{
        uint8_t mode = (uint8_t)Station.GetSolderingIron();
        mode++;
        if(mode<=(uint8_t)IRON_UNKNOWN){
          mode=(uint8_t)IRON_HAKKO_FX8801;
        }
        if(mode>=(uint8_t)IRON_CNT){
          mode=((uint8_t)(IRON_CNT)-1);
        }
        Station.SetSolderingIron(mode);
      }break;

      case RE_BTN_PRESS_END:{
        //If this is longer pressed than 30 seconds we will leave the config menu....
        if(btn_press_time>9999){
          //We change the state to Welcome back.....
          state=WELCOME_TITLE;
          rotary_EncoderMenuDisable();
          rotary_EncoderEnable();
        }
      }break;

      default:{

      }break;
   }
  }
 }
 

}


/*************************************************************************************************************
 *                                          faultMonitor()
 *************************************************************************************************************
 Function:    faultMonitor
 Input:       
 Output:       
 Description: 
 *************************************************************************************************************/    
void faultMonitor( uint8_t heatpwr_percent, uint16_t temperature , uint16_t target){

  static uint16_t prev_temp=0;
  static uint8_t failcount=0; 
  static uint8_t recovery_delay=0;
  static uint8_t undervoltage_cnt=0;

  /* this is what we need only during the call of this function */
  int32_t delta_t_target= (int32_t)target-(int32_t)temperature;
  int32_t delta_t_temp=(int32_t)temperature-(int32_t)prev_temp;

  switch( state ) {
 
    case POWERSAVE:
    case POWERSAVE_WAIT:
    case NOFAULT:{
      /* we need the current temp, the dest temp and the heatpower */
      if( (heatpwr_percent>0) && (delta_t_target>5) ){
        /* We assume we are heating, as we havent reached the target now */
        if( (delta_t_temp<3) && ( delta_t_temp >=0 ) ){
          /* We take this as error */
          failcount++;
        } else {
          
          if(delta_t_temp> -10){
            failcount=0;
          } else {
             failcount++;
          }
        }  
      } else {
        /* seems working fine */
        failcount=0;
      }
      
      if(failcount>6){
         ErrNo=1;
         failcount=0;
         state=TEMPSENS_FAIL;
         recovery_delay=10;
       }

       if(temperature>600){
        ErrNo=3;
        state=TEMPSENS_FAIL;
        recovery_delay=10;
      }

      if(Station.CheckLimits()==UNDERVOLTAGE){
        undervoltage_cnt++;
        if(undervoltage_cnt>10){
          state=UNDERVOLTAGE;
        }
      } else {
        undervoltage_cnt=0;
      }

      
    } 
    break;
    case SLEEP:
    case SLEEP_WAIT:{
  

      if(temperature>600){
        ErrNo=3;
        state=TEMPSENS_FAIL;
        recovery_delay=10;
      }
      
    } break;

    case TEMPSENS_FAIL:{
         if(recovery_delay>0){
              recovery_delay--;
         }
    } break;

    case WAIT:{
      if(recovery_delay>0){
              recovery_delay--;
         }
    } break;


    case RECOVER:{
      if(recovery_delay>0){
        recovery_delay--;
      } else {
        failcount=0;
        state=NOFAULT;
      }
      
    }

    case UNDERVOLTAGE:{

      if(Station.CheckLimits()!=UNDERVOLTAGE){
       state=NOFAULT;
       undervoltage_cnt=0;
      }
    } break;
    default:{
      _NOP();
    } break;
  }
  
  prev_temp = temperature; 
}
