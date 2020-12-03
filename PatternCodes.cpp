//
// Created by kamilla on 06.03.19.
//

#include"PatternCodes.h"



PatternsCodes::PatternsCodes( vector<string> a_patterns, NCS a_ncs      ){
    patterns=a_patterns;
    n_patterns = patterns.size();
    if(n_patterns > 0 )
      n_samples = patterns[0].length();
    else
      n_samples = 0;
    
    ncs = a_ncs;

    create_simplified_table();
    create_labeltype_flags();
    create_codes_table();
    calculate_group_ranks();
}

PatternsCodes::PatternsCodes() {
    patterns={};
    n_patterns = 0;
    n_samples = 0;
}


void PatternsCodes::setPatternsCodes(vector<string> a_patterns, NCS a_ncs ) {

    patterns=a_patterns;
    n_patterns = patterns.size();
    if(n_patterns > 0 )
      n_samples = patterns[0].length();
    else
      n_samples = 0;
    
    ncs = a_ncs;

    create_simplified_table();
    create_labeltype_flags();
    create_codes_table();  
    calculate_group_ranks();
}


void PatternsCodes::create_labeltype_flags(){
   have_labeltype_simplified_flag.resize(n_simplified);
   have_labeltype_pattern_flag.resize(n_patterns);
   have_labeltype_simplified_flag = false;
   have_labeltype_pattern_flag = false;
   /*cout<<"@@@  create_labeltype_flags @@@";*/
   for(int p=0; p<n_patterns; p++){
       string pattern = simple_form[p];
       string lab = "";
       int s = simple_ints[p];
       /*cout<<p<<" simple_form="<<pattern<<" "<<" s="<<s<<endl;*/
       for(int l=0; l< ncs.label_types.size(); l++){
          labeltype lt = ncs.label_types[l];
          if(find(pattern.begin(), pattern.end(), lt.name)!=pattern.end()){
             have_labeltype_pattern_flag[p][l] = true;
             have_labeltype_simplified_flag[s][l] = true;
          }
       }
   }
   simple_label = {};
   for(int s=0; s<n_simplified ; s++){
       string lab = "";
       for(int l=0;l<ncs.label_types.size();l++){
          char lt = ncs.label_types[l].name;
          char dot = '.';
          lab += have_labeltype_simplified_flag[s][l]?lt:dot;
       }
       simple_label.push_back(lab);
   }
};

void PatternsCodes::print_pattern_flags(ostream& out){
   for(int p=0; p<n_patterns; p++){
      int s = simple_ints[p];
      out<<"p= "<<setw(3)<<p<<" "<<patterns[p]<<" s="<<setw(3)<<s<<" "<<simple_form[s]<<" flags= \'";
      for(int l=0;l<ncs.label_types.size();l++){
         if (have_labeltype_pattern_flag[p][l]){
        out<<ncs.label_types[l].name;
         }else{
        out<<" ";
         }
      }
      out<<"\'"<<endl;
   }
};

void PatternsCodes::print_codes(string file){
    ofstream file_with_codes (file);
    if(file_with_codes.is_open()){
      print_codes(file_with_codes);
    }else{
       cerr<<"Can't open file "<<file<<" for wrinting codes table"<<endl;
    }
}

void PatternsCodes::print_codes(ostream &out){
   out<<"n_patterns=   "<<n_patterns<<endl;
   out<<"n_simplified= "<<n_simplified<<endl;
   out<<"n_codes=      "<<n_codes<<endl;
   out<<setw(n_samples)<<" "<<" ";
   for(int i=0; i<n_patterns; i++){
     out<<setw(n_samples)<<patterns[i]<<" ";
   }
   cout<<endl;
   for(int i=0; i<n_patterns; i++){
      out<<setw(3)<<patterns[i]<<" ";
      for(int j=0; j<n_patterns; j++)
         out<<setw(n_samples)<<calc_code_fast(i,j)<<" ";
         out<<endl;
      }
   out<<endl;
};


void PatternsCodes::print_simplified_patterns(ostream &out){
   out<< "    # "<<setw(n_samples)<<"GR"<<" "<<setw(ncs.label_types.size())<<"LAB"<<" "<<setw(4)<<"MULT"<<" "<<setw(4)<<"RANK"<<endl;
   for(int s=0; s<n_simplified ; s++){
      out<<"   "<<setw(2)<<s<<" "<<unique_simplified_patterns[s]<<" ";
      out<<simple_label[s];
      out<<" "<<setw(4)<<simple_multiplicity[s];
      out<<" "<<setw(4)<<group_rank[s]<<endl;
   }
};


void PatternsCodes::create_simplified_table()
{
    pattern_ints.clear();
    simple_form.clear();
    unique_simplified_patterns.clear();
    simple_ints.clear();
    simple_multiplicity.clear();
    group_pattern_ints.clear();
    group_pattern_text.clear();
    
    // fill pattern_ints with sequencial numbers, 
    // the numbers will refer patterns in the 'vector<string> patterns' 
    for (int i = 0; i < patterns.size(); i++) {
       pattern_ints.push_back(i);
    };

    map <string, int> simplified_map = {};
    int unique_simple_count = -1; /* will be incremented */
    int pattern_simple_int;
    for(int i =0; i<patterns.size(); i++){
       string simple_pattern = simplify_pattern(patterns[i]);
       auto seek_pattern = simplified_map.find(simple_pattern);
       if (seek_pattern == simplified_map.end() ){
          unique_simplified_patterns.push_back(simple_pattern);
          unique_simple_count++;
          pattern_simple_int = unique_simple_count;
          // update map for futher search of the simple_pattern
          simplified_map[simple_pattern]=pattern_simple_int;
          // Add pattern to the group of patterns
          group_pattern_ints.push_back({});
          group_pattern_text.push_back({});
          // update multiplicity
          simple_multiplicity.push_back(1);
       }else{
          pattern_simple_int = simplified_map[simple_pattern];
          simple_multiplicity[pattern_simple_int]++;
       }
       simple_form.push_back(simple_pattern);
       simple_ints.push_back(pattern_simple_int);
       group_pattern_ints[pattern_simple_int].push_back(i);
       group_pattern_text[pattern_simple_int].push_back(patterns[i]);
    }
    n_simplified = unique_simplified_patterns.size();


}


