// Convert raw IMU results to meaninful gestures

// maximum "high" value. With vigorous shaking, IMU outputs values to +/- 32767
// but in normal gentle use, max output is around +/-15000 to +/-20000 
// 16384 >> 2 = 4096, which is the max possible reading of faders when adcResolutionBits = 12

const int IMU_raw_max = 16384; 

int gesture(byte num) {

  switch (num) {
    case 0: // Lift Front

      if (flip) {
        return constrain(IMU_readings[4], 0, IMU_raw_max) >> 2;
      }
      else {
        return abs(constrain(IMU_readings[4], -IMU_raw_max, 0)) >> 2;
      }

      break;

    case 1: // Lift Back
      if (flip) {
        return abs(constrain(IMU_readings[4], -IMU_raw_max, 0)) >> 2;
      }
      else {
        return constrain(IMU_readings[4], 0, IMU_raw_max) >> 2;
      }
      break;

    case 2:// Lift Right

      if (flip) {
        return constrain(IMU_readings[3], 0, IMU_raw_max) >> 2;
      }
      else {
        return abs(constrain(IMU_readings[3], -IMU_raw_max, 0)) >> 2;
      }
      break;

    case 3: // lift Left
      if (flip) {
        return abs(constrain(IMU_readings[3], -IMU_raw_max, 0)) >> 2;
      }
      else {
        return constrain(IMU_readings[3], 0, IMU_raw_max) >> 2;
      }
      break;

    case 4: // Rotate clockwise

      return constrain(IMU_readings[2] >> 1, 0, IMU_raw_max) >> 2;

      break;

    case 5: // Rotate Anti-Clockwise
      return abs(constrain(IMU_readings[2] >> 1, -IMU_raw_max, 0)) >> 2;
      break;

    case 6: // Turn Over
      return constrain(IMU_readings[5], 0, IMU_raw_max) >> 2;
      break;

    case 7: // Right way up
      return abs(constrain(IMU_readings[5], -IMU_raw_max, 0)) >> 2;
      break;

  }
}
