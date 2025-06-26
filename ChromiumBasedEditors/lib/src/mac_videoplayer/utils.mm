#include "utils.h"

NSColor* NSColorFromHex(unsigned hex_value) {
	CGFloat red   = ((hex_value >> 16) & 0xFF) / 255.0;
	CGFloat green = ((hex_value >> 8)  & 0xFF) / 255.0;
	CGFloat blue  = (hex_value & 0xFF) / 255.0;
	return [NSColor colorWithCalibratedRed:red green:green blue:blue alpha:1.0];
}
