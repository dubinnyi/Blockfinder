#include"blockfinder.h"

BlockFinder::BlockFinder( int bsamples, NCS bncs, int bmin_depth, bool bblock_finder_mode, int bmin_t_free, int bbegin, int bend ){	
	samples = bsamples;
	ncs = bncs;
	min_depth = bmin_depth;
	check_t_free = false; 
	block_finder_mode = bblock_finder_mode;
	depth = 0;  
	string results_filename = ncs.name + "_" + to_string(samples) + "_" + to_string(min_depth) + "_cpp.elb";
	result_ofstream.open(results_filename);
	min_t_free = bmin_t_free;
	index_of_type_T = index_of_type(labeltype('T', 1, 1, 0));
	if (min_t_free >= 0) {
		check_t_free = true;
	} 
	if (min_depth <= 1) {
		min_depth = 2;
	}

	patterns_listl = generate_patterns(samples);

	patterns.push_back({});
	for (int i =0; i<patterns_listl.size(); i++){
	    patterns[0].push_back(i);
	}

	//patterns.push_back(generate_patterns(samples ));
	counter.push_back(0); 
	results_found = 0; 
	max_depth = 0;
	iterator = 0;
	//code_table.setPatternsCodes(patterns, ncs);

	code_table.setPatternsCodes(patterns_listl, ncs, min_depth);


	scheme.setscheme(code_table,"1", ncs, samples, {});

	begin = bbegin;
	end = bend; 
	result_ofstream << "[NCS = " << ncs.name << "]"<<endl<<
	                "[Deuterated = " << (ncs.deuterated?"True":"False")<< "]"<<endl;
	
	out1 = "";
	start_time = clock();
}



vector<string> BlockFinder::generate_patterns(int  bsamples, bool top ) {
	vector <string> new_set;
	vector <string>  current_set;
	if (bsamples == 0) {
		new_set = {"" }; //previously "0"
		return new_set;
}
	
	current_set = generate_patterns(bsamples - 1, false);
	//new_set = { };
	string new_pattern;
	for (string item : current_set) {
		for (labeltype option : ncs.label_types) {
			new_pattern = item + option.name;
			if (top==true ) {
				if (ncs.check_power(new_pattern, min_depth) ){


					new_set.push_back(new_pattern); 
				}
		}
			else {
				new_set.push_back(new_pattern);
			} 
		}
	}  
	return new_set;
}


int index_of_type(labeltype label_type) {
	labeltype typeX('X', 0, 0, 0);
	labeltype typeN('N', 1, 0, 0);
	labeltype typeC('C', 0, 0, 1);
	labeltype typeD('D', 1, 1, 1);
	labeltype typeA('A', 0, 1, 0);
	labeltype typeT('T', 1, 1, 0);
	labeltype typeS('S', 1, 0, 1);
	labeltype typeF('F', 0, 1, 1);
	static map<char, int>  basic_types = { {typeX.name, 0}, { typeN.name,1 } , {typeC.name, 2} , {typeD.name, 3} , {typeA.name, 4} , {typeT.name, 5} ,{ typeS.name, 6}, {typeF.name, 7} };
	return basic_types[label_type.name]; 
} 

void BlockFinder::start_blockfinder() {
	if (check_t_free) {
		cout << "started samples =" << samples << " min depth = " << min_depth << " min_t_free=" << min_t_free << endl; 
		out1 = "started samples =" + to_string(samples) + " min depth = " + to_string(min_depth) + " min_t_free=" + to_string(min_t_free);
	}
	else {
		out1 = "started samples =" + to_string(samples) + " min depth = " + to_string(min_depth);
		cout << "started samples =" << samples << " min depth = " << min_depth << endl;
	}
	cout << " total number of patterns is  " << patterns[0].size() << endl; 
}


/**
void BlockFinder::recoverfromcounters( vector <int> currentcounters){

    Scheme tmp("1", ncs, samples );

    vector <string> temp_patterns = patterns;

    for (int c: currentcounters){
        tmp.add_pattern(temp_patterns[c]);

        temp_patterns = get_next_patterns(temp_patterns, temp_patterns.size()-c, c);


    }



}**/

