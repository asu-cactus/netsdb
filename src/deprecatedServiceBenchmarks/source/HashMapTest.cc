/*
 * Author: Jia
 * Created on May 17, 2016 22:28
 */
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <unordered_map>
#include "STLScopedAllocator.h"
#include "STLSlabAllocator.h"
#include <iostream>
using namespace std;

char * new_string_from_integer(int num) {
   int ndigits = num == 0 ? 1 : (int)log10(num) + 1;
   char * str = (char *) malloc (ndigits + 1);
   sprintf(str, "%d", num);
   return str;
}

int main(int argc, char ** argv) {

   if(argc <= 3) {
       return 1;
   }
   int numKeysx1000000 = atoi(argv[1]);
   int i,j, value = 0;

   if(atoi(argv[2])==0) {
       cout << "unordered_map using std::Allocator\n";
       std::unordered_map<const char *, int64_t>* str_hash=new std::unordered_map<const char *, int64_t>();
       if(!strcmp(argv[3], "sequentialstring")) {
           time_t before = time(0);
           for(i = 0; i< numKeysx1000000; i++) {
               for (j = 0; j < 1000000; j++ ) {
                   str_hash->insert(std::unordered_map<const char *, int64_t>::value_type(new_string_from_integer(i*1000000+j), value));
               }
           }
           time_t after = time(0);
           cout << "std unordered_map sequential string time:"<<after-before<<"\n";
       }
       else if(!strcmp(argv[3], "randomstring")){
           srandom(1);
           time_t before = time(0);
           for(i = 0; i< numKeysx1000000; i++) {
               for (j = 0; j < 1000000; j++ ) {
                   str_hash->insert(std::unordered_map<const char *, int64_t>::value_type(new_string_from_integer((int)random()), value));
               }
           }
           time_t after = time(0);
           cout << "std unordered_map random string time:"<<after-before<<"\n";
       }
   }
   else if(atoi(argv[2])==1){

       cout << "unordered_map using ScopedAllocator\n";
       char * data = (char *) malloc (64*1024*1024*sizeof(char));
       STLScopedAllocator<std::pair<const char *, int64_t>> allocator = STLScopedAllocator<std::pair<const char *, int64_t>>((void *)data, 64*1024*1024);
       hash<const char*> hasher = hash<const char*>();
       equal_to<const char*> key_equal=equal_to<const char*>();
       std::unordered_map<const char*, int64_t, hash<const char*>, equal_to<const char*>,STLScopedAllocator<std::pair<const char *, int64_t>>>* str_hash = new std::unordered_map<const char*, int64_t, hash<const char*>, equal_to<const char*>,STLScopedAllocator<std::pair<const char *, int64_t>>> (10, hasher, key_equal, allocator);
       if(!strcmp(argv[3], "sequentialstring")) {
           time_t before = time(0);
           for(i = 0; i< numKeysx1000000; i++) {
               for (j = 0; j < 10; j++ ) {
                  try{
                   str_hash->insert(std::pair<const char*, int64_t>(new_string_from_integer(i*10+j), value));
                  }
                  catch(std::bad_alloc &e) {
                       cout <<"Caught the exception! Should add a new page, but simply exit now...\n";
                       exit(-1);
                  }
               }
           }
           time_t after = time(0);
           cout << "std unordered_map sequential string time:"<<after-before<<"\n";
       }
       else if(!strcmp(argv[3], "randomstring")){
           srandom(1);
           time_t before = time(0);
           for(i = 0; i< numKeysx1000000; i++) {
               for (j = 0; j < 10; j++ ) {
                   str_hash->insert(std::pair<const char*, int64_t>(new_string_from_integer((int)random()), value));
               }
           }
           time_t after = time(0);
           cout << "std unordered_map random string time:"<<after-before<<"\n";
       }

   } else {
       cout << "unordered_map using SlabAllocator\n";
       STLSlabAllocator<std::pair<const char *, int64_t>> allocator = STLSlabAllocator<std::pair<const char *, int64_t>>(64*1024*1024);
       hash<const char*> hasher = hash<const char*>();
       equal_to<const char*> key_equal=equal_to<const char*>();
       std::unordered_map<const char*, int64_t, hash<const char*>, equal_to<const char*>,STLSlabAllocator<std::pair<const char *, int64_t>>>* str_hash = new std::unordered_map<const char*, int64_t, hash<const char*>, equal_to<const char*>,STLSlabAllocator<std::pair<const char *, int64_t>>> (10, hasher, key_equal, allocator);
       if(!strcmp(argv[3], "sequentialstring")) {
           time_t before = time(0);
           for(i = 0; i< numKeysx1000000; i++) {
               for (j = 0; j < 10; j++ ) {
                  try{
                   str_hash->insert(std::pair<const char*, int64_t>(new_string_from_integer(i*10+j), value));
                  }
                  catch(std::bad_alloc &e) {
                       cout <<"Caught the exception! Iterate the hashmap and write items to disk.\n";
                       //iterate items and write items to disk
                       //create a buffer
                       exit(-1);
                       
                  }
               }
           }
           time_t after = time(0);
           cout << "std unordered_map sequential string time:"<<after-before<<"\n";
       }
       else if(!strcmp(argv[3], "randomstring")){
           srandom(1);
           time_t before = time(0);
           for(i = 0; i< numKeysx1000000; i++) {
               for (j = 0; j < 10; j++ ) {
                   str_hash->insert(std::pair<const char*, int64_t>(new_string_from_integer((int)random()), value));
               }
           }
           time_t after = time(0);
           cout << "std unordered_map random string time:"<<after-before<<"\n";
       }

   }

}
