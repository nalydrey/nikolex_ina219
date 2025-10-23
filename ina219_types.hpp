/*
 * ina219_types.hpp
 *
 *  Created on: Sep 28, 2025
 *      Author: Oleksiy
 */

#ifndef NIKOLEX_INA219_INA219_TYPES_HPP_
#define NIKOLEX_INA219_INA219_TYPES_HPP_

#include "stdint.h"
#include "ina219_events.hpp"
#include "ina219_defines.hpp"

namespace nikolex::ina219 {

	struct StackItem {
		Event event_type;
		uint8_t registerAddress;
		uint16_t value;
		uint16_t mask;
		void (*callback)(BaseEvent *);
		void* ctx;
	};



}



#endif /* NIKOLEX_INA219_INA219_TYPES_HPP_ */
