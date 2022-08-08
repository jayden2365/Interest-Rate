#include "Curve.h"

Curves* Curves::_instance = 0;
CurveIdType Curves::IdSeq = 0;

Curves*
Curves::Instance() {
	if (_instance == 0) {
		_instance = new Curves;
	}
	return _instance;
}

bool
Curves::insert(Curve* curve) {
	pair<CurveIdType,Curve*> entry = make_pair(curve->getCurveID(),curve);
	pair<CurvesMap::const_iterator, bool> result = m_curves.insert(entry);
	return result.second;
}

bool
Curves::find(CurveIdType Id, Curve *&c) {
	CurvesMap::iterator iterC =  m_curves.find(Id);
	if (iterC != m_curves.end()) {
		c = (*iterC).second;
		return true;
	} else {
		return false;
	}
}

Curve::Curve(char *ccy, long baseDt, int spot, char *holCtr, 
		       CashInput &cin, FuturesInput &fin, SwapsInput &sin)
	: m_currency(ccy), m_baseDate(baseDt), m_daysToSpot(spot), m_holidayCenter(holCtr), 
	  m_cashInput(cin), m_futuresInput(fin), m_swapsInput(sin) {

	m_Id = Curves::Instance()->newId();
	Curves::Instance()->insert(this);
}

bool
Curve::insertKeyPoint(date dt, DiscountFactorType df) {
	KeyPoint entry = make_pair(dt.getJulianDate(), df);
	pair<KeyPoints::const_iterator, bool> result = m_keyPoints.insert(entry);
	return result.second;
}

KeyPoints::const_iterator
Curve::firstKeyPoint() {
	if (m_keyPoints.size() > 0)
		return m_keyPoints.begin();
	else
		return m_keyPoints.end();
}

KeyPoint 
Curve::retrieveKeyPoint(KeyPoints::const_iterator ki) {
	if (ki == m_keyPoints.end())
		return make_pair(0L,0.0);
	else
		return *ki;
}

void
Curve::initProcess(){
	const char *ccstr, *p;
	char cstr[10];
	char *next_token;

	// Get cashYB
	ccstr = m_cashInput.m_cashBasis.c_str();
	strcpy_s(cstr, 10, ccstr);
	p = strtok_s(cstr,"/",&next_token);
	p = strtok_s(NULL," ",&next_token);
	cashYB = atoi(p);

	// Get futuresYB
	ccstr = m_futuresInput.m_futuresBasis.c_str();
	strcpy_s(cstr, 10, ccstr);
	p = strtok_s(cstr,"/", &next_token);
	p = strtok_s(NULL," ", &next_token);
	futuresYB = atoi(p);

	// Get swapsYB
	ccstr = m_swapsInput.m_swapsBasis.c_str();
	strcpy_s(cstr, 10, ccstr);
	p = strtok_s(cstr,"/",&next_token);
	p = strtok_s(NULL," ",&next_token);
	swapsYB = atoi(p);

	// Get nPerYear for swaps
	if (m_swapsInput.m_swapsFreq.compare("Annually") == 0)
		nPerYear = 1;
	else if (m_swapsInput.m_swapsFreq.compare("Semi-Annually") == 0)
		nPerYear = 2;
	else if (m_swapsInput.m_swapsFreq.compare("Quarterly") == 0)
		nPerYear = 4;

}

double
Curve::interpolate(date dt) {
	// TO DO

	// Because we can not interpolate the date smaller than base than, if the date is small than base date reject it and if date == base date we do not need to do anything
	if (dt.getJulianDate() <= m_baseDate.getJulianDate())
	{
		return 0.0;
	}

	// Now, we check the if date if fall bewteen any two juliandate exist in the keypoints otherwise reject it
	for (KeyPoints::const_iterator current_keypoint = firstKeyPoint(); current_keypoint != endKeyPoint(); ++current_keypoint)
	{
		KeyPoint retrieved_key_point = retrieveKeyPoint(current_keypoint);

		// if the dt juliandate is contained in the keypoints list, reject it. Because we do not need to do the interpolation
		if (dt.getJulianDate() == retrieved_key_point.first)
		{
			return 0.0;
		}

		// find the smallest julian date just larger than the dt juliandate else choose the next keypoint
		if (retrieved_key_point.first < dt.getJulianDate())
		{
			continue;
		}
		// Use this and get the previous julian date and discount factor for interpolating
		else if (retrieved_key_point.first > dt.getJulianDate())
		{
			// Get the largest julian date in key points just small than the dt julian date
			KeyPoints::const_iterator previous_keypoint = current_keypoint;
			--previous_keypoint;
			KeyPoint retrieved_previous_key_point = retrieveKeyPoint(previous_keypoint);
			// caluculate the current discount factor and return it
			// first import the libray math.h to calculate the power function
			#include <math.h> 
			// convert all the julian date to double for calculation
			return (retrieved_previous_key_point.second * pow((retrieved_key_point.second / retrieved_previous_key_point.second), (static_cast<double>(dt.getJulianDate()) - static_cast<double>(retrieved_previous_key_point.first)) / (static_cast<double>(retrieved_key_point.first) - static_cast<double>(retrieved_previous_key_point.first))));
		}
	}

	// if there is no juilian date in keypoint between and after dt juliandate, reject it. we can not do the interpolation
	return 0.0;
}

