#include "Sensors.h"

Sensors::Sensors() : sensor (I2C_SDA, I2C_SCL)
{ }

void Sensors::initSensors ()
{
	sensor.setOTPReload (false);
	sensor.setResolution (true);
	sensor.setScale (false);
}

void Sensors::getSensorsIntValue (int * temp, int * humi)
{
	sensor.update();

	*temp = (int) (sensor.getTemperature() * 10.0);	// Temperature - Sol
	*humi = (int) (sensor.getHumidity());			// Humidity - Sol
}

Sensors sensors = Sensors();