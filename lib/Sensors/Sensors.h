#ifndef SENSORS_H
#define SENSORS_H

#include "sht15.hpp"

class Sensors
{
public:
	Sensors();
	void initSensors ();
	void getSensorsIntValue (int * temp, int * humi);

	SHTx::SHT15 sensor;
};

extern Sensors sensors;

#endif	// ifndef SENSORS_H