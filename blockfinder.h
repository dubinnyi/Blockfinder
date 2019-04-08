//#pragma once
#include"scheme.h"
#include "tasks.h"
class BlockFinder {
public:
	

	vector<labeltype> types;
	int samples;
	NCS ncs;
	int min_depth;
	Scheme scheme;
	int begin, end; 
	//Scheme scheme("1", ncs, samples, {});
    void create_tasks();
    //vector <string> patterns; //temporary
	//new?
	vector <vector <int>> patterns;
	vector <string> patterns_list;
	int depth;
	int max_depth;
	int parallel_depth=1;
	int task_size=100;
	int task_id;
    vector <Task4run> tasks;
	bool check_t_free;
	string results_filename;
	ofstream result_ofstream;
	int  min_t_free = -1;
	vector <int> counter = {}; //temporary
	vector <Scheme> back_up_schemes = {};
	bool  block_finder_mode;
	int results_found;
	map <int, set< Scheme>> result;//soe
	PatternsCodes code_table;
	int iterator;
	int index_of_type_T;
	string out1;
	time_t start_time;
	//BlockFinder( NCS bncs, int bmin_depth, bool bblock_finder_mode, int  bmin_t_free = -1);
	vector <string> generate_patterns(int  samples, bool top = true);
	void start_blockfinder();
	void maincycle( vector <int> start={}, vector <int> end={}  );
    void recoverfromcounters( vector <int> currentcounters, int numbertask = -1);



    void next_iteration_output();
	void go_back();
	void save_result();
	bool check_have_enought_t_free(Scheme scheme, vector<int>  patterns_left);
	void  write_result(Scheme  new_scheme);
	vector <int> get_next_patterns(vector <int> patterns, int patterns_left, int  start_point);
	void go_deeper(vector <int> next_patterns);
	void go_parallel();
	void check_max_depth();
	void find_schemes();
	void blockfinder_finished();
	BlockFinder(int bsamples, NCS bncs, int bmin_depth, bool bblock_finder_mode, int bmin_t_free, int begin = -1, int  end = -1);
	//bfm? 
};

