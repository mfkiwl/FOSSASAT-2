#include "FossaSat2.h"

// Atom arduino-upload build configuration:
// STM32:stm32:Nucleo_64:pnum=NUCLEO_L452REP

void setup() {
  // initialize debug port
  FOSSASAT_DEBUG_PORT.begin(FOSSASAT_DEBUG_SPEED);
  while(!FOSSASAT_DEBUG_PORT);
  FOSSASAT_DEBUG_PORT.println();

  // initialize external flash
  PersistentStorage_Reset();
  PersistentStorage_Enter4ByteMode();

  // increment reset counter
  uint16_t restartCounter = 0xFFFF;
  PersistentStorage_Get(FLASH_RESTART_COUNTER_ADDR, restartCounter);
  FOSSASAT_DEBUG_PORT.print(F("Restart #"));
  FOSSASAT_DEBUG_PORT.println(restartCounter);
  PersistentStorage_Set(FLASH_RESTART_COUNTER_ADDR, ++restartCounter);

  // setup hardware interfaces
  Configuration_Setup();

#ifdef RESET_SYSTEM_INFO
  // reset system info (first sector in external flash)
  PersistentStorage_Reset_System_Info();
#endif

  // print power configuration
  // TODO add the rest of power configuration
  FOSSASAT_DEBUG_PORT.println(F("--- Power Configuration ---"));
  FOSSASAT_DEBUG_PORT.print(F("Transmissions enabled: "));
  uint8_t txEnabled = 0xFF;
  PersistentStorage_Get(FLASH_TRANSMISSIONS_ENABLED, txEnabled);
  FOSSASAT_DEBUG_PORT.println(txEnabled);
  FOSSASAT_DEBUG_PORT.println(F("---------------------------"));

  // initialize radio
  Communication_Set_Modem(currentModem);

  // initialize temperature sensors
  Sensors_Setup_Temp(tempSensorPanelY, TMP_100_RESOLUTION_12_BITS);
  Sensors_Setup_Temp(tempSensorTop, TMP_100_RESOLUTION_12_BITS);
  Sensors_Setup_Temp(tempSensorBottom, TMP_100_RESOLUTION_12_BITS);
  Sensors_Setup_Temp(tempSensorBattery, TMP_100_RESOLUTION_12_BITS);
  Sensors_Setup_Temp(tempSensorSecBattery, TMP_100_RESOLUTION_12_BITS);

  // initialize IMU
  Sensors_Setup_IMU();

  // initialize current sensors
  Sensors_Setup_Current(currSensorXA, CURR_SENSOR_X_A_BUS, CURR_SENSOR_X_A_ADDRESS);
  Sensors_Setup_Current(currSensorXB, CURR_SENSOR_X_B_BUS, CURR_SENSOR_X_B_ADDRESS);
  Sensors_Setup_Current(currSensorZA, CURR_SENSOR_Z_A_BUS, CURR_SENSOR_Z_A_ADDRESS);
  Sensors_Setup_Current(currSensorZB, CURR_SENSOR_Z_B_BUS, CURR_SENSOR_Z_B_ADDRESS);
  Sensors_Setup_Current(currSensorY, CURR_SENSOR_Y_BUS, CURR_SENSOR_Y_ADDRESS);
  Sensors_Setup_Current(currSensorMPPT, CURR_SENSOR_MPPT_OUTPUT_BUS, CURR_SENSOR_MPPT_OUTPUT_ADDRESS);

  // initialize light sensors
  Sensors_Setup_Light(lightSensorPanelY, LIGHT_SENSOR_Y_PANEL_BUS);
  Sensors_Setup_Light(lightSensorTop, LIGHT_SENSOR_TOP_PANEL_BUS);

  // check deployment
#ifdef ENABLE_DEPLOYMENT_SEQUENCE
  uint8_t attemptNumber = 0xFF;
  PersistentStorage_Get(FLASH_DEPLOYMENT_COUNTER_ADDR, attemptNumber);
  FOSSASAT_DEBUG_PORT.print(F("Deployment attempt #"));
  FOSSASAT_DEBUG_PORT.println(attemptNumber);

  // check number of deployment attempts
  if (attemptNumber == 0) {
    // print data for integration purposes (independently of FOSSASAT_DEBUG macro!)
    uint32_t start = millis();
    uint32_t lastSample = 0;
    while (millis() - start <= (uint32_t)DEPLOYMENT_DEBUG_LENGTH * (uint32_t)1000) {
      // update IMU
      Sensors_Update_IMU();

      // check if its time for next measurement
      if (millis() - lastSample >= (uint32_t)DEPLOYMENT_DEBUG_SAMPLE_PERIOD) {
        lastSample = millis();
        FOSSASAT_DEBUG_PORT.println();

        // temperature sensors
        FOSSASAT_DEBUG_PORT.println(F("Device\t\tT [deg. C]"));
        FOSSASAT_DEBUG_PORT.println(F("-------------------------------------------------------------"));
        FOSSASAT_DEBUG_PORT.print(F("Y panel \t"));
        FOSSASAT_DEBUG_PORT.println(Sensors_Read_Temperature(tempSensorPanelY));
        FOSSASAT_DEBUG_PORT.print(F("Top panel\t"));
        FOSSASAT_DEBUG_PORT.println(Sensors_Read_Temperature(tempSensorTop));
        FOSSASAT_DEBUG_PORT.print(F("Bottom panel\t"));
        FOSSASAT_DEBUG_PORT.println(Sensors_Read_Temperature(tempSensorBottom));
        FOSSASAT_DEBUG_PORT.print(F("Battery \t"));
        FOSSASAT_DEBUG_PORT.println(Sensors_Read_Temperature(tempSensorBattery));
        FOSSASAT_DEBUG_PORT.print(F("Second Battery\t"));
        FOSSASAT_DEBUG_PORT.println(Sensors_Read_Temperature(tempSensorSecBattery));
        FOSSASAT_DEBUG_PORT.println();

        // IMU
        FOSSASAT_DEBUG_PORT.println(F("Device\t\tomega [deg./s]\ta [m/s^2]\tB [gauss]"));
        FOSSASAT_DEBUG_PORT.println(F("-------------------------------------------------------------"));
        FOSSASAT_DEBUG_PORT.print(F("X axis\t\t"));
        FOSSASAT_DEBUG_PORT.print(imu.calcGyro(imu.gx));
        FOSSASAT_DEBUG_PORT.print(F("\t\t"));
        FOSSASAT_DEBUG_PORT.print(imu.calcAccel(imu.ax));
        FOSSASAT_DEBUG_PORT.print(F("\t\t"));
        FOSSASAT_DEBUG_PORT.println(imu.calcMag(imu.mx));
        FOSSASAT_DEBUG_PORT.print(F("Y axis\t\t"));
        FOSSASAT_DEBUG_PORT.print(imu.calcGyro(imu.gy));
        FOSSASAT_DEBUG_PORT.print(F("\t\t"));
        FOSSASAT_DEBUG_PORT.print(imu.calcAccel(imu.ay));
        FOSSASAT_DEBUG_PORT.print(F("\t\t"));
        FOSSASAT_DEBUG_PORT.println(imu.calcMag(imu.my));
        FOSSASAT_DEBUG_PORT.print(F("Z axis\t\t"));
        FOSSASAT_DEBUG_PORT.print(imu.calcGyro(imu.gz));
        FOSSASAT_DEBUG_PORT.print(F("\t\t"));
        FOSSASAT_DEBUG_PORT.print(imu.calcAccel(imu.az));
        FOSSASAT_DEBUG_PORT.print(F("\t\t"));
        FOSSASAT_DEBUG_PORT.println(imu.calcMag(imu.mz));
        FOSSASAT_DEBUG_PORT.println();

        // current sensors
        FOSSASAT_DEBUG_PORT.println(F("Device\t\tI [mA]\t\tV [mV]"));
        FOSSASAT_DEBUG_PORT.println(F("-------------------------------------------------------------"));
        FOSSASAT_DEBUG_PORT.print(F("X panel A\t"));
        FOSSASAT_DEBUG_PORT.print(currSensorXA.readCurrent());
        FOSSASAT_DEBUG_PORT.print(F("\t\t"));
        FOSSASAT_DEBUG_PORT.println(currSensorXA.readBusVoltage());
        FOSSASAT_DEBUG_PORT.print(F("X panel B\t"));
        FOSSASAT_DEBUG_PORT.print(currSensorXB.readCurrent());
        FOSSASAT_DEBUG_PORT.print(F("\t\t"));
        FOSSASAT_DEBUG_PORT.println(currSensorXB.readBusVoltage());

        FOSSASAT_DEBUG_PORT.print(F("Z panel A\t"));
        FOSSASAT_DEBUG_PORT.print(currSensorZA.readCurrent());
        FOSSASAT_DEBUG_PORT.print(F("\t\t"));
        FOSSASAT_DEBUG_PORT.println(currSensorZA.readBusVoltage());
        FOSSASAT_DEBUG_PORT.print(F("Z panel B\t"));
        FOSSASAT_DEBUG_PORT.print(currSensorZB.readCurrent());
        FOSSASAT_DEBUG_PORT.print(F("\t\t"));
        FOSSASAT_DEBUG_PORT.println(currSensorZB.readBusVoltage());

        FOSSASAT_DEBUG_PORT.print(F("Y panel \t"));
        FOSSASAT_DEBUG_PORT.print(currSensorY.readCurrent());
        FOSSASAT_DEBUG_PORT.print(F("\t\t"));
        FOSSASAT_DEBUG_PORT.println(currSensorY.readBusVoltage());

        FOSSASAT_DEBUG_PORT.print(F("MPPT output\t"));
        FOSSASAT_DEBUG_PORT.print(currSensorMPPT.readCurrent());
        FOSSASAT_DEBUG_PORT.print(F("\t\t"));
        FOSSASAT_DEBUG_PORT.println(currSensorMPPT.readBusVoltage());
        FOSSASAT_DEBUG_PORT.println();

        // TODO light sensors
        FOSSASAT_DEBUG_PORT.println(F("Device\t\tE [lx]"));
        FOSSASAT_DEBUG_PORT.println(F("-------------------------------------------------------------"));
        FOSSASAT_DEBUG_PORT.print(F("Y panel \t"));
        FOSSASAT_DEBUG_PORT.println(0/*lightSensorPanelY.readLux()*/);
        FOSSASAT_DEBUG_PORT.print(F("Top panel\t"));
        FOSSASAT_DEBUG_PORT.println(0/*lightSensorTop.readLux()*/);
        FOSSASAT_DEBUG_PORT.println();

        // ADCS H-bridge drivers
        FOSSASAT_DEBUG_PORT.println(F("Device\t\tFault #"));
        FOSSASAT_DEBUG_PORT.println(F("-------------------------------------------------------------"));
        FOSSASAT_DEBUG_PORT.print(F("X axis \t"));
        FOSSASAT_DEBUG_PORT.println(bridgeX.getFault());
        FOSSASAT_DEBUG_PORT.print(F("Y axis \t"));
        FOSSASAT_DEBUG_PORT.println(bridgeY.getFault());
        FOSSASAT_DEBUG_PORT.print(F("Z axis \t"));
        FOSSASAT_DEBUG_PORT.println(bridgeZ.getFault());

        FOSSASAT_DEBUG_PORT.println(F("============================================================="));
      }

      // pet watchdog
      if (millis() - lastHeartbeat >= WATCHDOG_LOOP_HEARTBEAT_PERIOD) {
        PowerControl_Watchdog_Heartbeat();
      }
    }

    // sleep before deployment
#ifdef ENABLE_DEPLOYMENT_SLEEP
    PowerControl_Wait(DEPLOYMENT_SLEEP_LENGTH, LOW_POWER_SLEEP);
#endif

    // check voltage
    uint32_t chargingStart = millis();
    while (currSensorMPPT.readBusVoltage() < DEPLOYMENT_BATTERY_LEVEL_LIMIT) {
      // voltage below 3.7V, wait until charged
      if (millis() - chargingStart >= (uint32_t)DEPLOYMENT_CHARGE_LIMIT * (uint32_t)3600 * (uint32_t)1000) {
        // reached maximum charging interval, stop further charging
        break;
      }

      // TODO sleep for variable amount of time
    }

    // voltage above 3.7V, deploy
    PowerControl_Deploy();

    // increment deployment counter
    PersistentStorage_Set(FLASH_DEPLOYMENT_COUNTER_ADDR, ++attemptNumber);
  }
#endif

  // set receive ISR
  radio.setDio1Action(Communication_Receive_Interrupt);
  radio.startReceive();

  // reset timestamps
  lastHeartbeat = millis();
}

void loop() {

}