void BlockFinder::maincycle() {
	vector<int> patternscurrent, next_patterns;
	int start_point;
	int patterns_left; 
	bool flag_t_free;
	bool st = false;
	vector <int >  ct;


    std::ofstream iterlog;          // поток для записи
    iterlog.open("iterlog.txt");


    ct.push_back(8);
	ct.push_back(2);
	ct.push_back(10);




	while (true) {
	    iterlog<<iterator<<" "<<depth<<" : ";
	    for(int d=0;d<depth+1; d++)
            iterlog<<counter[d]<<" ";
        iterlog<<endl;

		next_iteration_output();
 
		patternscurrent = patterns[depth];
		if (depth == 0 && ( (counter[0] + min_depth )> patternscurrent.size())) {
			break;
		}
		start_point = 1 + counter[depth];
		patterns_left = patternscurrent.size() - start_point;

		if(counter == ct){

		    st=true;



		}


		if(st){

		  //  cout << "patterns left " << patterns_left << endl;
           // cout << "start point " << start_point << endl;
          //  cout << " next patterns  " << next_patterns.size() << endl;
           // cout << "patterns current " << patternscurrent.size()   << endl;

        }


		back_up_schemes.push_back(scheme);





		scheme.add_pattern(patternscurrent[counter[depth]], code_table);
		if (patterns_left < (min_depth - depth - 1)   ) {
			go_back();
			continue;
		}
		if (patterns_left == 0) {
			if (scheme.patterns.size() >= min_depth) {
				save_result();
/**
				cout << "block 1" << endl;
                cout << " save " << endl;
                for (int c: counter){
                    cout << c<< " ";


                }

                cout << endl;**/

			}
			go_back();

			// temporary break;
			//break;


			continue;
		}
		next_patterns = get_next_patterns(patternscurrent, patterns_left, start_point);


		flag_t_free = true;
		if (check_t_free) {
			flag_t_free = check_have_enought_t_free(scheme, next_patterns);
		}
		if ( next_patterns.size() != 0 && flag_t_free){
			
			go_deeper(next_patterns);
		}
		else {
			if (scheme.patterns.size() >= min_depth) {
				save_result();

/**
                cout << "block 2" << endl;
                cout << " save " << endl;
                for (int c: counter){
                    cout << c<< " ";


                }

                cout << endl;**/


			}
			go_parallel();	
		}
		check_max_depth();
	}
	cout<< "BlockFinder finished after "<<iterator<< " iterations"<<endl;
	result_ofstream.close();
}

inline void BlockFinder::next_iteration_output()
{
    iterator++;
    if (iterator % 100000 == 0) {
      ostringstream log;
      log<< "[BlockFinder" << to_string(samples) << "]";
      log<< setw(9) << iterator;
      time_t now = clock();
      double time_in_run = (double)(now - start_time) / CLOCKS_PER_SEC;
      log << setw(8) << setprecision(2) << fixed << time_in_run << " sec";
      log << " max_P=" << setw(2) << setiosflags(ios::left) << max_depth + 1;
      log << " ELB_found= " << setw(6) << results_found;
      for(int d=0; d< depth && d<10; d++){
	log << " " << setw(3) << setiosflags(ios::right) << counter[d] << "/";
	log        << setw(3) << setiosflags(ios::left) << patterns[d].size() - min_depth + 1 + d;
      }
      cout << log.str() << endl;
    }
}

inline void BlockFinder::go_parallel() {
	scheme =back_up_schemes[depth];
	back_up_schemes.pop_back();
	counter[depth] = counter[depth] + 1;
}

inline void BlockFinder::check_max_depth() {
	if (depth > max_depth) {
		max_depth = depth;
		if (block_finder_mode) {
			out1 = "[BlockFinder" + to_string(samples) + " ] New max depth:" + to_string(max_depth); 
		}
	}

} 

vector <int> BlockFinder::get_next_patterns(vector <int> patterns, int patterns_left, int  start_point) {
	vector <int> next_patterns ;
	for (int i = 0; i < patterns_left; i++)  {
		if( scheme.try_pattern(patterns[i + start_point], code_table)) {
			next_patterns.push_back(patterns[i + start_point]);
		}
	}
	return next_patterns;
}

