#ifndef CURVEDATA_H
#define CURVEDATA_H

#include "Curve.h"

class CurveData {
public:
	char			*currency;
	long			baseDate;
	short			daysToSpot;
	char			*holidayCenter;
	CashInput		*cash;
	FuturesInput	*futures;
	SwapsInput		*swaps;
public:
	bool load(const char* filename);
};

#endif