void
Curve::processCash() {
	// TO DO

	// Set Holiday Center
	const HolCtrs holiday_center = { m_holidayCenter };

	// Creat a busDate variable for base case, "ON", "TN" and "3M" and resepctive discount factors
	busDate base_dt(m_baseDate, NO_ROLL, &holiday_center);

	busDate on_busdate(base_dt.getJulianDate(), MOD_FOLLOW, &holiday_center);
	busDate tn_busdate(base_dt.getJulianDate(), MOD_FOLLOW, &holiday_center);
	busDate m3_busdate(base_dt.getJulianDate(), MOD_FOLLOW, &holiday_center);
	double base_df = 1;
	double on_cash_df;
	double tn_cash_df;
	double m3_cash_df;

	// Insert the base case
	insertKeyPoint(base_dt, base_df);

	// Iterate until the last case
	for (vector<CashPoint>::iterator iter_cash = m_cashInput.m_cashPoints.begin(); iter_cash != m_cashInput.m_cashPoints.end(); ++iter_cash)
	{
		if ((*iter_cash).first == static_cast<CashMaturityType>("ON"))
		{
			// Get the ON date by adding one day from base date
			on_busdate.setYMD(base_dt.year(), base_dt.month(), base_dt.day() + 1);

			// Check if it's busdate is larger than the base busdate
			while (!on_busdate.isBusDay(on_busdate.getJulianDate(), holiday_center))
			{
				on_busdate.rollBusDate();
				if (on_busdate.getJulianDate() < base_dt.getJulianDate())
				{
					if (on_busdate.month() == 12)
					{
						on_busdate.setYMD(on_busdate.year() + 1, 1, 1);
					}
					else
					{
						on_busdate.setYMD(on_busdate.year(), on_busdate.month() + 1, 1);
					}
				}
			}

			// Calculate the discount factor
			on_cash_df = base_df / (1 + (*iter_cash).second * (on_busdate.getJulianDate() - base_dt.getJulianDate()) / cashYB);

			// Insetthe discount factor
			insertKeyPoint(on_busdate, on_cash_df);
		}
		else if ((*iter_cash).first == static_cast<CashMaturityType>("TN"))
		{
			// Update TN date by latest ON date and Get the TN date by adding one day from ON date
			tn_busdate.setYMD(on_busdate.year(), on_busdate.month(), on_busdate.day() + 1);

			// Check if it's busdate is larger than the base busdate
			while (!tn_busdate.isBusDay(tn_busdate.getJulianDate(), holiday_center))
			{
				tn_busdate.rollBusDate();
				if (tn_busdate.getJulianDate() < base_dt.getJulianDate())
				{
					if (tn_busdate.month() == 12)
					{
						tn_busdate.setYMD(tn_busdate.year() + 1, 1, 1);
					}
					else
					{
						tn_busdate.setYMD(tn_busdate.year(), tn_busdate.month() + 1, 1);
					}
				}
			}

			// Calculate the discount factor
			tn_cash_df = on_cash_df / (1 + (*iter_cash).second * (tn_busdate.getJulianDate() - on_busdate.getJulianDate()) / cashYB);

			// Insetthe discount factor
			insertKeyPoint(tn_busdate, tn_cash_df);
		}
		else if ((*iter_cash).first == static_cast<CashMaturityType>("3M"))
		{
			// Update 3M date by latest TN date and Get the TN date by adding one day from ON date
			m3_busdate.setYMD(tn_busdate.year(), tn_busdate.month() + 3, tn_busdate.day());

			// Check if the date isBusdate otherwise roll to the next business day
			while (!m3_busdate.isBusDay(m3_busdate.getJulianDate(), holiday_center))
			{
				m3_busdate.rollBusDate();
				if (m3_busdate.getJulianDate() < base_dt.getJulianDate())
				{
					if (m3_busdate.month() == 12)
					{
						m3_busdate.setYMD(m3_busdate.year() + 1, 1, 1);
					}
					else
					{
						m3_busdate.setYMD(m3_busdate.year(), m3_busdate.month() + 1, 1);
					}
				}
			}

			// Calculate the discount factor
			m3_cash_df = tn_cash_df / (1 + (*iter_cash).second * (m3_busdate.getJulianDate() - tn_busdate.getJulianDate()) / cashYB);

			// Insetthe discount factor
			insertKeyPoint(m3_busdate, m3_cash_df);
		}
	}

}

