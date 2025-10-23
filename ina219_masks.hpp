/*
 * ina219_masks.hpp
 *
 *  Created on: Sep 29, 2025
 *      Author: Oleksiy
 */

#ifndef NIKOLEX_INA219_INA219_MASKS_HPP_
#define NIKOLEX_INA219_INA219_MASKS_HPP_

namespace nikolex::ina219 {


	enum MASK {
		RESET_MASK = 				0x8000,
		BUS_VOLTAGE_RANGE_MASK = 	0x2000,
		PGA_MASK = 					0x1800,
		BUS_ADC_MASK = 				0x0780,
		SHUNT_ADC_MASK = 			0x0078,
		MODE_MASK = 				0x0007
	};

}



#endif /* NIKOLEX_INA219_INA219_MASKS_HPP_ */
