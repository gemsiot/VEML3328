#include <VEML3328.h>

VEML3328::VEML3328() 
{

}

int VEML3328::begin()
{
	#if defined(PARTICLE)
		if(!Wire.isEnabled()) Wire.begin(); //Only initialize I2C if not done already //INCLUDE FOR USE WITH PARTICLE 
	#else
		Wire.begin(); //For Arduino, initialize Wire as normally 
	#endif

	Wire.beginTransmission(ADR);
	Wire.write(0x00);
	Wire.write(0x00); //Set to default operation
	Wire.write(0x0C);
	return Wire.endTransmission(); //Return I2C status, just to check if device is connected to bus 

}

unsigned int VEML3328::GetValueRaw(Channel Param)
{
	unsigned int Val = 0; //General value to be returned 
	bool State = false; //Local I2C state variable 
	delay(VEML_WaitTime[VEML_IntIndex]); //Wait for new data //DEBUG!
	switch(Param) {
		case Channel::Red:
			Val = ReadWord(ADR, Regs::RED, State); //Grab red value
			break;

		case Channel::Green:
			Val = ReadWord(ADR, Regs::GREEN, State); //Grab green value
			break;
		
		case Channel::Blue:
			Val = ReadWord(ADR, Regs::BLUE, State); //Grab blue value
			break;

		case Channel::IR:
			Val = ReadWord(ADR, Regs::IR, State); //Grab blue value
			break;

		case Channel::Clear:
			Val = ReadWord(ADR, Regs::CLEAR, State); //Grab blue value
			break;
		
		default:
			#if defined(WHITE_RABBIT_OBJECT)
				Serial.print("Default val call"); 
			#endif
			Val = 0; //Set for zero error Otherwise //FIX??
			break;
	}

	#if defined(WHITE_RABBIT_OBJECT)
		Serial.print("Read Error = "); Serial.println(State);  
	#endif

	if(State) return 0; //FIX! Return error value if I2C failue
	else return Val; //Return resultant value
}

unsigned int VEML3328::GetValueRaw(Channel Param, bool &State)
{
	unsigned int Val = 0; //General value to be returned 
	delay(VEML_WaitTime[VEML_IntIndex]); //Wait for new data //DEBUG!
	switch(Param) {
		case Channel::Red:
			Val = ReadWord(ADR, Regs::RED, State); //Grab red value
			break;

		case Channel::Green:
			Val = ReadWord(ADR, Regs::GREEN, State); //Grab green value
			break;
		
		case Channel::Blue:
			Val = ReadWord(ADR, Regs::BLUE, State); //Grab blue value
			break;

		case Channel::IR:
			Val = ReadWord(ADR, Regs::IR, State); //Grab blue value
			break;

		case Channel::Clear:
			Val = ReadWord(ADR, Regs::CLEAR, State); //Grab blue value
			break;
		
		default:
			#if defined(WHITE_RABBIT_OBJECT)
				Serial.print("Default val call"); 
			#endif
			Val = 0; //Set for zero error Otherwise //FIX??
			break;
	}

	if(State) return 0; //FIX! Return error value if I2C failue
	else return Val; //Return resultant value
}

float VEML3328::GetValue(Channel Param)
{
	float Val = 0; //General value to be returned 
	bool State = false; //Local I2C state variable 
	Val = float(GetValueRaw(Param, State)); //Grab the raw value from the desied channel 
	#if defined(WHITE_RABBIT_OBJECT)
		Serial.print("Raw Val = "); Serial.println(Val/65536.0);  
		Serial.print("Gain Val = "); Serial.println(pow(2, VEML_GainVals[VEML_GainIndex]));  
		Serial.print("Int Val = "); Serial.println((1.0/float(VEML_IntTimes[VEML_IntIndex])));  
	#endif
	if(State) return -9999.0; //If I2C error, return error condition
	else { //Otherwise calculate and return value 
		Val = GetValueRaw(Param)*(1.0/float(VEML_GainValsDG[VEML_GainIndexDG]))*(0.5/pow(2, VEML_GainVals[VEML_GainIndex]))*(1.0/float(VEML_IntTimes[VEML_IntIndex]))*float(1.0 + 3.0*VEML_SenseGain)*(1.0/IrradianceConversion[static_cast<int>(Param)]); //Normalize to base value and multiply by gain
		return Val; //Return resultant value
	}
}

