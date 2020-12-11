#include "schemetest.h"

SchemeTest::SchemeTest(const vector<Scheme_compact>& v):
            Hsize(v.size())
{
  if (Hsize){
    Lsize = v.begin()->simplified.size();
    for(int i=0;i<Lsize;++i){
        for(int j=1;j<25;++j){
            booldata[i][j-1].resize(Hsize);
            auto pos=v.begin();
            for(int k=0;k<Hsize;++k,++pos){
                if(pos->simplified[i]>=j){
                    booldata[i][j-1].set(k);
                }
            }
        }
    }
  }
}

bool SchemeTest::check(const vector<int>& val)const{
    boost::dynamic_bitset<> bvec(Hsize); 
    bvec.set(); // All bits are 1
    for(int i=0;i<Lsize;++i){
        int j=val[i];
        if(j>0){
            bvec&=booldata[i][j-1];
        }
    }
    return !bvec.any();
}