void PatternsCodes::create_codes_table() {
    string symbol_code;
    unsigned long code_number;

    codes.resize(n_patterns*n_patterns);
    for(int i=0; i<n_patterns; i++){
        for (int j=0; j<n_patterns; j++){
            symbol_code= ncs.calc_code(patterns[i], patterns[j]);

            if( find(codes_list.begin(), codes_list.end(), symbol_code)!=codes_list.end()   ){
                code_number=distance(codes_list.begin(), find(codes_list.begin(), codes_list.end(), symbol_code) );
            }else{
                code_number=codes_list.size();
                code_to_number[symbol_code]=code_number;
                codes_list.push_back(symbol_code);
            }
            codes[i*n_patterns+j]=code_number;
        }
    }
    n_codes = codes_list.size();
}
    

void PatternsCodes::count_different_codes(const vector <int> &  patterns, int p_test, size_t & n_diff_row, size_t & n_diff_col){
   n_diff_col = 0;
   n_diff_row = 0;
   Vbool codes_row(false, n_codes);
   Vbool codes_col(false, n_codes);

   int code = calc_code_fast(p_test, p_test);
   codes_row[code] = true; 
   codes_col[code] = true; 
   n_diff_row++;
   n_diff_col++;

   for(int p : patterns){
      int code;
      if(p == p_test)
         continue;
      code = calc_code_fast(p, p_test);
      if(not codes_row[code]){
         codes_row[code] = true; 
         n_diff_row++;
      }
      code = calc_code_fast(p_test, p);
      if(not codes_col[code]){
         codes_col[code] = true; 
         n_diff_col++;
      }
   }
};


void PatternsCodes::count_different_codes_in_vector(const vector <int> &  patterns, 
    vector <size_t> &n_diff_row, vector <size_t> &n_diff_col){

   n_diff_row.assign(patterns.size(), 0);
   n_diff_col.assign(patterns.size(), 0);
   int i=0;
   for(int p : patterns){
      count_different_codes(patterns, p, n_diff_row[i], n_diff_col[i]);
      i++;
   }
};


void PatternsCodes::count_pairwise_compatible(const vector <int> &  patterns, int p1, size_t & n_compat, Vbool & compat){
   n_compat = 1; // all patterns are self-compatible
   compat.resize(patterns.size());
   compat = false;
   int i=0;
   int c11, c12, c21, c22;
   c11=calc_code_fast(p1, p1);
   for(int p2 : patterns){
     c12 = calc_code_fast(p1, p2);
     c21 = calc_code_fast(p2, p1);
     c22 = calc_code_fast(p2, p2);
     if (c11 != c12 and c11 != c21 and c11 != c22 and 
         c12 != c21 and c12 != c22 and c21 != c22 ){
         compat[i] = true;
         n_compat++;
     }
     i++;
   }
}; 


void PatternsCodes::count_pairwise_compatible(const vector <int> & patterns, vector< size_t> & n_compat){
   n_compat.assign(patterns.size(), 0);
   Vbool compat_flag_vec;
   int i=0;
   for(int p1 : patterns){
      count_pairwise_compatible(patterns, p1, n_compat[i], compat_flag_vec);
      i++;
   }
}


void PatternsCodes::calculate_group_ranks(){ 
   group_rank.assign(n_simplified, 2); // will be set 1 later
   vector<size_t> n_compat, n_diff_row, n_diff_col;
   for(int g=0; g<n_simplified;g++){
      // cout<<"Group "<<unique_simplified_patterns[g]<<" ";
      count_pairwise_compatible(group_pattern_ints[g], n_compat);
      count_different_codes_in_vector(group_pattern_ints[g], n_diff_row, n_diff_col);
      int max_n_compat = n_patterns + 1;
      int max_n_diff_row = n_patterns + 1;
      int max_n_diff_col = n_patterns + 1;
      for(int p=0; p<group_pattern_ints[g].size(); p++){
         if(n_compat[p]   < max_n_compat)   max_n_compat    = n_compat[p];
         if(n_diff_row[p] < max_n_diff_row) max_n_diff_row  = n_diff_row[p];
         if(n_diff_col[p] < max_n_diff_col) max_n_diff_col  = n_diff_col[p];
      };
      // cout<<" n_compat ="<<min_n_compat<<" n_diff_row= "<<min_n_diff_row<<" n_diff_col="<<min_n_diff_col<<endl;
      if( max_n_compat <= 1 or max_n_diff_row <= 1 or max_n_diff_col <= 1){
         group_rank[g] = 1;
      }
   }
   pattern_rank.resize(n_patterns);
   for(int p=0; p<n_patterns; p++){
      pattern_rank[p] =  group_rank[simple_ints[p]];
   }
}


int PatternsCodes::patterns_capacity_rank_correction(vector <int> & pints, int start_point){
   int capacity = 0;
   valarray<bool> group_flag(n_simplified, false);
   for(int i = start_point; i< pints.size() ; i++){
      capacity++;
   }
   return capacity;
}

