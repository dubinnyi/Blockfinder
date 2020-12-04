#include"scheme.h"

using namespace std;
//using namespace boost;


bool operator<(const Scheme& t1, const Scheme& t2) {
    return (t1.simplified < t2.simplified);
}


bool operator<(const Scheme_compact& t1, const Scheme_compact& t2) {
    return (t1.simplified < t2.simplified);
}


bool Scheme::operator<(const Scheme& t2) {
    return (this->simplified < t2.simplified);
}


bool operator==(const Scheme& t1, const Scheme& s2) {
    return (t1.simplified == s2.simplified);
}


bool operator==(const Scheme_compact& t1, const Scheme_compact& s2) {
    return (t1.simplified == s2.simplified);
}


Scheme::Scheme() {
    patterns={};
}


bool Scheme::check_codes() {
    int code;
    bool first = true; 
    codes = false;

    //cout << " check codes[1]" << codes[1] << endl;
    //codes= codes.shift(false);
    for (int i: patterns) {
        for ( int j : patterns) {
            //code =  patternscode.codes[ patternscode.patterns.size()*i  +j]; //!!!
            code =  code_tab_ptr->calc_code_fast(i, j);
            if (first) {
                first = false;
                continue;
            }
            else if (codes[code]==true) {
                return false;
            }
            else {
                //codes.insert(code);
                codes[code] = true;
            }
        }
    }
    return true;
}


void Scheme::setscheme( PatternsCodes *patternscode , string sname, NCS *sncs, int  bsamples, vector <int>  bpatterns) {
    name = sname;
    patterns=bpatterns;
    samples = bsamples;
    code_tab_ptr = patternscode;
    ncs_ptr = sncs;
    codes.resize(code_tab_ptr->n_codes, false);

    good = check_codes();
    simplify();

}



void Scheme::simplify() {
    simplified.assign(code_tab_ptr->n_simplified, 0);
    for (int pattern : patterns) {
       simplified[code_tab_ptr->simple_ints[pattern]]++;
    }
}


Scheme::Scheme(PatternsCodes *patternscode, string sname, NCS *sncs, int  bsamples, vector <int>  bpatterns) {
    name = sname;
    patterns = bpatterns;
    samples = bsamples;
    code_tab_ptr = patternscode; 
    ncs_ptr  = sncs;
    Vbool codes(false, code_tab_ptr->n_codes);
    good = check_codes();
    simplify();

}


Scheme::Scheme(PatternsCodes *patternscode, NCS *sncs, ifstream & in){
   bool debug = false;

   name = "file";
   code_tab_ptr = patternscode; 
   ncs_ptr  = sncs;
   codes.resize(code_tab_ptr->n_codes, false);

   string elb_re = "\\[ *ELB +samples *= *(?<samples>\\d+) +patterns *= *(?<patterns>\\d+) *\\]";
   string sv_re = "\\[ *SV *(?<sv>( +\\d+)+) *\\]";
   boost::regex  elb_e(elb_re);
   boost::regex  sv_e(sv_re);
   boost::smatch what;
   int n_patterns = -1;
   patterns.clear();
   string line;
   int l = 0;
   bool elb_flag = false, sv_flag = false, patterns_flag = false, scheme_flag = false;
   while ( not scheme_flag and getline(in, line) ){
      if(debug) cout<<"LINE "<<l<<" = \'"<<line<<"\'"<<endl;
      patterns_flag = ( patterns.size() == n_patterns );
      if(not elb_flag and boost::regex_match(line, what, elb_e)){
         samples = stoi(what["samples"]);
         n_patterns = stoi(what["patterns"]);
         elb_flag = true;
         if(debug)cout<<"NEW BLOCK: samples = "<<samples<<" patterns = "<<n_patterns<<endl;
      }else if(elb_flag and not sv_flag and boost::regex_match(line, what, sv_e)){
         sv_flag = true;
         //ignore sv at all
      }else if(elb_flag and not patterns_flag){
         string pattern = line;
         //if(debug) cout<<"LINE "<<l<<" = \'"<<line<<"\'"<<endl;
         auto seek_pattern = code_tab_ptr->pattern_to_number.find(pattern);
         if ( seek_pattern == code_tab_ptr->pattern_to_number.end()){
             cerr<<"ERROR: wrong pattern: \'"<<pattern<<"\'"<<endl;
         }else{
             patterns.push_back(code_tab_ptr->pattern_to_number[pattern]);
             patterns_flag = ( patterns.size() == n_patterns );
             if(debug)  cout<<"PATTERN_INT= "<<code_tab_ptr->pattern_to_number[pattern]<<endl;
         }
      }
      scheme_flag = elb_flag and patterns_flag;
      if (debug){
          cout<<"elb_flag = "<<elb_flag<<" patterns_flas = "<<patterns_flag<<endl;
      }
      l++;
   };
   //Vbool codes(false, code_tab_ptr->n_codes);
   if (debug) cout<<"check_codes"<<endl;
   good = check_codes();
   if (debug) cout<<"simplify"<<endl;
   simplify();
}


