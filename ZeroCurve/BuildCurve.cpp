#include "Curve.h"

long  buildCurve (
	char			*currency,
	long			baseDate,
	short			daysToSpot,
	char			*holidayCenter,
	CashInput		*cash,
	FuturesInput	*futures,
	SwapsInput		*swaps
	)
{
	Curve *curve = new Curve(currency, baseDate, daysToSpot, holidayCenter, *cash, *futures, *swaps);
	curve->initProcess();
	curve->processCash();
	curve->processFutures();
	curve->processSwaps();

	Curves::Instance()->insert(curve);

	return curve->getCurveID();
}


double getDiscountFactor(
	long	curveId,
	long	dt
)
{
	// TO DO
	// construct the curve which the discount factor need to be found  
	Curve* curve_pter;
	Curves::Instance()->find(curveId, curve_pter);

	// Now, the cp pointed to the curved where the discout factor need to be found
	KeyPoint Keypoint;
	for (KeyPoints::const_iterator iter_keypoints = curve_pter->firstKeyPoint(); iter_keypoints != curve_pter->endKeyPoint(); ++iter_keypoints)
	{
		// Retrieve the key point to see if the julian date value is matched
		Keypoint = curve_pter->retrieveKeyPoint(iter_keypoints);
		if (Keypoint.first == dt)
		{
			return Keypoint.second;
		}
	}
	// else do the interpolation
	if (curve_pter->interpolate(dt) != 0.0)
		return curve_pter->interpolate(dt);

	// if the curve if or dt does not exist, reject it
	return 0.0;
}
