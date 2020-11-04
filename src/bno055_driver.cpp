/*
  Author: Joey Yang
  Email: joeyyang.ai@gmail.com
  Description: 
    Implementation file of the ROS-agnostic Linux hardware driver for the BNO055 sensor.
*/

#include "ros_bno055/bno055_driver.h"

// Set to I2C Bus 3 for software I2C driver on Raspberry Pi.
const char* I2C_BUS = "/dev/i2c-3";

// Set to I2C Bus 1 for default.
//const char* I2C_BUS = "/dev/i2c-1";

const __u8 I2C_ADDRESS = 0x28;

bno055::Bno055Driver::Bno055Driver() : pow_mode_(bno055::PowMode::NORMAL_MODE), opr_mode_(bno055::OprMode::CONFIG_MODE)
{
  printf("BNO055 IMU driver initialized.\n");
}

int bno055::Bno055Driver::initI2c()
{
  if ((file_desc_ = open(I2C_BUS, O_RDWR)) < 0)
  {
    printf("ERROR: Could not open I2C bus: %s.\n", I2C_BUS); 
    perror("ERROR: \n");
    exit(-1);
  }
  else 
  {
    printf("Opened I2C bus: %s.\n", I2C_BUS);
  }
  
  if (ioctl(file_desc_, I2C_SLAVE, I2C_ADDRESS) < 0)
  {
    printf("ERROR: Could not locate BNO055 sensor at address: 0x%02X.\n", I2C_ADDRESS);
    perror("ERROR: \n");
    exit(-1);
  }
  else 
  {
    printf("Located BNO055 sensor at address: 0x%02X.\n", I2C_ADDRESS);
  }

  return 1;
}

int bno055::Bno055Driver::setConfigMode()
{
  if (i2c_smbus_write_byte_data(file_desc_, bno055::RegisterMap::OPR_MODE, bno055::OprMode::CONFIG_MODE) < 0)
  {
    printf("ERROR: Could not set operation mode to CONFIG.\n");
    perror("ERROR: \n");
    exit(-1);
  }
  else 
  {
    printf("Set operation mode to CONFIG: 0x%02X.\n", bno055::OprMode::CONFIG_MODE);
  } 
  opr_mode_ = bno055::OprMode::CONFIG_MODE;
  usleep(500000);

  return 1;
}

int bno055::Bno055Driver::setImuMode()
{
  // Reset to config mode first.
  setConfigMode();

  if (i2c_smbus_write_byte_data(file_desc_, bno055::RegisterMap::OPR_MODE, bno055::OprMode::IMU) < 0)
  {
    printf("ERROR: Could not set operation mode to IMU.\n");
    perror("ERROR: \n");
    exit(-1);
  } 
  else 
  {
    printf("Set operation mode to IMU: 0x%02X.\n", bno055::OprMode::IMU);
  } 
  opr_mode_ = bno055::OprMode::IMU;
  usleep(500000);

  return 1;
}

int bno055::Bno055Driver::setNdofMode()
{
  // Reset to config mode first.
  setConfigMode();

  if (i2c_smbus_write_byte_data(file_desc_, bno055::RegisterMap::OPR_MODE, bno055::OprMode::NDOF) < 0)
  {
    printf("ERROR: Could not set operation mode to NDOF.\n");
    perror("ERROR: \n");
    exit(-1);
  } 
  else 
  {
    printf("Set operation mode to NDOF: 0x%02X.\n", bno055::OprMode::NDOF);
  } 
  opr_mode_ = bno055::OprMode::NDOF;
  usleep(500000);

  return 1;
}

int bno055::Bno055Driver::getAcc()
{
  bno055::AccData acc_data;

  if (i2c_smbus_read_i2c_block_data(file_desc_, RegisterMap::ACC_DATA_X_LSB, 0x06, (__u8*)&acc_data) != 0x06) 
  {
    printf("ERROR: Could not read accelerometer data.\n");
    perror("ERROR: \n");
    exit(-1);
  }
 
  bno055::Bno055Driver::data_.acc_x_ = (double)acc_data.acc_x / 100.0;
  bno055::Bno055Driver::data_.acc_y_ = (double)acc_data.acc_y / 100.0;
  bno055::Bno055Driver::data_.acc_z_ = (double)acc_data.acc_z / 100.0;

  return 1;
}

int bno055::Bno055Driver::getMag()
{ 
  bno055::MagData mag_data;

  if (i2c_smbus_read_i2c_block_data(file_desc_, RegisterMap::MAG_DATA_X_LSB, 0x06, (__u8*)&mag_data) != 0x06) 
  {
    printf("ERROR: Could not read magnetometer data.\n");
    perror("ERROR: \n");
    exit(-1);
  }
  
  bno055::Bno055Driver::data_.mag_x_ = (double)mag_data.mag_x / 16.0;
  bno055::Bno055Driver::data_.mag_y_ = (double)mag_data.mag_y / 16.0;
  bno055::Bno055Driver::data_.mag_z_ = (double)mag_data.mag_z / 16.0;

  return 1;
}