float VEML3328::GetValue(Channel Param, bool &State)
{
	float Val = 0; //General value to be returned 
	Val = float(GetValueRaw(Param, State)); //Grab the raw value from the desied channel 
	#if defined(WHITE_RABBIT_OBJECT)
		Serial.print("Raw Val = "); Serial.println(Val/65536.0);  
		Serial.print("Gain Val = "); Serial.println(pow(2, VEML_GainVals[VEML_GainIndex]));  
		Serial.print("Int Val = "); Serial.println((1.0/float(VEML_IntTimes[VEML_IntIndex])));  
	#endif
	if(State) return -9999.0; //If I2C error, return error condition
	else { //Otherwise calculate and return value 
		Val = GetValueRaw(Param)*(1.0/float(VEML_GainValsDG[VEML_GainIndexDG]))*(0.5/pow(2, VEML_GainVals[VEML_GainIndex]))*(1.0/float(VEML_IntTimes[VEML_IntIndex]))*float(1.0 + 3.0*VEML_SenseGain)*(1.0/IrradianceConversion[static_cast<int>(Param)]); //Normalize to base value and multiply by gain
		return Val; //Return resultant value
	}
}

float VEML3328::GetLux()
{
	//FIX! Is this reasonable? 
	// return -9999.0; //FIX!
	return GetValueRaw(Channel::Green)*(1.0/float(VEML_GainValsDG[VEML_GainIndexDG]))*(0.5/pow(2, VEML_GainVals[VEML_GainIndex]))*(1.0/float(VEML_IntTimes[VEML_IntIndex]))*(float(LuxMax*(1.0 + 3.0*VEML_SenseGain))/65536.0); //Take green channel and scale by max lux
}

float VEML3328::GetPAR(float CoefRed, float CoefGreen, float CoefBlue)
{
	//FIX! 
	//Compenstae for gain value
	//Compensate for integration time
	//Compensate for spectral responsivity 
	// AutoRange(); //FIX! Use everytime??
	bool State[3] = {false};
	float PAR = GetValue(Channel::Red, State[0])*CoefRed + GetValue(Channel::Green, State[1])*CoefGreen + GetValue(Channel::Blue, State[2])*CoefBlue; //Multiple by coefficients
	if(State[0] || State[1] || State[2]) return -9999.0; //Return error condition
	else { //Otherwise calculate and return 
		PAR = (PAR/65536.0)*pow(2, VEML_GainVals[VEML_GainIndex])*13.3; //Normalize to base value and multiply by gain, mnultiply by counts/uW/cm^2
		return PAR;
	} 
}

int VEML3328::AutoRange() 
{
	// WriteByte(ADR, Regs::EN, 0x03); //Enable sensor, turn on power, ADC
	Shutdown(false);
	SetSensetivity(1); //Set to low sensetivity by default, FIX??
	// WriteByte(ADR, Regs::INT_TIME, VEML_IntTimes[0]); //Set default integration time (count = 1)
	SetIntTime(0); //Set into to default
	// WriteByte(ADR, Regs::GAIN, 0x00); //Set default gain (1x)
	SetGain(0); //Set gain to lowest
	delay(VEML_WaitTime[VEML_IntIndex]); //Wait for new data
	VEML_GainIndex = 0; //Reset gain/int vals
	VEML_IntIndex = 0;

	if(TestOverflow()) { //Within default range, no need for auto-range
		VEML_GainIndex = 0;
		VEML_IntIndex = 0;
	}

	else {
		boolean InRange = false; //Used to keep track of range status
		boolean OverflowState = true; //Used to track overflow state to prevent requirment to perform multiple reads

		uint8_t PrevIntIndex = 0; //used to track the last set of gain/integration values used
		uint8_t PrevGainIndex = 0;
		unsigned long LocalTime = millis();
		while(!InRange && (millis() - LocalTime) < 10000) { //Spend at most 10 seconds 


		OverflowState = TestOverflow(); //Check for overflow single time

		if(OverflowState) { //If overflowed, then use last set of values
			VEML_IntIndex = PrevIntIndex;
			VEML_GainIndex = PrevGainIndex;
			InRange = true; //Break from loop
		}
		if(!OverflowState && VEML_IntIndex < 3) { //If not in range and int time is less than max
			VEML_IntIndex += 1; //Incrment integration time, retry
		}

		if(!OverflowState && VEML_IntIndex >= 3 && VEML_GainIndex < 3) { //If no overflow, int at max, and gain not exceeded
			VEML_IntIndex = 0; //Reset to minimum
			VEML_GainIndex += 1; //Increment gain value
		}

		if(!OverflowState && VEML_IntIndex == 3 && VEML_GainIndex == 3) { //Max value
			InRange = true; //Exit loop
		}

		PrevIntIndex = VEML_IntIndex; //Store values after incrementing
		PrevGainIndex = VEML_GainIndex;
		#if defined(WHITE_RABBIT_OBJECT)
		Serial.print("Gain = ");
		Serial.print(VEML_GainIndex);
		Serial.print(" Int = ");
		Serial.println(VEML_IntIndex);
		Serial.print(" OVF = ");
		Serial.println(OverflowState);
		#endif
		}
	}
	#if defined(WHITE_RABBIT_OBJECT)
	Serial.print("Gain Res = ");
	Serial.print(VEML_GainIndex);
	Serial.print(" Int Res = ");
	Serial.println(VEML_IntIndex);
	#endif
	return 0; //FIX!

	
}

