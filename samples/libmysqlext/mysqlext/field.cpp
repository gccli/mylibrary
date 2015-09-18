#include "field.h"

using namespace MySQL;
Field::Field(const char n[], unsigned l, unsigned w)
{
	name.assign(n, l);
	width = w;
	
	value = new char [width];
	memset(value, 0, width);
	is_null = false;
}

Field::~Field(void) 
{
	if (value)
		delete[] value;
}

Field::operator char(void) {
	return value[0];
}

Field::operator char *(void) {
	return value;
}

Field::operator const char *(void) {
	return value;
}

Field::operator int(void) {
	return strtol(value, NULL, 10);
}

Field::operator unsigned(void) {
	return strtoul(value, NULL, 10);
}

Field::operator long(void) {
	return strtol(value, NULL, 10);
}

Field::operator unsigned long(void) {
	return strtoul(value, NULL, 10);
}
