
#ifndef SPEEDO_H_INCLUDED
#define SPEEDO_H_INCLUDED

#include <vector>
#include <boost/regex.hpp>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>

#include "ncs.h"
#include "speedo.h"
#include <stdexcept>
#include "PatternCodes.h"



/*
#ifdef __AVX512__
const int Align=64;
#else
const int Align=32;
#endif
* */

/*
class Vbool: public valarray<bool> {
    public:
    Vbool():valarray<bool>(){};
    Vbool(bool v, int N):valarray<bool>(v, N){};
    using valarray<bool>::operator =;
    Vbool& set_true(const uint *a, int n){
        for(int i=0; i<n; ++i)(*this)[a[i]]=true;
        return *this;
    }
    bool is_true(const uint *a, int n)const{
        for(int i=0; i<n; ++i){
            if((*this)[a[i]]) return true;
        }
        return false;
    }
};
* */

class Scheme {
public:
	string name;
	NCS *ncs_ptr;
	PatternsCodes *code_tab_ptr; 
	int samples;
	vector <int> patterns;
	Vbool codes;
	// Vbool new_codes;
    vector <int> simplified;

	bool good;
	Scheme();
	Scheme(PatternsCodes *patternscode, string sname , NCS *sncs, 
            int bsamples=0, vector<int> bpatterns = {});
	Scheme(PatternsCodes *patternscode, NCS *sncs, ifstream& in);

	bool check_codes();
	void simplify();
	bool check_patterns(vector <string> patterns);
	void sort();
	void add_new_codes(int new_pattern);
	void add_pattern(int new_pattern);
	bool try_pattern(int  new_pattern);
  bool try_pattern_speedo(int  new_pattern, Speedo & speedo);
	Scheme direct_product(Scheme scheme);
	string full_str();
	void setscheme(PatternsCodes *patternscode, string sname, NCS *sncs, 
            int bsamples=0, vector<int> bpatterns = {});

	bool operator<(const Scheme & t2);
};
bool operator==(const Scheme& s1, const Scheme& t2);
bool operator<(const Scheme& t1, const Scheme& t2);


class Scheme_compact{
public:
	int samples;
	PatternsCodes *code_tab_ptr;
    vector <int> patterns;
    vector <int> simplified;
    Scheme_compact(){};
	Scheme_compact(Scheme &scheme);
	void sort();

	string full_str();
  string simplified_vector_string();

};
bool operator==(const Scheme_compact& s1, const Scheme_compact& t2);
bool operator<(const Scheme_compact& t1, const Scheme_compact& t2);

void read_blocks_from_file(PatternsCodes *patternscode, NCS *sncs, string file, vector<Scheme_compact> & out, bool debug);

#endif
