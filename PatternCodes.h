//
// Created by kamilla on 06.03.19.
//

#ifndef PATTERNCODES_H_INCLUDED
#define PATTERNCODES_H_INCLUDED
#include"ncs.h"

typedef valarray<bool> Vbool;

class PatternsCodes{
public:
    vector <string> patterns;
    NCS ncs;
    int           n_samples;      /* number of samples */
    unsigned long n_patterns;     /* number of patterns */
    unsigned long n_simplified;   /* number of unique simplified patterns */
    unsigned long n_codes;        /* number of different codes in table */

    valarray <int>  codes;        /* of size n_patterns * n_patterns */
    vector <string> codes_list;  
    vector <string> simple_form;  /* of size n_patterns */
    vector <string> simple_label; /* of size n_simplified */
    vector <int>    pattern_ints; /* of size n_patterns, actually 0, 1, 2, .... n_patterns -1 */
    vector <int>    simple_ints;  /* of size n_patterns, 
	 index of simple_form in unique_simplified_patterns */
    vector <string> unique_simplified_patterns; /* of size n_simplified */
    map <string, int> code_to_number;
    vector <int>    simple_multiplicity; /* of size n_simplified */
      /* Number of patterns that are simplified to this simple form
       * The sum of vector elements is equal to n_patterns */
    vector < vector <int> >    group_pattern_ints;  /* of size n_simplified */
    vector < vector <string> > group_pattern_text;  /* of size n_simplified */
      /* Patterns in each groups as separate vector */
    vector <int>    group_rank;  /* of size n_simplified */
      // group rank = 1 or 2
      // group rank = 1 means than only one element from group could be
      // taken for the scheme building
      // group rank = 2 means that more then 2 elenemts could be taken

    valarray<bitset<N_LABELTYPES> > have_labeltype_simplified_flag; /* of size n_simplified */
    valarray<bitset<N_LABELTYPES> > have_labeltype_pattern_flag;    /* of size n_patterns */

    inline int calc_code_fast(int pattern1, int pattern2) const{
      return codes[pattern1*n_patterns+pattern2];
    }
    
    inline bool check_label_in_simplified(int simplified, int index_of_label_type) const {
      return have_labeltype_simplified_flag[simplified][index_of_label_type];
    }

    inline bool check_label_in_pattern(int pattern, int index_of_label_type) const {
      return have_labeltype_pattern_flag[pattern][index_of_label_type];
    }

    PatternsCodes( vector<string> a_patterns, NCS ncs );
    PatternsCodes();
    void setPatternsCodes(vector<string> a_patterns, NCS a_ncs );
    void print_pattern_flags(ostream &out);
    void print_codes(string file);
    void print_codes(ostream &out);
    void print_simplified_patterns(ostream &out);
    
    void  simplify_list_of_patterns(const vector<int> & list_of_patterns, vector<int> & result);
    tuple<int, int > count_type_in_list_of_simplified(const vector <int> & simplified, int index_of_type);
    tuple<int, int > count_type_in_list_of_patterns(const vector <int> &  patterns, int index_of_type);

    void count_different_codes(const vector <int> &  patterns, int p, size_t & n_diff_raw, size_t & n_diff_col);
    void count_different_codes_in_vector(const vector <int> &  patterns, vector <size_t> &n_diff_raw, vector <size_t> &n_diff_col);
    void count_pairwise_compatible(const vector <int> & patterns, int p, size_t & n_compat, Vbool & compat);
    void count_pairwise_compatible(const vector <int> & patterns, vector< size_t> & n_compat);
    
private:
    void create_simplified_table();
    void create_labeltype_flags();
    void create_codes_table();
    void calculate_group_ranks();
};



bool pattern_bigger(string pattern1, string  pattern2);

string simplify_pattern(string pattern);





#endif // PATTERNCODES_H_INCLUDED
