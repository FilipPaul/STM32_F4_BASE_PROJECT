#include "VL53L1X_RAFIONTECH.h"
#include "cmsis_os.h"
#include "stdio.h"

uint8_t _I2CBuffer[256];

#define SOFT_RESET											0x0000
#define VL53L1_I2C_SLAVE__DEVICE_ADDRESS					0x0001
#define VL53L1_VHV_CONFIG__TIMEOUT_MACROP_LOOP_BOUND        0x0008
#define ALGO__CROSSTALK_COMPENSATION_PLANE_OFFSET_KCPS 		0x0016
#define ALGO__CROSSTALK_COMPENSATION_X_PLANE_GRADIENT_KCPS 	0x0018
#define ALGO__CROSSTALK_COMPENSATION_Y_PLANE_GRADIENT_KCPS 	0x001A
#define ALGO__PART_TO_PART_RANGE_OFFSET_MM					0x001E
#define MM_CONFIG__INNER_OFFSET_MM							0x0020
#define MM_CONFIG__OUTER_OFFSET_MM 							0x0022
#define GPIO_HV_MUX__CTRL									0x0030
#define GPIO__TIO_HV_STATUS       							0x0031
#define SYSTEM__INTERRUPT_CONFIG_GPIO 						0x0046
#define PHASECAL_CONFIG__TIMEOUT_MACROP     				0x004B
#define RANGE_CONFIG__TIMEOUT_MACROP_A_HI   				0x005E
#define RANGE_CONFIG__VCSEL_PERIOD_A        				0x0060
#define RANGE_CONFIG__VCSEL_PERIOD_B						0x0063
#define RANGE_CONFIG__TIMEOUT_MACROP_B_HI  					0x0061
#define RANGE_CONFIG__TIMEOUT_MACROP_B_LO  					0x0062
#define RANGE_CONFIG__SIGMA_THRESH 							0x0064
#define RANGE_CONFIG__MIN_COUNT_RATE_RTN_LIMIT_MCPS			0x0066
#define RANGE_CONFIG__VALID_PHASE_HIGH      				0x0069
#define VL53L1_SYSTEM__INTERMEASUREMENT_PERIOD				0x006C
#define SYSTEM__THRESH_HIGH 								0x0072
#define SYSTEM__THRESH_LOW 									0x0074
#define SD_CONFIG__WOI_SD0                  				0x0078
#define SD_CONFIG__INITIAL_PHASE_SD0        				0x007A
#define ROI_CONFIG__USER_ROI_CENTRE_SPAD					0x007F
#define ROI_CONFIG__USER_ROI_REQUESTED_GLOBAL_XY_SIZE		0x0080
#define SYSTEM__SEQUENCE_CONFIG								0x0081
#define VL53L1_SYSTEM__GROUPED_PARAMETER_HOLD 				0x0082
#define SYSTEM__INTERRUPT_CLEAR       						0x0086
#define SYSTEM__MODE_START                 					0x0087
#define VL53L1_RESULT__RANGE_STATUS							0x0089
#define VL53L1_RESULT__DSS_ACTUAL_EFFECTIVE_SPADS_SD0		0x008C
#define RESULT__AMBIENT_COUNT_RATE_MCPS_SD					0x0090
#define VL53L1_RESULT__FINAL_CROSSTALK_CORRECTED_RANGE_MM_SD0				0x0096
#define VL53L1_RESULT__PEAK_SIGNAL_COUNT_RATE_CROSSTALK_CORRECTED_MCPS_SD0 	0x0098
#define VL53L1_RESULT__OSC_CALIBRATE_VAL					0x00DE
#define VL53L1_FIRMWARE__SYSTEM_STATUS                      0x00E5
#define VL53L1_IDENTIFICATION__MODEL_ID                     0x010F
#define VL53L1_ROI_CONFIG__MODE_ROI_CENTRE_SPAD				0x013E


const uint8_t VL51L1X_DEFAULT_CONFIGURATION[] = {
0x00, /* 0x2d : set bit 2 and 5 to 1 for fast plus mode (1MHz I2C), else don't touch */
0x00, /* 0x2e : bit 0 if I2C pulled up at 1.8V, else set bit 0 to 1 (pull up at AVDD) */
0x00, /* 0x2f : bit 0 if GPIO pulled up at 1.8V, else set bit 0 to 1 (pull up at AVDD) */
0x01, /* 0x30 : set bit 4 to 0 for active high interrupt and 1 for active low (bits 3:0 must be 0x1), use SetInterruptPolarity() */
0x02, /* 0x31 : bit 1 = interrupt depending on the polarity, use CheckForDataReady() */
0x00, /* 0x32 : not user-modifiable */
0x02, /* 0x33 : not user-modifiable */
0x08, /* 0x34 : not user-modifiable */
0x00, /* 0x35 : not user-modifiable */
0x08, /* 0x36 : not user-modifiable */
0x10, /* 0x37 : not user-modifiable */
0x01, /* 0x38 : not user-modifiable */
0x01, /* 0x39 : not user-modifiable */
0x00, /* 0x3a : not user-modifiable */
0x00, /* 0x3b : not user-modifiable */
0x00, /* 0x3c : not user-modifiable */
0x00, /* 0x3d : not user-modifiable */
0xff, /* 0x3e : not user-modifiable */
0x00, /* 0x3f : not user-modifiable */
0x0F, /* 0x40 : not user-modifiable */
0x00, /* 0x41 : not user-modifiable */
0x00, /* 0x42 : not user-modifiable */
0x00, /* 0x43 : not user-modifiable */
0x00, /* 0x44 : not user-modifiable */
0x00, /* 0x45 : not user-modifiable */
0x20, /* 0x46 : interrupt configuration 0->level low detection, 1-> level high, 2-> Out of window, 3->In window, 0x20-> New sample ready , TBC */
0x0b, /* 0x47 : not user-modifiable */
0x00, /* 0x48 : not user-modifiable */
0x00, /* 0x49 : not user-modifiable */
0x02, /* 0x4a : not user-modifiable */
0x0a, /* 0x4b : not user-modifiable */
0x21, /* 0x4c : not user-modifiable */
0x00, /* 0x4d : not user-modifiable */
0x00, /* 0x4e : not user-modifiable */
0x05, /* 0x4f : not user-modifiable */
0x00, /* 0x50 : not user-modifiable */
0x00, /* 0x51 : not user-modifiable */
0x00, /* 0x52 : not user-modifiable */
0x00, /* 0x53 : not user-modifiable */
0xc8, /* 0x54 : not user-modifiable */
0x00, /* 0x55 : not user-modifiable */
0x00, /* 0x56 : not user-modifiable */
0x38, /* 0x57 : not user-modifiable */
0xff, /* 0x58 : not user-modifiable */
0x01, /* 0x59 : not user-modifiable */
0x00, /* 0x5a : not user-modifiable */
0x08, /* 0x5b : not user-modifiable */
0x00, /* 0x5c : not user-modifiable */
0x00, /* 0x5d : not user-modifiable */
0x01, /* 0x5e : not user-modifiable */
0xcc, /* 0x5f : not user-modifiable */
0x0f, /* 0x60 : not user-modifiable */
0x01, /* 0x61 : not user-modifiable */
0xf1, /* 0x62 : not user-modifiable */
0x0d, /* 0x63 : not user-modifiable */
0x01, /* 0x64 : Sigma threshold MSB (mm in 14.2 format for MSB+LSB), use SetSigmaThreshold(), default value 90 mm  */
0x68, /* 0x65 : Sigma threshold LSB */
0x00, /* 0x66 : Min count Rate MSB (MCPS in 9.7 format for MSB+LSB), use SetSignalThreshold() */
0x80, /* 0x67 : Min count Rate LSB */
0x08, /* 0x68 : not user-modifiable */
0xb8, /* 0x69 : not user-modifiable */
0x00, /* 0x6a : not user-modifiable */
0x00, /* 0x6b : not user-modifiable */
0x00, /* 0x6c : Intermeasurement period MSB, 32 bits register, use SetIntermeasurementInMs() */
0x00, /* 0x6d : Intermeasurement period */
0x0f, /* 0x6e : Intermeasurement period */
0x89, /* 0x6f : Intermeasurement period LSB */
0x00, /* 0x70 : not user-modifiable */
0x00, /* 0x71 : not user-modifiable */
0x00, /* 0x72 : distance threshold high MSB (in mm, MSB+LSB), use SetD:tanceThreshold() */
0x00, /* 0x73 : distance threshold high LSB */
0x00, /* 0x74 : distance threshold low MSB ( in mm, MSB+LSB), use SetD:tanceThreshold() */
0x00, /* 0x75 : distance threshold low LSB */
0x00, /* 0x76 : not user-modifiable */
0x01, /* 0x77 : not user-modifiable */
0x0f, /* 0x78 : not user-modifiable */
0x0d, /* 0x79 : not user-modifiable */
0x0e, /* 0x7a : not user-modifiable */
0x0e, /* 0x7b : not user-modifiable */
0x00, /* 0x7c : not user-modifiable */
0x00, /* 0x7d : not user-modifiable */
0x02, /* 0x7e : not user-modifiable */
0xc7, /* 0x7f : ROI center, use SetROI() */
0xff, /* 0x80 : XY ROI (X=Width, Y=Height), use SetROI() */
0x9B, /* 0x81 : not user-modifiable */
0x00, /* 0x82 : not user-modifiable */
0x00, /* 0x83 : not user-modifiable */
0x00, /* 0x84 : not user-modifiable */
0x01, /* 0x85 : not user-modifiable */
0x00, /* 0x86 : clear interrupt, use ClearInterrupt() */
0x00  /* 0x87 : start ranging, use StartRanging() or StopRanging(), If you want an automatic start after VL53L1X_init() call, put 0x40 in location 0x87 */
};

