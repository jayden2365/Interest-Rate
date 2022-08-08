#include "CurveData.h"
#include <string>
#include <fstream>
#include <cstring>

using std::string;
using std::ifstream;
using std::ofstream;

#define CurveDatalog	"C:\\temp\\CurveData.log"

const char* MONTH[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

bool date2YMD(char *dt, int &year, int &month, int &day) {
	// dt format dd-mmm-yy
	char *month_s, *next_token;
	day = atoi(strtok_s(dt, "-", &next_token));
	month_s = strtok_s(NULL, "-", &next_token);
	int i;
	for (i=0; i<12; ++i) {
		if (!strcmp(month_s, MONTH[i])) {
			month = i+1;
			break;
		}
	}
	if (i == 12) return false;
	year = 2000 + atoi(strtok_s(NULL, "-", &next_token));
	return true;
}

bool
CurveData::load(const char *filename) {
	// TO DO

	// input the curvedata file by using istream
	ifstream curvedata_file(filename);
	ofstream logFile(CurveDatalog);

	// If the file does not exist return false, stop loading the file
	if (!curvedata_file) return false;

	// Extract each line in the file and store into 
	char line[maxLineSize];

	// Create an heap object for later use, since the file can be opened.
	cash = new CashInput;
	futures = new FuturesInput;
	swaps = new SwapsInput;

	// Similar set up in the Holiday.cpp file
	// Furthermore, if the string in betweern "," or next line matches the data member of CurveData
	// Store the data in the data member
	// until end of the file
	while (curvedata_file.getline(line, maxLineSize).gcount()) {
		;
		if (strlen(line) == maxLineSize - 1) {
			do {
				logFile << line;
				curvedata_file.clear(curvedata_file.rdstate() & ~ios::failbit);
			} while (curvedata_file.getline(line, maxLineSize).gcount() == maxLineSize - 1);
			logFile << line;
		}
		else {
			// type represents the first term in that line
			// character represents the character of the type
			// number represents the number of that character with different in different data member
			char *type, *next_token;
			type = strtok_s(line, ",", &next_token);

			// define string for checking
			string CURRENCY = "Currency";
			string BASE_DATE = "Base Date";
			string DAYS_TO_SPOT = "Days to Spot";
			string CASH_BASIS = "Cash Basis";
			string FUTURES_BASIS = "Futures Basis";
			string SWAPS_BASIS = "Swaps Basis";
			string SWAP_FREQ = "Swaps Freq";
			string HOLIDAY = "Holiday";
			string CASH = "Cash";
			string FUTURES = "Futures";
			string SWAPS = "Swaps";

			// if the type equals the string "Currency", input the character into data member currency
			if (type == CURRENCY)
			{
				currency = strtok_s(NULL, ",", &next_token);
			}

			// if the type equals the string "Base Date", input the character into data member baseDate
			else if (type == BASE_DATE)
			{
				char *character;
				character = strtok_s(NULL, ",", &next_token);
				// if data is in format dd-mmm-yy, format it
				int year, month, day;
				if (date2YMD(character, year, month, day))
				{
					baseDate = date::YMD2julianDate(year, month, day);
				}
				else
				{
					baseDate = stol(character);
				}
			}

			// if the type equals the string "Days to Spot", input the character into data member daysToSpot
			else if (type == DAYS_TO_SPOT)
			{
				daysToSpot = stoi(strtok_s(NULL, ",", &next_token));
			}

			// if the type equals the string "Cash Basis", input the character into data member cash
			else if (type == CASH_BASIS)
			{
				cash->m_cashBasis = strtok_s(NULL, ",", &next_token);
			}

			// if the type equals the string "Futures Basis", input the character into data member futures
			else if (type == FUTURES_BASIS)
			{
				futures->m_futuresBasis = strtok_s(NULL, ",", &next_token);
			}

			// if the type equals the string "Swaps Basis", input the character into data member swaps
			else if (type == SWAPS_BASIS)
			{
				swaps->m_swapsBasis = strtok_s(NULL, ",", &next_token);
			}

			// if the type equals the string "Swaps Freq", input the character into data member swaps
			else if (type == SWAP_FREQ)
			{
				swaps->m_swapsFreq = strtok_s(NULL, ",", &next_token);
			}

			// if the type equals the string "Holiday", input the character into data member holidayCenter
			else if (type == HOLIDAY)
			{
				holidayCenter = strtok_s(NULL, ",", &next_token);
			}

			// if the type equals the string "Cash", input the character into data member cash
			else if (type == CASH)
			{
				CashPoint cash_pt_pair;

				cash_pt_pair.first = strtok_s(NULL, ",", &next_token);
				cash_pt_pair.second = stod(strtok_s(NULL, ",", &next_token));
				cash->m_cashPoints.push_back(cash_pt_pair);
			}

			// if the type equals the string "Futures", input the character into data member futures
			else if (type == FUTURES)
			{
				FuturesPoint futures_pt_pair;
				char *character;
				int year, month, day;
				character = strtok_s(NULL, ",", &next_token);

				// if data is in format dd-mmm-yy, format it
				if (date2YMD(character, year, month, day))
				{
					futures_pt_pair.first = date::YMD2julianDate(year, month, day);
				}
				else
				{
					futures_pt_pair.first = stol(character);
				}
				futures_pt_pair.second = stod(strtok_s(NULL, ",", &next_token));
				futures->m_futuresPoints.push_back(futures_pt_pair);
			}
			// if the type equals the string "Swaps", input the character into data member swaps
			else if (type == SWAPS)
			{
				SwapsPoint swap_pt_pair;

				swap_pt_pair.first = strtok_s(NULL, ",", &next_token);
				swap_pt_pair.second = stod(strtok_s(NULL, ",", &next_token));
				swaps->m_swapsPoints.push_back(swap_pt_pair);
			}
		}
	}
	return true;
}