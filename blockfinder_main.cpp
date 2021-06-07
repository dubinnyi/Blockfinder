#include<pthread.h>
//temporary
#include<time.h>
#include<iomanip>
#include<iostream>
#include<fstream>
#include<sstream>

#include<boost/asio.hpp> // host_name
#include<boost/program_options.hpp>
namespace po = boost::program_options;
//temporary


//#include<boost/chrono.hpp> // process_system_cpu_clock

#include "ctpl.h"
#include "blockfinder.h"
#include "speedo.h"
#include "schemetest.h"

using namespace std;
using namespace boost::asio::ip;

int main(int argc, char *argv[]) {
   
   unsigned int ncpu_onboard = std::thread::hardware_concurrency(), 
                ncpu = ncpu_onboard;
   string name_ncs;
   string restart_file;
   bool   restart_flag;
   string blocks_file;
   bool   blocks_flag;
   string print_codes_file;
   bool   print_codes_flag;
   bool   dry_run_flag;
   bool   run_groups_flag;
   bool   no_dry_table_flag;
   bool   sort_alpha_flag;
   NCS ncs;
   int samples, min_depth, parallel_depth, task_size;
   int auto_min_t_free = -1;
   bool list_ncs;
      
   po::options_description desc("Usage:\nblockfinder NCS samples min-patterns [options]\nAvailable options");
   po::positional_options_description pos_desc;
  
   try {

      desc.add_options()
         ("help", "produce help message")
         ("NCS", po::value<string>(&name_ncs), "Name of NCS, e.g. NC2. Use option --list-ncs for all supporten NCS")
         ("samples", po::value<int>(&samples), "Number of samples, e.g. 3")
         ("min-patterns", po::value<int>(&min_depth), "Minimal number of patterns in scheme, e.g. 3")
         ("ncpu,n", po::value<unsigned int>(&ncpu)->default_value(ncpu_onboard), "Number of CPU to use")
         ("parallel-depth,d", po::value<int>(&parallel_depth)->default_value(1), "Execute in parallel from that depth")
         ("task-size,task", po::value<int>(&task_size)->default_value(200), "The size of task to be executed in parallel")
         ("restart", po::value<string>(&restart_file), "Restart file with unfinished tasks, e.g. \"restart.txt\". The file is created by python script viewrun.txt")
         ("print-codes", po::value<string>(&print_codes_file), "Print codes to separate file")
         ("log-speed", po::bool_switch())
         ("log-state", po::bool_switch())
         ("dry-run", po::bool_switch(&dry_run_flag),"Make all preparations, do not start BlocFinder for actual calculations")
         ("run-groups", po::bool_switch(&run_groups_flag),"Run blockfinder for each simplified group separately")
         ("no-dry-table", po::bool_switch(&no_dry_table_flag),"Do not dry table of codes to exclude useless patterns")
         ("sort-alpha", po::bool_switch(&sort_alpha_flag),"Sort patterns alphabetically (not recommended)")
         ("blocks", po::value<string>(&blocks_file), "Blocks file with initial blocks")
         ("list-ncs", "List all supporten NCS")
      ;
      pos_desc.add("NCS", 1)
              .add("samples", 1)
              .add("min-patterns", 1);
      po::variables_map vm;
      po::store(po::command_line_parser(argc, argv).
              options(desc).positional(pos_desc).run(), vm);
      po::notify(vm);
     
      if(ncpu>ncpu_onboard){
        cout<<"Too many CPU selected: "<<ncpu<<", set ncpu to "<<ncpu_onboard<<endl;
        ncpu=ncpu_onboard;
      }


      if (vm.count("help")) {
         cout << desc << endl;
         return 1;
      }
      
      if (vm.count("list-ncs")) {
         cout << "The list of supported NCS:" << endl;
         for(string s: ncs.ALL_NCS_NAME)
           cout<<s<<" ";
         cout<<endl;
         return 1;
      }

      if (vm.count("NCS")) {
         name_ncs = vm["NCS"].as<string>();
         try{
            NCS ncs_new = get_NCS(name_ncs);
            ncs = ncs_new;
         }catch(UnknownNCS& e){
            cout<< e.what() << std::endl;
            return(-1);
         }
      } else {
         cout <<"Missing recquired argument: NCS"<<endl;
         cout << desc << endl;
         return -1;
      };


      if (! vm.count("samples")) {
         cout <<"Missing recquired argument: samples"<<endl;
         cout << desc << endl;
         return -1;
      }
      
      if (! vm.count("min-patterns")) {
         cout <<"Missing recquired argument: min-patterns"<<endl;
         cout << desc << endl;
         return -1;
      }
      
      if( vm.count("restart") )
        restart_flag = true;
      else
        restart_flag = false;
            
      if( vm.count("blocks") )
        blocks_flag = true;
      else
        blocks_flag = false;

            
      if( vm.count("print-codes") )
        print_codes_flag = true;
      else
        print_codes_flag = false;


   }catch(const po::error & ex){
      cerr << ex.what() <<endl;
      cerr << desc << endl;
      cerr << "exiting"<<endl;
      return(-1);
   };

   Speedo overall_timer;
   overall_timer.start();
   cout <<"===== BlockFinder started ====="<<endl;
   cout <<"Host:     "<<host_name()<<endl;
   cout <<"Command:  ";

   for(int arg=0; arg<argc;arg++)cout<<argv[arg]<<" ";
   cout<<endl;
   cout<<"Date/Time: "<<overall_timer.readable_date_time()<<endl;

   cout << "NCS with name "<<ncs.name << " generated" << endl;
   if( (ncs.name=="ALT12" || ncs.name=="NCDAT12" ) && samples == 3){
       auto_min_t_free = 8;
       cout<<"Automatically set min_t_free = 8"<<endl;
   }
   else if( ( ncs.name=="ALT12" || ncs.name=="NCDAT12" ) && samples == 4){
      auto_min_t_free = 16;
      cout<<"Automatically set min_t_free = 16"<<endl;
   }else{
      cout<<"Checks of min_t_free is swithced off"<<endl;
   }


   PatternsCodes empty_table;
   BlockFinder blockfinder(ncs, samples);

   blockfinder.min_depth = min_depth;
   blockfinder.parallel_depth = parallel_depth;
   blockfinder.task_size = task_size;
   blockfinder.dry_table_flag = ( not no_dry_table_flag );
   blockfinder.sort_patterns_flag = ( not sort_alpha_flag );
   blockfinder.min_t_free = auto_min_t_free; 
   blockfinder.setup_blockfinder();

   if(print_codes_flag){
      blockfinder.code_table.print_codes(print_codes_file);
      cout<<"Codes are written to file "<<print_codes_file<<endl;
      exit(0);
   }

   vector<Scheme_compact> initial_blocks;
   if(blocks_flag){
      read_blocks_from_file(&blockfinder.code_table, &ncs, blocks_file, initial_blocks, true);
   }
   SchemeTest scheme_tester(initial_blocks);
   if(blocks_flag){
      blockfinder.scheme_tester = & scheme_tester; 
      //cout<<"Self-test of the initial blocks:"<<endl;
      for(auto scheme : initial_blocks){
         bool result = scheme_tester.check(scheme.simplified);
         //cout<<scheme.simplified_vector_string()<<" -- "<<(result?"True":"False")<<endl;
      }
   }

   if(run_groups_flag){
      cout<<endl<<"RUN blockfinder for each group separately"<<endl;
      Task4run group_task; // default task, runs all iterations
      for(int group=0; group<blockfinder.code_table.n_simplified; group++){
         int group_min_depth = 2;
         BlockFinder bgroup(ncs, samples, group_min_depth, auto_min_t_free, empty_table);
         bgroup.generate_initial_patterns(blockfinder.code_table.group_pattern_text[group]);
         group_task.name = blockfinder.code_table.unique_simplified_patterns[group];
         cout<<endl<<endl<<"RUN group "<<group<<" "<<group_task.name<<endl<<endl<<endl;
         bgroup.run_task_flag = true;
         bgroup.task_id = group;
         bgroup.maincycle(group_task);
      }
   }

   cout<<"CREATE TASKS STARTED "<<endl;
   blockfinder.create_tasks();
   cout<<"CREATE TASKS FINISHED. "<<to_string(blockfinder.tasks.size())<<" TASKS CREATED"<<endl;

   vector<Task4run> run_tasks(0);
   if(!restart_flag){
      run_tasks = blockfinder.tasks;
   }else{
      cout<<endl;
      cout<<"RESTART FILE "<<restart_file<<" WILL BE USED TO RUN UNFINISHED TASKS ONLY"<<endl;
      string line;
      ifstream restart (restart_file);
      if (restart.is_open()) {
        int i=0;
        while ( getline (restart,  line) ){
          if(i<blockfinder.tasks.size()){
            Task4run restart_task(line);
       int task_number = restart_task.number;
       if(task_number >= blockfinder.tasks.size()){
              cerr<<line<<endl;
         cerr<<"   Error: task number "<<task_number<<
                " is out of range: "<<blockfinder.tasks.size()<<endl;
         i++;
         continue;
       }
       Task4run generated_task = blockfinder.tasks[restart_task.number];
       if(! ( restart_task == generated_task) ){
         cerr<<line<<endl;
         cerr<<"  -- Tasks do not match! "<<endl;
         cerr<<"    restart_task ="<<  (string)restart_task<<endl;
         cerr<<"  generated_task ="<< (string)generated_task<<endl;
       }else{
         run_tasks.push_back(restart_task);
         if (i<10){
            cout<<line<<"  -- OK"<<endl;
         }
         if(i==10)cout<<"..."<<endl;
       }
          }
          i++;
        }
        restart.close();
     }else 
        cout <<" UNABLE TO OPEN FILE "<<restart_file<<endl;

        cout<<"RESTART FILE '"<<restart_file<<"' PARSED, "<<
        run_tasks.size()<<" TAKS OF " <<blockfinder.tasks.size()<<" ARE READY FOR RESTART"<<endl;
     //exit(1);
   }

   if(dry_run_flag){
     cout<<"--dry-run option is given, exiting"<<endl;
     exit(0);
   }


   ctpl::thread_pool p(ncpu);

   ofstream taskfile;

   cout << endl;
   int numbertask=0;
   // std::future<void> qw = p.push(find_schemes,
   cout<<"RUNNING ALL "<<to_string(run_tasks.size())<<" IN PARALLEL ON "<<to_string(p.size())<<" THREADS"<<endl;

   cout_locker Cout_Lock;
   blockfinder.cout_lock = & Cout_Lock;
   for (Task4run task : run_tasks){
      p.push(find_schemes, blockfinder, task);
      numbertask=numbertask+1;
   }

   // Wait for all jobs to finish
   p.stop(true);
   cout<<"EXECUTION OF ALL "<<to_string(numbertask)<<" TASKS FINISHED"<<endl;
   overall_timer.stop();
   
   printf("The CPU usage time:   %f seconds\n", overall_timer.cpu_time);
   printf("The wall clock time:  %f seconds\n", overall_timer.wall_time);
   printf("CPU/wall clock ratio: %5.2f\n", overall_timer.cpu_time/overall_timer.wall_time);
   cout<<"Date/Time: "<<overall_timer.readable_date_time()<<endl;

   return 0;
}