bool Scheme::check_patterns(vector <string> patterns) {
    if (patterns.size() == 0) {
        return false;
    }
    int sizep;
    sizep = patterns[0].size();
    for (string pattern : patterns) {
        for (char label_type : pattern) {
            if (find(ncs_ptr->label_types.begin(), ncs_ptr->label_types.end(), to_string(label_type)) == ncs_ptr->label_types.end()) { //-/ 
                return false;
            }
        }
        if (pattern.size() != sizep) {
            return false;
        }
    }
    return true;
}


void Scheme::sort(){

    std::sort(patterns.begin(), patterns.end());

}


void Scheme_compact::sort(){

    std::sort(patterns.begin(), patterns.end());

}


void Scheme::add_new_codes(int new_pattern) {
    int m=code_tab_ptr->patterns.size();
    int n=new_pattern;
    int j=0;
    for (int i :patterns) {
        codes[code_tab_ptr->calc_code_fast(i,n)]=true;
        codes[code_tab_ptr->calc_code_fast(n,i)]=true;
    }
    codes[code_tab_ptr->calc_code_fast(n,n)]=true;
}


void Scheme::add_pattern(int new_pattern) {
    patterns.push_back(new_pattern);
    add_new_codes(new_pattern);
    simplify();
}

bool Scheme::try_pattern_speedo(int  new_pattern, Speedo & speedo) {
  // This function have an unaceptable overhead
  // It slows the calculations ~10x times
  // Use try_pattern instead
  Speedo local_speedo;
  bool ret;
  local_speedo.start();
  ret = try_pattern(new_pattern);
  local_speedo.stop();
  local_speedo.count(2*patterns.size()+1);
  speedo+= local_speedo;
  return ret;
}

bool Scheme::try_pattern(int  new_pattern) {
    if (good == false) {
        return false;
    }
    
    int i_code[2*patterns.size()+1];
     
    int n = new_pattern;

    for (int i=0; i<patterns.size(); i++) {
        int i_code1 = code_tab_ptr->calc_code_fast(patterns[i],n);
        if(codes[i_code1]) return false;
        int i_code2 = code_tab_ptr->calc_code_fast(n,patterns[i]);
        if(codes[i_code2]) return false;
        i_code[2*i] = i_code1;
        i_code[2*i+1] = i_code2;
    }
    int i_code0 = code_tab_ptr->calc_code_fast(n,n);
    if(codes[i_code0]) return false;
    i_code[2*patterns.size()] = i_code0;
    
    Vbool new_codes(false, code_tab_ptr->n_codes);
    for(int i=0; i<2*patterns.size()+1; ++i){
        if(new_codes[i_code[i]]) return false;
        else new_codes[i_code[i]]=true;
    } 
    
    return true;
}


string Scheme::full_str() {
    string s = "";
    string all_p = "";

    for (int i : patterns) {
        all_p = all_p + code_tab_ptr->patterns[i] + "\n";
    }
    s = "[ELB samples = " + to_string(samples) + " patterns = " + to_string(patterns.size()) + "]\n" + all_p;
    return s;
}


Scheme_compact::Scheme_compact(Scheme &scheme) {
    code_tab_ptr = scheme.code_tab_ptr;
    patterns     = scheme.patterns;
    simplified   = scheme.simplified;
    samples      = scheme.samples;


}



string Scheme_compact::full_str() {
    string header= "[ELB ";
    string all_p = "";
    string sv = "[SV";
    string s;

    header = header + "samples = " + to_string(samples) + " patterns = " + to_string(patterns.size()) + "]\n";
    for (int int_simple : simplified) {
      sv = sv + " "+ to_string(int_simple);
    }
    sv = sv + " ]\n";
    for (int i : patterns) {
        all_p = all_p + code_tab_ptr->patterns[i] + "\n";
    }
    s = header + sv + all_p;
    //s = header + all_p;
    return s;
}