int bno055::Bno055Driver::getGyr()
{
  bno055::GyrData gyr_data;

  if (i2c_smbus_read_i2c_block_data(file_desc_, RegisterMap::GYR_DATA_X_LSB, 0x06, (__u8*)&gyr_data) != 0x06) 
  {
    printf("ERROR: Could not read gyroscope data.\n");
    perror("ERROR: \n");
    exit(-1);
  }
  
  bno055::Bno055Driver::data_.gyr_x_ = (double)gyr_data.gyr_x / 900.0;
  bno055::Bno055Driver::data_.gyr_y_ = (double)gyr_data.gyr_y / 900.0;
  bno055::Bno055Driver::data_.gyr_z_ = (double)gyr_data.gyr_z / 900.0;

  return 1;
}

int bno055::Bno055Driver::getEul()
{
  bno055::EulData eul_data;

  if (i2c_smbus_read_i2c_block_data(file_desc_, RegisterMap::EUL_HEADING_LSB, 0x06, (__u8*)&eul_data) != 0x06)
  {
    printf("ERROR: Could not read euler angles data.\n");
    perror("ERROR: \n");
    exit(-1);
  }
  
  bno055::Bno055Driver::data_.eul_heading_ = (double)eul_data.eul_heading / 16.0;
  bno055::Bno055Driver::data_.eul_roll_ = (double)eul_data.eul_roll / 16.0;
  bno055::Bno055Driver::data_.eul_pitch_ = (double)eul_data.eul_pitch / 16.0;

  return 1;
}

int bno055::Bno055Driver::getQua()
{
  bno055::QuaData qua_data;

  if (i2c_smbus_read_i2c_block_data(file_desc_, RegisterMap::QUA_DATA_W_LSB, 0x08, (__u8*)&qua_data) != 0x08)
  {
    printf("ERROR: Could not read quaternions data.\n");
    perror("ERROR: \n");
    exit(-1);
  }
  
  bno055::Bno055Driver::data_.qua_w_ = (double)qua_data.qua_w / 16384.0;
  bno055::Bno055Driver::data_.qua_x_ = (double)qua_data.qua_x / 16384.0;
  bno055::Bno055Driver::data_.qua_y_ = (double)qua_data.qua_y / 16384.0;
  bno055::Bno055Driver::data_.qua_z_ = (double)qua_data.qua_z / 16384.0;

  return 1;
}

int bno055::Bno055Driver::getLia()
{
  bno055::LiaData lia_data;

  if (i2c_smbus_read_i2c_block_data(file_desc_, RegisterMap::LIA_DATA_X_LSB, 0x06, (__u8*)&lia_data) != 0x06) 
  {
    printf("ERROR: Could not read linear acceleration data.\n");
    perror("ERROR: \n");
    exit(-1);
  }
 
  bno055::Bno055Driver::data_.lia_x_ = (double)lia_data.lia_x / 100.0;
  bno055::Bno055Driver::data_.lia_y_ = (double)lia_data.lia_y / 100.0;
  bno055::Bno055Driver::data_.lia_z_ = (double)lia_data.lia_z / 100.0;

  return 1;
}

int bno055::Bno055Driver::getGrv()
{
  bno055::GrvData grv_data;

  if (i2c_smbus_read_i2c_block_data(file_desc_, RegisterMap::GRV_DATA_X_LSB, 0x06, (__u8*)&grv_data) != 0x06) 
  {
    printf("ERROR: Could not read gravity vector data.\n");
    perror("ERROR: \n");
    exit(-1);
  }
 
  bno055::Bno055Driver::data_.grv_x_ = (double)grv_data.grv_x / 100.0;
  bno055::Bno055Driver::data_.grv_y_ = (double)grv_data.grv_y / 100.0;
  bno055::Bno055Driver::data_.grv_z_ = (double)grv_data.grv_z / 100.0;

  return 1;
}

int bno055::Bno055Driver::getTemp()
{
  bno055::TempData temp_data;

  if (i2c_smbus_read_i2c_block_data(file_desc_, RegisterMap::TEMP, 0x01, (__u8*)&temp_data) != 0x01) 
  {
    printf("ERROR: Could not read temperature data.\n");
    perror("ERROR: \n");
    exit(-1);
  }
 
  bno055::Bno055Driver::data_.temp_ = (double)temp_data.temp;

  return 1;
}

