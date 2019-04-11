#include"blockfinder.h"

BlockFinder::BlockFinder( int bsamples, NCS &bncs, int bmin_depth, int bmin_t_free, bool generation){
   samples = bsamples;
   ncs = bncs;
   min_depth = bmin_depth;
   check_t_free = false; 
   run_task_flag = false;
   create_task_flag = false;
   task_id = -1;
   depth = 0;  
   min_t_free = bmin_t_free;
   if (min_t_free >= 0) {
      check_t_free = true;
      index_of_type_T = ncs.index_of_labeltype('T');
      cout<<"index of type T is "<<index_of_type_T<<endl;
   } 
   if (min_depth <= 1) {
      min_depth = 2;
   }
    if(generation) {
        patterns_listl = generate_patterns(samples);

        patterns.push_back({});
        for (int i = 0; i < patterns_listl.size(); i++) {
            patterns[0].push_back(i);
        }
    }
   //patterns.push_back(generate_patterns(samples ));
   counter.push_back(0); 
   results_found = 0; 
   max_depth = 0;
   iterator = 0;
   //code_table.setPatternsCodes(patterns, ncs);
    if (generation) {
        code_table.setPatternsCodes(patterns_listl, ncs);
        cout << "Code Table generated, " << code_table.n_patterns <<
             " patterns, " << code_table.n_simplified << " simplified" << endl;

    }
   scheme.setscheme(&code_table,"1", &bncs, samples, {});
   
   out1 = "";
   start_cpu_time = clock();
   clock_gettime(CLOCK_MONOTONIC, &start_wall_time);
   tick_cpu_time = start_cpu_time;
   tick_wall_time = start_wall_time;

}


void find_schemes ( int id,  int bsamples, NCS &bncs, int bmin_depth, int bmin_t_free, int numbertask,PatternsCodes &patternscode, vector <string> &patterns_listl1, vector <int> &patterns1, vector <int> counter_start, vector <int> counter_end ) {

    BlockFinder b (bsamples, bncs, bmin_depth, bmin_t_free, false )   ;
    b.patterns_listl=patterns_listl1;
    b.patterns.push_back(patterns1);
    b.code_table=patternscode;

    b.recover_from_counters(counter_start, numbertask);
    b.maincycle(counter_start, counter_end);

    /*
    for ( auto c : b.scheme.simplified ){
        cout << c.first << " " << c.second << endl;
    }*/


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
     ostringstream task_id_string;
     task_id_string<<setw(4)<<setfill('0')<<task_id;
     results_filename = ncs.name + "_"+to_string(samples)+"_"+to_string(min_depth)+"_"+task_id_string.str()+"_cpp.elb";
     result_ofstream.open(results_filename);
     run_name = "[Task" + task_id_string.str() + "]";
   }else{
     results_filename = ncs.name + "_" + to_string(samples) + "_" + to_string(min_depth) + "_cpp.elb";
     result_ofstream.open(results_filename);
     run_name = "[BlockFinder"+to_string(samples)+"]";
   }
   if(result_ofstream.is_open()){
      result_ofstream << "[NCS = " << ncs.name << "]"<<endl<< "[Deuterated = " << (ncs.deuterated?"True":"False")<< "]"<<endl;
      result_ofstream.flush();
   }

}


