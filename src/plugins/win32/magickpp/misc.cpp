#include "magickpp.hpp"

// Blob
MAGICK_SUBCLASS(Blob) {
	NCB_CONSTRUCTOR(());
	PROP_RO(length);
	// 
}

// CoderInfo
MAGICK_SUBCLASS(CoderInfo) {
	NCB_CONSTRUCTOR((StringT const&));

	PROP_RO(name);
	PROP_RO(description);
	PROP_RO(isReadable);
	PROP_RO(isWritable);
	PROP_RO(isMultiFrame);
}

// Geometry
MAGICK_SUBCLASS(Geometry) {
	NCB_CONSTRUCTOR((StringT const&));

	PROP_RW(unsigned int, width);
	PROP_RW(unsigned int, height);
	PROP_RW(unsigned int, xOff);
	PROP_RW(unsigned int, yOff);
	PROP_RW(bool,         xNegative);
	PROP_RW(bool,         yNegative);
	PROP_RW(bool,         percent);
	PROP_RW(bool,         aspect);
	PROP_RW(bool,         greater);
	PROP_RW(bool,         less);
	PROP_RW(bool,         isValid);

	NCB_PROPERTY_DETAIL_RO(string, Const, StringT, ClassT::operator StringT, ());
}

// TypeMetric
MAGICK_SUBCLASS(TypeMetric) {
	NCB_CONSTRUCTOR(());

	PROP_RO(ascent);
	PROP_RO(descent);
	PROP_RO(textWidth);
	PROP_RO(textHeight);
	PROP_RO(maxHorizontalAdvance);
}


