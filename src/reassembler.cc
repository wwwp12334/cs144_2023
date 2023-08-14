#include "reassembler.hh"
#include <algorithm>
#include <iostream>
#include <unistd.h>

using namespace std;

Reassembler::Reassembler() : buffer(), first_unassembled_index(0), is_last(false), is_last_index(0), buffer_size(0) {}

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring, Writer& output )
{
  // Your code here.
  if (output.is_closed()) {
    return ;
  }

  if (is_last_substring) {
    //std::cout<<"pp"<<std::endl;
    is_last = true;
    is_last_index = first_index + data.size();
  }
  //std::cout<<"data: "<<data<<std::endl;
//std::cout<<"begin-first-unassembled: "<<first_unassembled_index<<std::endl;
  uint64_t first_unacceptable_index = first_unassembled_index + output.available_capacity();
  //std::cout<<"available: "<<output.available_capacity()<<std::endl;

  uint64_t end_index = first_index + data.size();

  uint64_t j = 0;
  if (first_unassembled_index >= first_index) {
    j += first_unassembled_index - first_index;
  }

  first_index = max(first_index, first_unassembled_index);
  end_index = min(end_index, first_unacceptable_index);
  //std::cout<<"first: "<<first_index<<"end_index "<<end_index<<endl;
  
//std::cout<<"j: "<<j<<std::endl;
  for (uint64_t i = first_index; i < end_index; ++i, ++j) {
    if (!buffer.count(i)) {
      buffer[i] = data[j];
      //std::cout<<"i: "<<i<<" buffer: "<<buffer[i]<<std::endl;
      //usleep(1000);
      //++buffer_size;
    }
  }

  //buffer_size += end_index - first_index;
  

 /* if (buffer.count(first_unassembled_index)) {
    int pre = first_unassembled_index - 1;
    //printf("pre: %d\n",pre);
    string s;
    //vector<int> to_move;
    for (auto begin = buffer.lower_bound(first_unassembled_index); begin != buffer.end(); ++begin) {
      //printf("begin-pre: %d\n", begin->first - pre);
      if (begin->first - pre != 1)
        break;

      s += begin->second;
      //std::cout<<"s: "<<s<<std::endl;
      
      pre = begin->first;
      //buffer.erase(begin->first);
      //to_move.push_back(begin->first);
      --buffer_size;
    }

    //for (int i : to_move) {
    //  buffer.erase(i);
    //}

    output.push(s);
    first_unassembled_index = pre + 1;
    //std::cout<<"end-first-unassembled: "<<first_unassembled_index<<std::endl;
  }
*/

  string s;
  while (buffer.count(first_unassembled_index)) {
    s += buffer[first_unassembled_index];
    //--buffer_size;
    buffer.erase(first_unassembled_index);
    ++first_unassembled_index;
  }

  output.push(s);

  if (is_last && first_unassembled_index >= is_last_index) {
    output.close();
    //std::cout<<"wp "<<std::endl;
  }
}

uint64_t Reassembler::bytes_pending() const
{
  // Your code here. 
  return buffer.size();
}
