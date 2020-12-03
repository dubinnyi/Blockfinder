
#include "blockfinder.h"
#include "sort_permutation.h"

//#define DEBUG false

// instance of static private member
std::mutex cout_locker::mtx;

BlockFinder::BlockFinder( int bsamples, NCS &bncs, int bmin_depth, int bmin_t_free, PatternsCodes &patternscode, bool generation):
   samples(bsamples),
   ncs(bncs),
   min_depth(bmin_depth),
   min_t_free(bmin_t_free),
   check_t_free(false),
   create_task_flag(false),
   check_counter_limits(false),
   task_id(-1),
   depth(0),
   max_depth(0),
   out1("")
{
   if (min_t_free >= 0) {
      check_t_free = true;
      index_of_type_T = ncs.index_of_labeltype('T');
   } 
   if (min_depth <= 1) {
      min_depth = 2;
   }
   counter.push_back(0); 

   if(generation) {
      patterns_text = generate_all_text_patterns(samples);
      generate_initial_patterns(patterns_text);
   }else{
      code_table= patternscode;
   }
   scheme.setscheme(&code_table,"1", &bncs, samples, {});
   
   speedo_results.clear();
   speedo_codes.clear();
   speedo_iterations.clear();

}


void find_schemes ( int id,  int bsamples, NCS &bncs, int bmin_depth, int bmin_t_free, PatternsCodes &patternscode, 
                    vector<string> &patterns_text, vector <int> &patterns_ints, Task4run & task_for_run, cout_locker *cl) {

    BlockFinder b (bsamples, bncs, bmin_depth, bmin_t_free, patternscode, false )   ;
    b.cout_lock = cl;
    b.patterns_text=patterns_text;
    b.patterns.push_back(patterns_ints);
    b.recover_from_counters(task_for_run);
    b.maincycle(task_for_run);
}