//static const uint8_t status_rtn[24] = { 255, 255, 255, 5, 2, 4, 1, 7, 3, 0,
//	255, 255, 9, 13, 255, 255, 255, 255, 10, 6,
//	255, 255, 11, 12
//};



int _I2CWrite(I2C_HandleTypeDef* hi2c, uint16_t address, uint8_t *pdata, uint32_t count, uint32_t timeout) {
    return (int)HAL_I2C_Master_Transmit(hi2c, (uint16_t)(address << 1), pdata, count, timeout);
}

int _I2CRead(I2C_HandleTypeDef* hi2c, uint16_t address, uint8_t *pdata, uint32_t count, uint32_t timeout) {
    return (int)HAL_I2C_Master_Receive(hi2c, (uint16_t)(address << 1), pdata, count, timeout);
}

int8_t VL53L1_WriteMulti(I2C_HandleTypeDef* hi2c, uint16_t addr, uint16_t index, uint8_t *pdata, uint32_t count, uint32_t timeout) {
    _I2CBuffer[0] = index>>8;
    _I2CBuffer[1] = index&0xFF;
    memcpy(&_I2CBuffer[2], pdata, count);
    return _I2CWrite(hi2c, addr, _I2CBuffer, count + 2, timeout);
}

int8_t VL53L1_ReadMulti(I2C_HandleTypeDef* hi2c, uint16_t addr, uint16_t index, uint8_t *pdata, uint32_t count, uint32_t timeout){
    int32_t status_int;
    _I2CBuffer[0] = index>>8;
    _I2CBuffer[1] = index&0xFF;
    status_int = _I2CWrite(hi2c, addr, _I2CBuffer, 2, timeout);
    if (status_int != 0) {
        return status_int;
    }
    return _I2CRead(hi2c, addr, pdata, count, timeout);

}

int8_t VL53L1_WrByte(I2C_HandleTypeDef* hi2c, uint16_t addr, uint16_t index, uint8_t data, uint32_t timeout) {
    _I2CBuffer[0] = index>>8;
    _I2CBuffer[1] = index&0xFF;
    _I2CBuffer[2] = data;
    return _I2CWrite(hi2c, addr, _I2CBuffer, 3, timeout);
}

int8_t VL53L1_WrWord(I2C_HandleTypeDef* hi2c, uint16_t addr, uint16_t index, uint16_t data, uint32_t timeout) {
    _I2CBuffer[0] = index>>8;
    _I2CBuffer[1] = index&0xFF;
    _I2CBuffer[2] = data >> 8;
    _I2CBuffer[3] = data & 0x00FF;
    return _I2CWrite(hi2c, addr, _I2CBuffer, 4, timeout);
}

int8_t VL53L1_WrDWord(I2C_HandleTypeDef* hi2c, uint16_t addr, uint16_t index, uint32_t data, uint32_t timeout) {
    _I2CBuffer[0] = index>>8;
    _I2CBuffer[1] = index&0xFF;
    _I2CBuffer[2] = (data >> 24) & 0xFF;
    _I2CBuffer[3] = (data >> 16) & 0xFF;
    _I2CBuffer[4] = (data >> 8)  & 0xFF;
    _I2CBuffer[5] = (data >> 0 ) & 0xFF;

    return _I2CWrite(hi2c, addr, _I2CBuffer, 6, timeout);
}

int8_t VL53L1_RdByte(I2C_HandleTypeDef* hi2c, uint16_t addr, uint16_t index, uint8_t *data, uint32_t timeout) {
	_I2CBuffer[0] = index>>8;
	_I2CBuffer[1] = index&0xFF;
	int32_t status_int = _I2CWrite(hi2c, addr, _I2CBuffer, 2, timeout);
	if( status_int ){
		return status_int;
	}
	return _I2CRead(hi2c, addr, data, 1, timeout);
}

