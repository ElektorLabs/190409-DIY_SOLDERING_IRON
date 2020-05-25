#ifndef ENUMS_H_
    #define ENUMS_H_
/* Enums */
#include <stdint.h>
typedef enum {
  NOFAULT =0,
  TEMPSENS_FAIL,
  UNDERVOLTAGE,
  WAIT,
  RECOVER,
  POWERSAVE,
  POWERSAVE_WAIT,
  SLEEP,
  SLEEP_WAIT,
  WELCOME_LOGO,
  WELCOME_LOGO_WAIT,
  WELCOME_TITLE,
  WELCOME_TITLE_WAIT,
  CONIFG_MODE_WELCOME,
  CONIFG_MODE_WELCOME_WAIT,
  CONIFG_MODE
} fsmstate_t;

typedef struct{
  bool overcurrent;
  uint16_t limit;
  uint16_t value;
} overcurrent_t;

typedef enum{
  VIN_UNDEFINED=0,
  VIN_12V,
  VIN_24V
} VIN_TARGETVOLTAGE_t;

typedef enum{
  IRON_UNKNOWN = 0,
  IRON_HAKKO_FX8801,
  IRON_JBC_T245A,
  IRON_WELLER_RT,
  IRON_CNT
}SolderingIronType_t;

typedef enum{
  Type_Unkonwn=0,
  Type_C,
  Type_K,
  Type_CNT
} ThermoeElementType_t;

typedef enum{
  RE_BTN_PRESS_BEGIN=0,
  RE_BTN_PRESS_END,
  RE_MOVE_LEFT_SLOW,
  RE_MOVE_LEFT_MID,
  RE_MOVE_LEFT_FAST,
  RE_MOVE_RIGHT_SLOW,
  RE_MOVE_RIGHT_MID,
  RE_MOVE_RIGHT_FAST,
  RE_EVNT_CNT
} RotaryEncoderEvent_t;

#endif
