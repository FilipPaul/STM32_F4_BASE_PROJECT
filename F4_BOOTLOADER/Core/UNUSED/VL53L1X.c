#include "VL53L1X.h"
#include "VL53L1X_api.h"
#include "cmsis_os.h"

// Helper functions & variables
#define _VL53L1X_LOG(fmt, ...) { if (vl53l1x_log_func != NULL) vl53l1x_log_func("TOF Debug: " fmt, ##__VA_ARGS__); }
#define _VL53L1X_MAX_SENSORS 10

VL53L1X* registered_sensors[_VL53L1X_MAX_SENSORS] = { 0 };
const size_t registered_sensors_cnt = _VL53L1X_MAX_SENSORS;
static int32_t (*vl53l1x_log_func)(const char*, ...) = NULL;


void TOF_reset_all_flags(VL53L1X* sensor_struct)
{
	sensor_struct->out_of_liminit_detected_single_flag = false;
	sensor_struct->inside_limit_detected_single_flag = false;
}

void TOF_reset_inside_limit_flag(VL53L1X* sensor_struct)
{
	sensor_struct->inside_limit_detected_single_flag = false;
}

void TOF_reset_out_of_limit_flag(VL53L1X* sensor_struct)
{
	sensor_struct->out_of_liminit_detected_single_flag = false;
}

void TOF_start_automated_measurement(VL53L1X* sensor_struct)
{
	sensor_struct->automated_measurement_is_running_flag = true;
}

void TOF_stop_automated_measurement(VL53L1X* sensor_struct)
{
	sensor_struct->automated_measurement_is_running_flag = false;
}

void TOF_set_distance_limits(VL53L1X* sensor_struct, int32_t min_distance_mm, int32_t max_distance_mm)
{
	sensor_struct->min_distance_liminit_mm = min_distance_mm;
	sensor_struct->max_distance_liminit_mm = max_distance_mm;
}

bool TOF_is_automated_measurement_running(VL53L1X* sensor_struct)
{
	return sensor_struct->automated_measurement_is_running_flag;
}

bool TOF_is_out_of_limit_detected(VL53L1X* sensor_struct)
{
	return sensor_struct->out_of_liminit_detected_single_flag;
}

bool TOF_is_inside_limit_detected(VL53L1X* sensor_struct)
{
	return sensor_struct->inside_limit_detected_single_flag;
}

void TOF_evaluate_limits(VL53L1X* sensor_struct, uint16_t distance_mm)
{
	if (!sensor_struct->automated_measurement_is_running_flag)
	{
		return;
	}

	if (distance_mm < sensor_struct->min_distance_liminit_mm)
	{
		sensor_struct->out_of_liminit_detected_single_flag = true;
	}
	else if (distance_mm > sensor_struct->max_distance_liminit_mm)
	{
		sensor_struct->out_of_liminit_detected_single_flag = true;
	}
	else
	{
		sensor_struct->inside_limit_detected_single_flag = true;
	}
}

