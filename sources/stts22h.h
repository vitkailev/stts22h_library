#ifndef STTS22H_H
#define STTS22H_H

#ifdef __cplusplus
extern "C" {
#endif

#include "i2c.h"

enum STTS22H_Errors {
    STTS22H_SUCCESS = 0,

    STTS22H_NOT_INIT = -I2C_NUMBER_ERRORS - 1,
    STTS22H_WRONG_DATA = -I2C_NUMBER_ERRORS - 2,
    STTS22H_BUSY = -I2C_NUMBER_ERRORS - 3,
};

enum STTS22H_AVG {
    STTS22H_AVG_25Hz = 0,
    STTS22H_AVG_50Hz,
    STTS22H_AVG_100Hz,
    STTS22H_AVG_200Hz,
};

typedef union {
    struct STTS22H_ControlRegister {
        unsigned one_shot: 1; // 1 - a new one-shot temperature acquisition is executed
        unsigned time_out_dis: 1; // 1 - the timeout function of SMBus is disabled
        unsigned freerun: 1; // enables freerun mode
        unsigned if_add_inc: 1; // 1 - the automatic address increment is enabled when multiple I2C read and write transactions are used
        unsigned avg: 2; // set the number of averages configuration. When in freerun mode, these bits also set the ODR
        unsigned bdu: 1; // 1 - BDU enabled (if BDU is used, TEMP_L_OUT must be read first)
        unsigned low_odr_start: 1; // enables 1Hz ODR operating mode
    } fields;
    uint8_t full;
} STTS22H_Control_Def;

typedef union {
    struct STTS22H_StatusRegister {
        unsigned busy: 1; // 1 - the conversion is in progress
        unsigned over_thh: 1; // 1 - high limit temperature exceeded. The bit is automatically reset to 0 upon reading the STATUS register
        unsigned under_thl: 1; // 1 - low limit temperature exceeded. The bit is automatically reset to 0 upon reading the STATUS register
        unsigned : 5;
    } fields;
    uint8_t full;
} STTS22H_Status_Def;

typedef struct {
    bool isInit;
    bool isConnected;

    bool isReading;
    bool addrSent;
    uint8_t regAddr;

    uint8_t dataSize; // bytes
    STTS22H_Control_Def settings;
    STTS22H_Status_Def status;
    float temp; // C

    uint8_t devAddr;
    I2CDef *i2c;
} STTS22H_Def;

int STTS22H_init(STTS22H_Def *stts, I2CDef *i2c, uint8_t addr);

int STTS22H_checkConnection(STTS22H_Def *stts);

bool STTS22H_isConnected(const STTS22H_Def *stts);

int STTS22H_setting(STTS22H_Def *stts, uint8_t controlReg);

int STTS22H_setLimits(STTS22H_Def *stts, float minTemp, float maxTemp, bool isSetLimits);

int STTS22H_measure(STTS22H_Def *stts);

float STTS22H_getTemp_C(const STTS22H_Def *stts);

float STTS22H_getTemp_F(const STTS22H_Def *stts);

bool STTS22H_isOverheated(const STTS22H_Def *stts);

bool STTS22H_isOvercooled(const STTS22H_Def *stts);

void STTS22H_update(STTS22H_Def *stts);

#ifdef __cplusplus
}
#endif

#endif // STTS22H_H