bool VEML3328::TestOverflow()
{
	// WriteByte(ADR, Regs::INT_TIME, VEML_IntTimes[VEML_IntIndex]); //Update integration time
	// WriteByte(ADR, Regs::GAIN, VEML_GainVals[VEML_GainIndex]); //Update gain value
	SetIntTime(VEML_IntIndex); //Update integration time
	SetGain(VEML_GainIndex); //Update gain value

	delay(VEML_WaitTime[VEML_IntIndex]);
	// long OverflowCount = min((256 - long(VEML_IntTimes[VEML_IntIndex]))*1024, 65535); //Find the maximum count value for a given integration time
	// long OverflowCount = 65536; //DEBUG!
	// #if defined(WHITE_RABBIT_OBJECT)
	// Serial.print("Overflow val = "); Serial1.println(OverflowCount);
	// #endif
	// float EdgeRange = 0.9; //Acceptable range utilization
	// long MaxCount = OverflowCount >> 1; //Maximum count is half of overflow value, this is maximum desired range to utilize
	long MaxCount = 32768; //DEBUG!
	for(int i = 0; i < 4; i++) {
	if(ReadWord(ADR, Regs::CLEAR + i) > MaxCount) return true; //If there is overflow on any channel, return fail
	}
	return false; //Otherwise return no overflow
}

int VEML3328::SetSensetivity(bool State) //State = 1 -> low sensetivity, State = 0 -> high sensetivity 
{
	bool Error = 1; //Assume error
	int Val = ReadWord(ADR, Regs::COMMAND, Error);
	if(Error) return -1; //Indicate error to system

	if(!State) Val = Val & 0xFFBF; //Clear sensetivity bit
	if(State) Val = Val | 0x0040; //Set sensetivity bit
	return WriteWord(ADR, Regs::COMMAND, Val); //Write adjusted value back
}

int VEML3328::Shutdown(bool State)
{
	bool Error = 1; //Assume error
	int Val = ReadWord(ADR, Regs::COMMAND, Error);
	if(Error) return -1; //Indicate error to system

	if(!State) Val = Val & 0x7FFE; //Clear shutdown bits
	if(State) Val = Val | 0x8001; //Set shutdown bits
	return WriteWord(ADR, Regs::COMMAND, Val); //Write adjusted value back
}



int VEML3328::SetGain(uint8_t GainVal) //Set gain val from 0 to 3 (index of gain value), coresponding to 1/2, 1, 2, 4
{
	if(GainVal > 3) return -2; //Indicate input error 
	bool Error = 1; //Assume error
	int ValIn = ReadWord(ADR, Regs::COMMAND, Error);
	if(Error) return -1; //Indicate error to system

	int Val = ValIn & 0xF3FF; //Clear gain bits
	Val = Val | VEML_GainSet[GainVal] << 10;

	#if defined(WHITE_RABBIT_OBJECT)
		// Serial.print("I2C Error = "); Serial.println(State);  
		Serial.print("GAIN = "); Serial.print(GainVal);
		Serial.print(" CMD Read = 0x"); Serial.print(ValIn, HEX);
		Serial.print(" CMD Out = 0x"); Serial.println(Val, HEX);   
	#endif

	return WriteWord(ADR, Regs::COMMAND, Val); //Write modiffied value back
}

