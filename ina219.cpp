/*
 * ina219.cpp
 *
 *  Created on: Sep 30, 2025
 *      Author: Oleksiy
 */
#include "ina219.hpp"


using namespace nikolex::ina219;


Device::Device(
		uint8_t slaveAddress,
		I2C_Transmit_Receiver& transmitReceiver,
		float currentLSB,
		uint16_t calibrationValue
		) :
		_slaveAddress(slaveAddress << 1),
		_calibrationValue(calibrationValue),
		_currentLSB(currentLSB),
		_transmitReceiver(&transmitReceiver)
{

	_transmitReceiver->setContext(this);

	_transmitReceiver->setTransmitedCb([](void * ctx){
		Device* self = (Device*)ctx;
		self->createEvent();
	});

	_transmitReceiver->setReceivedCb([](void * ctx){
		Device* self = (Device*)ctx;
		self->createEvent();

	});

	reset([](ModifyRegisterEvent* evt){
		evt->device->setCalibrationValue(evt->device->_calibrationValue, [](WriteRegisterEvent * evt){
			evt->device->ready = true;
		});
	});

	while(!ready){};

}


void Device::readRegister(uint8_t registerAddress, void (*callback)(ReadRegisterEvent*), void* userData)
{
	StackItem stackItem;
	stackItem.event_type = Event::READ_REGISTER;
	stackItem.registerAddress = registerAddress;
	stackItem.callback = (void(*)(BaseEvent*))callback;
	stackItem.ctx = userData;
	stack.push(stackItem);


	stackItem = {};
	stackItem.callback = [](BaseEvent* evt){
		evt->device->_transmitReceiver->receive(evt->device->_slaveAddress, evt->device->buffer, 2);
	};
	stack.push(stackItem);


	_transmitReceiver->transmit(_slaveAddress, &registerAddress, 1);
}


void Device::writeRegister(
		uint8_t registerAddress,
		uint16_t value,
		void (*callback)(WriteRegisterEvent*),
		void* userData)
{
	StackItem stackItem;

	stackItem.event_type = Event::WRITE_REGISTER;
	stackItem.registerAddress = registerAddress;
	stackItem.value = value;
	stackItem.callback = (void (*)(BaseEvent*))callback;
	stackItem.ctx = userData;
	stack.push(stackItem);

	uint8_t data[] = {registerAddress, (uint8_t) ((value >> 8) & 0xFF), (uint8_t)(value & 0xFF)};

	_transmitReceiver->transmit(_slaveAddress, data, 3);
}


void Device::modifyRegister(
		uint8_t registerAddress,
		uint16_t mask,
		uint16_t value,
		void (*callback)(ModifyRegisterEvent*),
		void* userData)
{
	StackItem stackItem;

	stackItem.event_type = Event::MODIFY_REGISTER;
	stackItem.registerAddress = registerAddress;
	stackItem.value = value;
	stackItem.mask = mask;
	stackItem.callback = (void (*)(BaseEvent*))callback;
	stackItem.ctx = userData;
	stack.push(stackItem);

	readRegister(registerAddress, [](ReadRegisterEvent* evt){
		StackItem* item = evt->device->stack.getCurrentItem();
		uint16_t result = (evt->registerData & ~item->mask) | (item->value & item->mask);
		evt->device->writeRegister(evt->registerAddress, result, [](WriteRegisterEvent* evt){
			evt->device->createEvent();
		});
	});
}


void Device::readShuntVoltage(void (* callback)(ReadShuntVoltageEvent*), void* userData)
{
	StackItem stackItem;
	stackItem.event_type = Event::READ_SHUNT_VOLTAGE;
	stackItem.callback = (void (*)(BaseEvent*))callback;
	stackItem.ctx = userData;
	stack.push(stackItem);
	readRegister(REGISTER::SHUNT_VOLTAGE_REGISTER, [](ReadRegisterEvent* evt){
		StackItem* item = evt->device->stack.getCurrentItem();
		item->registerAddress = evt->registerAddress;
		evt->device->createEvent();
	});
}


void Device::readBusVoltageRegister(void (* callback)(ReadBusVoltageEvent *), void * userData)
{
	StackItem stackItem;
	stackItem.event_type = Event::READ_BUS_VOLTAGE;
	stackItem.callback = (void (*)(BaseEvent*))callback;
	stackItem.ctx = userData;
	stack.push(stackItem);
	readRegister(REGISTER::BUS_VOLTAGE_REGISTER, [](ReadRegisterEvent * evt){
		StackItem* item = evt->device->stack.getCurrentItem();
		item->registerAddress = evt->registerAddress;
		evt->device->createEvent();
	});
}


void Device::readCurrent(void (*callback)(ReadCurrentEvent*), void* userData)
{
	StackItem stackItem;
	stackItem.event_type = Event::READ_CURRENT;
	stackItem.callback = (void(*)(BaseEvent*))callback;
	stackItem.ctx = userData;
	stack.push(stackItem);
	readRegister(REGISTER::CURRENT_REGISTER, [](ReadRegisterEvent* evt){
		StackItem* item = evt->device->stack.getCurrentItem();
		item->registerAddress = evt->registerAddress;
		evt->device->createEvent();
	});
}


void Device::readPower(void (*callback)(ReadRegisterEvent*), void* userData)
{
	readRegister(REGISTER::POWER_REGISTER, callback, userData);
}


void Device::setCalibrationValue(uint16_t value, void (*callback)(WriteRegisterEvent*), void* userData)
{
	writeRegister(REGISTER::CALIBRATION_REGISTER, value, callback, userData);
}