void
Curve::processFutures() {
	// TO DO

	// Set Holiday Center
	const HolCtrs holiday_center = { m_holidayCenter };

	// Generate a previous futures factor for calculating the current discount factor
	double previous_future_discount_factor;

	// Generat a previous futures date for calculating the current discount factor
	busDate previous_futures_date;

	// Generat a previous future price
	double previous_futures_price;

	// Iterate until the last case 
	for (vector<FuturesPoint>::iterator iter_futures = m_futuresInput.m_futuresPoints.begin(); iter_futures != m_futuresInput.m_futuresPoints.end(); ++iter_futures)
	{
		if (iter_futures == m_futuresInput.m_futuresPoints.begin())
		{
			// Create a varible to store the 3M discount factor and 3M julianDate
			double m3_df;
			double m3_date;

			// Set the current_future_date, get the current futures price and current futures date and current discount factor
			busDate current_futures_date((*iter_futures).first, NO_ROLL, &holiday_center);
			double current_future_price = (*iter_futures).second;
			double current_futures_julian_date = (*iter_futures).first;
			double current_discount_factor;

			// Retrive the next future to calculate the 1st futures discount factor and increment it;
			vector<FuturesPoint>::iterator next_iter_future = iter_futures;
			++next_iter_future;
			// Get the date of the next futures;
			double next_futures_date = (*next_iter_future).first;

			// Get the 3M date and discount factor for calculating the 1st futures discount factors becuase the 3M must be saved at the last
			KeyPoints::const_iterator keypoint = endKeyPoint();
			--keypoint;
			KeyPoint retrieved_key_pt = retrieveKeyPoint(keypoint);
			m3_date = retrieved_key_pt.first;
			m3_df = retrieved_key_pt.second;

			// Calculate the current discount factor and import the library math for calculating the power
			#include <math.h> 
			current_discount_factor = (m3_df)* pow((1 + ((100 - current_future_price) / 100) * ((next_futures_date - current_futures_julian_date) / futuresYB)), ((m3_date - current_futures_julian_date) / (next_futures_date - current_futures_date)));

			// Insert the keypoint
			insertKeyPoint(current_futures_date, current_discount_factor);

			// Update the current futures data to the previous futures data
			previous_futures_date = current_futures_date;
			previous_future_discount_factor = current_discount_factor;
			previous_futures_price = current_future_price;

		}

		// For the rest of the futures
		// Set the current_future_date, get the current futures price, date and current discount factor
		busDate current_futures_date((*iter_futures).first, NO_ROLL, &holiday_center);
		double current_futures_julian_date = (*iter_futures).first;
		double current_discount_factor;
		double current_future_price = (*iter_futures).second;

		// Set the previous julian date
		double previous_futures_julian_date = previous_futures_date.getJulianDate();

		current_discount_factor = previous_future_discount_factor / (1 + (100 - previous_futures_price) / 100 * (current_futures_julian_date - previous_futures_julian_date) / futuresYB);
		insertKeyPoint(current_futures_date, current_discount_factor);

		// Update the current futures data to the previous futures data
		previous_futures_date = current_futures_date;
		previous_future_discount_factor = current_discount_factor;
		previous_futures_price = current_future_price;

		// For the last Futures we calculate the end futures discount factor
		if (iter_futures == --m_futuresInput.m_futuresPoints.end())
		{
			// Set the next IMM day from the last futures
			busDate end_futures_date = current_futures_date;
			end_futures_date.nextIMMDay();

			// Setup the variable for calculating the discount factor, jilian date
			double end_futures_discount_factor;
			double end_futures_julian_date = end_futures_date.getJulianDate();

			// calculate the end discount factor
			end_futures_discount_factor = current_discount_factor / (1 + (100 - current_future_price) / 100 * (end_futures_julian_date - current_futures_julian_date) / futuresYB);

			insertKeyPoint(end_futures_date, end_futures_discount_factor);
		}
	}
}

