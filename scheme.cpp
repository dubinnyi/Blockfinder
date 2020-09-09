#include"scheme.h"

template<class InputIt1, class InputIt2, class Compare>
bool lexicographical_compare(InputIt1 first1, InputIt1 last1,
                             InputIt2 first2, InputIt2 last2,
                             Compare comp)
{
    for ( ; (first1 != last1) && (first2 != last2); ++first1, (void) ++first2 ) {
        if (comp(*first1, *first2)) return true;
        if (comp(*first2, *first1)) return false;
    }
    return (first1 == last1) && (first2 != last2);
}

bool operator<(const Scheme& t1, const Scheme& t2) {

    return std::lexicographical_compare(std::begin(t1.simplified), std::end(t1.simplified),
                                        std::begin(t2.simplified), std::end(t2.simplified));
}

bool operator<(const Scheme_compact& t1, const Scheme_compact& t2) {

    return std::lexicographical_compare(std::begin(t1.simplified), std::end(t1.simplified),
                                        std::begin(t2.simplified), std::end(t2.simplified));

}

bool Scheme::operator<(const Scheme& t2) {

    return std::lexicographical_compare(std::begin(this->simplified), std::end(this->simplified),
                                        std::begin(t2.simplified), std::end(t2.simplified));
}

bool operator==(const Scheme& t1, const Scheme& t2) {

    if ( t1.number_of_patterns !=  t2.number_of_patterns){
        return false;
    }

    for (int i=0; i< t2.number_of_patterns; i++){
        if (t1.simplified[i] != t2.simplified[i])
        return false;
    }
    return true;
}

bool operator==(const Scheme_compact& t1, const Scheme_compact& t2) {

    if ( t1.number_of_patterns !=  t2.number_of_patterns){
        return false;
    }

    for (int i=0; i< t2.number_of_patterns; i++){
        if (t1.simplified[i] != t2.simplified[i])
        return false;
    }
    return true;
}

Scheme::Scheme() {
	patterns={};
}

bool Scheme::check_codes() {
	int code;
	bool first = true; 
	codes = false;
    for ( int i=0; i<number_of_patterns; i++ ) {
        for ( int j=0; j<number_of_patterns; j++) {
			code =  code_tab_ptr->calc_code_fast(patterns[i], patterns[j]);
			if (first) {
				first = false;
				continue;
			}
			else if (codes[code]==true) {
				return false;
			}
			else {
				codes[code] = true;
			}
		}
	}
	return true;
}


void Scheme::setscheme(int listsize, PatternsCodes *patternscode , string sname, NCS *sncs, int  bsamples, vector <int>  bpatterns) {
	name = sname;
    patterns.resize(listsize, -1);
    simplified.resize(listsize, -1);
    number_of_patterns = bpatterns.size(); // пока с учетом того, что сначала массив паттернов пустой

    for (int i =0; i < number_of_patterns; i++)
    {
        patterns[ i] = bpatterns[ i];
    }
    samples = bsamples;
	code_tab_ptr = patternscode;
	ncs_ptr = sncs;
	codes.resize(code_tab_ptr->n_codes, false);
	new_codes.resize(code_tab_ptr->n_codes, false) ;
	good = check_codes();
	simplify();
}

void Scheme::simplify() {
	simplified.resize(code_tab_ptr->n_simplified, 0);
    for (int i=0; i<number_of_patterns; i++) {
	   simplified[code_tab_ptr->simple_ints[patterns[i]]]++;
	}
}

Scheme::Scheme(int listsize, PatternsCodes *patternscode, string sname, NCS *sncs, int  bsamples, vector <int>  bpatterns) {
	name = sname;
    patterns.resize(listsize, 0);
    number_of_patterns = bpatterns.size();
	samples = bsamples;
	code_tab_ptr = patternscode; 
	ncs_ptr  = sncs;
	valarray <bool> codes(false, code_tab_ptr->n_codes); //
	good = check_codes();
	simplify();
	valarray <bool> new_codes(false, code_tab_ptr->n_codes);
}

void Scheme::sort(){
	if(number_of_patterns>1) {
        std::sort(&patterns[0], &patterns[number_of_patterns ]); //
    }
}

void Scheme_compact::sort(){
    if(number_of_patterns>1) {
        std::sort(&patterns[0], &patterns[number_of_patterns ]);
    }
}

void Scheme::add_new_codes(int new_pattern) {
    int m=code_tab_ptr->patterns.size();
    int n=new_pattern;
	for (int i=0; i<number_of_patterns; i++) {
	    codes[code_tab_ptr->calc_code_fast(patterns[i],n)] = true;
		codes[code_tab_ptr->calc_code_fast(n,patterns[i]) ] = true;

	}
    codes [  code_tab_ptr->calc_code_fast(n,n)] = true;

}

void Scheme::add_pattern(int new_pattern) {
	patterns[number_of_patterns]=new_pattern;
	number_of_patterns=number_of_patterns+1;
	add_new_codes(new_pattern);
	simplify();
}

bool Scheme::try_pattern(int  new_pattern) {
	if (good == false) {
		return false;
	}
	valarray <bool> double_codes;
	double_codes.resize(code_tab_ptr->n_codes, false) ;
	new_codes = false;
	int code_1, code_2;
   	int n = new_pattern;
    for (int i=0; i<number_of_patterns; i++) {
		code_1 = code_tab_ptr->calc_code_fast(patterns[i],n);
		code_2 = code_tab_ptr->calc_code_fast(n,patterns[i]);
		
		if (codes[code_1]==true || codes[code_2]==true || (code_2 == code_1) || new_codes[code_1]==true || new_codes[code_2]==true) {

			return false;
		}
		else {
			new_codes[code_1]=true;
			new_codes[code_2]=true;
		}
	}
	int self_code = code_tab_ptr->calc_code_fast(n,n);
	if (codes[self_code] ==true|| new_codes[self_code]==true) {
		return false;
	}
	else {
		new_codes[self_code]=true;
	}
	return true;
}


string Scheme::full_str() {
	string s = "";
	string all_p = "";
    for (int i=0; i<number_of_patterns; i++) {
		all_p = all_p + code_tab_ptr->patterns[i] + "\n";
	}
	s = "[ELB samples = " + to_string(samples) + " patterns = " + to_string(number_of_patterns) + "]\n" + all_p;
	return s;
}

Scheme_compact::Scheme_compact() {}

Scheme_compact::Scheme_compact(Scheme &scheme) {
    code_tab_ptr = scheme.code_tab_ptr;
    patterns     = scheme.patterns;
    simplified   = scheme.simplified;
    samples      = scheme.samples;
    number_of_patterns = scheme.number_of_patterns;
}

string Scheme_compact::full_str() {
    string header= "[ELB ";
    string all_p = "";
    string sv = "[SV";
    string s;

    header = header + "samples = " + to_string(samples) + " patterns = " + to_string(number_of_patterns) + "]\n";

    for (int int_simple : simplified) {
        sv = sv + " "+ to_string(int_simple);
    }
    sv = sv + " ]\n";
    for (int i=0; i<number_of_patterns; i++) {
        all_p = all_p + code_tab_ptr->patterns[patterns[i]] + "\n";
    }
    s = header + sv + all_p;
    return s;
}
