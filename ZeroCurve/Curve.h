#ifndef Curve_H
#define Curve_H

#include "CurveInput.h"
#include "../date/busDate.h"

#include <map>
#include <utility>
#include <math.h>
#include <stdlib.h>

using std::map;
using std::pair;
using std::make_pair;

class Curve;
class SwapsCashFlow;
typedef long CurveIdType;
typedef double DiscountFactorType;
typedef map<CurveIdType,Curve*> CurvesMap;
typedef pair<julianDate,DiscountFactorType> KeyPoint;
typedef map<julianDate,DiscountFactorType> KeyPoints;
typedef vector<SwapsCashFlow> SwapsCashFlows;

class Curves {
private:
	static Curves* _instance;
	static CurveIdType IdSeq;		// for curve Id
	Curves() {}
	CurvesMap m_curves;
public:
	CurveIdType getIdSeq() { return IdSeq; }
	CurveIdType newId() { return ++IdSeq; }
	static Curves* Instance();
	bool insert(Curve*);
	bool find(CurveIdType, Curve*&);
};

class Curve {
private:
	int cashYB, futuresYB, swapsYB, nPerYear;
	busDate dt3M;

protected:
	CurveIdType m_Id;
	KeyPoints m_keyPoints;
public:
	CurrencyType m_currency;
	date m_baseDate;
	int m_daysToSpot;
	HolCtr m_holidayCenter;
	CashInput m_cashInput;
	FuturesInput m_futuresInput;
	SwapsInput m_swapsInput;

	CurveIdType getCurveID() { return m_Id; }

	Curve(char *ccy, long baseDt, int spot, char *holCtr, 
		       CashInput &cin, FuturesInput &fin, SwapsInput &sin);

	bool insertKeyPoint(date,DiscountFactorType);
	KeyPoints::const_iterator firstKeyPoint();
	KeyPoints::const_iterator endKeyPoint() { return m_keyPoints.end(); }
	KeyPoint retrieveKeyPoint(KeyPoints::const_iterator);
	double interpolate(date);		// return 0 if unsuccessful

	void initProcess();
	void processCash();
	void processFutures();

	void processSwaps();

};

long buildCurve (
	char			*currency,
	long			baseDate,
	short			daysToSpot,
	char			*holidayCenter,
	CashInput		*cash,
	FuturesInput	*futures,
	SwapsInput		*swaps
	);

double getDiscountFactor (
	long	curveId,
	long	dt
	);

class SwapsCashFlow {
public:
	busDate	dt;		// fixed leg cash flow date
	double	CF;		// fixed leg cash flow
	double	DF;		// fixed leg cash flow X discount factor
};

#endif