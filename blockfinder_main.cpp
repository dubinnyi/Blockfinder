#include<pthread.h>
#include<time.h>
#include<iomanip>
#include<iostream>
#include<sstream>
#include<boost/asio.hpp> // host_name
//#include<boost/chrono.hpp> // process_system_cpu_clock
#include<chrono>

#include "ctpl.h"
#include "blockfinder.h"

using namespace std;
using namespace boost::asio::ip;
using namespace std::chrono;

int main(int argc, char *argv[]) {
	string name_ncs;
	int begin, end, samples, min_depth;
	int auto_min_t_free = -1;
	cout << "started " << argc << endl;
	if (argc <= 1) {


		cout << "No arguments" << endl;


		return -1;
	}
	else if(argc ==4){

		name_ncs = argv[1];
		stringstream convert(argv[2]);
		convert >> samples;
		stringstream convertdepth(argv[3]);
		convertdepth >> min_depth;
		// cout << "readed" << endl;

		NCS ncs = get_NCS(name_ncs);
		//cout << "////" << endl;
		system_clock::time_point now_time = system_clock::now();
		auto readable_time = system_clock::to_time_t(now_time);
		cout <<"===== BlockFinder started ====="<<endl;
		cout <<"Host:     "<<host_name()<<endl;
		cout <<"Command:  ";
		for(int arg=0; arg<4;arg++)cout<<argv[arg]<<" ";
		cout<<endl;
		cout<<"Date/Time: "<<std::ctime(&readable_time)<<endl;

		cout << "NCS with name "<<ncs.name << " generated" << endl;
		if(ncs.name=="ALT12" && samples == 3){
			auto_min_t_free = 8;
			cout<<"Automatically set min_t_free = 8"<<endl;
		}
		else if(ncs.name=="ALT12" && samples == 4){
			auto_min_t_free = 16;
			cout<<"Automatically set min_t_free = 8"<<endl;
		}else{
			cout<<"Checks of  min_t_free is swithced off"<<endl;
		}



		BlockFinder b(samples, ncs, min_depth, auto_min_t_free);

	unsigned int ncpu = std::thread::hardware_concurrency();
        ctpl::thread_pool p(ncpu);

    




        ofstream taskfile;

	cout<<"CREATE TASKS STARTED "<<endl;
        b.create_tasks();
	cout<<"CREATE TASKS FINISHED. "<<to_string(b.tasks.size())<<" TASKS CREATED"<<endl;

	//BlockFinder test(samples, ncs, min_depth, auto_min_t_free);
	//test.recover_from_counters(b.tasks[0].counter_start, 99);
	//test.maincycle(b.tasks[0].counter_start, b.tasks[0].counter_end);
	//exit(0);


	// Initialize timers
	struct timespec wall_clock_start, wall_clock_finish;
	clock_t cpu_usage_start, cpu_usage_finish;	
	cpu_usage_start = clock();
	clock_gettime(CLOCK_MONOTONIC, &wall_clock_start);


		cout << endl;
		int numbertask=0;
	//	std::future<void> qw = p.push(find_schemes,
	cout<<"RUNNING ALL "<<to_string(b.tasks.size())<<" IN PARALLEL ON "<<to_string(p.size())<<" THREADS"<<endl;

        for (Task4run t : b.tasks){
            //cout << " numbertask : " << numbertask<< endl;
			//BlockFinder b_test(samples, ncs, min_depth, true, -1);
			//b_test.recoverfromcounters(t.counter_start, numbertask);


            p.push(find_schemes, samples, ncs, min_depth,  -1, numbertask,b.code_table, b.patterns_listl, b.patterns[0], t.counter_start, t.counter_end );
            //break;
           // b_test.maincycle(t.counter_start, t.counter_end);
          //  break ;

            numbertask=numbertask+1;




        }



	// Wait for all jobs to finish
	p.stop(true);
	cout<<"EXECUTION OF ALL "<<to_string(numbertask)<<" TASKS FINISHED"<<endl;
	cpu_usage_finish = clock();
	clock_gettime(CLOCK_MONOTONIC, &wall_clock_finish);

       // taskfile.open("tasks.cpp");


        /**
        b_c.counter= {17, 0, 0, 0};
        b_c.depth =3;
        b_c.find_schemes();
        cout << "b_c " << b_c.results_found;**/




	double cpu_usage_seconds = (double)(cpu_usage_finish - cpu_usage_start) / CLOCKS_PER_SEC;
	double wall_clock_seconds = (wall_clock_finish.tv_sec - wall_clock_start.tv_sec);
	wall_clock_seconds += (wall_clock_finish.tv_nsec - wall_clock_start.tv_nsec) / 1000000000.0;
	
	printf("The CPU usage time:   %f seconds\n", cpu_usage_seconds);
	printf("The wall clock time:  %f seconds\n", wall_clock_seconds);
	printf("CPU/wall clock ratio: %5.2f\n", cpu_usage_seconds/wall_clock_seconds);

	//cout << b.results_found << " found " << endl;
	//cout << b.result.size() << " size of results" << endl;
	//cout << b.out1 << endl;
	//blocks_file << b.result_string;

	//blocks_file.close();
	//printf("Solutions found: %d\n", b.results_found);
	//cout << "Blocks found (" << b.results_found << ") are written to file " << block_filename << endl;








	}
	else {
		name_ncs = argv[1];
		stringstream convert(argv[2]);
		convert >> samples;
		stringstream convertdepth(argv[3]);
		convertdepth >> min_depth;

		stringstream convertbegin(argv[4]);
		convertbegin >> begin;

		stringstream convertend(argv[5]);
		convertend >> end;

		cout << "readed" << endl;

		NCS ncs = get_NCS(name_ncs);
		cout << "////" << endl;

		cout << ncs.name << " ?" << endl;


		//BlockFinder b(samples, ncs, min_depth, true, -1);


		cout << " Q" << endl;
		fstream blocks_file;
		string block_filename = ncs.name + to_string(samples) + "_" + to_string(min_depth) + "_cpp.elb";
		//blocks_file.open(block_filename, fstream::out);
		clock_t start = clock();

		//b.find_schemes();

		clock_t end = clock();
		double seconds = (double)(end - start) / CLOCKS_PER_SEC;




		getchar();




	}



	return 0;
}
