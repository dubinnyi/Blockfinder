#include "test.h"
#include "timer.h"
#include <iostream>

const int Ntests=100000000;
int main()
{
    static int A[Ntests];
    bool v[Nval];
    
    for(int i=0; i<Ntests; ++i){
        A[i]=rand()%Nval;
    }
    for(int i=0; i<Nval; ++i){
        v[i]=rand()%2;
    }
    
    //alignas(Align) Vbool4 tab;
    Vbool6 tab(Nval);
    for(int i=0; i<Nval; ++i){
        tab[i]=v[i];
    }
    
    Timer t;
    int n;
    for(int i=0; i<Ntests; ++i){
        if(tab[A[i]])n++;
    }
    
    std::cout << "Time taken: " << t.elapsed() << '\n';
    std::cout << "Result: " << n << '\n';
}
