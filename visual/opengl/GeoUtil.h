
#ifndef __GEO_UTIL_H__
#define __GEO_UTIL_H__


#define TVP_PI 3.14159265358979323846264338327f


static inline float TVPDegToRad( float deg ) {
	return deg * TVP_PI / 180.0f;
}
static inline float TVPRadToDeg( float rad ) {
	return rad / TVP_PI * 180.0f;
}

#endif