int8_t VL53L1_RdWord(I2C_HandleTypeDef* hi2c, uint16_t addr, uint16_t index, uint16_t *data, uint32_t timeout) {
    int32_t status_int;

    _I2CBuffer[0] = index>>8;
	_I2CBuffer[1] = index&0xFF;
    status_int = _I2CWrite(hi2c, addr, _I2CBuffer, 2, timeout);

    if( status_int ){
        return status_int;
    }
    status_int = _I2CRead(hi2c, addr, _I2CBuffer, 2, timeout);
    if (status_int != 0) {
        return status_int;
    }

    *data = ((uint16_t)_I2CBuffer[0]<<8) + (uint16_t)_I2CBuffer[1];
    return 0;
}

int8_t VL53L1_RdDWord(I2C_HandleTypeDef* hi2c, uint16_t addr, uint16_t index, uint32_t *data, uint32_t timeout) {
    int32_t status_int;

    _I2CBuffer[0] = index>>8;
	_I2CBuffer[1] = index&0xFF;

	status_int = _I2CWrite(hi2c, addr, _I2CBuffer, 2, timeout);
    if (status_int != 0) {
        return status_int;
    }
    status_int = _I2CRead(hi2c, addr, _I2CBuffer, 4, timeout);
    if (status_int != 0) {
        return status_int;
    }

    *data = ((uint32_t)_I2CBuffer[0]<<24) + ((uint32_t)_I2CBuffer[1]<<16) + ((uint32_t)_I2CBuffer[2]<<8) + (uint32_t)_I2CBuffer[3];
    return 0;
}

int32_t TOF_reset_all_flags(VL53L1X* sensor_struct)
{
	if (sensor_struct == NULL)
	{
		return -99; //invalid sensor pointer
	}
	sensor_struct->out_of_liminit_detected_single_flag = false;
	sensor_struct->inside_limit_detected_single_flag = false;
	sensor_struct->is_currently_in_limits_flag = false;
	return 0;
}

int32_t TOF_reset_persistent_inside_limit_flag(VL53L1X* sensor_struct)
{
	if (sensor_struct == NULL)
	{
		return -99; //invalid sensor pointer
	}
	sensor_struct->inside_limit_detected_single_flag = false;
	return 0;
}

int32_t TOF_reset_persistent_out_of_limit_flag(VL53L1X* sensor_struct)
{
	if (sensor_struct == NULL)
	{
		return -99; //invalid sensor pointer
	}
	sensor_struct->out_of_liminit_detected_single_flag = false;
	return 0;
}

int32_t TOF_reset_all_persistent_flags(VL53L1X* sensor_struct)
{
	if (sensor_struct == NULL)
	{
		return -99; //invalid sensor pointer
	}
	sensor_struct->inside_limit_detected_single_flag = false;
	sensor_struct->out_of_liminit_detected_single_flag = false;
	return 0;
}

int32_t TOF_start_automated_measurement(VL53L1X* sensor_struct)
{
	if (sensor_struct == NULL)
	{
		return -99; //invalid sensor pointer
	}
	sensor_struct->automated_measurement_is_running_flag = true;
	return 0;
}

int32_t TOF_stop_automated_measurement(VL53L1X* sensor_struct)
{
	if (sensor_struct == NULL)
	{
		return -99; //invalid sensor pointer
	}
	sensor_struct->automated_measurement_is_running_flag = false;
	return 0;
}

int32_t TOF_set_distance_limits(VL53L1X* sensor_struct, int32_t min_distance_mm, int32_t max_distance_mm)
{
	if (sensor_struct == NULL)
	{
		return -99; //invalid sensor pointer
	}

	sensor_struct->min_distance_liminit_mm = min_distance_mm;
	sensor_struct->max_distance_liminit_mm = max_distance_mm;
	return 0;
}

int32_t TOF_is_automated_measurement_running(VL53L1X* sensor_struct)
{
	if (sensor_struct == NULL)
	{
		return -99; //invalid sensor pointer
	}
	return sensor_struct->automated_measurement_is_running_flag;
}

int32_t TOF_is_out_of_limit_detected(VL53L1X* sensor_struct)
{
	if (sensor_struct == NULL)
	{
		return -99; //invalid sensor pointer
	}
	return sensor_struct->out_of_liminit_detected_single_flag;
}

int32_t TOF_is_inside_limit_detected(VL53L1X* sensor_struct)
{
	if (sensor_struct == NULL)
	{
		return -99; //invalid sensor pointer
	}
	return sensor_struct->inside_limit_detected_single_flag;
}

int32_t TOF_is_currently_in_limits(VL53L1X* sensor_struct)
{
	if (sensor_struct == NULL)
	{
		return -99; //invalid sensor pointer
	}

	if (!sensor_struct->automated_measurement_is_running_flag)
	{
		return -1; //automated measurement not running
	}

	if (sensor_struct->detected_error_during_automated_measurement)
	{
		return -2; //error during automated measurement
	}
	return sensor_struct->is_currently_in_limits_flag;
}

int32_t TOF_evaluate_limits(VL53L1X* sensor_struct, uint16_t distance_mm)
{
	if (sensor_struct == NULL)
	{
		return -99; //invalid sensor pointer
	}

	if (!sensor_struct->automated_measurement_is_running_flag)
	{
		return -1; //automated measurement not running
	}

	if (distance_mm < sensor_struct->min_distance_liminit_mm)
	{
		if(sensor_struct->out_of_liminit_detected_single_flag == false)
		{
			printf("Distance %u mm is below minimum limit of %ld mm\n", (unsigned int)distance_mm, (long)sensor_struct->min_distance_liminit_mm);
		}
		sensor_struct->out_of_liminit_detected_single_flag = true;
		sensor_struct->is_currently_in_limits_flag = false;
	}
	else if (distance_mm > sensor_struct->max_distance_liminit_mm)
	{
		if(sensor_struct->out_of_liminit_detected_single_flag == false)
		{
			printf("Distance %u mm is above maximum limit of %ld mm\n", (unsigned int)distance_mm, (long)sensor_struct->max_distance_liminit_mm);
		}
		sensor_struct->out_of_liminit_detected_single_flag = true;
		sensor_struct->is_currently_in_limits_flag = false;
	}
	else
	{
		sensor_struct->inside_limit_detected_single_flag = true;
		sensor_struct->is_currently_in_limits_flag = true;
	}
	return 0;
}

int32_t TOF_calibrate_to_distance(VL53L1X* sensor, uint32_t calib_time_ms, uint32_t distance_offset_percentage)
{

	if (sensor == NULL)
	{
		return -99; //invalid sensor pointer
	}

	//do measurements until calib_time_ms is reached:
	uint32_t start_time = HAL_GetTick();
	uint32_t sum = 0;
	uint32_t count = 0;

	uint32_t maximal_detected_distance_mm = 0;
	uint32_t minimal_detected_distance_mm = 0xFFFFFFFF;

	while ((HAL_GetTick() - start_time) < calib_time_ms)
	{
		osDelay(50); //delay a bit to avoid spamming the sensor with requests, also gives it time to settle between measurements
		uint16_t distance_mm;
		int32_t status = TOF_GetDistance(sensor, calib_time_ms, &distance_mm); //get distance with timeout of calib_time_ms to avoid getting stuck if sensor is not responding
		if (status != 0) //invalid reading
		{
			return status;
		}
		sum += distance_mm;
		count++;

		if (distance_mm == 0xffff) //invalid reading, skip it
		{
			return -1;

		}

		if (distance_mm > maximal_detected_distance_mm)
		{
			maximal_detected_distance_mm = distance_mm;
		}

		if (distance_mm < minimal_detected_distance_mm)
		{
			minimal_detected_distance_mm = distance_mm;
		}
	}

	//now set limits based on detected min/max distance and offset percentage
	uint32_t lower_limit_offset_mm = minimal_detected_distance_mm - (minimal_detected_distance_mm * distance_offset_percentage / 100);
	uint32_t upper_limit_offset_mm = maximal_detected_distance_mm + (maximal_detected_distance_mm * distance_offset_percentage / 100);

	TOF_set_distance_limits(sensor, lower_limit_offset_mm, upper_limit_offset_mm);
	return 0;
}

