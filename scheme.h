//#pragma once
#include"ncs.h"
#include "PatternCodes.h"
class Scheme {
public:
	string name;
	NCS *ncs_ptr;
	PatternsCodes *code_tab_ptr; 
	int samples;
	valarray<int> patterns;
	int number_of_patterns;
	valarray <bool> codes;
	valarray <bool> new_codes;
    valarray<int> simplified;

	bool good;
	Scheme();
	Scheme(int listsize, PatternsCodes *patternscode, string sname , NCS *sncs , int  bsamples = 0, vector <int>  bpatterns = {});

	bool check_codes();
	void simplify(); 
	void sort();
	void add_new_codes(int new_pattern);
	void add_pattern(int new_pattern);
	bool try_pattern(int  new_pattern);
	Scheme direct_product(Scheme scheme);
	string full_str();
	void setscheme(int listsize, PatternsCodes *patternscode, string sname, NCS *sncs , int  bsamples = 0, vector <int>  bpatterns = {});

	bool operator<(const Scheme & t2);
};
bool operator==(const Scheme& s1, const Scheme& t2);
bool operator<(const Scheme& t1, const Scheme& t2);

class Scheme_compact{
public:
	int samples;
	PatternsCodes *code_tab_ptr;
	valarray <int> patterns;
	valarray <int> simplified;

    int number_of_patterns;

    Scheme_compact();
	Scheme_compact(Scheme &scheme);
	void sort();

	string full_str();

};


bool operator==(const Scheme_compact& s1, const Scheme_compact& t2);
bool operator<(const Scheme_compact& t1, const Scheme_compact& t2);
