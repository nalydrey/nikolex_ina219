/*
 * ina219_events.hpp
 *
 *  Created on: Sep 30, 2025
 *      Author: Oleksiy
 */

#ifndef NIKOLEX_INA219_INA219_EVENTS_HPP_
#define NIKOLEX_INA219_INA219_EVENTS_HPP_

#include "stdint.h"
#include "ina219.hpp"

namespace nikolex::ina219 {

	class Device;

	struct BaseEvent {
		void * userData = nullptr;
		Device* device;


	};


	struct ReadRegisterEvent : BaseEvent {
		uint8_t registerAddress = 0;
		uint16_t registerData = 0;

		ReadRegisterEvent(BaseEvent& base): BaseEvent(base){};
	};

	struct WriteRegisterEvent : BaseEvent {
		uint8_t registerAddress = 0;
		uint16_t value = 0;

		WriteRegisterEvent(BaseEvent& base): BaseEvent(base){};
	};

	struct ModifyRegisterEvent : BaseEvent {
		uint8_t registerAddress = 0;
		uint16_t mask = 0;
		uint16_t value = 0;

		ModifyRegisterEvent(BaseEvent& base): BaseEvent(base){};
	};

	struct ReadCurrentEvent : ReadRegisterEvent {
		float current = 0;

		ReadCurrentEvent(BaseEvent& base): ReadRegisterEvent(base){};
	};


	struct ReadShuntVoltageEvent : ReadRegisterEvent {
		float voltage = 0;

		ReadShuntVoltageEvent(BaseEvent& base): ReadRegisterEvent(base){};
	};

	struct ReadBusVoltageEvent : ReadRegisterEvent {
		bool overflowFlag = false;
		bool conversionReadyFlag = false;
		uint16_t voltage = 0;

		ReadBusVoltageEvent(BaseEvent& base): ReadRegisterEvent(base){};
	};
}




#endif /* NIKOLEX_INA219_INA219_EVENTS_HPP_ */
