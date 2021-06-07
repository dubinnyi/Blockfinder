#include<iomanip>
#include<iostream>
#include<fstream>
#include<sstream>
#include<string>

#include<boost/program_options.hpp>
namespace po = boost::program_options;

#include "schemetest.h"
#include "blockfinder.h"

using namespace std;

int main(int argc, char *argv[]) {

  string name_ncs;
  int samples, min_depth;
  string reference_file;
  string check_file;
  NCS ncs;

  po::options_description desc("Usage:\ntest_scheme-check reference.elb check.elb "  
      "-- read the blocks from reference.elb file and then verify every block from check.elb\n");
  po::positional_options_description pos_desc;

  
  try {
      desc.add_options()
        ("NCS", po::value<string>(&name_ncs), "Name of NCS, e.g. NC2.")
        ("samples", po::value<int>(&samples), "Number of samples, e.g. 3")
        ("min-patterns", po::value<int>(&min_depth), "Minimal number of patterns in scheme, e.g. 3")
        ("reference", po::value<string>(&reference_file), "Reference file with blocks")
        ("check", po::value<string>(&check_file), "File with blocks to be checked")
        ;

      pos_desc.add("NCS", 1)
              .add("samples", 1)
              .add("min-patterns", 1)
              .add("reference", 1)
              .add("check", 1);

      po::variables_map vm;
      po::store(po::command_line_parser(argc, argv).
              options(desc).positional(pos_desc).run(), vm);
      po::notify(vm);

      if (vm.count("help")) {
         cout << desc << endl;
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



   }catch(const po::error & ex){
      cerr << ex.what() <<endl;
      cerr << desc << endl;
      cerr << "exiting"<<endl;
      return(-1);
   };

   PatternsCodes empty_table;
   BlockFinder blockfinder(ncs, samples);
   blockfinder.min_depth = min_depth;
   blockfinder.setup_blockfinder();
   
   cout<<"blockfinder "<<ncs.name<<" "<<samples<<" generated"<<endl;

   vector<Scheme_compact> reference_blocks, check_blocks;
   cout<<"Reading "<<reference_file<<endl;
   read_blocks_from_file(&blockfinder.code_table, &ncs, reference_file, reference_blocks, true);
   cout<<"Reading "<<check_file<<endl;
   read_blocks_from_file(&blockfinder.code_table, &ncs, check_file,     check_blocks, true);
   SchemeTest scheme_tester(reference_blocks);

   cout<<"TEST of blocks from "<<check_file<<" with respect to "<<reference_file<<":"<<endl;
   for(auto scheme : check_blocks){
      bool result = scheme_tester.check(scheme.simplified);
      cout<<scheme.simplified_vector_string()<<" -- "<<(result?"True":"False")<<endl;
   }

}
