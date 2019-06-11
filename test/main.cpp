#include <mbed.h>
#include <sht15.hpp>
#include <millis.h>

DigitalOut motor (D8);
static SHTx::SHT15 sensor (I2C_SDA, I2C_SCL);
int flag = 0;

int main ()
{
	sensor.setOTPReload (false);
	sensor.setResolution (true);
	sensor.setScale (false);

	millisStart();

	while (1)
	{
		float temp, humi;

		sensor.update();

		temp = sensor.getTemperature();	// Temperature - Air 1
		humi = sensor.getHumidity();	// Humidity - Air 1

		if (temp < 0)
		{
			printf ("Erreur : Capteur non connecté");
			return 1;
		}

		printf ("Température : %.1f°C\n\r", temp);
		printf ("Humidité : %.1f%%\n\r\n\r", humi);

		if (humi < 60 && flag == 0)
		{
			printf ("Motor on\n\r\n\r");
			motor = 1;
			wait (0.3);
			printf ("Motor off\n\r\n\r");
			motor = 0;

			millisStart();
			flag = 1;
		}

		if (flag == 1 && millis() >= 10000)
			flag = 0;


		wait (1);
	}
}	// main