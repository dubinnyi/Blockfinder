#pragma once
#include <iostream>
#include <stdexcept>
#include <array>
#include <bitset>
#include <cstring>
#include <valarray>
#include <vector>
#include <boost/align/aligned_allocator.hpp>
using std::cerr;
using std::endl;


const int Nval=512;
#ifdef __AVX512__
const int Align=64;
#else
const int Align=32;
#endif
template <typename T>
using aligned_vector = std::vector<T, boost::alignment::aligned_allocator<T, Align>>;
typedef std::valarray<bool> Vbool1;

class Vbool2: public std::vector<uint8_t> {
public:
    Vbool2(int N): std::vector<uint8_t>(N){};
    Vbool2& operator=(bool val) {
        for(auto i=begin(); i!=end(); ++i)
            *i=val;
        return *this;
    }
};

class Vbool3: public aligned_vector<uint8_t> {
public:
    Vbool3(int N): aligned_vector<uint8_t>(N){}
    Vbool3& operator=(bool val) {
        for(auto i=begin(); i!=end(); ++i)
            *i=val;
        return *this;
    }
};

typedef std::array<bool, Nval> Vbool4; 
typedef std::vector<bool> Vbool5;
typedef std::bitset<Nval> Vbool6;

class Vbool {
private:
    int Nalloc;
    int Nsize;
    uint8_t *data;
public:
    Vbool(){
        // cerr<<"Vbool empty constructor\n";
        Nalloc=0; Nsize=0;}
    Vbool(bool v, int N){
        // cerr<<"Vbool constructor "<<N<<"\n";
        Nsize=N;
        Nalloc = (Nsize % Align ? (Nsize/Align+1)*Align : Nsize);
        data = (uint8_t*)aligned_alloc(Align, Nalloc);
        memset(data, (uint8_t)v, Nalloc);
    };
    Vbool(const Vbool& v){
        // cerr<<"Vbool copy constructor "<<v.Nsize<<endl;
        Nsize=v.Nsize;
        Nalloc=v.Nalloc;
        data = (uint8_t*)aligned_alloc(Align, Nalloc);
        if(Nalloc) memcpy(data, v.data, Nalloc);
    };
    ~Vbool(){
        if(Nalloc)free(data);
    };
    Vbool& operator=(const Vbool& v){
        // cerr<<"Operator Vbool= "<<v.Nsize<<endl;
        if(Nalloc>=v.Nsize){
            Nsize=v.Nsize;
        }
        else {
            free(data);
            Nsize=v.Nsize;
            Nalloc=v.Nalloc;
            data = (uint8_t*)aligned_alloc(Align, Nalloc);
        }
        if(Nalloc) memcpy(data, v.data, Nalloc);
        return *this;
    };
    Vbool& operator=(bool v){
        if(Nalloc) memset(data, v, Nalloc);
        return *this;
    };
    Vbool& resize(uint N, uint8_t v){
        // cerr<<"Resize from "<<Nsize<<" to "<<N<<endl;
        if(N<=Nsize) {
            Nsize=N;
        }
        else if(N<=Nalloc) {
            memset(data+Nsize, v, N-Nsize);
            Nsize=N;
        }
        else {
            Nalloc = (N % Align ? (N/Align+1)*Align : N);
            uint8_t *newdata = (uint8_t*)aligned_alloc(Align, Nalloc);
            if(Nsize){
                memcpy(newdata, data, Nsize);
                free(data);
            }
            data=newdata;
            memset(data+Nsize, v, N-Nsize);
            Nsize=N;
        }
        return *this;
    };
    // uint8_t &operator[](uint n){return data[n];}
    bool operator[](uint n)const{
        if(n<Nsize) return (bool)data[n];
        else throw(std::out_of_range("Vbool vector out of range"));
    } 
    
    uint8_t &operator[](uint n){
        if(n<Nsize) return data[n];
        else throw(std::out_of_range("Vbool vector out of range"));
    } 
    uint size()const {
        return(Nsize);
    }
}; 