void BlockFinder::generate_initial_patterns(vector<string> &p_text){
   bool new_algo = true;
   bool sort_patterns = true;
   bool debug = true;

   patterns_text = p_text;
   code_table.setPatternsCodes(patterns_text, ncs);
   patterns.clear();
   patterns.push_back(code_table.pattern_ints);
   
   cout << "Code Table generated, " << code_table.n_patterns <<
           " patterns, " << code_table.n_simplified << " simplified" << endl;
   
   cout<<"List of unique simplified patterns with multiplicities:"<<endl;
   code_table.print_simplified_patterns(cout);

   if (new_algo) {
     //cout<<"CODES"<<endl;
     //code_table.print_codes(cout);
     //cout<<endl;
     vector<size_t> n_diff_row, n_diff_col, n_compat;
     vector<int> group_simple_ints = code_table.simple_ints;
     if(debug){
        cout<<"START THE CODE TABLE DRYING/SORTING"<<endl;
        cout<<"Calculate different codes in each row and column (ROWS and COLS below)"<<endl;
     }
     code_table.count_different_codes_in_vector(code_table.pattern_ints, n_diff_row, n_diff_col);

     if(debug)
        cout<<"Calculate number of campatible patterns for each pattern using pairwise checking (#Compat)"<<endl;
     code_table.count_pairwise_compatible(code_table.pattern_ints, n_compat);

     vector<string> patterns_text_sorted = p_text;
     vector<int> prank = code_table.pattern_rank;
     if(sort_patterns){
        if(debug){
           cout<<"Sorting all patterns by #Compat"<<endl;
        }
        vector<size_t> p(n_compat.size());
        iota(p.begin(), p.end(), 0);
        vector<int> & si = code_table.simple_ints;
        vector<string> & pt = patterns_text_sorted;
        sort(p.begin(), p.end(), [&prank, &n_compat, &si, &pt](const size_t a, const size_t b){ 
            return (
               prank[a] < prank[b] or 
               prank[a] == prank[b] and n_compat[a] < n_compat[b] or 
               prank[a] == prank[b] and n_compat[a] == n_compat[b] and si[a] < si[b] or 
               prank[a] == prank[b] and n_compat[a] == n_compat[b] and si[a] == si[b] and pt[a] < pt[b]) ; 
            } );
        //   auto p  = sort_permutation(n_compat, [&n_compat](const size_t &a, const size_t &b){ return (n_compat[a] > n_compat[b]); } );

        //vector<string> patterns_text_sorted = apply_permutation(patterns_text, p);
        apply_permutation_in_place(prank, p);
        apply_permutation_in_place(patterns_text_sorted, p);
        apply_permutation_in_place(group_simple_ints, p);
        apply_permutation_in_place(n_diff_row, p);
        apply_permutation_in_place(n_diff_col, p);
        apply_permutation_in_place(n_compat, p);
     }
     
     Vbool n_codes_Ok(false, code_table.n_patterns);
     Vbool n_compat_Ok(false, code_table.n_patterns);
     Vbool pattern_Ok(false, code_table.n_patterns);

     vector<string> filtered_patterns_text;
     int n_filtered = 0;
     for(int i=0; i<code_table.n_patterns; i++){
        if( n_diff_row[i]>=min_depth and n_diff_col[i]>= min_depth ){
           n_codes_Ok[i] = true;
        }
        if( n_compat[i]>= min_depth ){
           n_compat_Ok[i] = true;
        }
        if( n_codes_Ok[i] and n_compat_Ok[i]){
           pattern_Ok[i] = true;
           filtered_patterns_text.push_back(patterns_text_sorted[i]);
           n_filtered++;
        }
     }

     if(debug){
        cout<<setw(4)<<"#"<<" "<<setw(code_table.n_samples)<<"PAT"<<" ";
        cout<<setw(3)<<"GR"<<" "<<setw(4)<<"RANK"<<" "<<setw(4)<<"ROWS"<<" "<<setw(4)<<"COLS"<<" ";
        cout<<setw(6)<<"Codes?"<<" "<<setw(7)<<"#Compat"<<" "<<setw(7)<<"Compat?"<<" "<<setw(6)<<"Pattern?";
        cout<<endl;
        for(int i=0; i<code_table.n_patterns;i++){
           cout<<setw(4)<<i<<" "<<patterns_text_sorted[i]<<" "<<setw(3)<<group_simple_ints[i]<<" ";
           cout<<setw(4)<<prank[i]<<" ";
           cout<<setw(4)<<n_diff_row[i]<<" "<<setw(4)<<n_diff_col[i]<<" ";
           cout<<setw(6)<<(n_codes_Ok[i]?"Ok":"!!!")<<" ";
           cout<<setw(7)<<(n_compat[i])<<" ";
           cout<<setw(7)<<(n_compat_Ok[i]?"Ok":"!!!")<<" ";
           cout<<setw(6)<<(pattern_Ok[i]?"Ok":"!!!")<<" ";
           cout<<endl;
        };

        cout<<"n_filtered = "<<n_filtered<<", n_patterns = "<<code_table.n_patterns<<endl;
     }
     if(filtered_patterns_text != p_text){
        cout<<endl<<"The list of patterns was changed."<<endl;
        cout<<"RECURSIVELY CALL GENERATE_INITIAL_PATTERNS"<<endl<<endl;
        generate_initial_patterns(filtered_patterns_text);
     }
   }
}


