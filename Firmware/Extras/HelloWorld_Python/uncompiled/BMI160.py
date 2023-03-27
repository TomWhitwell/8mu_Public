from time import sleep
from struct import unpack
from busio import I2C
import board 


def clamp(my_value, min_value, max_value):
	return(max(min(my_value, max_value), min_value))

# command definitions
START_FOC       = const(0x03)
ACC_MODE_NORMAL = const(0x11)
GYR_MODE_NORMAL = const(0x15)
# FIFO_FLUSH      = const(0xB0)
INT_RESET       = const(0xB1)
# STEP_CNT_CLR    = const(0xB2)
SOFT_RESET      = const(0xB6)

## bit field offsets and lengths
ACC_PMU_STATUS_BIT  = const(4)
ACC_PMU_STATUS_LEN  = const(2)
GYR_PMU_STATUS_BIT  = const(2)
GYR_PMU_STATUS_LEN  = const(2)
GYRO_RANGE_SEL_BIT  = const(0)
GYRO_RANGE_SEL_LEN  = const(3)
GYRO_RATE_SEL_BIT   = const(0)
GYRO_RATE_SEL_LEN   = const(4)
GYRO_DLPF_SEL_BIT   = const(4)
GYRO_DLPF_SEL_LEN   = const(2)
ACCEL_DLPF_SEL_BIT  = const(4)
ACCEL_DLPF_SEL_LEN  = const(3)
ACCEL_RANGE_SEL_BIT = const(0)
ACCEL_RANGE_SEL_LEN = const(4)
STATUS_FOC_RDY = (3)
ACC_OFFSET_EN = const(6)
GYR_OFFSET_EN = const(7)

## Gyroscope Sensitivity Range options
# see setFullScaleGyroRange()
GYRO_RANGE_2000     = const(0)    # +/- 2000 degrees/second
GYRO_RANGE_1000     = const(1)    # +/- 1000 degrees/second
GYRO_RANGE_500      = const(2)    # +/-  500 degrees/second
GYRO_RANGE_250      = const(3)    # +/-  250 degrees/second
GYRO_RANGE_125      = const(4)    # +/-  125 degrees/second

ACCEL_RANGE_2G      = const(0X03) # +/-  2g range
ACCEL_RANGE_4G      = const(0X05) # +/-  4g range
ACCEL_RANGE_8G      = const(0X08) # +/-  8g range
ACCEL_RANGE_16G     = const(0X0C) # +/- 16g range

FOC_ACC_Z_BIT       = const(0)
FOC_ACC_Z_LEN       = const(2)
FOC_ACC_Y_BIT       = const(2)
FOC_ACC_Y_LEN       = const(2)
FOC_ACC_X_BIT       = const(4)
FOC_ACC_X_LEN       = const(2)
FOC_GYR_EN          = const(6)
CHIP_ID           = const(0x00)
PMU_STATUS        = const(0x03)
GYRO_X_L          = const(0x0C)
GYRO_X_H          = const(0x0D)
GYRO_Y_L          = const(0x0E)
GYRO_Y_H          = const(0x0F)
GYRO_Z_L          = const(0x10)
GYRO_Z_H          = const(0x11)
ACCEL_X_L         = const(0x12)
ACCEL_X_H         = const(0x13)
ACCEL_Y_L         = const(0x14)
ACCEL_Y_H         = const(0x15)
ACCEL_Z_L         = const(0x16)
ACCEL_Z_H         = const(0x17)
STATUS            = const(0x1B)
INT_STATUS_0      = const(0x1C)
INT_STATUS_1      = const(0x1D)
INT_STATUS_2      = const(0x1E)
INT_STATUS_3      = const(0x1F)
TEMP_L            = const(0x20)
TEMP_H            = const(0x21)
ACCEL_CONF        = const(0X40)
ACCEL_RANGE       = const(0X41)
GYRO_CONF         = const(0X42)
GYRO_RANGE        = const(0X43)
INT_EN_0          = const(0x50)
INT_EN_1          = const(0x51)
INT_EN_2          = const(0x52)
INT_OUT_CTRL      = const(0x53)
INT_LATCH         = const(0x54)
INT_TAP_0         = const(0x63)
INT_TAP_1         = const(0x64)
FOC_CONF          = const(0x69)
OFFSET_6          = const(0x77)
CMD               = const(0x7E)

