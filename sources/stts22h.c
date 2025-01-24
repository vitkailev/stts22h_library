#include "stts22h.h"

static const uint8_t WHOAMI = 0xA0;

enum STTS22H_RegAddresses {
    WHOAMI_ADDR = 0x01,
    TEMP_H_LIMIT_ADDR,
    TEMP_L_LIMIT_ADDR,
    CTRL_ADDR,
    STATUS_ADDR,
    TEMP_L_OUT_ADDR,
    TEMP_H_OUT_ADDR
};

/**
 * @brief Check, that the temperature sensor is initialized
 * @param stts is the STTS22H data structure
 * @return True - sensor has been initialized, otherwise - False
 */
static bool isInit(const STTS22H_Def *stts) {
    return stts->isInit;
}

/**
 * @brief Check, that the temperature sensor is reading register values
 * @param stts is the STTS22H data structure
 * @return True - is reading, otherwise - False
 */
static bool isReading(const STTS22H_Def *stts) {
    return stts->isReading;
}

/**
 * @brief The temperature sensor initialization
 * @param stts is the STTS22H data structure
 * @param i2c is the base I2C interface data structure
 * @param addr is the device address (on I2C bus)
 * @return STTS22H_Errors values
 */
int STTS22H_init(STTS22H_Def *stts, I2CDef *i2c, uint8_t addr) {
    if (stts == NULL || i2c == NULL || addr == 0)
        return STTS22H_WRONG_DATA;

    stts->i2c = i2c;
    stts->devAddr = addr;
    stts->temp = -273.15f;
    stts->isInit = true;
    return STTS22H_SUCCESS;
}

/**
 * @brief Read the value of the "WHOAMI" register to check the connection between MCU and the temperature sensor
 * @param stts is the STTS22H data structure
 * @return STTS22H_Errors values
 */
int STTS22H_checkConnection(STTS22H_Def *stts) {
    if (!isInit(stts))
        return STTS22H_NOT_INIT;
    if (isReading(stts))
        return STTS22H_BUSY;

    stts->regAddr = WHOAMI_ADDR;
    stts->dataSize = 1;

    int result = I2C_writeData(stts->i2c, stts->devAddr, &stts->regAddr, sizeof(uint8_t), false);
    if (result == I2C_SUCCESS)
        stts->isReading = true;

    return result;
}

/**
 * @brief Check, that the temperature sensor is connected to MCU (I2C interface)
 * @param stts is the STTS22H data structure
 * @return True - there is a connection between MCU and the sensor, otherwise - False
 */
bool STTS22H_isConnected(const STTS22H_Def *stts) {
    return stts->isConnected;
}

/**
 * @brief The temperature sensor setting
 * @param stts is the STTS22H data structure
 * @param controlReg is the control register value (STTS22H_ControlReg_Def.full)
 * @return STTS22H_Errors values
 */
int STTS22H_setting(STTS22H_Def *stts, uint8_t controlReg) {
    if (!isInit(stts))
        return STTS22H_NOT_INIT;

    uint8_t data[2] = {0};
    data[0] = CTRL_ADDR;
    data[1] = controlReg;

    int result = I2C_writeData(stts->i2c, stts->devAddr, data, 2, true);
    if (result == I2C_SUCCESS)
        stts->settings.full = controlReg;

    return result;
}

/**
 * @brief Convert temperature threshold (float) to uint8_t value
 * @param value is the required value (degrees Celsius)
 * @return valid register value (TEMP_H_LIMIT or TEMP_L_LIMIT)
 */
static uint8_t calculateThreshold(float value) {
    // Datasheet, DS12606, Rev7, Aug 2022, page 14

    value /= 0.64f;
    value += 63.0f;
    return (uint8_t) value;
}

/**
 * @brief Turn ON/OFF two interrupt thresholds
 * @param stts is the STTS22H data structure
 * @param minTemp is the required low threshold value (degrees Celsius, > -39.5C)
 * @param maxTemp is the required high threshold value (degrees Celsius, < +122.5C)
 * @param isSetLimits is a flag (True - set new levels and turn ON interrupts, False - turn OFF interrupts)
 * @return STTS22H_Errors values
 */