vector<string> BlockFinder::generate_all_text_patterns(int  bsamples, bool top ){
   vector <string> new_set;
   vector <string> current_set;
   if (bsamples == 0) {
      new_set = {""}; //previously "0"
      return new_set;
    }
   
   current_set = generate_all_text_patterns(bsamples - 1, false);
   //new_set = { };
   string new_pattern;
   for (string item : current_set) {
      for (labeltype option : ncs.label_types){
         new_pattern = item + option.name;
         if (top==true){
            if (ncs.check_power(new_pattern, min_depth)){
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
   cout_lock->lock();
   if(!run_task_flag){
     if (check_t_free) {
        cout << "Started maincycle. Samples =" << samples << " min depth = " << min_depth << " min_t_free=" << min_t_free << endl; 
        out1 = "started samples =" + to_string(samples) + " min depth = " + to_string(min_depth) + " min_t_free=" + to_string(min_t_free);
     }
     else {
        out1 = "Started maincycle. Samples =" + to_string(samples) + " min depth = " + to_string(min_depth);
        cout << "started samples =" << samples << " min depth = " << min_depth << endl;
     }
     cout << " Total number of patterns is  " << patterns[0].size() << endl;
   }
   if(run_task_flag){
     results_filename = ncs.name + "_"+to_string(samples)+"_"+to_string(min_depth)+"_"+task.name+"_cpp.elb";
     result_ofstream.open(results_filename);
     run_name = "[" + task.name + "]";
   }else{
     results_filename = ncs.name + "_" + to_string(samples) + "_" + to_string(min_depth) + "_cpp.elb";
     result_ofstream.open(results_filename);
     run_name = "[BlockFinder"+to_string(samples)+"]";
   }
   if(result_ofstream.is_open()){
      result_ofstream << "[NCS = " << ncs.name << "]"<<endl<< "[Deuterated = " << (ncs.deuterated?"True":"False")<< "]"<<endl;
      result_ofstream.flush();
   }else{
      cout<<"No file to save results for run "<<run_name<<"results_filename= "<<results_filename<<endl;
   }
   cout_lock->unlock();

   speedo_results.start();
   speedo_codes.start(); 
   speedo_iterations.start();
}


void BlockFinder::maincycle( Task4run & task_for_run   ) {
   vector<int> *patterns_current_ptr; 
   vector <int> next_patterns;
   int start_point;
   int patterns_left;
   int patterns_capacity_left; 
   bool flag_t_free;

   task = task_for_run;
   check_counter_limits=true;
   if (task.start.size()==0 && task.end.size()==0){
       check_counter_limits = false;
   }

   start_blockfinder();

   while (true) {
      if( check_counters_reached_the_end_of_task() ){
         break;
      }
      next_iteration_output();
      //
      // the vector<int>  is copied
      patterns_current_ptr = &(patterns[depth]);
      if (depth == 0 && ( (counter[0] + min_depth )> patterns_current_ptr->size())) {
         break;
      }
      start_point = 1 + counter[depth];
      patterns_left = patterns_current_ptr->size() - start_point;
      patterns_capacity_left = code_table.patterns_capacity_rank_correction(*patterns_current_ptr, start_point);
      //
      // The scheme is copied
      back_up_schemes.push_back(scheme);
      scheme.add_pattern((*patterns_current_ptr)[counter[depth]]);
      if (patterns_capacity_left < (min_depth - depth - 1)   ) {
         go_back();
         continue;
      }
      if (patterns_left == 0) {
         if (scheme.patterns.size() >= min_depth) {
            save_result();
         }
         go_back();
         continue;
      }
      get_next_patterns(*patterns_current_ptr, patterns_left, start_point, next_patterns);
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
         }
         go_parallel(); 
      }
      check_max_depth();
   }
   cout_lock->lock();
   cout<< run_name<<" finished task "<<to_string(task_id)<<" after "<<speedo_iterations<< " iterations"<<endl;
   cout_lock->unlock();
}


void BlockFinder::create_tasks() {
   vector<int> *patterns_current_ptr;
   vector<int> next_patterns;
   int start_point;
   int patterns_left;
   int patterns_capacity_left; 
   bool flag_t_free;

   create_task_flag = true;
   int task_counter = 0;
   int task_number  = 0;
   cout<<"create_tasks: Starting with parallel_depth= "<<parallel_depth<<" and task_size= "<<task_size<<endl;

   vector <Task4run> t;
   Task4run task1;
   task1.start={0};
   task1.number = task_number;
   task1.update_name();
   task1.end={};
   tasks.push_back(task1);
   task_number++;

   int kt=0;
   start_blockfinder();

   while (true) {

        next_iteration_output();

        patterns_current_ptr = &(patterns[depth]);
        if (depth == 0 && ( (counter[0] + min_depth )> patterns_current_ptr->size())) {
            break;
        }
        start_point = 1 + counter[depth];
        patterns_left = patterns_current_ptr->size() - start_point;
        patterns_capacity_left = code_table.patterns_capacity_rank_correction(*patterns_current_ptr, start_point);

        back_up_schemes.push_back(scheme);

        scheme.add_pattern((*patterns_current_ptr)[counter[depth]]);
        if (patterns_capacity_left < (min_depth - depth - 1)   ) {
            go_back();
            continue;
        }
        if (patterns_left == 0) {
            if (scheme.patterns.size() >= min_depth) {
                save_result();
            }
            go_back();
            continue;
        }
        get_next_patterns(*patterns_current_ptr, patterns_left, start_point, next_patterns);

        flag_t_free = true;
        if (check_t_free) {
            flag_t_free = check_have_enought_t_free(scheme, next_patterns);
        }
        if ( next_patterns.size() != 0 && flag_t_free){

            if(depth == parallel_depth){

                task_counter=task_counter+1;

                if (task_counter%task_size==0){

                    tasks.back().end = counter;
                    task1.start=counter;
                    task1.end={};
                    task1.number = task_number;
                    task1.update_name();
                    tasks.push_back(task1);
                    task_number++;
                    task_counter=0;
                }

                go_parallel();
            }
            if(depth<parallel_depth){
                go_deeper(next_patterns);
            }

        }
        else {
            if (scheme.patterns.size() >= min_depth) {
                save_result();
            }
            go_parallel();
        }
        check_max_depth();
    }

   tasks.back().end=counter;


   ofstream file1("tasksend.txt");

   for (Task4run c: tasks){
      file1<<(string)c<<endl;

   }
   file1.close();

   cout<< "create_tasks: Finished after "<<speedo_iterations<< " iterations"<<", "<<tasks.size()<<" tasks generated"<<endl;
}




bool BlockFinder::check_counters_reached_the_end_of_task(){
    if(!check_counter_limits)return(false);
    
    bool end_of_task = false;
    int order=0;
    
    if(counter.size()==task.end.size()){
       for (int c=0; c< task.end.size(); c++ ){
          if (counter[c]<task.end[c]){
              break;
          }
          order =order+1;
       }
       if (order==task.end.size()){
          end_of_task=true;
       }
    }
    return end_of_task;
}


void BlockFinder::recover_from_counters( const Task4run & task_for_recover ){
   recover_from_counters(task_for_recover.start, task_for_recover.number);
}


void BlockFinder::recover_from_counters( const vector <int> & recover_counters, int numbertask){

   vector<int> current_patterns = patterns[0];
   vector<int> new_patterns;
   for (int c=0; c<recover_counters.size()-1; c++){

      back_up_schemes.push_back(scheme);

      scheme.add_pattern(current_patterns[recover_counters[c]]);

      get_next_patterns(current_patterns, current_patterns.size()-recover_counters[c]-1, recover_counters[c]+1, new_patterns);

      patterns.push_back(new_patterns);

      current_patterns = new_patterns;

   }

   depth = recover_counters.size()-1;
   counter=recover_counters;
   if(result_ofstream.is_open())result_ofstream.close();
   run_task_flag = true;
   task_id = numbertask;
}


void BlockFinder::next_iteration_output(){
    static const long long LOG_ITERATOR = 1000000; /* Log every one million of iterations */
    speedo_iterations++;
    if (speedo_iterations.counter % LOG_ITERATOR == 0) {
        cout_lock->lock();
        ostringstream log;
        string name;
//        double try_pattern_speed_M = (double)count_try_pattern/speedo_iterations.wall_time/1000000.;
        log << run_name;
        log << setw(6) << speedo_iterations.counter/LOG_ITERATOR<<"M ";
        log << setw(8) << setprecision(0) << fixed << speedo_iterations.wall_time <<" sec ";
        log << setw(7) << setprecision(2) << fixed << (double)(speedo_iterations.wall_speed()/1000.) << " Kiter/sec ";
        log << setw(7) << setprecision(2) << fixed << (double)(speedo_codes.wall_speed()/1000000.)  << " Mcodes/sec";
        log << " max_P=" << setw(2) << setiosflags(ios::left) << max_depth + 1;
        log << " ELB_found= " << setw(6) << speedo_results.counter;
        //log << endl;
        log << " Counters: ";
        for(int d=0; d< depth && d<13; d++){
          log << " " << setw(3) << setiosflags(ios::right) << counter[d] << "/";
          log        << setw(3) << setiosflags(ios::left) << patterns[d].size() - min_depth + 1 + d;
        };
        
      
      cout << log.str() << endl;
     
      if(result_ofstream.is_open())result_ofstream.flush();
      speedo_iterations.check_point();
      speedo_codes.check_point();
      cout_lock->unlock();

    }
}


void BlockFinder::check_max_depth(){
   if (depth > max_depth){
      max_depth = depth;
//      out1 = "[BlockFinder" + to_string(samples) + " ] New max depth:" + to_string(max_depth); 
//      cout<<out1<<endl;
   }
} 


void BlockFinder::get_next_patterns(vector <int> & patterns1, int patterns_left, 
                        int  start_point, vector<int> & result_next_patterns) {
   result_next_patterns = {};
   for (int i = 0; i < patterns_left; i++)  {
      if( scheme.try_pattern(patterns1[i + start_point])) {
         result_next_patterns.push_back(patterns1[i + start_point]);
      }
   }
   speedo_codes.count((2*scheme.patterns.size()+1)*patterns_left);
}


void BlockFinder::get_next_patterns_speedo(vector <int> & patterns1, int patterns_left, 
                        int  start_point, vector<int> & result_next_patterns) {
   //static Speedo local_speedo;
   //local_speedo.start();

   result_next_patterns = {};
   for (int i = 0; i < patterns_left; i++)  {
      if( scheme.try_pattern_speedo(patterns1[i + start_point], speedo_codes)) {
         result_next_patterns.push_back(patterns1[i + start_point]);
      }
   }
   
   //local_speedo.stop((2*scheme.patterns.size()+1)*patterns_left);
   //speedo_codes+= local_speedo;
}


void BlockFinder::go_parallel(){
   scheme =back_up_schemes[depth];
   back_up_schemes.pop_back();
   counter[depth] = counter[depth] + 1;
}




void BlockFinder::go_deeper(const vector <int> & next_patterns) {
   patterns.push_back(next_patterns);
   counter.push_back(0);
   depth = depth + 1;
}


void BlockFinder::go_back() {
   depth = depth -1;
   patterns.pop_back();
   counter.pop_back();
   counter[counter.size() - 1] = counter[counter.size() - 1] + 1;


   back_up_schemes.pop_back();
   scheme = back_up_schemes[back_up_schemes.size() - 1];
   back_up_schemes.pop_back();

}


void BlockFinder::save_result() {
   vector<int> empty_vec = {};
   if (check_t_free && !(check_have_enought_t_free(scheme, empty_vec))) {
      return; 
   };
   if( not scheme.check_codes()){
      cout_lock->lock();
      cerr<<"check_codes failed for the scheme found!"<<endl;
      cerr<<scheme.full_str();
      cout_lock->unlock();
   };
   int depth_of_scheme;
   depth_of_scheme= scheme.patterns.size();
   Scheme_compact new_scheme;
   new_scheme.code_tab_ptr = scheme.code_tab_ptr;
   new_scheme.samples      = scheme.samples;
   new_scheme.patterns     = scheme.patterns;
   new_scheme.simplified   = scheme.simplified;
   //cout << "see new scheme " << new_scheme.patterns[0] << endl;
   //new_scheme = scheme;
   new_scheme.sort();
   if(   result.find(depth_of_scheme) != result.end() ){ 
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

void BlockFinder:: blockfinder_finished() {
   cout_lock->lock();
   out1 = "[BlockFinder] finished search in" + to_string(samples) + "samples after " + 
     to_string(speedo_iterations.counter) + " iterations and " + 
     to_string(speedo_codes.counter) + " codes checked. " + 
     to_string(speedo_results.counter) + " ELB schemes found";
   cout_lock->unlock();
}



bool BlockFinder::check_have_enought_t_free(const Scheme & scheme, const vector<int> &  patterns_left) {
   tuple <int, int> t;
   vector <int> simplified_scheme;
   simplified_scheme.assign( begin(scheme.simplified), end(scheme.simplified)  );
  // t = code_table.count_type_in_list_of_simplified(scheme.simplified, index_of_type_T);
   t = code_table.count_type_in_list_of_simplified( simplified_scheme, index_of_type_T);
   int scheme_t = get<0>(t);
   int scheme_t_free = get<1>(t); 
   tuple <int, int> t2; 
   t2 = code_table.count_type_in_list_of_patterns(patterns_left, index_of_type_T);
   int left_t = get<0>(t2);
   int left_t_free = get<1>(t2);
   bool result = scheme_t_free + left_t_free >= min_t_free;
#ifdef DEBUG
   cout<<"check_have_enought_t_free"<<endl;
   cout<<"  scheme_t_free= "<<scheme_t_free<<" left_t_free= "<<left_t_free;
   cout<<" t_free="<<scheme_t_free + left_t_free<<" check = "<<result<<endl;;
   getchar();
#endif
   return (result); 
}


void PatternsCodes::simplify_list_of_patterns(const vector<int>& list_of_patterns, vector<int>& result) {
   result.assign(n_simplified, 0);
   for (int pattern : list_of_patterns) {
      result[simple_ints[pattern]]++;
   }
}


tuple<int, int > PatternsCodes::count_type_in_list_of_simplified(const vector <int>& arg_simplified, int index_of_type) {
   int count_type = 0;
   int count_all = 0;
   int has_t;
#ifdef DEBUG
   cout<<"count_type_in_list_of_simplified"<<endl;
#endif
   for( int s=0; s<n_simplified; s++) {
      if(!arg_simplified[s])continue;
      has_t = check_label_in_simplified(s, index_of_type);
      count_type = count_type + has_t * arg_simplified[s];
      count_all = count_all  + arg_simplified[s];
#ifdef DEBUG
      cout<<"s= "<<setw(3)<<s<<" "<<unique_simplified_patterns[s]<<":"<<setw(2)<<arg_simplified[s]<<" has_t="<<has_t<<endl;
#endif
   }
#ifdef DEBUG
   cout<<"count_type= "<<count_type<<" count_free= "<<count_all - count_type<<endl;
   getchar();
#endif
   return  make_tuple(count_type, count_all - count_type);
}


tuple<int, int > PatternsCodes::count_type_in_list_of_patterns(
    const vector <int>& arg_patterns, int index_of_type) {
   int count_type = 0;
   int count_all = 0;
   int has_t;
   int p;
#ifdef DEBUG
   cout<<"count_type_in_list_of_patterns"<<endl;
#endif
   for( int i=0; i< arg_patterns.size(); i++) {
      p = arg_patterns[i];
      has_t = 0;
      if (check_label_in_pattern(p, index_of_type) )
        has_t = 1;
#ifdef DEBUG
      cout<<"p= "<<setw(3)<<p<<" "<<patterns[p]<<" has_t="<<has_t<<endl;
#endif
      count_type = count_type + has_t;
      count_all = count_all  + 1;
   }
#ifdef DEBUG
   cout<<"count_type= "<<count_type<<" count_free= "<<count_all - count_type<<endl;
   getchar();
#endif
   return  make_tuple(count_type, count_all - count_type);
}


void  BlockFinder::write_result(Scheme_compact  new_scheme) {
//   results_found = results_found + 1;
   speedo_results++;
   if(result_ofstream.is_open()){
      result_ofstream << "# iterator = " <<speedo_iterations << endl;
      result_ofstream << new_scheme.full_str()<<endl;
      //result_ofstream.flush();
   }
}
