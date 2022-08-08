#ifndef CurveInput_H
#define CurveInput_H

#include <string>
#include <vector>

using std::string;
using std::vector;
using std::pair;

typedef double RateType;
typedef string CurrencyType;
typedef double FuturesPriceType;
typedef string CashMaturityType;
typedef long FuturesMaturityType;
typedef string SwapsMaturityType;
typedef string SwapsFreqType;
typedef string CashBasis;
typedef string FuturesBasis;
typedef string SwapsBasis;
typedef pair<CashMaturityType,RateType> CashPoint;
typedef pair<FuturesMaturityType,FuturesPriceType> FuturesPoint;
typedef pair<SwapsMaturityType,RateType> SwapsPoint;
typedef vector<CashPoint> CashPoints;
typedef vector<FuturesPoint> FuturesPoints;
typedef vector<SwapsPoint> SwapsPoints;

class CashInput {
public:
	CashBasis m_cashBasis;
	CashPoints m_cashPoints;
};

class FuturesInput {
public:
	FuturesBasis m_futuresBasis;
	FuturesPoints m_futuresPoints;
};

class SwapsInput {
public:
	SwapsBasis m_swapsBasis;
	SwapsFreqType m_swapsFreq;
	SwapsPoints m_swapsPoints;
};

#endif
