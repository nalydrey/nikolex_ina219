/*
 * ina219.hpp
 *
 *  Created on: Sep 28, 2025
 *      Author: Oleksiy
 */

#ifndef NIKOLEX_INA219_INA219_HPP_
#define NIKOLEX_INA219_INA219_HPP_

#include "stdint.h"
#include "ina219_types.hpp"
#include "ina219_registers.hpp"
#include "ina219_masks.hpp"
#include "ina219_defines.hpp"
#include "../interfaces/i2c_transmit_receiver.hpp"
#include "../utils/stack.hpp"
#include "ina219_events.hpp"

namespace nikolex::ina219{

class Device {

	uint8_t buffer[2];
	uint8_t _slaveAddress;
	uint16_t _calibrationValue;
	bool ready = false;
	float _currentLSB;
	bool pending = false;


	I2C_Transmit_Receiver *_transmitReceiver;
	Stack<StackItem, 5> stack;

	void createEvent();

public:

	Device(
			uint8_t slaveAddress,
			I2C_Transmit_Receiver& transmitReceiver,
			float currentLSB,
			uint16_t calibrationValue);

	void readRegister(
			uint8_t registerAddress,
			void (*callback)(ReadRegisterEvent*) = nullptr,
			void* userData = nullptr);

	void writeRegister(
			uint8_t registerAddress,
			uint16_t value,
			void (*callback)(WriteRegisterEvent*) = nullptr,
			void* userData = nullptr);


	void modifyRegister(
			uint8_t registerAddress,
			uint16_t mask,
			uint16_t value,
			void (*callback)(ModifyRegisterEvent*) = nullptr,
			void* userData = nullptr);

	void readShuntVoltage(
			void (* callback)(ReadShuntVoltageEvent*) = nullptr,
			void* userData = nullptr);

	void readBusVoltageRegister(
			void (* callback)(ReadBusVoltageEvent *) = nullptr,
			void * userData = nullptr);

	void readPower(
			void (*callback)(ReadRegisterEvent*) = nullptr,
			void* userData = nullptr);

	void readCurrent(
			void (*callback)(ReadCurrentEvent*) = nullptr,
			void* userData = nullptr);


	void setCalibrationValue(
			uint16_t value,
			void (*callback)(WriteRegisterEvent*) = nullptr,
			void* userData = nullptr);


	void readCalibrationValue(
			void (*callback)(ReadRegisterEvent*) = nullptr,
			void* userData = nullptr);


	void reset(
			void (*callback)(ModifyRegisterEvent*) = nullptr,
			void* userData = nullptr);


	void setBusVoltageRange(
			BUS_VOLTAGE_RANGE value,
			void (*callback)(ModifyRegisterEvent*) = nullptr,
			void* userData = nullptr);


	void setPGA(
			PGA value,
			void (*callback)(ModifyRegisterEvent*) = nullptr,
			void* userData = nullptr);


	void setBusADC(
			ADC_RESOLUTION_AVERAGING value,
			void (*callback)(ModifyRegisterEvent*) = nullptr,
			void* userData = nullptr);


	void setShuntADC(
			ADC_RESOLUTION_AVERAGING value,
			void (*callback)(ModifyRegisterEvent*) = nullptr,
			void* userData = nullptr);

	void setMode(
			MODE value,
			void (*callback)(ModifyRegisterEvent*) = nullptr,
			void* userData = nullptr);

	void waitForConversion(void(* callback)(BaseEvent*) = nullptr, void* userData = nullptr);

	void readCurrentOneShot(void(* callback)(ReadCurrentEvent*) = nullptr, void* userData = nullptr);

	void waitUntilReady();
};
}




#endif /* NIKOLEX_INA219_INA219_HPP_ */
