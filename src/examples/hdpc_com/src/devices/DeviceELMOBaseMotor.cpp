/*!
 * @file 	DeviceELMOBaseMotor.cpp
 * @brief
 * @author 	Christian Gehring
 * @date 	Jan, 2012
 * @version 1.0
 * @ingroup robotCAN, device
 *
 */

#include "DeviceELMOBaseMotor.hpp"
#include <stdio.h>
#include <math.h>


DeviceELMOBaseMotor::DeviceELMOBaseMotor(int nodeId, DeviceELMOMotorParametersHDPC* deviceParams)
:Device(nodeId),deviceParams_(deviceParams)
{
	sdoStatusWord_ =  SDOReadStatusWord::SDOReadStatusWordPtr(new SDOReadStatusWord(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_));
	sdoStatusWordDisabled_ = SDOReadStatusWord::SDOReadStatusWordPtr(new SDOReadStatusWord(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_));
}

DeviceELMOBaseMotor::~DeviceELMOBaseMotor()
{
	if (deviceParams_ != NULL){
		delete deviceParams_;
	}

}


DeviceELMOMotorParametersHDPC* DeviceELMOBaseMotor::getDeviceParams()
{
	return deviceParams_;
}

void DeviceELMOBaseMotor::addRxPDOs()
{

}

void DeviceELMOBaseMotor::addTxPDOs()
{
	/* add PositionVelocity TxPDO */
	txPDOPositionVelocity_ = new TxPDOPositionVelocity(nodeId_, deviceParams_->txPDO3SMId_);
	bus_->getTxPDOManager()->addPDO(txPDOPositionVelocity_);

	/* add AnalogCurrent TxPDO */
	txPDOAnalogCurrent_ = new TxPDOAnalogCurrent(nodeId_, deviceParams_->txPDO4SMId_);
	bus_->getTxPDOManager()->addPDO(txPDOAnalogCurrent_);
}



double DeviceELMOBaseMotor::getPosition()
{
	return ((double) txPDOPositionVelocity_->getPosition()) / ( deviceParams_->gearratio_motor * deviceParams_->RAD_TO_TICKS);

}

double DeviceELMOBaseMotor::getVelocity()
{

	return ((double)txPDOPositionVelocity_->getVelocity()) / ( deviceParams_->rad_s_Gear_to_counts_s_Motor);
}

double DeviceELMOBaseMotor::getCurrent()
{
	return ((double) txPDOAnalogCurrent_->getCurrent());
}


double DeviceELMOBaseMotor::getAnalog()
{
	return ((double) txPDOAnalogCurrent_->getAnalog());
}


void DeviceELMOBaseMotor::setPositionLimits(double * positionLimit_rad)
{


	double minPositionLimit, maxPositionLimit;

	if (positionLimit_rad[0] < positionLimit_rad[1]) {
		minPositionLimit = positionLimit_rad[0];
		maxPositionLimit = positionLimit_rad[1];
	} else {
		minPositionLimit = positionLimit_rad[1];
		maxPositionLimit = positionLimit_rad[0];
	}

	int minLimit_ticks =  (int) (minPositionLimit * deviceParams_->gearratio_motor * deviceParams_->RAD_TO_TICKS);
	int maxLimit_ticks =  (int) (maxPositionLimit * deviceParams_->gearratio_motor * deviceParams_->RAD_TO_TICKS);

	bus_->getSDOManager()->addSDO(new SDOSetMinPositionLimit(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, minLimit_ticks));
	bus_->getSDOManager()->addSDO(new SDOSetMaxPositionLimit(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, maxLimit_ticks));

}

void DeviceELMOBaseMotor::setMotorParameters()
{

}


bool DeviceELMOBaseMotor::initDevice()
{
	configTxPDOs();
	return true;
}

void DeviceELMOBaseMotor::configTxPDOs()
{
	SDOManager* SDOManager = bus_->getSDOManager();

	/* deactivate all TxPDOs */
	SDOManager->addSDO(new SDOTxPDO1SetNumberOfMappedApplicationObjects(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x00));
	SDOManager->addSDO(new SDOTxPDO2SetNumberOfMappedApplicationObjects(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x00));
	SDOManager->addSDO(new SDOTxPDO3SetNumberOfMappedApplicationObjects(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x00));
	SDOManager->addSDO(new SDOTxPDO4SetNumberOfMappedApplicationObjects(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x00));

	configTxPDOPositionVelocity();
	configTxPDOAnalogCurrent();
}

void DeviceELMOBaseMotor::configRxPDOs()
{

}

