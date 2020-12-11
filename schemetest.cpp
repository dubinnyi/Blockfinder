#include "schemetest.h"

SchemeTest::SchemeTest(const vector<Scheme_compact>& v):
            Hsize(v.size())
{
  if (Hsize){
    Lsize = v.begin()->simplified.size();
    maxval.assign(Lsize, 0);
    for(int i=0;i<Lsize;++i){
        for(int k=0;k<Hsize;++k){
           if(v[k].simplified[i] > maxval[i]){
               maxval[i] = v[k].simplified[i];
           }
        }
    }
    boost::dynamic_bitset<> tmp_bitset(Hsize); 
    for(int i=0;i<Lsize;++i){
       booldata.push_back({});
       for(int j=1;j<=maxval[i];++j){
          booldata[i].push_back(tmp_bitset);
       }
    }
    for(int i=0;i<Lsize;++i){
        for(int j=1;j<=maxval[i];++j){
            //booldata[i][j-1].resize(Hsize);
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
   for(int i=0;i<Lsize;++i){
      if(val[i] > maxval[i]){
         return true;
      }
   }
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