void Device::readCalibrationValue(void (*callback)(ReadRegisterEvent*),	void* userData)
{
	readRegister(REGISTER::CALIBRATION_REGISTER, callback, userData);
}


void Device::reset(void (*callback)(ModifyRegisterEvent*), void* userData)
{
	modifyRegister(REGISTER::CONFIG_REGISTER, MASK::RESET_MASK, 1<<15, callback, userData);
}


void Device::setBusVoltageRange(BUS_VOLTAGE_RANGE value, void (*callback)(ModifyRegisterEvent*),void* userData)
{
	modifyRegister(
			REGISTER::CONFIG_REGISTER,
			MASK::BUS_VOLTAGE_RANGE_MASK,
			(value << 13),
			callback,
			userData);
}


void Device::setPGA(PGA value,	void (*callback)(ModifyRegisterEvent*),	void* userData)
{
	modifyRegister(REGISTER::CONFIG_REGISTER, MASK::PGA_MASK, (value << 11), callback, userData);
}


void Device::setBusADC(ADC_RESOLUTION_AVERAGING value, void (*callback)(ModifyRegisterEvent*), void* userData)
{
	modifyRegister(REGISTER::CONFIG_REGISTER, MASK::BUS_ADC_MASK, (value << 7), callback, userData);
}


void Device::setShuntADC(ADC_RESOLUTION_AVERAGING value, void (*callback)(ModifyRegisterEvent*),void* userData)
{
	modifyRegister(REGISTER::CONFIG_REGISTER, MASK::SHUNT_ADC_MASK, (value << 3), callback, userData);
}


void Device::setMode(MODE value, void (*callback)(ModifyRegisterEvent*), void* userData)
{
	modifyRegister(REGISTER::CONFIG_REGISTER, MASK::MODE_MASK, value, callback, userData);
}


void Device::waitForConversion(void(* callback)(BaseEvent*), void* userData){

	StackItem stackItem;

	stackItem.callback = (void(*)(BaseEvent*))callback;
	stackItem.ctx = userData;
	stack.push(stackItem);

	readBusVoltageRegister([](ReadBusVoltageEvent* evt){
		if(evt->conversionReadyFlag){
//			printf("conversion complete! \r\n");
			evt->device->createEvent();
		}
		else{
//			printf("! \r\n");
			StackItem* item = evt->device->stack.pop();
			evt->device->waitForConversion(item->callback, item->ctx);
		}
	});
}


void Device::readCurrentOneShot(void(* callback)(ReadCurrentEvent*), void* userData)
{
	StackItem stackItem;
	stackItem.event_type = Event::READ_CURRENT;
	stackItem.callback = (void(*)(BaseEvent*))callback;
	stackItem.ctx = userData;
	stack.push(stackItem);
	setMode(MODE::SHUNT_VOLTAGE_TRIGGERED_MODE, [](ModifyRegisterEvent * evt){
		evt->device->waitForConversion([](BaseEvent* evt){
			evt->device->readCurrent([](ReadCurrentEvent* evt){
				evt->device->createEvent();
			});
		});
	});
}


void Device::waitUntilReady(){
	_transmitReceiver->waitUntilReady();
}


void Device::createEvent(){

	StackItem* currentItem = stack.pop();

	BaseEvent base;
	base.userData = currentItem->ctx;
	base.device = this;

	BaseEvent* pBaseEvt;
	uint16_t dataFromBuffer = (buffer[0] << 8) | buffer[1];

	switch(currentItem->event_type){
		case Event::READ_BUS_VOLTAGE: {
			ReadBusVoltageEvent evt(base);
			evt.registerAddress =  currentItem->registerAddress;
			evt.registerData = dataFromBuffer;
			evt.overflowFlag = dataFromBuffer & 0x01;
			evt.conversionReadyFlag	= (dataFromBuffer >> 1) & 0x01;
			evt.voltage = (dataFromBuffer >> 3) * 4e-6;
			pBaseEvt = &evt;
			break;
		}
		case Event::READ_SHUNT_VOLTAGE: {
			ReadShuntVoltageEvent evt(base);
			evt.registerAddress = currentItem->registerAddress;
			evt.registerData = dataFromBuffer;
			evt.voltage = (float)dataFromBuffer * 10e-6f;
			pBaseEvt = &evt;
			break;
		}
		case Event::READ_CURRENT: {
			ReadCurrentEvent evt(base);
			evt.registerAddress = currentItem->registerAddress;
			evt.registerData =dataFromBuffer;
			evt.current = (int16_t)dataFromBuffer * _currentLSB;
			pBaseEvt = &evt;
			break;
		}
		case Event::MODIFY_REGISTER: {
			ModifyRegisterEvent evt(base);
			evt.registerAddress = currentItem->registerAddress;
			evt.value = currentItem->value;
			evt.mask = currentItem->mask;
			pBaseEvt = &evt;
			break;
		}
		case Event::WRITE_REGISTER: {
			WriteRegisterEvent evt(base);
			evt.registerAddress = currentItem->registerAddress;
			evt.value = currentItem->value;
			pBaseEvt = &evt;
			break;
		}
		case  Event::READ_REGISTER: {
			ReadRegisterEvent evt(base);
			evt.registerAddress = currentItem->registerAddress;
			evt.registerData = dataFromBuffer;
			pBaseEvt = &evt;
			break;
		}
		default: {
			pBaseEvt = &base;
		}
	}
	if(pBaseEvt && currentItem->callback) currentItem->callback(pBaseEvt);
}
