#ifndef VL53L1X_H_
#define VL53L1X_H_

#include "main.h"
#include "stdint.h"
#include "stdbool.h"

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
	
	bool automated_measurement_is_running_flag; 
	VL53L1X_Error detected_error_during_automated_measurement;
	uint32_t last_measured_distance_mm;

} VL53L1X;



void TOF_reset_all_flags(VL53L1X* sensor_struct);
void TOF_reset_inside_limit_flag(VL53L1X* sensor_struct);
void TOF_reset_out_of_limit_flag(VL53L1X* sensor_struct);
void TOF_start_automated_measurement(VL53L1X* sensor_struct);
void TOF_stop_automated_measurement(VL53L1X* sensor_struct);
void TOF_set_distance_limits(VL53L1X* sensor_struct, int32_t min_distance_mm, int32_t max_distance_mm);
bool TOF_is_automated_measurement_running(VL53L1X* sensor_struct);
bool TOF_is_out_of_limit_detected(VL53L1X* sensor_struct);
bool TOF_is_inside_limit_detected(VL53L1X* sensor_struct);
void TOF_evaluate_limits(VL53L1X* sensor_struct, uint16_t distance_mm);
bool TOF_calibrate_to_distance(VL53L1X* sensor, uint32_t calib_time_ms, uint32_t distance_offset_percentage);

int32_t TOF_InitStruct(VL53L1X* sensor, I2C_HandleTypeDef* hi2c, uint8_t address, GPIO_TypeDef* xshut_port, uint16_t xshut_pin);

int32_t TOF_ResetSensor(const VL53L1X* sensor);
int32_t TOF_BootSensor(VL53L1X* sensor);
int32_t TOF_ReinitSensor(VL53L1X* sensor);
int32_t TOF_RecoverBus(const VL53L1X* sensor);
int32_t TOF_BootMultipleSensors(VL53L1X** sensors, uint8_t count);

void TOF_TurnOn(const VL53L1X* sensor);
void TOF_TurnOff(const VL53L1X* sensor);

int32_t TOF_SetMode(const VL53L1X* sensor, uint16_t distance_mode);
int32_t TOF_SetTimingBudget(const VL53L1X* sensor, uint16_t timing_budget_ms);
int32_t TOF_SetInterMeasurement(const VL53L1X* sensor, uint32_t intermeasurement_ms);
int32_t TOF_GetDistanceWithTimeout(VL53L1X* sensor, uint16_t* reading, uint32_t timeout_ms);
uint16_t TOF_GetDistance(VL53L1X* sensor, uint32_t timeout_ms);
void TOF_SetLogFunction(int32_t (*prnt)(const char*, ...));

#endif // VL53L1X_H_