int32_t _VL53L1X_GetTimingBudgetInMs(VL53L1X* sensor, uint16_t* pTimingBudgetInMs)
{
	uint16_t Temp;
	int32_t status = 0;

	status = VL53L1_RdWord(sensor->hi2c, sensor->address, RANGE_CONFIG__TIMEOUT_MACROP_A_HI, &Temp, sensor->i2c_timeout_ms);
	if (status != 0){
		return status;
	}
	
	switch (Temp) {
		case 0x0001:
			*pTimingBudgetInMs = 10;
			break;
		case 0x001D :
			*pTimingBudgetInMs = 15;
			break;
		case 0x0051 :
		case 0x001E :
			*pTimingBudgetInMs = 20;
			break;
		case 0x00D6 :
		case 0x0060 :
			*pTimingBudgetInMs = 33;
			break;
		case 0x1AE :
		case 0x00AD :
			*pTimingBudgetInMs = 50;
			break;
		case 0x02E1 :
		case 0x01CC :
			*pTimingBudgetInMs = 100;
			break;
		case 0x03E1 :
		case 0x02D9 :
			*pTimingBudgetInMs = 200;
			break;
		case 0x0591 :
		case 0x048F :
			*pTimingBudgetInMs = 500;
			break;
		default:
			status = 1;
			*pTimingBudgetInMs = 0;
	}
	return status;
}

int32_t _VL53L1X_GetDistanceMode(VL53L1X* sensor, uint16_t *DM)
{
	if (sensor == NULL)
	{
		return -99; //invalid sensor pointer
	}
	uint8_t TempDM;
	int32_t status=0;

	status = VL53L1_RdByte(sensor->hi2c, sensor->address, PHASECAL_CONFIG__TIMEOUT_MACROP, &TempDM, sensor->i2c_timeout_ms);
	if (status != 0){
		return status;
	}

	if (TempDM == 0x14)
		*DM=1;
	if(TempDM == 0x0A)
		*DM=2;
	return status;
}

int32_t TOF_SetTimingBudget(VL53L1X* sensor, uint16_t TimingBudgetInMs)
{
	if (sensor == NULL)
	{
		return -99; //invalid sensor pointer
	}
	uint16_t distance_mode;
	int32_t status=0;

	status |= _VL53L1X_GetDistanceMode(sensor, &distance_mode);
	if (distance_mode == 0)
		return 1;
	else if (distance_mode == 1) {	/* Short DistanceMode */
		switch (TimingBudgetInMs) {
		case 15: /* only available in short distance mode */
			VL53L1_WrWord(sensor->hi2c, sensor->address, RANGE_CONFIG__TIMEOUT_MACROP_A_HI,0x01D,sensor->i2c_timeout_ms);
			VL53L1_WrWord(sensor->hi2c, sensor->address, RANGE_CONFIG__TIMEOUT_MACROP_B_HI,0x0027,sensor->i2c_timeout_ms);
			break;
		case 20:
			VL53L1_WrWord(sensor->hi2c, sensor->address, RANGE_CONFIG__TIMEOUT_MACROP_A_HI,0x0051,sensor->i2c_timeout_ms);
			VL53L1_WrWord(sensor->hi2c, sensor->address, RANGE_CONFIG__TIMEOUT_MACROP_B_HI,0x006E,sensor->i2c_timeout_ms);
			break;
		case 33:
			VL53L1_WrWord(sensor->hi2c, sensor->address, RANGE_CONFIG__TIMEOUT_MACROP_A_HI,0x00D6,sensor->i2c_timeout_ms);
			VL53L1_WrWord(sensor->hi2c, sensor->address, RANGE_CONFIG__TIMEOUT_MACROP_B_HI,0x006E,sensor->i2c_timeout_ms);
			break;
		case 50:
			VL53L1_WrWord(sensor->hi2c, sensor->address, RANGE_CONFIG__TIMEOUT_MACROP_A_HI,0x1AE,sensor->i2c_timeout_ms);
			VL53L1_WrWord(sensor->hi2c, sensor->address, RANGE_CONFIG__TIMEOUT_MACROP_B_HI,0x01E8,sensor->i2c_timeout_ms);
			break;
		case 100:
			VL53L1_WrWord(sensor->hi2c, sensor->address, RANGE_CONFIG__TIMEOUT_MACROP_A_HI,0x02E1,sensor->i2c_timeout_ms);
			VL53L1_WrWord(sensor->hi2c, sensor->address, RANGE_CONFIG__TIMEOUT_MACROP_B_HI,0x0388,sensor->i2c_timeout_ms);
			break;
		case 200:
			VL53L1_WrWord(sensor->hi2c, sensor->address, RANGE_CONFIG__TIMEOUT_MACROP_A_HI,0x03E1,sensor->i2c_timeout_ms);
			VL53L1_WrWord(sensor->hi2c, sensor->address, RANGE_CONFIG__TIMEOUT_MACROP_B_HI,0x0496,sensor->i2c_timeout_ms);
			break;
		case 500:
			VL53L1_WrWord(sensor->hi2c, sensor->address, RANGE_CONFIG__TIMEOUT_MACROP_A_HI,0x0591,sensor->i2c_timeout_ms);
			VL53L1_WrWord(sensor->hi2c, sensor->address, RANGE_CONFIG__TIMEOUT_MACROP_B_HI,0x05C1,sensor->i2c_timeout_ms);
			break;
		default:
			status = 1;
			break;
		}
	} else {
		switch (TimingBudgetInMs) {
		case 20:
			VL53L1_WrWord(sensor->hi2c, sensor->address, RANGE_CONFIG__TIMEOUT_MACROP_A_HI,0x001E,sensor->i2c_timeout_ms);
			VL53L1_WrWord(sensor->hi2c, sensor->address, RANGE_CONFIG__TIMEOUT_MACROP_B_HI,0x0022,sensor->i2c_timeout_ms);
			break;
		case 33:
			VL53L1_WrWord(sensor->hi2c, sensor->address, RANGE_CONFIG__TIMEOUT_MACROP_A_HI,0x0060,sensor->i2c_timeout_ms);
			VL53L1_WrWord(sensor->hi2c, sensor->address, RANGE_CONFIG__TIMEOUT_MACROP_B_HI,0x006E,sensor->i2c_timeout_ms);
			break;
		case 50:
			VL53L1_WrWord(sensor->hi2c, sensor->address, RANGE_CONFIG__TIMEOUT_MACROP_A_HI,0x00AD,sensor->i2c_timeout_ms);
			VL53L1_WrWord(sensor->hi2c, sensor->address, RANGE_CONFIG__TIMEOUT_MACROP_B_HI,0x00C6,sensor->i2c_timeout_ms);
			break;
		case 100:
			VL53L1_WrWord(sensor->hi2c, sensor->address, RANGE_CONFIG__TIMEOUT_MACROP_A_HI,0x01CC,sensor->i2c_timeout_ms);
			VL53L1_WrWord(sensor->hi2c, sensor->address, RANGE_CONFIG__TIMEOUT_MACROP_B_HI,0x01EA,sensor->i2c_timeout_ms);
			break;
		case 200:
			VL53L1_WrWord(sensor->hi2c, sensor->address, RANGE_CONFIG__TIMEOUT_MACROP_A_HI,0x02D9,sensor->i2c_timeout_ms);
			VL53L1_WrWord(sensor->hi2c, sensor->address, RANGE_CONFIG__TIMEOUT_MACROP_B_HI,0x02F8,sensor->i2c_timeout_ms);
			break;
		case 500:
			VL53L1_WrWord(sensor->hi2c, sensor->address, RANGE_CONFIG__TIMEOUT_MACROP_A_HI,0x048F,sensor->i2c_timeout_ms);
			VL53L1_WrWord(sensor->hi2c, sensor->address, RANGE_CONFIG__TIMEOUT_MACROP_B_HI,0x04A4,sensor->i2c_timeout_ms);
			break;
		default:
			status = 1;
			break;
		}
	}
	return status;
}