void BlockFinder::maincycle( const vector <int> start, const vector <int> end   ) {
   vector<int> patternscurrent, next_patterns;
   int start_point;
   int patterns_left; 
   bool flag_t_free;
   //bool st = false;
   //vector <int >  ct;


//    std::ofstream iterlog;          // поток для записи
//    iterlog.open("iterlog"+to_string(task_id)+".txt");


    bool limits=true;
    bool check_limits=false;
    if (start.size()==0 && end.size()==0){
        limits = false;
        check_limits=false;
    }
    int order = 0;







    start_blockfinder();

    while (true) {
        order=0;

        if(limits && counter.size()==end.size()){
            for (int c=0; c< end.size(); c++ ){
                if (counter[c]<end[c]){
                    //order =order+1;
                    //check_limits=true;
                    break;
                }
                order =order+1;
            }
/**
         if(counter[end.size()-1]>=end.back()){
            order=order+1;

         }**/

            if (order==end.size()){
                check_limits=true;
/*
                cout << "end while "<< endl;
                for (int q: counter){
                    cout << q << " ";
                }

                cout << endl;

*/
            }

        }

        if(check_limits){
            break;
        }










//        iterlog<<iterator<<" "<<depth<<" : ";
//          for(int d=0;d<depth+1; d++)
//            iterlog<<counter[d]<<" ";
//        iterlog<<endl;

      next_iteration_output();
 
      patternscurrent = patterns[depth];
      if (depth == 0 && ( (counter[0] + min_depth )> patternscurrent.size())) {
         break;
      }
      start_point = 1 + counter[depth];
      patterns_left = patternscurrent.size() - start_point;



      back_up_schemes.push_back(scheme);





      scheme.add_pattern(patternscurrent[counter[depth]]);
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
      get_next_patterns(patternscurrent, patterns_left, start_point, next_patterns);

//		cout << iterator << " next patterns size : " << next_patterns.size() << endl;


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
   cout<< run_name<<" finished task "<<to_string(task_id)<<" after "<<iterator<< " iterations"<<endl;
}
















void BlockFinder::create_tasks() {
    vector<int> patternscurrent, next_patterns;
    int start_point;
    int patterns_left;
    bool flag_t_free;
    //bool st = false;
    //vector <int >  ct;

    create_task_flag = true;
    int task_counter=0;
    cout<<"create_tasks: Starting with parallel_depth= "<<parallel_depth<<" and task_size= "<<task_size<<endl;

    vector <Task4run> t;
    //Task4run task1({0}, {});
    Task4run task1;
    task1.counter_start={0};
    task1.counter_end={};
    tasks.push_back(task1);

    int kt=0;



//    std::ofstream iterlog;          // поток для записи  
//    iterlog.open("CreateTastks_iterlog.txt");
    start_blockfinder();

    while (true) {
        // iterlog<<iterator<<" "<<depth<<" : ";
        // for(int d=0;d<depth+1; d++)
        //     iterlog<<counter[d]<<" ";
        // iterlog<<endl;

        next_iteration_output();

        patternscurrent = patterns[depth];
        if (depth == 0 && ( (counter[0] + min_depth )> patternscurrent.size())) {
            break;
        }
        start_point = 1 + counter[depth];
        patterns_left = patternscurrent.size() - start_point;



        back_up_schemes.push_back(scheme);





        scheme.add_pattern(patternscurrent[counter[depth]]);
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
        get_next_patterns(patternscurrent, patterns_left, start_point, next_patterns);


        flag_t_free = true;
        if (check_t_free) {
            flag_t_free = check_have_enought_t_free(scheme, next_patterns);
        }
        if ( next_patterns.size() != 0 && flag_t_free){



            if(depth == parallel_depth){


                //out << "back up " << endl;






                task_counter=task_counter+1;
                //cout << "this depth " << endl;
                //kt=kt+counter[pool_depth_test];

                if (task_counter%task_size==0){

                    tasks.back().counter_end = counter;

                    //Task4run task1(counter, {});
                    task1.counter_start=counter;
                    task1.counter_end={};

                    tasks.push_back(task1);



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

   tasks.back().counter_end=counter;


   ofstream file1("tasksend.txt");
   //blockfinder_finished();

   for (Task4run c: tasks){

      for (int i: c.counter_start){
         file1 <<setw(3)<< i << " ";

      }
      file1<<" : ";
      for (int i: c.counter_end){
         file1 <<setw(3)<< i << " ";

      }

      file1 << endl;

   }


   //iterlog.close();
   file1.close();

   cout<< "create_tasks: Finished after "<<iterator<< " iterations"<<", "<<tasks.size()<<" tasks generated"<<endl;
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





















void BlockFinder::next_iteration_output()
{
    static const long long LOG_ITERATOR = 1000000; /* Log every one million of iterations */
    iterator++;
    if (iterator % LOG_ITERATOR == 0) {
      ostringstream log;
      string name;
      log<< run_name;
      log<< setw(6) << iterator/LOG_ITERATOR<<"M ";
      //time_t now_cpu_time = clock();
      //double cpu_time_per_log = (double)(now_cpu_time - start_cpu_time) / CLOCKS_PER_SEC;
      
      struct timespec now_wall_time;
      clock_gettime(CLOCK_MONOTONIC, &now_wall_time);
      double wall_time_per_run = (double)(now_wall_time.tv_sec - start_wall_time.tv_sec);
      log << setw(8) << setprecision(0) << fixed << wall_time_per_run <<" sec ";

      double tick_time_per_log = (now_wall_time.tv_sec - tick_wall_time.tv_sec);
      tick_time_per_log += (now_wall_time.tv_nsec - tick_wall_time.tv_nsec) / 1000000000.0;
      log << setw(7) << setprecision(2) << fixed << (double)(LOG_ITERATOR/tick_time_per_log/1000) << " Kiter/sec";

      log << " max_P=" << setw(2) << setiosflags(ios::left) << max_depth + 1;
      log << " ELB_found= " << setw(6) << results_found;
      for(int d=0; d< depth && d<13; d++){
   log << " " << setw(3) << setiosflags(ios::right) << counter[d] << "/";
   log        << setw(3) << setiosflags(ios::left) << patterns[d].size() - min_depth + 1 + d;
      }
      cout << log.str() << endl;
     
      if(result_ofstream.is_open())result_ofstream.flush();
      //tick_cpu_time = now_cpu_time;
      tick_wall_time = now_wall_time;

    }
}

void BlockFinder::go_parallel() {
   scheme =back_up_schemes[depth];
   back_up_schemes.pop_back();
   counter[depth] = counter[depth] + 1;
}

void BlockFinder::check_max_depth() {
   if (depth > max_depth) {
      max_depth = depth;
//      out1 = "[BlockFinder" + to_string(samples) + " ] New max depth:" + to_string(max_depth); 
//      cout<<out1<<endl;
   }

} 

void BlockFinder::get_next_patterns(vector <int> & patterns1, int patterns_left, int  start_point, vector<int> & result_next_patterns) {
   result_next_patterns = {};
   for (int i = 0; i < patterns_left; i++)  {
      if( scheme.try_pattern(patterns1[i + start_point])) {
         result_next_patterns.push_back(patterns1[i + start_point]);
      }
   }
}













void BlockFinder::go_deeper(vector <int> next_patterns) {
   patterns.push_back(next_patterns);
   counter.push_back(0);
   depth = depth + 1;
}


void BlockFinder:: blockfinder_finished() {
   out1 = "[BlockFinder] finished search in" + to_string(samples) + "samples after " + to_string(iterator) + " iterations " + to_string(results_found) + " ELB schemes found";
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
   if (check_t_free && !(check_have_enought_t_free(scheme, {}))) {
      return; 
   }
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

bool BlockFinder::check_have_enought_t_free(const Scheme & scheme, const vector<int> &  patterns_left) {
   tuple <int, int> t;
   t = code_table.count_type_in_list_of_simplified(scheme.simplified, index_of_type_T); 
   int scheme_t = get<0>(t); 
   int scheme_t_free = get<1>(t); 
   tuple <int, int> t2; 
   t2 = code_table.count_type_in_list_of_patterns(patterns_left, index_of_type_T);
   int left_t = get<0>(t2);
   int left_t_free = get<1>(t2);
   return (scheme_t_free + left_t_free >= min_t_free); 
}

void PatternsCodes::simplify_list_of_patterns(const vector<int>& list_of_patterns, vector<int>& result) {
   result.assign(n_simplified, 0);
   for (int pattern : list_of_patterns) {
      result[simple_ints[pattern]]++;
   }
}

tuple<int, int > PatternsCodes::count_type_in_list_of_simplified(
    const vector <int>& simplified, int index_of_type) {
   int count_type = 0;
   int count_all = 0;
   int has_t;
   for( int sp : simplified) {
      has_t = 0;
      if (check_label_in_simplified(sp, index_of_type) )
        has_t = 1;
      count_type = count_type + has_t * simplified[sp];
      count_all = count_all  + simplified[sp];
   }
   return  make_tuple(count_type, count_all - count_type);
}

tuple<int, int > PatternsCodes::count_type_in_list_of_patterns(
    const vector <int>& patterns, int index_of_type) {
   int count_type = 0;
   int count_all = 0;
   int has_t;
   for( int p : patterns) {
      has_t = 0;
      if (check_label_in_pattern(p, index_of_type) )
        has_t = 1;
      count_type = count_type + has_t;
      count_all = count_all  + 1;
   }
   return  make_tuple(count_type, count_all - count_type);
}


void  BlockFinder::write_result(Scheme_compact  new_scheme) {
   results_found = results_found + 1;
   if(result_ofstream.is_open()){
      result_ofstream << "# iterator = " + to_string(iterator) << endl;
      result_ofstream << new_scheme.full_str()<<endl;
      //result_ofstream.flush();
   }
}