int STTS22H_setLimits(STTS22H_Def *stts, float minTemp, float maxTemp, bool isSetLimits) {
    if (!isInit(stts))
        return STTS22H_NOT_INIT;

    // Datasheet, DS12606, Rev7, Aug 2022, page 18
    if (minTemp < -39.5f || maxTemp > 122.5f)
        return STTS22H_WRONG_DATA;

    uint8_t data[3] = {0};
    data[0] = TEMP_H_LIMIT_ADDR;
    if (isSetLimits) {
        data[1] = calculateThreshold(maxTemp);
        data[2] = calculateThreshold(minTemp);
    }

    return I2C_writeData(stts->i2c, stts->devAddr, data, 3, true);
}

/**
 * @brief Read the status and temperature registers values
 * @param stts is the STTS22H data structure
 * @return STTS22H_Errors values
 */
int STTS22H_measure(STTS22H_Def *stts) {
    if (!isInit(stts))
        return STTS22H_NOT_INIT;
    if (isReading(stts))
        return STTS22H_BUSY;

    stts->regAddr = STATUS_ADDR;
    stts->dataSize = 3;

    int result = I2C_writeData(stts->i2c, stts->devAddr, &stts->regAddr, sizeof(uint8_t), false);
    if (result == I2C_SUCCESS)
        stts->isReading = true;

    return result;
}

/**
 * @brief Get the last measured temperature value (C)
 * @param stts is the STTS22H data structure
 * @return temperature value (degrees Celsius)
 */
float STTS22H_getTemp_C(const STTS22H_Def *stts) {
    return stts->temp;
}

/**
 * @brief Get the last measured temperature value (F)
 * @param stts is the STTS22H data structure
 * @return temperature value (degrees Fahrenheit)
 */
float STTS22H_getTemp_F(const STTS22H_Def *stts) {
    float value = stts->temp;
    value = 32.0f + value * 9.0f / 5.0f;
    return value;
}

/**
 * @brief Check, that the temperature sensor has detected a value that is higher than the high limit
 * @param stts is the STTS22H data structure
 * @return True - is overheated, otherwise - False
 */
bool STTS22H_isOverheated(const STTS22H_Def *stts) {
    return stts->status.fields.over_thh;
}

/**
 * @brief Check, that the temperature sensor has detected a value that is lower than the low limit
 * @param stts is the STTS22H data structure
 * @return True - is overcooled, otherwise - False
 */
bool STTS22H_isOvercooled(const STTS22H_Def *stts) {
    return stts->status.fields.under_thl;
}

/**
 * @brief Convert sensor register values to degrees
 * @param hOut is the "TEMP_H_OUT" register value
 * @param lOut is the "TEMP_L_OUT" register value
 * @return temperature value (degrees Celsius)
 */
static float calculateTemp(uint8_t hOut, uint8_t lOut) {
    // Datasheet, DS12606, Rev7, Aug 2022, page 17

    int32_t temp = ((int32_t) hOut << 8) | lOut;
    temp = (temp >= ((int32_t) 1 << 15)) ? (temp - ((int32_t) 1 << 16)) : temp;
    return (float) temp / 100.0f;
}

/**
 * @brief Update current state of the STTS22H
 * @param stts is the STTS22H data structure
 */
void STTS22H_update(STTS22H_Def *stts) {
    if (!isInit(stts))
        return;
    if (!isReading(stts))
        return;

    if (I2C_isReading(stts->i2c) || I2C_isWriting(stts->i2c))
        return;

    if (stts->addrSent) {
        stts->addrSent = false;
        stts->isReading = false;

        if (!I2C_isFailed(stts->i2c)) {
            const uint8_t *data = (const uint8_t *) I2C_getReceivedData(stts->i2c);
            switch (stts->regAddr) {
                case WHOAMI_ADDR:
                    stts->isConnected = (WHOAMI == *data);
                    break;
                case TEMP_H_LIMIT_ADDR:
                    break;
                case CTRL_ADDR:
                    break;
                case STATUS_ADDR:
                    stts->status.full = data[0];
                    if (!stts->status.fields.busy)
                        stts->temp = calculateTemp(data[2], data[1]);
                    break;
                case TEMP_L_OUT_ADDR:
                    break;
            }
        }
    } else {
        int result = I2C_readData(stts->i2c, stts->devAddr, stts->dataSize);
        if (result == I2C_SUCCESS)
            stts->addrSent = true;
        else
            stts->isReading = false;
    }
}