int32_t _VL53L1X_SetDistanceMode(VL53L1X* sensor, uint16_t DM)
{
	if (sensor == NULL)
	{
		return -99; //invalid sensor pointer
	}

	uint16_t pTimingBudgetInMs;
	int32_t status = 0;
	status = _VL53L1X_GetTimingBudgetInMs(sensor, &pTimingBudgetInMs);
	if (status != 0)
	{
		return -2; //error getting timing budget
	}

	switch (DM) {
	case 1: //Short distance mode
		status = VL53L1_WrByte(sensor->hi2c, sensor->address, PHASECAL_CONFIG__TIMEOUT_MACROP, 0x14, sensor->i2c_timeout_ms);
		status = VL53L1_WrByte(sensor->hi2c, sensor->address, RANGE_CONFIG__VCSEL_PERIOD_A, 0x07, sensor->i2c_timeout_ms);
		status = VL53L1_WrByte(sensor->hi2c, sensor->address, RANGE_CONFIG__VCSEL_PERIOD_B, 0x05, sensor->i2c_timeout_ms);
		status = VL53L1_WrByte(sensor->hi2c, sensor->address, RANGE_CONFIG__VALID_PHASE_HIGH, 0x38, sensor->i2c_timeout_ms);
		status = VL53L1_WrWord(sensor->hi2c, sensor->address, SD_CONFIG__WOI_SD0, 0x0705, sensor->i2c_timeout_ms);
		status = VL53L1_WrWord(sensor->hi2c, sensor->address, SD_CONFIG__INITIAL_PHASE_SD0, 0x0606, sensor->i2c_timeout_ms);
		break;
	case 2://Medium distance mode
		status = VL53L1_WrByte(sensor->hi2c, sensor->address, PHASECAL_CONFIG__TIMEOUT_MACROP, 0x0A, sensor->i2c_timeout_ms);
		status = VL53L1_WrByte(sensor->hi2c, sensor->address, RANGE_CONFIG__VCSEL_PERIOD_A, 0x0F, sensor->i2c_timeout_ms);
		status = VL53L1_WrByte(sensor->hi2c, sensor->address, RANGE_CONFIG__VCSEL_PERIOD_B, 0x0D, sensor->i2c_timeout_ms);
		status = VL53L1_WrByte(sensor->hi2c, sensor->address, RANGE_CONFIG__VALID_PHASE_HIGH, 0xB8, sensor->i2c_timeout_ms);
		status = VL53L1_WrWord(sensor->hi2c, sensor->address, SD_CONFIG__WOI_SD0, 0x0F0D, sensor->i2c_timeout_ms);
		status = VL53L1_WrWord(sensor->hi2c, sensor->address, SD_CONFIG__INITIAL_PHASE_SD0, 0x0E0E, sensor->i2c_timeout_ms);
		break;
	default:
		status = 1;
		break;
	}

	if (status == 0)
	{
		return TOF_SetTimingBudget(sensor, pTimingBudgetInMs); //restore timing budget after changing distance mode
	}
	return -1; //error setting distance mode
}

int32_t _VL53L1X_GetInterruptPolarity(VL53L1X* sensor, uint8_t* pInterruptPolarity)
{	
	if (sensor == NULL)
	{
		return -99; //invalid sensor pointer
	}

	uint8_t Temp;
	int32_t status = 0;
	status = VL53L1_RdByte(sensor->hi2c, sensor->address, GPIO_HV_MUX__CTRL, &Temp, sensor->i2c_timeout_ms);
	if (status != 0)
	{
		return status; //error reading interrupt polarity
	}
	
	Temp = Temp & 0x10;
	*pInterruptPolarity = !(Temp>>4);
	return 0;

}

int32_t _VL53L1X_CheckForDataReady(VL53L1X* sensor, uint8_t* isDataReady)
{
	if (sensor == NULL)
	{
		return -99; //invalid sensor pointer
	}
	
	uint8_t Temp;
	uint8_t IntPol;
	int32_t status = 0;

	status |= _VL53L1X_GetInterruptPolarity(sensor, &IntPol);
	status |= VL53L1_RdByte(sensor->hi2c, sensor->address, GPIO__TIO_HV_STATUS, &Temp, sensor->i2c_timeout_ms);
	/* Read in the register to check if a new value is available */
	if (status == 0){
		if ((Temp & 1) == IntPol)
			*isDataReady = 1;
		else
			*isDataReady = 0;
	}
	return status;
}

int32_t _VL53L1X_StartRanging(VL53L1X* sensor)
{
	if (sensor == NULL)
	{
		return -99; //invalid sensor pointer
	}
	return VL53L1_WrByte(sensor->hi2c, sensor->address, SYSTEM__MODE_START, 0x40, sensor->i2c_timeout_ms);
}

int32_t _VL53L1X_ClearInterrupt(VL53L1X* sensor)
{
	if (sensor == NULL)
	{
		return -99; //invalid sensor pointer
	}

	return VL53L1_WrByte(sensor->hi2c, sensor->address, SYSTEM__INTERRUPT_CLEAR, 0x01, sensor->i2c_timeout_ms);
}