bool TOF_calibrate_to_distance(VL53L1X* sensor, uint32_t calib_time_ms, uint32_t distance_offset_percentage){
	//do measurements until calib_time_ms is reached:
	uint32_t start_time = HAL_GetTick();
	uint32_t sum = 0;
	uint32_t count = 0;

	uint32_t maximal_detected_distance_mm = 0;
	uint32_t minimal_detected_distance_mm = 0xFFFFFFFF;

	while ((HAL_GetTick() - start_time) < calib_time_ms)
	{
		osDelay(50); //delay a bit to avoid spamming the sensor with requests, also gives it time to settle between measurements
		uint16_t distance_mm = TOF_GetDistance(sensor, calib_time_ms);
		sum += distance_mm;
		count++;

		if (distance_mm == 0xffff) //invalid reading, skip it
		{
			return false;
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
	return true;
}

static int32_t TOF_GetBusPins(I2C_HandleTypeDef* hi2c,
	GPIO_TypeDef** scl_port,
	uint16_t* scl_pin,
	GPIO_TypeDef** sda_port,
	uint16_t* sda_pin)
{
	if ((hi2c == NULL) || (scl_port == NULL) || (scl_pin == NULL) || (sda_port == NULL) || (sda_pin == NULL))
	{
		return 1;
	}

	if (hi2c->Instance == I2C1)
	{
		*scl_port = GPIOB;
		*scl_pin = GPIO_PIN_8;
		*sda_port = GPIOB;
		*sda_pin = GPIO_PIN_7;
		return 0;
	}

	if (hi2c->Instance == I2C2)
	{
		*scl_port = I2C_EEPROM_SCL_GPIO_Port;
		*scl_pin = I2C_EEPROM_SCL_Pin;
		*sda_port = I2C_EEPROM_SDA_GPIO_Port;
		*sda_pin = I2C_EEPROM_SDA_Pin;
		return 0;
	}

	if (hi2c->Instance == I2C3)
	{
		*scl_port = I2C_EXPANDER_SCL_GPIO_Port;
		*scl_pin = I2C_EXPANDER_SCL_Pin;
		*sda_port = I2C_EXPANDER_SDA_GPIO_Port;
		*sda_pin = I2C_EXPANDER_SDA_Pin;
		return 0;
	}

	return 1;
}

static void TOF_ConfigBusPin(GPIO_TypeDef* port, uint16_t pin)
{
	GPIO_InitTypeDef gpio_init = { 0 };

	gpio_init.Pin = pin;
	gpio_init.Mode = GPIO_MODE_OUTPUT_OD;
	gpio_init.Pull = GPIO_NOPULL;
	gpio_init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(port, &gpio_init);
}

static void TOF_ReleaseBus(GPIO_TypeDef* port, uint16_t pin)
{
	HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
}

static void TOF_DriveBusLow(GPIO_TypeDef* port, uint16_t pin)
{
	HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
}

// Function definitions
int32_t TOF_InitStruct(VL53L1X* sensor, I2C_HandleTypeDef* hi2c, uint8_t address, GPIO_TypeDef* xshut_port, uint16_t xshut_pin)
{
	if (sensor == NULL)
	{
		return 1;
	}

	if (address == 0x29)
		_VL53L1X_LOG("Trying to set 0x29 as the address for the ToF, this can cause issues\r\n");

	for (uint8_t registered_id = 0; registered_id < _VL53L1X_MAX_SENSORS; registered_id++)
	{
		if (registered_sensors[registered_id] == sensor)
		{
			sensor->hi2c = hi2c;
			sensor->address = address;
			sensor->xshut_port = xshut_port;
			sensor->xshut_pin = xshut_pin;
			sensor->id = registered_id;
			return 0;
		}
	}

	sensor->hi2c = hi2c;
	sensor->address = address;

	sensor->xshut_port = xshut_port;
	sensor->xshut_pin = xshut_pin;

	// Find free slot
	uint8_t id;
	for (id = 0; (id < _VL53L1X_MAX_SENSORS) && registered_sensors[id] != NULL; id++);

	if (id >= _VL53L1X_MAX_SENSORS)
	{
		_VL53L1X_LOG("Added too many sensors, increase _VL53L1X_MAX_SENSORS value, currently %d\r\n", registered_sensors_cnt);
		return 1;
	}

	sensor->id = id;
	registered_sensors[id] = sensor;
	return 0;
}

int32_t TOF_ResetSensor(const VL53L1X* sensor)
{
	if ((sensor == NULL) || (sensor->xshut_port == NULL))
	{
		return 1;
	}

	(void) VL53L1X_StopRanging(sensor->id);
	TOF_TurnOff(sensor);
	HAL_Delay(5);
	TOF_TurnOn(sensor);
	HAL_Delay(5);
	return 0;
}

int32_t TOF_RecoverBus(const VL53L1X* sensor)
{
	GPIO_TypeDef* scl_port = NULL;
	GPIO_TypeDef* sda_port = NULL;
	uint16_t scl_pin = 0;
	uint16_t sda_pin = 0;

	if ((sensor == NULL) || (sensor->hi2c == NULL))
	{
		return 1;
	}

	if (TOF_GetBusPins(sensor->hi2c, &scl_port, &scl_pin, &sda_port, &sda_pin) != 0)
	{
		return 1;
	}

	HAL_I2C_DeInit(sensor->hi2c);

	TOF_ConfigBusPin(scl_port, scl_pin);
	TOF_ConfigBusPin(sda_port, sda_pin);
	TOF_ReleaseBus(scl_port, scl_pin);
	TOF_ReleaseBus(sda_port, sda_pin);
	HAL_Delay(1);

	for (uint8_t pulse = 0; pulse < 64U; pulse++)
	{
		if (HAL_GPIO_ReadPin(sda_port, sda_pin) == GPIO_PIN_SET)
		{
			break;
		}

		TOF_ReleaseBus(scl_port, scl_pin);
		HAL_Delay(1);
		TOF_DriveBusLow(scl_port, scl_pin);
		HAL_Delay(1);
	}

	TOF_DriveBusLow(sda_port, sda_pin);
	HAL_Delay(1);
	TOF_ReleaseBus(scl_port, scl_pin);
	HAL_Delay(1);
	TOF_ReleaseBus(sda_port, sda_pin);
	HAL_Delay(1);

	//set I2C speed to 20KHz for recovery
	sensor->hi2c->Init.ClockSpeed = 20000;
	if (HAL_I2C_Init(sensor->hi2c) != HAL_OK)
	{
		return 1;
	}

	return 0;
}

void TOF_TurnOn(const VL53L1X* sensor)
{
	HAL_GPIO_WritePin(sensor->xshut_port, sensor->xshut_pin, GPIO_PIN_SET);
}

void TOF_TurnOff(const VL53L1X* sensor)
{
	HAL_GPIO_WritePin(sensor->xshut_port, sensor->xshut_pin, GPIO_PIN_RESET);
}

int32_t TOF_BootMultipleSensors(VL53L1X** sensors, uint8_t count)
{
	int32_t status = 0;

	for (uint8_t n = 0; n < count; n++) {
		TOF_TurnOff(sensors[n]);
	}

	for (uint8_t n = 0; n < count; n++) {
		status |= TOF_BootSensor(sensors[n]);
	}

	return status;
}

int32_t TOF_BootSensor(VL53L1X* sensor)
{
	int32_t status = 0;
	TOF_TurnOn(sensor);
    HAL_Delay(2); // Sometimes this is too fast and line can't settle

    // Check if sensor is already configured with the correct address
    uint16_t id = 0;
    if (VL53L1X_GetSensorId(sensor->id, &id) != 0)
    {
    	uint8_t real_address = sensor->address;
    	sensor->address = 0x29;

    	status = VL53L1X_SetI2CAddress(sensor->id, real_address);

    	sensor->address = real_address;

    	if (status)
    	{
    		_VL53L1X_LOG("Unable to configure address for sensor 0x%02X\r\n", sensor->address);
    		return status;
    	}
    }

	// Initialise sensor
    status = VL53L1X_SensorInit(sensor->id);
	if (status)
	{
		_VL53L1X_LOG("Unable to initialise sensor 0x%02X\r\n", sensor->address);
		return status;
	}
	status = VL53L1X_StartRanging(sensor->id);
	if (status)
	{
		_VL53L1X_LOG("Unable to start measurements on sensor 0x%02X\r\n", sensor->address);
		return status;
	}

	_VL53L1X_LOG("Sensor 0x%02X configured successfully\r\n", sensor->address);
	return 0;
}

int32_t TOF_ReinitSensor(VL53L1X* sensor)
{
	if ((sensor == NULL) || (sensor->hi2c == NULL) || (sensor->xshut_port == NULL))
	{
		return 1;
	}


	if (TOF_RecoverBus(sensor) != 0)
	{
		osDelay(50);
		return 1;
	}


	return TOF_BootSensor(sensor);
}

int32_t TOF_SetMode(const VL53L1X* sensor, uint16_t distance_mode)
{
	int32_t status = 0;

	if ((sensor == NULL) || ((distance_mode != 1U) && (distance_mode != 2U)))
	{
		return 1;
	}

	status |= VL53L1X_StopRanging(sensor->id);
	status |= VL53L1X_SetDistanceMode(sensor->id, distance_mode);
	status |= VL53L1X_StartRanging(sensor->id);

	if (status != 0)
	{
		_VL53L1X_LOG("Failed to set mode %u for sensor 0x%02X\r\n", distance_mode, sensor->address);
	}

	return status;
}

int32_t TOF_SetTimingBudget(const VL53L1X* sensor, uint16_t timing_budget_ms)
{
	int32_t status = 0;

	if ((sensor == NULL) || (timing_budget_ms == 0U))
	{
		return 1;
	}

	status |= VL53L1X_StopRanging(sensor->id);
	status |= VL53L1X_SetTimingBudgetInMs(sensor->id, timing_budget_ms);
	status |= VL53L1X_StartRanging(sensor->id);

	if (status != 0)
	{
		_VL53L1X_LOG("Failed to set timing budget %u ms for sensor 0x%02X\r\n", timing_budget_ms, sensor->address);
	}

	return status;
}

int32_t TOF_SetInterMeasurement(const VL53L1X* sensor, uint32_t intermeasurement_ms)
{
	int32_t status = 0;

	if ((sensor == NULL) || (intermeasurement_ms == 0U))
	{
		return 1;
	}

	status |= VL53L1X_StopRanging(sensor->id);
	status |= VL53L1X_SetInterMeasurementInMs(sensor->id, intermeasurement_ms);
	status |= VL53L1X_StartRanging(sensor->id);

	if (status != 0)
	{
		_VL53L1X_LOG("Failed to set intermeasurement %lu ms for sensor 0x%02X\r\n", intermeasurement_ms, sensor->address);
	}

	return status;
}

int32_t TOF_GetDistanceWithTimeout(VL53L1X* sensor, uint16_t* reading, uint32_t timeout_ms)
{
	uint8_t is_data_ready = 0;
	uint32_t start_tick = HAL_GetTick();

	if (reading == NULL)
	{
		return 6;
	}


	if (sensor == NULL)
	{
		return 5;
	}

	while (HAL_GetTick() - start_tick < timeout_ms)
	{
		if (VL53L1X_CheckForDataReady(sensor->id, &is_data_ready) != 0)
		{
			_VL53L1X_LOG("Failed to poll data-ready state for sensor 0x%02X\r\n", sensor->address);
			return 4;
		}

		if (is_data_ready != 0U)
		{
			break;
		}
	}

	if (is_data_ready == 0U)
	{
		_VL53L1X_LOG("Timed out waiting for measurement from sensor 0x%02X\r\n", sensor->address);
		return 3;
	}

	if(VL53L1X_GetDistance(sensor->id, reading) != 0)
	{
		_VL53L1X_LOG("Failed to read distance from sensor 0x%02X\r\n", sensor->address);
		return 2;
	}

	if (VL53L1X_ClearInterrupt(sensor->id) != 0)
	{
		_VL53L1X_LOG("Failed to clear interrupt for sensor 0x%02X\r\n", sensor->address);
		return 1;
	}

	sensor->last_measured_distance_mm = *reading;
	return 0;
}

uint16_t TOF_GetDistance(VL53L1X* sensor, uint32_t timeout_ms)
{
	uint16_t reading = 0;

	if (TOF_GetDistanceWithTimeout(sensor, &reading, timeout_ms) != 0)
	{
		return 0xFFFF;
	}
	sensor->last_measured_distance_mm = reading;
	return reading;
}

void TOF_SetLogFunction(int32_t (*prnt)(const char*, ...))
{
	vl53l1x_log_func = prnt;
}