int bno055::Bno055Driver::getCalibStat()
{
  bno055::CalibStatData calib_stat_data;

  if (i2c_smbus_read_i2c_block_data(file_desc_, RegisterMap::CALIB_STAT, 0x01, (__u8*)&calib_stat_data) != 0x01) 
  {
    printf("ERROR: Could not read calibration status data.\n");
    perror("ERROR: \n");
    exit(-1);
  }
 
  // Bitwise AND to clear (reset to 0) irrelevant bits.
  bno055::Bno055Driver::data_.calib_stat_sys_ = (calib_stat_data.calib_stat & 0b11000000) >> 6;
  bno055::Bno055Driver::data_.calib_stat_gyr_ = (calib_stat_data.calib_stat & 0b00110000) >> 4;
  bno055::Bno055Driver::data_.calib_stat_acc_ = (calib_stat_data.calib_stat & 0b00001100) >> 2;
  bno055::Bno055Driver::data_.calib_stat_mag_ = (calib_stat_data.calib_stat & 0b00000011);

  printf("System Calibration Status: %d.\n", bno055::Bno055Driver::data_.calib_stat_sys_);
  printf("Accelerometer Calibration Status: %d.\n", bno055::Bno055Driver::data_.calib_stat_acc_);
  printf("Magnetometer Calibration Status: %d.\n", bno055::Bno055Driver::data_.calib_stat_mag_);
  printf("Gyroscope Calibration Status: %d.\n", bno055::Bno055Driver::data_.calib_stat_gyr_);

  return 1;
}

int bno055::Bno055Driver::getCalibOffset()
{
 // Reset to config mode first.
  setConfigMode();

  bno055::CalibOffsetData calib_offset_data;

  // Read 18 bytes into offset data structure (6 bytes for acc, 6 bytes for mag, 6 bytes for gyr).
  if (i2c_smbus_read_i2c_block_data(file_desc_, RegisterMap::ACC_OFFSET_X_LSB, 0x12, (__u8*)&calib_offset_data) != 0x12) 
  {
    printf("ERROR: Could not read calibration offset data.\n");
    perror("ERROR: \n");
    exit(-1);
  }
 
  bno055::Bno055Driver::data_.acc_offset_x_ = calib_offset_data.acc_offset_x;
  bno055::Bno055Driver::data_.acc_offset_y_ = calib_offset_data.acc_offset_y;
  bno055::Bno055Driver::data_.acc_offset_z_ = calib_offset_data.acc_offset_z;
  bno055::Bno055Driver::data_.mag_offset_x_ = calib_offset_data.mag_offset_x;
  bno055::Bno055Driver::data_.mag_offset_y_ = calib_offset_data.mag_offset_y;
  bno055::Bno055Driver::data_.mag_offset_z_ = calib_offset_data.mag_offset_z;
  bno055::Bno055Driver::data_.gyr_offset_x_ = calib_offset_data.gyr_offset_x;
  bno055::Bno055Driver::data_.gyr_offset_y_ = calib_offset_data.gyr_offset_y;
  bno055::Bno055Driver::data_.gyr_offset_z_ = calib_offset_data.gyr_offset_z;

  printf("Accelerometer Offsets: X: %d, Y: %d, Z: %d.\n", bno055::Bno055Driver::data_.acc_offset_x_, bno055::Bno055Driver::data_.acc_offset_y_, bno055::Bno055Driver::data_.acc_offset_z_);  
  printf("Magnetometer Offsets: X: %d, Y: %d, Z: %d.\n", bno055::Bno055Driver::data_.mag_offset_x_, bno055::Bno055Driver::data_.mag_offset_y_, bno055::Bno055Driver::data_.mag_offset_z_);  
  printf("Gyroscope Offsets: X: %d, Y: %d, Z: %d.\n", bno055::Bno055Driver::data_.gyr_offset_x_, bno055::Bno055Driver::data_.gyr_offset_y_, bno055::Bno055Driver::data_.gyr_offset_z_);  

  // Reset to original mode.
  if (i2c_smbus_write_byte_data(file_desc_, bno055::RegisterMap::OPR_MODE, bno055::OprMode::NDOF) < 0)
  {
    printf("ERROR: Could not set operation mode to NDOF.\n");
    perror("ERROR: \n");
    exit(-1);
  } 
  else 
  {
    printf("Set operation mode to NDOF: 0x%02X.\n", bno055::OprMode::NDOF);
  } 
  opr_mode_ = bno055::OprMode::NDOF;
  usleep(500000);

  return 1;
}

bno055::Bno055Driver::~Bno055Driver()
{
  printf("BNO055 IMU driver destroyed.\n");  
}
