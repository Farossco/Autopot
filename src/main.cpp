#include <mbed.h>
#include <sht15.hpp>

SHTx::SHT15 soilSensor (I2C_SDA, I2C_SCL);

int main ()
{
	soilSensor.setOTPReload (false);
	soilSensor.setResolution (true);
	soilSensor.setScale (false);

	while (1)
	{
		soilSensor.update();

		float temp = (soilSensor.getTemperature());	// Temperature - Sol
		float humi = (soilSensor.getHumidity());	// Humidity - Sol

		printf ("Température : %.2f°C\n", temp);
		printf ("Humidité : %.0f%%\n\n\n", humi);

		wait (2);
	}
}