class BMI160:

    ## Power on and prepare for general usage.
    # This will activate the device and take it out of sleep mode (which must be done
    # after start-up). This function also sets both the accelerometer and the gyroscope
    # to default range settings, namely +/- 2g and +/- 250 degrees/sec.
    def __init__(self):
        # Issue a soft-reset to bring the device into a clean state
        self._reg_write(CMD, SOFT_RESET)
        sleep(0.001)

        # Issue a dummy-read to force the device into I2C comms mode
        self._reg_read(0x7F)
        sleep(0.001)

        # Power up the accelerometer
        self._reg_write(CMD, ACC_MODE_NORMAL)
        # Wait for power-up to complete
        while (1 != self._reg_read_bits(PMU_STATUS, ACC_PMU_STATUS_BIT, ACC_PMU_STATUS_LEN)):
            pass
        sleep(0.001)

        # Power up the gyroscope
        self._reg_write(CMD, GYR_MODE_NORMAL)
        sleep(0.001)
        # Wait for power-up to complete
        while (1 != self._reg_read_bits(PMU_STATUS, GYR_PMU_STATUS_BIT, GYR_PMU_STATUS_LEN)):
            pass
        sleep(0.001)

        self.setFullScaleGyroRange(GYRO_RANGE_250, 250.0)
        self.setFullScaleAccelRange(ACCEL_RANGE_2G, 2.0)

    def _reg_read_bits(self, reg, pos, len):
        b = self._reg_read(reg)
        mask = (1 << len) - 1
        b >>= pos
        b &= mask
        return b

    def _reg_write_bits(self, reg, data, pos, len):
        b = self._reg_read(reg)
        mask = ((1 << len) - 1) << pos
        data <<= pos  # shift data into correct position
        data &= mask  # zero all non-important bits in data
        b &= ~(mask)  # zero all important bits in existing byte
        b |= data     # combine data with existing byte
        self._reg_write(reg, b)

    def _is_bit_set(self, value, bit):
        return value & (1 << bit)

    def setFullScaleGyroRange(self, range, real):
        self._reg_write_bits(GYRO_RANGE, range, GYRO_RANGE_SEL_BIT, GYRO_RANGE_SEL_LEN)
        gyro_range = real

    def setFullScaleAccelRange(self, range, real):
        self._reg_write_bits(ACCEL_RANGE, range, ACCEL_RANGE_SEL_BIT, ACCEL_RANGE_SEL_LEN)
        accel_range = real

    def setAccelOffsetEnabled(self, enabled):
        self._reg_write_bits(OFFSET_6, 1 if enabled else 0, ACC_OFFSET_EN, 1)

    def autoCalibrateAll(self):
        self._reg_write(FOC_CONF, 0x3 << FOC_ACC_X_BIT)
        self._reg_write(FOC_CONF, 0x3 << FOC_ACC_Y_BIT)
        self._reg_write(FOC_CONF, 0x2 << FOC_ACC_Z_BIT)
        self._reg_write(CMD, START_FOC)        
        while (not (self._reg_read_bits(STATUS, STATUS_FOC_RDY, 1))):
            sleep(0.001)
        foc_conf = (1 << FOC_GYR_EN)
        self._reg_write(FOC_CONF, foc_conf)
        self._reg_write(CMD, START_FOC)
        while (not (self._reg_read_bits(STATUS, STATUS_FOC_RDY, 1))):
            sleep(0.001)
            


    def setGyroOffsetEnabled(self, enabled):
        self._reg_write_bits(OFFSET_6, 0x1 if enabled else 0, GYR_OFFSET_EN, 1)

    def autoCalibrateGyroOffset(self):
        foc_conf = (1 << FOC_GYR_EN)
        self._reg_write(FOC_CONF, foc_conf)
        self._reg_write(CMD, START_FOC)
        while (not (self._reg_read_bits(STATUS, STATUS_FOC_RDY, 1))):
            sleep(0.001)
  
    def getMotion6(self):
        raw = self._regs_read(GYRO_X_L, 12)
        vals = unpack('<6h', bytes(raw))
        return vals
    
    def getGestures8(self):
        results = self.getMotion6()  

        liftFront = abs(clamp(results[4], -16256, 0))
        liftBack = clamp(results[4], 0, 16256)
        liftLeft = abs(clamp(results[3], -16256, 0))
        liftRight = clamp(results[3], 0, 16256)
        rotateCw = clamp(results[2]>>1, 0, 16256)
        rotateAcw = abs(clamp(results[2]>>1, -16256, 0))
        turnOver = clamp(results[5], 0, 16256)
        TBC = 0 
        gesturesNew = [liftFront, liftBack, liftLeft, liftRight, rotateCw, rotateAcw, turnOver, TBC] 
        return gesturesNew 

    def getTemperature(self):
        raw = self._regs_read(TEMP_L, 2)
        val = unpack('<h', bytes(raw))[0]
        deg_c = (val*0.001953185)+23
        return deg_c

    def getRegister(self, reg):
        return self._reg_read(reg)


    def setRegister(self, reg, data):
        self._reg_write(reg, data)
            

class BMI160_I2C(BMI160):
    def __init__(self, i2c, addr=0x68):
        self.i2c = i2c
        self.addr = addr
        super().__init__()

    def _reg_write(self, reg, data):
        self.i2c.writeto(self.addr, bytes([reg, data]))

    def _reg_read(self, reg):
        return self._regs_read(reg, 1)[0]

    def _regs_read(self, reg, n):
        self.i2c.writeto(self.addr, bytes([reg]))
        sleep(0.00002)
        result = bytearray(n)
        self.i2c.readfrom_into(self.addr, result)
        return result
        
def startIMU(calibrate): 
    #set up IMU 
    i2c = I2C(sda=board.SDA, scl=board.SCL)
    while not i2c.try_lock():
        pass
    bmi160 = BMI160_I2C(i2c)
    if calibrate:
        bmi160.autoCalibrateAll()
        bmi160.setAccelOffsetEnabled(1)
        bmi160.setGyroOffsetEnabled(1)

    return bmi160
