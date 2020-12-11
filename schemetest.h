
#ifndef SCHEMETEST_H_INCLUDED
#define SCHEMETEST_H_INCLUDED


#include <boost/dynamic_bitset.hpp>
#include <vector>
#include "scheme.h"

// Vector length <= 32
typedef unsigned char Eltype;
const int Vsize=32;

class SchemeTest {
private:
   int Lsize;
   int Hsize;
   boost::dynamic_bitset<> booldata[Vsize][24];
public:
    SchemeTest(const vector<Scheme_compact>& v);
    bool check(const Scheme_compact& val)const;
    bool check(const vector<int>& val)const;
};

#endif // SCHEMETEST_H_INCLUDED