int VEML3328::SetIntTime(uint8_t IntVal) //Set integration time from 0 to 3 (index of integration time value), coresponding to 50, 100, 200, 400 ms
{
	if(IntVal > 3) return -2; //Indicate input error 
	bool Error = 1; //Assume error
	int ValIn = ReadWord(ADR, Regs::COMMAND, Error);
	if(Error) return -1; //Indicate error to system

	int Val = ValIn & 0xFFCF; //Clear integration time bits
	Val = Val | (IntVal << 4);

	#if defined(WHITE_RABBIT_OBJECT)
		// Serial.print("I2C Error = "); Serial.println(State);  
		Serial.print("INT = "); Serial.print(IntVal);
		Serial.print(" CMD Read = 0x"); Serial.print(ValIn, HEX);
		Serial.print(" CMD Out = 0x"); Serial.println(Val, HEX);   
	#endif
	return WriteWord(ADR, Regs::COMMAND, Val); //Write modiffied value back
}

int VEML3328::WriteByte(uint8_t Adr, uint8_t Reg, uint8_t Val) 
{
	Wire.beginTransmission(Adr);
	Wire.write(Reg);
	Wire.write(Val);
	return Wire.endTransmission(); //Return error condition
}

int VEML3328::WriteWord(uint8_t Adr, uint8_t Reg, uint16_t Val) 
{
	Wire.beginTransmission(Adr);
	Wire.write(Reg);
	Wire.write(Val & 0xFF); //Write low byte
	Wire.write(Val >> 8); //Write high byte
	return Wire.endTransmission(); //Return error condition
}

unsigned int VEML3328::ReadWord(uint8_t Adr, uint8_t Reg)
{
	uint16_t TempData1 = 0; //Temp data for stoage/concat
	uint16_t TempData2 = 0;
	Wire.beginTransmission(Adr); //Set pointer to begining of word
	Wire.write(Reg);
	Wire.endTransmission(false);

	Wire.requestFrom(Adr, 2); //Get word
	TempData1 = Wire.read(); //Read low byte
	TempData2 = Wire.read(); //Read upper byte
	#if defined(WHITE_RABBIT_OBJECT)
		// Serial.print("I2C Error = "); Serial.println(State);  
		Serial.print("I2C Data = "); Serial.println((TempData2 << 8) | TempData1, HEX);  
	#endif
	return (TempData2 << 8) | TempData1; //Concatonate values, retun
}

unsigned int VEML3328::ReadWord(uint8_t Adr, uint8_t Reg, bool &State)
{
	uint16_t TempData1 = 0; //Temp data for stoage/concat
	uint16_t TempData2 = 0;
	Wire.beginTransmission(Adr); //Set pointer to begining of word
	Wire.write(Reg);
	uint8_t Error = Wire.endTransmission(false);

	if(Error == 0) State = false;
	else State = true; //If anything but good I2C response, state error

	Wire.requestFrom(Adr, 2); //Get word
	TempData1 = Wire.read(); //Read low byte
	TempData2 = Wire.read(); //Read upper byte

	#if defined(WHITE_RABBIT_OBJECT)
		Serial.print("I2C Error = "); Serial.println(State);  
		Serial.print("I2C Data = "); Serial.println((TempData2 << 8) | TempData1, HEX);  
	#endif
	return (TempData2 << 8) | TempData1; //Concatonate values, retun
}

// int VEML3328::ReadByte(uint8_t Adr, uint8_t Reg)
// {
// 	uint8_t TempData = 0; //Temp data for stoage/concat
// 	Wire.beginTransmission(Adr); //Set pointer to begining of word
// 	Wire.write(Reg);
// 	int Error = Wire.endTransmission();

// 	Wire.requestFrom(Adr, 1); //Get word
// 	TempData = Wire.read(); //Read low byte
// 	if(Error != 0) return -1;
// 	else return TempData; //Concatonate values, retun
// }

char VEML3328::nibbleToHex(uint8_t n)
{
	if (n <= 9) { return '0' + n; }
	else { return 'a' + (n - 10); }
}