void DeviceELMOBaseMotor::configTxPDOPositionVelocity()
{
	SDOManager* SDOManager = bus_->getSDOManager();

	// Transmit PDO 3 Parameter
	///< configure COB-ID Transmit PDO 3
	SDOManager->addSDO(new SDOTxPDO3ConfigureCOBID(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_));
	///< Set Transmission Type: SYNC 0x01
	SDOManager->addSDO(new SDOTxPDO3SetTransmissionType(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x01)); // SYNC
	///< Number of Mapped Application Objects
	SDOManager->addSDO(new SDOTxPDO3SetNumberOfMappedApplicationObjects(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x00));
	///< Mapping "Position actual value"
	SDOManager->addSDO(new SDOTxPDO3SetMapping(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x01, 0x60640020));
	///< Mapping "demand Velocity actual value"
	// velocity demand value 0x606B0020 (working)
	// velocity target value 0x60FF0020
	SDOManager->addSDO(new SDOTxPDO3SetMapping(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x02, 0x60690020));
	///< Number of Mapped Application Objects
	SDOManager->addSDO(new SDOTxPDO3SetNumberOfMappedApplicationObjects(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x02));

}

void DeviceELMOBaseMotor::configTxPDOAnalogCurrent()
{
	SDOManager* SDOManager = bus_->getSDOManager();

	// Transmit PDO 4 Parameter
	///< configure COB-ID Transmit PDO 4
	SDOManager->addSDO(new SDOTxPDO4ConfigureCOBID(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_));
	///< Set Transmission Type: SYNC 0x01
	SDOManager->addSDO(new SDOTxPDO4SetTransmissionType(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x01)); // SYNC
	///< Number of Mapped Application Objects
	SDOManager->addSDO(new SDOTxPDO4SetNumberOfMappedApplicationObjects(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x00));
	///< Mapping "Analog value"
	SDOManager->addSDO(new SDOTxPDO4SetMapping(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x01, 0x22050110));
	///< Mapping "Digital value"
	/*	SDOManager->addSDO(new SDOTxPDO4SetMapping(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x02, 0x22000020));*/

	///< Mapping "actual current value - works!"
	SDOManager->addSDO(new SDOTxPDO4SetMapping(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x02, 0x60780010));

	///< Number of Mapped Application Objects
	SDOManager->addSDO(new SDOTxPDO4SetNumberOfMappedApplicationObjects(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x02));

}


void DeviceELMOBaseMotor::initMotor()
{
	SDOManager* SDOManager = bus_->getSDOManager();
	SDOManager->addSDO(new SDOFaultReset(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_));
	SDOManager->addSDO(new SDOShutdown(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_));
	SDOManager->addSDO(new SDOSwitchOn(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_));
	SDOManager->addSDO(new SDOEnableOperation(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_));
}


void DeviceELMOBaseMotor::setEnableMotor()
{
	SDOManager* SDOManager = bus_->getSDOManager();
	SDOManager->addSDO(new SDOFaultReset(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_));
	SDOManager->addSDO(new SDOShutdown(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_));
	SDOManager->addSDO(new SDOSwitchOn(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_));
	SDOManager->addSDO(new SDOEnableOperation(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_));
}

void DeviceELMOBaseMotor::setDisableMotor()
{
	SDOManager* SDOManager = bus_->getSDOManager();
	SDOManager->addSDO(new SDODisableOperation(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_));
}


bool DeviceELMOBaseMotor::getIsMotorEnabled(bool &flag)
{
	SDOManager* SDOManager = bus_->getSDOManager();


	if (!sdoStatusWord_->hasTimeOut()) {
		if (!sdoStatusWord_->getIsReceived()) {
			if (!sdoStatusWord_->getIsWaiting()) {
				if (!sdoStatusWord_->getIsQueuing()) {
					SDOManager->addSDO((SDOMsgPtr)sdoStatusWord_);
				}
			}
		} else {
			sdoStatusWord_->isEnabled(flag);
			sdoStatusWord_.reset();
			sdoStatusWord_ =  SDOReadStatusWord::SDOReadStatusWordPtr(new SDOReadStatusWord(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_));
			return true;
		}
	}

	return false;

}

bool DeviceELMOBaseMotor::getIsMotorDisabled(bool &flag)
{
	SDOManager* SDOManager = bus_->getSDOManager();


	if (!sdoStatusWordDisabled_->hasTimeOut()) {
		if (!sdoStatusWordDisabled_->getIsReceived()) {
			if (!sdoStatusWordDisabled_->getIsWaiting()) {
				if (!sdoStatusWordDisabled_->getIsQueuing()) {
					SDOManager->addSDO((SDOMsgPtr)sdoStatusWordDisabled_);
				}
			}
		} else {
			sdoStatusWordDisabled_->isDisabled(flag);
			sdoStatusWordDisabled_.reset();
			sdoStatusWordDisabled_ =  SDOReadStatusWord::SDOReadStatusWordPtr(new SDOReadStatusWord(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_));
			return true;
		}
	}

	return false;

}

