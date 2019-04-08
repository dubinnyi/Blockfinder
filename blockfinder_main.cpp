#include<time.h> 
#include<iomanip>
#include<iostream>
#include <sstream>
#include"blockfinder.h"

using namespace std;

int main(int argc, char *argv[]) {
	string name_ncs;
	int begin, end, samples, min_depth;
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
		cout << "readed" << endl;

		NCS ncs = get_NCS(name_ncs);
		cout << "////" << endl;

		cout << ncs.name << " ?" << endl;


		BlockFinder b(samples, ncs, min_depth, true, -1);

        BlockFinder b_c(samples, ncs, min_depth, true, -1);

        //blocks_file.open(block_filename);


       // b_c.find_schemes();
        cout << "b_c " << b.results_found;

      //  PatternsCodes q(b_c.scheme.patterns, b_c.ncs);


        cout << " scheme size " << b.scheme.patterns.size() << endl;
       // cout << " q size " << q.codes.size() << endl;

/**
        for( vector<int> a : q.codes){
            for (int b : a){

                cout << b<< " ";
            }
            cout << endl;
        }**/


		//ofstream blocks_file;
		//string block_filename = ncs.name + "_" + to_string(samples) + "_" + to_string(min_depth) + "_cpp.txt";
		//blocks_file.open(block_filename, fstream::out);

		clock_t start = clock();

		//b.find_schemes();


        ofstream taskfile;

        b.create_tasks();


		cout << endl;
		int numbertask=0;

		for (Task4run t : b.tasks){

			BlockFinder b_test(samples, ncs, min_depth, true, -1);
			b_test.recoverfromcounters(t.counter_start, numbertask);
			b_test.maincycle(t.counter_start, t.counter_end);

            ostringstream block_ofilename;
            block_ofilename<<ncs.name<<"_"<<setw(3)<<setfill('0')<<numbertask<<"_cpp.elb";
            //block_filename = block_ofilename.str();

            //blocks_file.open(block_filename);
            //blocks_file << b.result_string;
            //blocks_file<< endl;
            //blocks_file.close();


            numbertask=numbertask+1;


        }




		clock_t end = clock();




        /**

        b_c.counter= {17, 0, 0, 0};
        b_c.depth =3;
        b_c.find_schemes();
        cout << "b_c " << b_c.results_found;**/




		double seconds = (double)(end - start) / CLOCKS_PER_SEC;
		cout << b.results_found << " found " << endl;
		//cout << b.result.size() << " size of results" << endl;
		//cout << b.out1 << endl;
		//blocks_file << b.result_string;

		//blocks_file.close();
		printf("Solutions found: %d\n", b.results_found);
		//cout << "Blocks found (" << b.results_found << ") are written to file " << block_filename << endl;
		printf("The time: %f seconds\n", seconds);
		printf("Average rate: %6.2f iter/sec\n", b.iterator/seconds);








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