int32_t _VL53L1X_StopRanging(VL53L1X* sensor)
{
	if (sensor == NULL)
	{
		return -99; //invalid sensor pointer
	}

	return VL53L1_WrByte(sensor->hi2c, sensor->address, SYSTEM__MODE_START, 0x00, sensor->i2c_timeout_ms);
}

int32_t TOF_initSensor(VL53L1X* sensor)
{
	if (sensor == NULL)
	{
		return -99; //invalid sensor pointer
	}

	int32_t status = 0;
	for (uint8_t WriteAddr = 0x2D; WriteAddr <= 0x87; WriteAddr++){
		status = VL53L1_WrByte(sensor->hi2c,sensor->address, WriteAddr, VL51L1X_DEFAULT_CONFIGURATION[WriteAddr - 0x2D], sensor->i2c_timeout_ms);
		if (status != 0)
		{
			return -1; //error writing to sensor
		}
		osDelay(1);
	}

	//Start ranging
	status = _VL53L1X_StartRanging(sensor);
	if (status != 0)
	{
		return -2; //error starting ranging
	}

	//Wait untill data will be ready
	uint32_t time_start = HAL_GetTick();
	uint32_t timeout = time_start + 1000; //1 second timeout for sensor to boot up
	uint8_t data_ready = 0;
	while (timeout > HAL_GetTick() && !data_ready)
	{
		status |= _VL53L1X_CheckForDataReady(sensor, &data_ready);
		osDelay(10);
	}

	if (!data_ready)
	{
		return -3; //timeout waiting for sensor to boot up
	}

	//Clear interrupt
	status = _VL53L1X_ClearInterrupt(sensor);
	if (status != 0)
	{
		return -4; //error clearing interrupt
	}

	//Stop ranging
	status = _VL53L1X_StopRanging(sensor);
	if (status != 0)
	{
		return -5; //error stopping ranging
	}

	status = VL53L1_WrByte(sensor->hi2c, sensor->address, VL53L1_VHV_CONFIG__TIMEOUT_MACROP_LOOP_BOUND, 0x09, sensor->i2c_timeout_ms); /* two bounds VHV */
	if (status != 0)
	{
		return -6; //error writing VHV config
	}

	status = VL53L1_WrByte(sensor->hi2c, sensor->address, 0x0B, 0, sensor->i2c_timeout_ms); /* start VHV from the previous temperature */
	if (status != 0)
	{
		return -7; //error starting VHV
	}

	return 0;
}

int32_t TOF_SetMode(VL53L1X* sensor, uint16_t distance_mode)
{
	int32_t status = 0;

	if(sensor == NULL)
	{
		return -1; //invalid sensor pointer
	}

	if (distance_mode != 0 && distance_mode != 1) //SHORT / LONG mode
	{
		return -2; //invalid distance mode
	}

	status = _VL53L1X_StopRanging(sensor);
	if (status != 0)
	{
		return -3; //error stopping ranging
	}


	status = _VL53L1X_SetDistanceMode(sensor, distance_mode);
	status = _VL53L1X_StartRanging(sensor);

	if (status != 0)
	{
		return -4; //error setting distance mode
	}

	return status;
}

int32_t TOF_BootState(VL53L1X* sensor, uint8_t *state)
{
	int32_t status = 0;
	uint8_t tmp = 0;

	status |= VL53L1_RdByte(sensor->hi2c, sensor->address, VL53L1_FIRMWARE__SYSTEM_STATUS, &tmp, sensor->i2c_timeout_ms);
	*state = tmp;
	return status;
}

int32_t TOF_SetInterMeasurementInMs(VL53L1X* sensor, uint32_t InterMeasMs)
{
	uint16_t ClockPLL;
	int32_t status = 0;

	status =  VL53L1_RdWord(sensor->hi2c, sensor->address, VL53L1_RESULT__OSC_CALIBRATE_VAL, &ClockPLL, sensor->i2c_timeout_ms);
	if (status != 0)
	{
		return status; //error reading clock pll value
	}

	ClockPLL = ClockPLL&0x3FF;
	status = VL53L1_WrDWord(sensor->hi2c, sensor->address, VL53L1_SYSTEM__INTERMEASUREMENT_PERIOD,(uint32_t)(ClockPLL * InterMeasMs * 1.075), sensor->i2c_timeout_ms);
	return status;//error writing intermeasurement period

}

int32_t TOF_GetDistance(VL53L1X* sensor, uint32_t timeout, uint16_t* measured_distance_value)
{
	int32_t status = 0;
	uint16_t tmp;

	status = VL53L1_RdWord(sensor->hi2c, sensor->address,VL53L1_RESULT__FINAL_CROSSTALK_CORRECTED_RANGE_MM_SD0, &tmp, sensor->i2c_timeout_ms);
	if (status != 0)
	{
		return -1; //error reading distance
	}
	*measured_distance_value = tmp;
	return 0; //success
}

int32_t TOF_TurnOffByXSHUT(VL53L1X* sensor)
{
	if (sensor == NULL)
	{
		return -99; //invalid sensor pointer
	}

	if (sensor->xshut_port == NULL)
	{
		return -1; //XSHUT pin not configured
	}

	if (sensor->xshut_pin == 0)
	{
		return -2; //XSHUT pin not configured
	}

	if (sensor->address == 0)
	{
		return -4; //I2C address not configured
	}

	HAL_GPIO_WritePin(sensor->xshut_port, sensor->xshut_pin, GPIO_PIN_SET);
	return 0;
}

int32_t TOF_TurnOnByXSHUT(VL53L1X* sensor)
{
	if (sensor == NULL)
	{
		return -99; //invalid sensor pointer
	}

	if (sensor->xshut_port == NULL)
	{
		return -1; //XSHUT pin not configured
	}

	if (sensor->xshut_pin == 0)
	{
		return -2; //XSHUT pin not configured
	}

	if (sensor->address == 0)
	{
		return -4; //I2C address not configured
	}

	HAL_GPIO_WritePin(sensor->xshut_port, sensor->xshut_pin, GPIO_PIN_RESET);
	osDelay(10);

	return 0;
}

int32_t TOF_SetROI(VL53L1X* sensor,  uint16_t X, uint16_t Y)
{
	uint8_t OpticalCenter;
	int32_t status = 0;

	status |=VL53L1_RdByte(sensor->hi2c,sensor->address, VL53L1_ROI_CONFIG__MODE_ROI_CENTRE_SPAD, &OpticalCenter, sensor->i2c_timeout_ms);
	if (X > 16)
		X = 16;
	if (Y > 16)
		Y = 16;
	if (X > 10 || Y > 10){
		OpticalCenter = 199;
	}
	status |= VL53L1_WrByte(sensor->hi2c, sensor->address, ROI_CONFIG__USER_ROI_CENTRE_SPAD, OpticalCenter, sensor->i2c_timeout_ms);
	status |= VL53L1_WrByte(sensor->hi2c, sensor->address, ROI_CONFIG__USER_ROI_REQUESTED_GLOBAL_XY_SIZE,
		       (Y - 1) << 4 | (X - 1), sensor->i2c_timeout_ms);
	return status;
}

