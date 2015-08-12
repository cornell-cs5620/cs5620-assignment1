#ifndef standard_h
#define standard_h

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <string.h>
#include <ctime>
#include <sys/time.h>
#include <algorithm>

using namespace std;

template <class type>
string to_string(type value)
{
	ostringstream os;
	os << value;
	return os.str();
}

#endif
