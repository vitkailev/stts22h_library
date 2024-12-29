# STTS22H I2C library

This is a library that covers all the capabilities of the STTS22H temperature sensor via the I2C interface  
[Official STMicroelectronics data source](https://www.st.com/en/mems-and-sensors/stts22h.html)

## Sensor description

- Operating temperature -40&deg;C to +125&deg;C;
- Temperature accuracy (max): &pm;0.5&deg;C (-10&deg;C to +60&deg;C);
- 16-bit temperature data output;
- One-shot, freerun and low-ODR modes;
- ODR: 25/50/100/200 Hz;
- I2C and SMBus 3.0;
- Up to 1MHz serial clock;
- Up to 4 I2C/SMBus slave addresses;
- Supply voltage: 1.5 to 3.6 V;
- Ultralow current: 1.75 uA (one-shot mode);
- Factory calibrated;
- NIST traceability;

## Library features

- Check the connection between MCU and the sensor (read "WHOAMI" register value);
- Setting sensor (you need to turn ON the automatic address increment);
- Setting the temperature threshold level (for ALERT/INT pin);
- Reading the values of status and temperature registers;
