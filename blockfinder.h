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
        //Scheme scheme("1", ncs, samples, {});
        void create_tasks(  );
        //vector <string> patterns; //temporary
        //new?
        vector <vector <int>> patterns;
        vector <string> patterns_listl;
        int depth;
        int max_depth;
        int parallel_depth=1;
        int task_size= 50;

        int  task_id;    /* Number of task to be executed              */
        string run_name;
        vector <Task4run> tasks;
        bool check_t_free;
        int  min_t_free = -1;
        bool run_task_flag;     /* true after recover_from_counters           */
        bool create_task_flag;  /* true if tasks are created                */
        string results_filename;
        ofstream result_ofstream;
        vector <int> counter = {}; //temporary
        vector <Scheme> back_up_schemes = {};
        unsigned long long results_found;
        map <unsigned long long, set< Scheme_compact>> result;//soe
        PatternsCodes code_table;
        unsigned long long iterator;
        int index_of_type_T;
        string out1;
        time_t start_cpu_time, tick_cpu_time;
        struct timespec start_wall_time, tick_wall_time;
        vector <string> generate_patterns(int  samples, bool top = true);
        void open_files();
        void start_blockfinder();
        void maincycle( const vector <int> start , const vector <int> end   );
        void recover_from_counters( const vector <int> & current_counters, int numbertask = -1);



        void next_iteration_output();
        void go_back();
        void save_result();
        bool check_have_enought_t_free(const Scheme & scheme, const vector<int> & patterns_left);
        void write_result(Scheme_compact  new_scheme);
        void get_next_patterns(vector <int> & patterns, int patterns_left, int start_point, vector<int> & result);
        void go_deeper(vector <int> next_patterns);
        void go_parallel();
        void check_max_depth();
        void find_schemes();
        void blockfinder_finished();
        BlockFinder(int bsamples, NCS &bncs, int bmin_depth, int bmin_t_free, bool generation=true);
        //bfm? 


};




void find_schemes (int id,   int bsamples, NCS &bncs, int bmin_depth, int bmin_t_free, int numbertask, PatternsCodes &patternscode, vector <string> &patterns_listl, vector <int> &patterns, vector <int> counter_start = {}, vector <int> counter_end ={} );


void find (const BlockFinder b);












