#ifndef CARP_WIRINGPI_BIND_INCLUDED
#define CARP_WIRINGPI_BIND_INCLUDED

#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <softPwm.h>
#include <softTone.h>
#include <pcf8591.h>
#include <pcf8574.h>
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
			
			.endNamespace();
	}
};

#endif