#include "reassembler.hh"

using namespace std;



void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring, Writer& output )
{
  // Your code here.
  //init assemblerBuf and flagBuf
  assembledBuf.resize(output.available_capacity(),0);
  flagBuf.resize(output.available_capacity(),0);

  //the last index
  if(is_last_substring){
    end_index=first_index+data.size();
  }
  first_unassembled_index=output.bytes_pushed();
  first_unacceptable_index=first_unassembled_index+output.available_capacity();

  //operator data
  uint64_t str_begin;
  uint64_t str_end;
  uint64_t str_len=first_index+data.size();
  if(!data.empty()){
    if(str_len<first_unassembled_index||str_len>=first_unacceptable_index){      // the data not store in availble_capacity
      data="";
    }else{
      str_begin=first_index;
      str_end=str_len-1;
      if(first_index<first_unassembled_index) str_begin=first_unassembled_index; //throw head
      if(str_len>first_unacceptable_index) str_end=first_unacceptable_index-1;    //throw tai;
      for(auto i=str_begin;i<=str_end;i++){
        assembledBuf[i-first_unassembled_index]=data[i-first_index];
        flagBuf[i-first_unassembled_index]=true; 
      }
    }
  }
  //pop
  string str="";
  while(flagBuf.front()){
    str+=assembledBuf.front();
    assembledBuf.pop_front();       //del a space
    assembledBuf.push_back(0);      //add a space
    flagBuf.pop_front();            //del a space
    flagBuf.push_back(false);       //add a space
  }
  output.push(str);                 //push into stram

  if(output.bytes_pushed()==end_index){
    output.close();
  }

  (void)first_index;
  (void)data;
  (void)is_last_substring;
  (void)output;
}

uint64_t Reassembler::bytes_pending() const
{
  // Your code here.
  uint64_t count=0;
  for(auto i=assembledBuf.begin();i!=assembledBuf.end();i++)
    count++;
  return count;
}
