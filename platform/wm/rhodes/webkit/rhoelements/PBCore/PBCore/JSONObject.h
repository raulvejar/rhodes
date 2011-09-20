/**
 * \file JSONObject.h
 * \brief Class to wrap JSON functionality in C++. 
 */

#include "..\..\Common\Public\PB_Defines.h"
#include <vector>

TCHAR *wcsistr(LPCTSTR szStringToBeSearched, 
				LPCTSTR szSubstringToSearchFor);


/**
 * Class to provide a user friendly way of using JSON in C++.
 */
class JSONObject
{
public:

	JSONObject();
	~JSONObject();
	void put(TCHAR* tcName, LPCTSTR tcValue);
	bool toString(TCHAR* tcJSONAsString, int iMaximumLength);

private:
	std::vector<TCHAR*> vTuples;
};