void
Curve::processSwaps() {
	// TO DO (Bonus: extra 3%)

	// Set Holiday Center
	const HolCtrs holiday_center = { m_holidayCenter };

	// Calculate the swap discount factor for each cases and insert the discount factor
	for (vector<SwapsPoint>::iterator iter_swap = m_swapsInput.m_swapsPoints.begin(); iter_swap != m_swapsInput.m_swapsPoints.end(); ++iter_swap)
	{
		// Erase the last character Y
		string swap_year_string = (*iter_swap).first;
		swap_year_string.pop_back();
		// Convert the string to interger to get the number of year to be swapped
		double swap_year = stod(swap_year_string);
		// Convert the freq to double
		double freq = static_cast<double>(nPerYear);
		// point the the last key point with the lastest key points vector
		KeyPoints::const_iterator last_keypoint = --endKeyPoint();
		// Creat a vector to store all the SwapsCashFlow
		SwapsCashFlows swap_cash_flows;

		// Set up the swap_cash_flows;
		for (double amount = swap_year; amount >= 0; amount = amount - 1 / freq)
		{
			SwapsCashFlow swap_cash_flow{ m_baseDate };
			swap_cash_flow.dt.setHolCtrs(holiday_center);
			swap_cash_flow.dt.setRollConv(MOD_FOLLOW);
			swap_cash_flow.dt.addDays(m_daysToSpot);
			// Add the years
			int add_years = swap_year - amount;
			swap_cash_flow.dt.addYMD(add_years, 0, 0);
			// Add the month if the amount is not an intenger
			if (amount != static_cast<int>(amount))
			{
				swap_cash_flow.dt.addYMD(0, 12 / freq, 0);
			}
			// Roll to the business date
			swap_cash_flow.dt.rollBusDate();
			// For the base case, set up the CF, df and append it to SwapsCashFlow
			if (amount == swap_year)
			{
				// Set CF
				swap_cash_flow.CF = -100;
				// Set discount factor
				if (interpolate(swap_cash_flow.dt) != 0.0)
				{
					swap_cash_flow.DF = interpolate(swap_cash_flow.dt);
				}
				else // Find the correspording discount factor
				{
					for (KeyPoints::const_iterator current_keypoint = firstKeyPoint(); current_keypoint != endKeyPoint(); ++current_keypoint)
					{
						KeyPoint retrieved_key_point = retrieveKeyPoint(current_keypoint);
						if (swap_cash_flow.dt.getJulianDate() == retrieved_key_point.first)
						{
							swap_cash_flow.DF = retrieved_key_point.second;
						}
					}
				}
				// Add the details to SwapsCashFlow
				swap_cash_flows.push_back(swap_cash_flow);
			}
			// For other cases which the discount factor can be calculated by using interporate or exist in the KeyPoints and it is not the last case in the swap
			else if (retrieveKeyPoint(last_keypoint).first > swap_cash_flow.dt.getJulianDate() && amount != 0)
			{
				// Set discount factor
				if (interpolate(swap_cash_flow.dt) != 0.0)
				{
					swap_cash_flow.DF = interpolate(swap_cash_flow.dt);
				}
				else // Find the correspording discount factor
				{
					for (KeyPoints::const_iterator current_keypoint = firstKeyPoint(); current_keypoint != endKeyPoint(); ++current_keypoint)
					{
						KeyPoint retrieved_key_point = retrieveKeyPoint(current_keypoint);
						if (swap_cash_flow.dt.getJulianDate() == retrieved_key_point.first)
						{
							swap_cash_flow.DF = retrieved_key_point.second;
						}
					}
				}
				// Retrieve the previous swap cash flow date in swap cash flow date
				double last_swap_cash_date = (*(--(swap_cash_flows.end()))).dt.getJulianDate();
				// Set CF
				swap_cash_flow.CF = (100 * (*iter_swap).second * (swap_cash_flow.dt.getJulianDate() - last_swap_cash_date)) / swapsYB;
				// Add the details to SwapsCashFlow
				swap_cash_flows.push_back(swap_cash_flow);
			}
			// For the those cant not be calculate by unterporate and last case
			else
			{
				// Inilitize the discount factor as zero for later checking 
				swap_cash_flow.DF = 0;

				// Set discount factor if the discount factor exist
				if (interpolate(swap_cash_flow.dt) != 0.0)
				{
					swap_cash_flow.DF = interpolate(swap_cash_flow.dt);
				}
				else // Find the correspording discount factor
				{
					for (KeyPoints::const_iterator current_keypoint = firstKeyPoint(); current_keypoint != endKeyPoint(); ++current_keypoint)
					{
						KeyPoint retrieved_key_point = retrieveKeyPoint(current_keypoint);
						if (swap_cash_flow.dt.getJulianDate() == retrieved_key_point.first)
						{
							swap_cash_flow.DF = retrieved_key_point.second;
						}
					}
				}
				// For the last case
				if (amount == 0)
				{
					// Retrieve the previous swap cash flow date in swap cash flow date
					double last_swap_cash_date = (*(--(swap_cash_flows.end()))).dt.getJulianDate();
					// Set CF
					swap_cash_flow.CF = 100 + (100 * (*iter_swap).second * (swap_cash_flow.dt.getJulianDate() - last_swap_cash_date)) / swapsYB;
					// Add the details to SwapsCashFlow
					swap_cash_flows.push_back(swap_cash_flow);
				}
				else // For othre cases
				{
					// Retrieve the previous swap cash flow date in swap cash flow date
					double last_swap_cash_date = (*(--(swap_cash_flows.end()))).dt.getJulianDate();
					// Set CF
					swap_cash_flow.CF = (100 * (*iter_swap).second * (swap_cash_flow.dt.getJulianDate() - last_swap_cash_date)) / swapsYB;
					// Add the details to SwapsCashFlow
					swap_cash_flows.push_back(swap_cash_flow);
				}
			}

		}

		// To calculate the rest discount curve
		// If at least one of the DF does not exist
		if ((*(--swap_cash_flows.end())).DF == 0)
		{
			SwapsCashFlows::const_iterator iter_scf = swap_cash_flows.begin();
			// Get the last keypoint just smaller than the, because it is smaller than the current swap_cash_flows juliandate
			KeyPoints::const_iterator iter_keypoint = (--endKeyPoint());
			KeyPoint retrieved_key_point = retrieveKeyPoint(iter_keypoint);
			// Sum all the DF*CF value when DF > 0
			double sum_all_exist_CF_times_DF_value = 0;
			// For testing the k value
			double test_sum = 0;

			// Iterate until the first occurance of DF == 0 and sum all the value
			for (; (*iter_scf).DF != 0; ++iter_scf)
			{
				sum_all_exist_CF_times_DF_value += ((*iter_scf).DF) * ((*iter_scf).CF);
			}

			// Perform the bineary search 
			// Do the set up to conduct the binary search to search for k
			// Set up all the variable
			double const EPLISON = 0.00000000001;
			// The discount factor is in between 0 and 1
			double b_head = 0;
			double b_tail = 1.5;
			double k = (b_head + b_tail) / 2;
			double test_sum_value = sum_all_exist_CF_times_DF_value;
			SwapsCashFlows::const_iterator iter_scf_zero = iter_scf;
			// Import <math.h> for calculate the power
			#include <math.h>
			// Perform the search
			do {
				k = (b_head + b_tail) / 2;
				iter_scf_zero = iter_scf;
				test_sum_value = sum_all_exist_CF_times_DF_value;

				// Sum all before last case. CF * DF
				for (; iter_scf_zero != --swap_cash_flows.end(); ++iter_scf_zero)
				{
					test_sum_value += (*iter_scf_zero).CF * ((retrieved_key_point.second) * pow(k, (*iter_scf_zero).dt.getJulianDate() - retrieved_key_point.first));
				}
				// for the last case
				test_sum_value += ((*iter_scf_zero).CF) * ((retrieved_key_point.second) * pow(k, (*iter_scf_zero).dt.getJulianDate() - retrieved_key_point.first));

				// Update the b_head and tail value
				if (test_sum_value < 0)
				{
					b_head = k;
				}
				else if (test_sum_value > 0)
				{
					b_tail = k;
				}

				k = (b_head + b_tail) / 2;
			} while (test_sum_value > EPLISON || test_sum_value < -EPLISON);

			// Countinue to edit the SwapsCashFlows vector with those DF factor value = 0 and insert the key list
			for (; iter_scf != swap_cash_flows.end(); ++iter_scf)
			{
				insertKeyPoint((*iter_scf).dt, (retrieved_key_point.second) * pow(k, (*iter_scf).dt.getJulianDate() - retrieved_key_point.first));
			}
		}
	}
}