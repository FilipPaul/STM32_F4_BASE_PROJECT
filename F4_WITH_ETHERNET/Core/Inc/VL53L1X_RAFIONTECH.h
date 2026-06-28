#ifndef VL53L1X_RAFIONTECH_H_
#define VL53L1X_RAFIONTECH_H_

#include "main.h"
#include "stdint.h"
#include "stdbool.h"
#include "string.h"

typedef enum
{
	VL53L1X_ERROR_NONE = 0,
	VL53L1X_ERROR_I2C = 1,
	VL53L1X_ERROR_TIMEOUT = 2,
	VL53L1X_ERROR_INVALID_PARAM = 3,
	VL53L1X_ERROR_NOT_SUPPORTED = 4,
	VL53L1X_ERROR_UNKNOWN = 5,
	VL53L1X_ERROR_EXPANDER_INIT = 6,
	VL53L1X_ERROR_EXPANDER_CHANNEL = 7,
	VL53L1X_ERROR_I2C_RESTORE = 8,
	VL53L1X_ERROR_INIT = 9,
	VL53L1X_ERROR_REINIT = 10,
	VL53L1X_ERROR_SET_MODE = 11,
	VL53L1X_ERROR_SET_TIMING_BUDGET = 12,
	VL53L1X_ERROR_SET_INTERMEASUREMENT = 13,
	VL53L1X_ERROR_MEASUREMENT = 14
} VL53L1X_Error;

typedef struct vl53l1x
{
	// I2C information
	I2C_HandleTypeDef* hi2c;
	uint8_t address;

	// XSHUT pin
	GPIO_TypeDef* xshut_port;
	uint16_t xshut_pin;

	// Used for driver back-end, not directly related to VL53L1X
	uint8_t id;

	//USED FOR EXTENDED LOGIC
	int32_t min_distance_liminit_mm;
	int32_t max_distance_liminit_mm;

	//FLAGS FOR AUTONOMOUSE MEASUREMENTS
	//AUTOMATICALY SETS TO 1 when conditions are met, needs to be manually cleared by user
	bool out_of_liminit_detected_single_flag;
	bool inside_limit_detected_single_flag;

	bool is_currently_in_limits_flag; //used to track if the current distance is in limits or not, updated with every measurement, used for edge detection of limit crossings
	
	bool automated_measurement_is_running_flag; 
	VL53L1X_Error detected_error_during_automated_measurement;
	uint32_t last_measured_distance_mm;

	uint32_t i2c_timeout_ms;
	uint32_t i2c_speed_Hz;

	//I2C PORTS FOR RECOVERING STALLED I2C BUS
	GPIO_TypeDef* scl_port;
	uint16_t scl_pin;
	GPIO_TypeDef* sda_port;
	uint16_t sda_pin;
} VL53L1X;


extern uint8_t _I2CBuffer[256];
int32_t TOF_reset_all_flags(VL53L1X* sensor_struct);
int32_t TOF_reset_persistent_inside_limit_flag(VL53L1X* sensor_struct);
int32_t TOF_reset_persistent_out_of_limit_flag(VL53L1X* sensor_struct);
int32_t TOF_is_currently_in_limits(VL53L1X* sensor_struct);
int32_t TOF_reset_all_persistent_flags(VL53L1X* sensor_struct);
int32_t TOF_start_automated_measurement(VL53L1X* sensor_struct);
int32_t TOF_stop_automated_measurement(VL53L1X* sensor_struct);
int32_t TOF_set_distance_limits(VL53L1X* sensor_struct, int32_t min_distance_mm, int32_t max_distance_mm);
int32_t TOF_is_automated_measurement_running(VL53L1X* sensor_struct);
int32_t TOF_is_out_of_limit_detected(VL53L1X* sensor_struct);
int32_t TOF_is_inside_limit_detected(VL53L1X* sensor_struct);
int32_t TOF_evaluate_limits(VL53L1X* sensor_struct, uint16_t distance_mm);
int32_t TOF_calibrate_to_distance(VL53L1X* sensor, uint32_t calib_time_ms, uint32_t distance_offset_percentage);
int32_t TOF_SetTimingBudget(VL53L1X* sensor, uint16_t TimingBudgetInMs);
int32_t TOF_initSensor(VL53L1X* sensor);
int32_t TOF_SetMode(VL53L1X* sensor, uint16_t distance_mode);
int32_t TOF_BootState(VL53L1X* sensor, uint8_t *state);
int32_t TOF_SetInterMeasurementInMs(VL53L1X* sensor, uint32_t InterMeasMs);
int32_t TOF_GetDistance(VL53L1X* sensor, uint32_t timeout, uint16_t* measured_distance_value);
int32_t TOF_TurnOffByXSHUT(VL53L1X* sensor);
int32_t TOF_TurnOnByXSHUT(VL53L1X* sensor);

int32_t TOF_GetDistanceThresholdHigh(VL53L1X* sensor, uint16_t *high);
int32_t TOF_GetDistanceThresholdLow(VL53L1X* sensor, uint16_t *low);
int32_t TOF_GetDistanceThresholdWindow(VL53L1X* sensor, uint16_t *window);
int32_t TOF_SetDistanceThreshold(VL53L1X* sensor, uint16_t ThreshLow,uint16_t ThreshHigh, uint8_t Window,uint8_t IntOnNoTarget);

int32_t TOF_SetROI(VL53L1X* sensor,  uint16_t X, uint16_t Y);
int32_t TOF_SetROICenter(VL53L1X* sensor, uint8_t ROICenter);
int32_t TOF_GetROICenter(VL53L1X* sensor, uint8_t *ROICenter);

int32_t TOF_SetSignalThreshold(VL53L1X* sensor, uint16_t Signal);
int32_t TOF_GetSignalThreshold(VL53L1X* sensor, uint16_t *signal);
int32_t TOF_SetSigmaThreshold(VL53L1X* sensor, uint16_t Sigma);
int32_t TOF_GetSigmaThreshold(VL53L1X* sensor, uint16_t *sigma);
int32_t TOF_GetTimingBudget(VL53L1X* sensor, uint16_t *TimingBudgetInMs);
int32_t TOF_GetInterMeasurementInMs(VL53L1X* sensor, uint32_t *InterMeasMs);
int32_t TOF_GetDistanceMode(VL53L1X* sensor, uint16_t *distance_mode);

int32_t TOF_CleanInterrupt(VL53L1X* sensor);
int32_t TOF_check_for_GPIO_interrupt(VL53L1X* sensor, uint8_t* out_register_value);
int32_t TOF_check_Window_critera_match(VL53L1X* sensor, uint8_t* out_register_value);
int8_t VL53L1_RdByte(I2C_HandleTypeDef* hi2c, uint16_t addr, uint16_t index, uint8_t *data, uint32_t timeout);
int8_t VL53L1_RdWord(I2C_HandleTypeDef* hi2c, uint16_t addr, uint16_t index, uint16_t *data, uint32_t timeout);
int8_t VL53L1_WrByte(I2C_HandleTypeDef* hi2c, uint16_t addr, uint16_t index, uint8_t data, uint32_t timeout);
int8_t VL53L1_WrWord(I2C_HandleTypeDef* hi2c, uint16_t addr, uint16_t index, uint16_t data, uint32_t timeout);
int32_t TOF_Set100HzMeasurementMode(VL53L1X* sensor);


#endif // VL53L1X_RAFIONTECH_H_
