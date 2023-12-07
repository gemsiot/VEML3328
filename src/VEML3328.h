//© 2023 Regents of the University of Minnesota. All rights reserved.

#ifndef VEML3328_h
#define VEML3328_h

#if defined(ARDUINO) //Conditionally include Arduino.h and Wire.h 
#include <Arduino.h>
#include <Wire.h>
#elif defined(PARTICLE) //Conditionally include Particle.h
#include <Particle.h>
#endif

#include <math.h>

// #define VEML_REG_EN 0x80
// #define VEML_REG_IR 0x94
// #define VEML_REG_RED 0x96
// #define VEML_REG_GREEN 0x98
// #define VEML_REG_BLUE 0x9A

// #define VEML_REG_INT_TIME 0x81
// #define VEML_REG_GAIN 0x8F
#define WHITE_RABBIT_OBJECT

// namespace Commands {
// 			constexpr uint16_t SLEEP = 0xB098;
// 			constexpr uint16_t WAKE = 0x3517;
// 			constexpr uint16_t READ_RH_LP = 0x44DE;
// 			constexpr uint16_t READ_RH = 0x5C24;
// 			constexpr uint16_t READ_TEMP_LP = 0x6458;
// 			constexpr uint16_t READ_TEMP = 0x7CA2;
// 		};



class VEML3328
{
	public:
		struct Regs { // Should be namespace but C++ doesn't allow namespaces in classes
			// static constexpr uint8_t EN = 0x80;
			static constexpr uint8_t IR = 0x08;
			static constexpr uint8_t RED = 0x05;
			static constexpr uint8_t GREEN = 0x06;
			static constexpr uint8_t BLUE = 0x07;
			static constexpr uint8_t CLEAR = 0x04;
			// static constexpr uint8_t INT_TIME = 0x81;
			// static constexpr uint8_t GAIN = 0x9A;
			static constexpr uint8_t COMMAND = 0x00;
		};

		enum class Channel: uint8_t
		{
			Clear = 0,
			Red = 1,
			Green = 2,
			Blue = 3,
			IR = 4,
		};

		VEML3328();
		int begin();
		unsigned int GetValueRaw(Channel Param, bool &State); //Overload with State reading
		unsigned int GetValueRaw(Channel Param); 
		float GetValue(Channel Param, bool &State); //Overload with State reading
		float GetValue(Channel Param);
		float GetLux(); //FIX? Include??
		float GetPAR(float CoefRed = 1.0, float CoefGreen = 0.0, float CoefBlue = 2.0); //Use R+2B as default coef
		int AutoRange();
		int Shutdown(bool State);
		int SetGain(uint8_t GainVal);
		int SetIntTime(uint8_t IntVal);
		int SetGainDG(uint8_t GainValDG);
		int SetSensetivity(bool State);

	private:

		bool TestOverflow();
		int WriteByte(uint8_t Adr, uint8_t Reg, uint8_t Val);
		int WriteWord(uint8_t Adr, uint8_t Reg, uint16_t Val);
		unsigned int ReadWord(uint8_t Adr, uint8_t Reg); 
		unsigned int ReadWord(uint8_t Adr, uint8_t Reg, bool &State); //Overload with State reading
		char nibbleToHex(uint8_t n);
		const int ADR = 0x10; //Default address
		const unsigned long LuxMax = 50331; //Max Lux for high sensetivity 
		uint8_t VEML_GainIndex = 0; //Use 1x gain by default
		uint8_t VEML_IntIndex = 0; //Use count = 1 by default
		uint8_t VEML_GainIndexDG = 0; //Use 1x gain by default
		uint8_t VEML_SenseGain = 0; //Default to high sensetivity 
		// uint8_t VEML_GainValDG = 1; //Default to 1x
		int VEML_IntTimes[4] = {1, 2, 4, 8}; //Register values for integration times (increasing)
		int VEML_GainValsDG[3] = {1, 2, 4}; //Values for DG gain stage
		//Cycles = 256 - VEML_IntTimes[i] = 1, 10, 37, 64, 256
		int VEML_GainVals[4] = {-1, 0, 1, 2}; //Register values for various gain multiples (increasing)
		uint8_t VEML_GainSet[4] = {0b11, 0b00, 0b01, 0b10}; //Register values for gains, coresponding to 1/2, 1, 2, 4
		//Gain = 4^VEML_GainVals[i] = 1, 4, 16, 64
		long VEML_WaitTime[4] = {750, 750, 750, 750}; //Wait times for each respective integration time [ms] //FIX!
		float IrradianceConversion[5] = {14.25, 10.25, 9.75, 8.5, 6.25}; //counts per uW/cm^2 for Clear, Red, Green, Blue, IR at minimal gain and integration (factor of 4 div vs data sheet)
		const unsigned long maxRange = 65536; //Max count for any reading
		// bool VEML_Sensetivity = 0; //Keep track of sensitivity range used 
};

#endif