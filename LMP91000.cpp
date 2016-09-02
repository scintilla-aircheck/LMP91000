#include "LMP91000.h"


LMP91000::Configuration::Configuration(
	ETIAGain tia_gain, ERLoad r_load, bool ext_ref, EIntZero int_zero, bool bias_sign,
	EBias bias, bool fet_short, EOpMode op_mode)
{
	TIAGain = tia_gain;
	RLoad = r_load;
	ExtRef = ext_ref;
	IntZero = int_zero;
	BiasSign = bias_sign;
	Bias = bias;
	FETShort = fet_short;
	OpMode = op_mode;
}


LMP91000::LMP91000() {}


LMP91000::LMP91000(byte menb_pin)
{
	// Associate MENB pin
	MENBPin_ = menb_pin;
}


LMP91000::~LMP91000() {}


void LMP91000::Begin()
{
	// Set MENB pin state
	pinMode(MENBPin_, OUTPUT);
	digitalWrite(MENBPin_, HIGH);
}


bool LMP91000::isReady()
{
	return getValue(ERegister::Status);
}


bool LMP91000::isLocked()
{
	return getValue(ERegister::Lock);
}


void LMP91000::isLocked(bool locked)
{
	setValue(ERegister::Lock, locked);
}


LMP91000::ETIAGain LMP91000::TIAGain()
{
	return (ETIAGain)getValue(ERegister::TIACN, 7, 2);
}


void LMP91000::TIAGain(ETIAGain gain)
{
	setValue(ERegister::TIACN, (byte)gain, 7, 2);
}


LMP91000::ERLoad LMP91000::RLoad()
{
	return (ERLoad)getValue(ERegister::TIACN, 3, 0);
}


void LMP91000::RLoad(ERLoad resistance)
{
	setValue(ERegister::TIACN, (byte)resistance, 3, 0);
}


bool LMP91000::ExtRef()
{
	return getValue(ERegister::REFCN, 1, 7);
}


void LMP91000::ExtRef(bool external)
{
	setValue(ERegister::REFCN, external, 1, 7);
}


LMP91000::EIntZero LMP91000::IntZero()
{
	return (EIntZero)getValue(ERegister::REFCN, 3, 5);
}


void LMP91000::IntZero(EIntZero int_z)
{
	setValue(ERegister::REFCN, (byte)int_z, 3, 5);
}


bool LMP91000::BiasSign()
{
	return getValue(ERegister::REFCN, 1, 4);
}


void LMP91000::BiasSign(bool positive)
{
	setValue(ERegister::REFCN, positive, 1, 4);
}


LMP91000::EBias LMP91000::Bias()
{
	return (EBias)getValue(ERegister::REFCN, 13, 0);
}


void LMP91000::Bias(EBias bias)
{
	setValue(ERegister::REFCN, (byte)bias, 13, 0);
}


bool LMP91000::FETShort()
{
	return getValue(ERegister::MODECN, 1, 7);
}


void LMP91000::FETShort(bool enabled)
{
	setValue(ERegister::MODECN, enabled, 1, 7);
}


LMP91000::EOpMode LMP91000::OpMode()
{
	return (EOpMode)getValue(ERegister::MODECN, 7);
}


void LMP91000::OpMode(EOpMode mode)
{
	setValue(ERegister::MODECN, (byte)mode, 7);
}


/// Configures all LMP91000 memory registers
void LMP91000::Configure(Configuration config)
{
	byte buffer;

	isLocked(false);

	// Sets TIACN register
	buffer = (byte)config.RLoad;
	buffer |= (byte)config.TIAGain << 2;
	setRawValue(ERegister::TIACN, buffer);

	// Sets REFCN register
	buffer = (byte)config.Bias;
	buffer |= (byte)config.BiasSign << 4;
	buffer |= (byte)config.IntZero << 5;
	buffer |= (byte)config.ExtRef << 7;
	setRawValue(ERegister::REFCN, buffer);

	// Sets MODECN register
	buffer = (byte)config.OpMode;
	buffer |= (byte)config.FETShort << 7;
	setRawValue(ERegister::MODECN, buffer);

	isLocked(true);
}


/// Sets the LMP91000 memory register to an intended target
byte LMP91000::setRegister(ERegister target)
{
	digitalWrite(MENBPin_, LOW);

	Wire.beginTransmission(I2CAddress);
	Wire.write((byte)target);
	byte error = Wire.endTransmission();

	digitalWrite(MENBPin_, HIGH);

	return error;
}


/// Sets a target LMP91000 memory register to an intended value
byte LMP91000::getRawValue(ERegister target)
{
	// Set target register
	setRegister(target);

	// Read target register
	digitalWrite(MENBPin_, LOW);
	Wire.requestFrom((int)I2CAddress, 1);
	digitalWrite(MENBPin_, HIGH);
	return Wire.read();
}


/// Sets a target LMP91002 memory register to an intended value
byte LMP91000::setRawValue(ERegister target, byte value)
{
	digitalWrite(MENBPin_, LOW);
	Wire.beginTransmission(I2CAddress);
	Wire.write((byte)target);
	Wire.write(value);
	byte error = Wire.endTransmission();
	digitalWrite(MENBPin_, HIGH);

	return error;
}


/// Gets a specific value from a target LMP91002 register
byte LMP91000::getValue(ERegister target, byte mask, byte offset)
{
	mask <<= offset;
	byte ext_value = getRawValue(target);
	return (ext_value & mask) >> offset;
}


/// Sets a specific value to a target LMP91002 memory register
byte LMP91000::setValue(ERegister target, byte value, byte mask, byte offset)
{
	value <<= offset;
	mask <<= offset;
	byte ext_value = getRawValue(target);
	ext_value = (ext_value & ~mask) | value;
	return setRawValue(target, ext_value);
}