int32_t TOF_CleanInterrupt(VL53L1X* sensor)
{
	return _VL53L1X_ClearInterrupt(sensor);
}

int32_t TOF_GetDistanceThresholdHigh(VL53L1X* sensor, uint16_t *high){
	uint16_t tmp;
	int32_t status = 0;

	status = VL53L1_RdWord(sensor->hi2c, sensor->address, SYSTEM__THRESH_HIGH, &tmp, sensor->i2c_timeout_ms);
	*high = tmp;
	return status;
}

int32_t TOF_GetDistanceThresholdLow(VL53L1X* sensor, uint16_t *low){
	uint16_t tmp;
	int32_t status = 0;

	status = VL53L1_RdWord(sensor->hi2c, sensor->address, SYSTEM__THRESH_LOW, &tmp, sensor->i2c_timeout_ms);
	*low = tmp;
	return status;
}

int32_t TOF_GetDistanceThresholdWindow(VL53L1X* sensor, uint16_t *window)
{
	int32_t status = 0;
	uint8_t tmp;
	status |= VL53L1_RdByte(sensor->hi2c, sensor->address, SYSTEM__INTERRUPT_CONFIG_GPIO, &tmp, sensor->i2c_timeout_ms);
	*window = (uint16_t)(tmp & 0x7);
	return status;
}

int32_t TOF_SetDistanceThreshold(VL53L1X* sensor, uint16_t ThreshLow,uint16_t ThreshHigh, uint8_t Window,uint8_t IntOnNoTarget)
{
	int32_t status = 0;
	uint8_t Temp = 0;

	status |= VL53L1_RdByte(sensor->hi2c, sensor->address, SYSTEM__INTERRUPT_CONFIG_GPIO, &Temp, sensor->i2c_timeout_ms);
	Temp = Temp & 0x47;
	if (IntOnNoTarget == 0) {
		status = VL53L1_WrByte(sensor->hi2c, sensor->address, SYSTEM__INTERRUPT_CONFIG_GPIO,
			       (Temp | (Window & 0x07)), sensor->i2c_timeout_ms);
	} else {
		status = VL53L1_WrByte(sensor->hi2c, sensor->address, SYSTEM__INTERRUPT_CONFIG_GPIO,
			       ((Temp | (Window & 0x07)) | 0x40), sensor->i2c_timeout_ms);
	}
	status |= VL53L1_WrWord(sensor->hi2c, sensor->address, SYSTEM__THRESH_HIGH, ThreshHigh, sensor->i2c_timeout_ms);
	status |= VL53L1_WrWord(sensor->hi2c, sensor->address, SYSTEM__THRESH_LOW, ThreshLow, sensor->i2c_timeout_ms);
	return status;
}


int32_t TOF_SetROICenter(VL53L1X* sensor, uint8_t ROICenter)
{
	return VL53L1_WrByte(sensor->hi2c, sensor->address, ROI_CONFIG__USER_ROI_CENTRE_SPAD, ROICenter, sensor->i2c_timeout_ms);
}

int32_t TOF_GetROICenter(VL53L1X* sensor, uint8_t *ROICenter)
{
	return VL53L1_RdByte(sensor->hi2c, sensor->address, ROI_CONFIG__USER_ROI_CENTRE_SPAD, ROICenter, sensor->i2c_timeout_ms);
}

int32_t TOF_SetSignalThreshold(VL53L1X* sensor, uint16_t Signal)
{
	return VL53L1_WrWord(sensor->hi2c, sensor->address, RANGE_CONFIG__MIN_COUNT_RATE_RTN_LIMIT_MCPS, Signal>>3, sensor->i2c_timeout_ms);

}

int32_t TOF_GetSignalThreshold(VL53L1X* sensor, uint16_t *Signal)
{
	int32_t status = 0;
	uint16_t tmp;

	status |= VL53L1_RdWord(sensor->hi2c, sensor->address, RANGE_CONFIG__MIN_COUNT_RATE_RTN_LIMIT_MCPS, &tmp, sensor->i2c_timeout_ms);
	*Signal = tmp <<3;
	return status;
}

int32_t TOF_SetSigmaThreshold(VL53L1X* sensor, uint16_t Sigma)
{
	return VL53L1_WrWord(sensor->hi2c, sensor->address, RANGE_CONFIG__SIGMA_THRESH, Sigma>>2, sensor->i2c_timeout_ms);
}

int32_t TOF_GetSigmaThreshold(VL53L1X* sensor, uint16_t *Sigma)
{
	if (sensor == NULL)
	{
		return -99; //invalid sensor pointer
	}
	int32_t status = 0;
	uint16_t tmp;

	status |= VL53L1_RdWord(sensor->hi2c, sensor->address, RANGE_CONFIG__SIGMA_THRESH, &tmp, sensor->i2c_timeout_ms);
	*Sigma = tmp <<2;
	return status;
}

int32_t TOF_GetTimingBudget(VL53L1X* sensor, uint16_t* TimingBudgetInMs)
{
	if (sensor == NULL)
	{
		return -99; //invalid sensor pointer
	}
	return _VL53L1X_GetTimingBudgetInMs(sensor, TimingBudgetInMs);
}

int32_t TOF_GetDistanceMode(VL53L1X* sensor, uint16_t *distance_mode)
{
	if (sensor == NULL)
	{
		return -99; //invalid sensor pointer
	}
	return _VL53L1X_GetDistanceMode(sensor, distance_mode);
}

int32_t TOF_GetInterMeasurementInMs(VL53L1X* sensor, uint32_t* InterMeasMs)
{
	if (sensor == NULL)
	{
		return -99; //invalid sensor pointer
	}
	uint16_t ClockPLL;
	int32_t status = 0;
	uint32_t tmp;

	status |= VL53L1_RdDWord(sensor->hi2c, sensor->address, VL53L1_SYSTEM__INTERMEASUREMENT_PERIOD, &tmp, sensor->i2c_timeout_ms);
	*InterMeasMs = (uint16_t)tmp;
	status |= VL53L1_RdWord(sensor->hi2c, sensor->address, VL53L1_RESULT__OSC_CALIBRATE_VAL, &ClockPLL, sensor->i2c_timeout_ms);
	ClockPLL = ClockPLL&0x3FF;
	*InterMeasMs= (uint16_t)(*InterMeasMs/(ClockPLL*1.065));
	return status;
}


int32_t TOF_check_Window_critera_match(VL53L1X* sensor, uint8_t* out_register_value){
	if (sensor == NULL)
	{
		return -99; //invalid sensor pointer
	}
	uint8_t Temp;
	int32_t status = 0;
	//CHECK THE STATUS OF INTERRUPT AT GPIO
	status |= VL53L1_RdByte(sensor->hi2c, sensor->address, 0x88, &Temp, sensor->i2c_timeout_ms);

	*out_register_value = Temp;
	return status;
}

