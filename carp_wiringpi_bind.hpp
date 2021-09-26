#ifndef CARP_WIRINGPI_BIND_INCLUDED
#define CARP_WIRINGPI_BIND_INCLUDED

#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <softPwm.h>
#include <softTone.h>
#include <pcf8591.h>
#include <pcf8574.h>
#include <pca9685/src/pca9685.h>
#include "carp_lua.hpp"

class CarpWiringPiBind
{
public:
	static void Bind(lua_State* l_state)
	{
		luabridge::getGlobalNamespace(l_state)
			.beginNamespace("wiringPi")
			.addFunction("wiringPiSetup", wiringPiSetup)
			.addFunction("wiringPiSetupSys", wiringPiSetupSys)
			.addFunction("pinMode", pinMode)
			.addFunction("softPwmCreate", softPwmCreate)
			.addFunction("softPwmWrite", softPwmWrite)
			.addFunction("pwmWrite", pwmWrite)
			.addFunction("softToneCreate", softToneCreate)
			.addFunction("softToneWrite", softToneWrite)
			.addFunction("digitalWrite", digitalWrite)
			.addFunction("digitalRead", digitalRead)
			.addFunction("pcf8591Setup", pcf8591Setup)
			.addFunction("pcf8574Setup", pcf8574Setup)
			.addFunction("analogWrite", analogWrite)
			.addFunction("analogRead", analogRead)
			.addFunction("delay", delay)
			.addFunction("delayMicroseconds", delayMicroseconds)
			.addFunction("pullUpDnControl", pullUpDnControl)
			.addFunction("wiringPiI2CSetup", wiringPiI2CSetup)
			.addFunction("wiringPiI2CWrite", wiringPiI2CWrite)
			.addFunction("wiringPiI2CRead", wiringPiI2CRead)

			.addFunction("pca9685Setup", pca9685Setup)
			.addFunction("pca9685PWMFreq", pca9685PWMFreq)
			.addFunction("pca9685PWMReset", pca9685PWMReset)
			.addFunction("pca9685PWMWrite", pca9685PWMWrite)
			.addFunction("pca9685PWMRead", Pca9685PWMRead)
			.addFunction("pca9685FullOn", pca9685FullOn)
			.addFunction("pca9685FullOff", pca9685FullOff)
			
			.endNamespace();
	}

	static int Pca9685PWMRead(lua_State* l_state)
	{
		int fd = (int)luaL_checkinteger(l_state, 1);
		int pin = (int)luaL_checkinteger(l_state, 2);
		int on = 0;
		int off = 0;
		pca9685PWMRead(fd, pin, &on, &off);

		lua_pushinteger(l_state, on);
		lua_pushinteger(l_state, off);
		return 2;
	}
};

#endif