void BlockFinder::go_deeper(vector <int> next_patterns) {
	patterns.push_back(next_patterns);
	counter.push_back(0);
	depth = depth + 1;
}

void BlockFinder::find_schemes() {
	start_blockfinder();
	maincycle();
	blockfinder_finished();
}


void BlockFinder:: blockfinder_finished() {
	out1 = "[BlockFinder] finished search in" + to_string(samples) + "samples after " + to_string(iterator) + " iterations " + to_string(results_found) + " ELB schemes found";
}

inline void BlockFinder::go_back() {
	depth = depth -1;
	patterns.pop_back();
	counter.pop_back();
	counter[counter.size() - 1] = counter[counter.size() - 1] + 1;


	back_up_schemes.pop_back();
	scheme = back_up_schemes[back_up_schemes.size() - 1];
	back_up_schemes.pop_back();

}

inline void BlockFinder::save_result() {
	if (check_t_free && !(check_have_enought_t_free(scheme, {}))) {
		return; 
	}
	int depth_of_scheme;
	depth_of_scheme= scheme.patterns.size();
	Scheme new_scheme;
	new_scheme = scheme;
	new_scheme.sort();
	if(	result.find(depth_of_scheme) != result.end() ){ 
		if (result[depth_of_scheme].find(new_scheme) == result[depth_of_scheme].end()) {

			result[depth_of_scheme].insert(new_scheme);
			write_result(new_scheme);
		}
	} 
	else {
		result[depth_of_scheme] = { new_scheme }; 
		write_result(new_scheme);
	}
}

bool BlockFinder::check_have_enought_t_free(Scheme scheme, vector<int>  patterns_left) {
	tuple <int, int> t;
	t = count_type_in_list_of_simplified(scheme.simplified, index_of_type_T); 
	int scheme_t = get<0>(t); 
	int scheme_t_free = get<1>(t); 
	tuple <int, int> t2; 
	t2 = count_type_in_list_of_patterns(patterns_left, labeltype('T', 1, 1, 0), code_table);
	int left_t = get<0>(t2);
	int left_t_free = get<1>(t2);
	return (scheme_t_free + left_t_free >= min_t_free); 
}

map <string, int>  simplify_list_of_patterns(vector<int> list_of_patterns, PatternsCodes &patternscode) {
	map <string, int> simplified;
	string simple_pattern;
	for (int pattern : list_of_patterns) {
		simple_pattern = patternscode.simple_form[pattern];
		if(  simplified.size() != 0 &&   simplified.count(simple_pattern)!=0  ){
			simplified[simple_pattern] = simplified[simple_pattern] + 1;
	}
		else {
			simplified[simple_pattern] = 1;
		} 
	}
	return simplified;
}

tuple<int, int > count_type_in_list_of_simplified(map <string, int> simplified, int index_of_type) {
	int count_type = 0;
	int count_all = 0;
	int has_t;
	for(  auto sp : simplified) {
		has_t = 0;
			if (int(sp.first[index_of_type])) {
				has_t = 1;
			}
			count_type = count_type + has_t * sp.second;
			count_all = count_all  + sp.second;
	}
	return  make_tuple(count_type, count_all - count_type);
}

tuple<int, int > count_type_in_list_of_patterns(vector<int> patterns, labeltype label_type, PatternsCodes &patternscode) {
	map <string, int>  simplified = simplify_list_of_patterns(patterns, patternscode);
	//vector <string> p;

	/** map <int, string> simplified;
	for (int c : patterns){
		simplified[c]=patternscode.simple_form[c];

	}**/

	int index_of_t = index_of_type(label_type);
	return count_type_in_list_of_simplified(simplified, index_of_t);
}

inline void  BlockFinder::write_result(Scheme  new_scheme) {
	results_found = results_found + 1;
	result_ofstream << "# iterator = " + to_string(iterator) << endl;
	string s =  new_scheme.full_str(code_table);

	result_ofstream << s<<endl;
}