int32_t TOF_check_for_GPIO_interrupt(VL53L1X* sensor, uint8_t* out_register_value){
	if (sensor == NULL)
	{
		return -99; //invalid sensor pointer
	}
	uint8_t Temp;
	int32_t status = 0;
	//CHECK THE STATUS OF INTERRUPT AT GPIO
	status |= VL53L1_RdByte(sensor->hi2c, sensor->address, 0x31, &Temp, sensor->i2c_timeout_ms);

	*out_register_value = Temp;
	return status;
}

/* -----------------------------------------------------------------------
 * Dynamic timing-budget helpers
 *
 * The ULP lookup table only goes down to 15 ms.  For 10 ms (100 Hz) we
 * must compute the MACROP timeout registers at runtime, exactly as the
 * full ST VL53L1 API does (vl53l1_core.c: calc_macro_period_us /
 * calc_encoded_timeout / encode_timeout).
 * ----------------------------------------------------------------------- */

/* Returns macro period in fixed-point 12.12 us units.
 * vcsel_period_reg is the raw register value; actual pclks = (reg+1)*2.     */
static uint32_t _VL53L1X_calc_macro_period_us(VL53L1X* sensor, uint8_t vcsel_period_reg)
{
	uint16_t fast_osc_freq = 0;
	VL53L1_RdWord(sensor->hi2c, sensor->address, VL53L1_RESULT__OSC_CALIBRATE_VAL, &fast_osc_freq, sensor->i2c_timeout_ms);
	fast_osc_freq &= 0x3FFU;

	/* pll_period_us in 9.23 fixed-point: (1<<30) / fast_osc_freq */
	uint32_t pll_period_us = (uint32_t)(0x01UL << 30) / fast_osc_freq;

	uint8_t vcsel_period_pclks = (vcsel_period_reg + 1U) << 1U;

	/* 2304 * pll_period_us >> 6 * vcsel_period_pclks >> 6 */
	uint32_t macro_period_us = (uint32_t)2304U * pll_period_us;
	macro_period_us >>= 6;
	macro_period_us *= (uint32_t)vcsel_period_pclks;
	macro_period_us >>= 6;

	return macro_period_us;
}

/* Encodes a timeout in macro-clocks to the VL53L1X register format
 * (mantissa | exponent<<8).                                                  */
static uint16_t _VL53L1X_encode_timeout(uint32_t timeout_mclks)
{
	if (timeout_mclks == 0U)
	{
		return 0U;
	}
	uint32_t ls_byte = timeout_mclks - 1U;
	uint16_t ms_byte = 0U;
	while ((ls_byte & 0xFFFFFF00U) != 0U)
	{
		ls_byte >>= 1U;
		ms_byte++;
	}
	return (ms_byte << 8U) | (uint16_t)(ls_byte & 0xFFU);
}

/* Sets the ranging timeout registers for the currently active VCSEL periods.
 * budget_us is the total measurement timing budget in microseconds.
 * LITE_RANGING mode without MM steps uses a 1000 us internal timing guard.  */
static int32_t _VL53L1X_SetTimingBudgetMicroSeconds(VL53L1X* sensor, uint32_t budget_us)
{
	const uint32_t timing_guard_us = 1000U; /* LITE_RANGING, no MM steps */

	if (budget_us <= timing_guard_us)
	{
		return -1; /* budget too small */
	}

	uint32_t range_timeout_us = budget_us - timing_guard_us;

	uint8_t vcsel_period_a = 0;
	uint8_t vcsel_period_b = 0;
	VL53L1_RdByte(sensor->hi2c, sensor->address, RANGE_CONFIG__VCSEL_PERIOD_A, &vcsel_period_a, sensor->i2c_timeout_ms);
	VL53L1_RdByte(sensor->hi2c, sensor->address, RANGE_CONFIG__VCSEL_PERIOD_B, &vcsel_period_b, sensor->i2c_timeout_ms);

	uint32_t macro_period_a = _VL53L1X_calc_macro_period_us(sensor, vcsel_period_a);
	uint32_t macro_period_b = _VL53L1X_calc_macro_period_us(sensor, vcsel_period_b);

	/* timeout_mclks = ((timeout_us << 12) + (macro_period >> 1)) / macro_period */
	uint32_t timeout_mclks_a = ((range_timeout_us << 12U) + (macro_period_a >> 1U)) / macro_period_a;
	uint32_t timeout_mclks_b = ((range_timeout_us << 12U) + (macro_period_b >> 1U)) / macro_period_b;

	uint16_t encoded_a = _VL53L1X_encode_timeout(timeout_mclks_a);
	uint16_t encoded_b = _VL53L1X_encode_timeout(timeout_mclks_b);

	int32_t status = 0;
	status |= VL53L1_WrWord(sensor->hi2c, sensor->address, RANGE_CONFIG__TIMEOUT_MACROP_A_HI, encoded_a, sensor->i2c_timeout_ms);
	status |= VL53L1_WrWord(sensor->hi2c, sensor->address, RANGE_CONFIG__TIMEOUT_MACROP_B_HI, encoded_b, sensor->i2c_timeout_ms);
	return status;
}

/* -----------------------------------------------------------------------
 * TOF_Set100HzMeasurementMode
 *
 * Configures the sensor for ~100 Hz ranging:
 *   - Short distance mode (only mode that supports 10 ms budget)
 *   - 10 ms timing budget (computed dynamically for any sensor oscillator)
 *   - 10 ms inter-measurement period -> 100 Hz
 *   - First interrupt is discarded as required by the fast-ranging startup
 *     sequence described in ST application note.
 *
 * After this call, start polling/interrupt handling:
 *   TOF_GetDistance() then TOF_CleanInterrupt() each cycle.
 * ----------------------------------------------------------------------- */
int32_t TOF_Set100HzMeasurementMode(VL53L1X* sensor)
{
	int32_t status = 0;

	if (sensor == NULL)
	{
		return -99; //invalid sensor pointer
	}

	status = _VL53L1X_StopRanging(sensor);
	if (status != 0)
	{
		return -1; //error stopping ranging
	}

	/* Short distance mode is required for 10 ms timing budget */
	status = _VL53L1X_SetDistanceMode(sensor, 1);
	if (status != 0)
	{
		return -2; //error setting short distance mode
	}

	/* 10 ms timing budget — dynamic computation, not a lookup table */
	status = _VL53L1X_SetTimingBudgetMicroSeconds(sensor, 10000U);
	if (status != 0)
	{
		return -3; //error setting 10 ms timing budget
	}

	/* 10 ms inter-measurement period targets 100 Hz */
	status = TOF_SetInterMeasurementInMs(sensor, 10U);
	if (status != 0)
	{
		return -4; //error setting intermeasurement period
	}

	status = _VL53L1X_StartRanging(sensor);
	if (status != 0)
	{
		return -5; //error starting ranging
	}

	/* Discard the first interrupt — required by fast-ranging startup sequence */
	status = _VL53L1X_ClearInterrupt(sensor);
	if (status != 0)
	{
		return -6; //error clearing first interrupt
	}

	return 0;
}