#include "reassembler.hh"
#include <algorithm>
#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>

using namespace std;

Reassembler::Reassembler() : buffer(), first_unassembled_index(0), is_last(false), 
                              is_last_index(0), buffer_size(0), delay_delete() {}

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring, Writer& output )
{
  if (output.is_closed()) {
    return ;
  }

  if (is_last_substring) {
    is_last = true;
    is_last_index = first_index + data.size();
  }

  if (is_last && first_unassembled_index >= is_last_index) {
    output.close();
    return ;
  }

  uint64_t first_unacceptable_index = first_unassembled_index + output.available_capacity();//开区间)
  uint64_t end_index = first_index + data.size();

  if (first_index >= first_unacceptable_index || end_index <= first_unassembled_index)
    return ;

  uint64_t absolute_index = 0;
  if (first_unassembled_index > first_index) {
    absolute_index += first_unassembled_index - first_index;
  }

  first_index = max(first_index, first_unassembled_index);
  end_index = min(end_index, first_unacceptable_index);//[区间)

  bool flag = true;
  auto it = buffer.lower_bound(first_index);
  if (it != buffer.end() && it->first == first_index) {
    if (first_index - end_index <= (it->second).size())
      return ;

    absolute_index += it->first + (it->second).size() - first_index;
    first_index = it->first + (it->second).size();
  } else {
    if (it == buffer.begin()) {
      flag = false;
    } else {
      --it;
      absolute_index += (first_index >= it->first + (it->second).size()) ? 0 : (it->first + (it->second).size() - first_index);
      first_index = max(it->first + (it->second).size(), first_index);
    }
  } 

  if (flag)
    ++it;

  while (it != buffer.end()) {
    if (end_index <= it->first) {
      break;
    } else if (end_index > it->first + (it->second).size()) { 
      buffer_size -= (it->second).size();
      delay_delete.insert(it->first);
    } else {
      end_index = it->first;
      break;
    }

    ++it;
  }

  string s;
  if (end_index - first_index > 0 && absolute_index < data.size())
    s = data.substr(absolute_index, end_index - first_index);

  if (s.empty())
    return ;

  buffer_size += s.size();

  for (auto p = delay_delete.begin(); p != delay_delete.end(); ++p) {
    buffer.erase(*p);
  }
  delay_delete.clear();

  buffer[first_index] = s;

  for (it = buffer.begin(); it != buffer.end();) {
    if (it->first == first_unassembled_index) {
      first_unassembled_index += (it->second).size();
      buffer_size -= (it->second).size();
      output.push(move(it->second));
      it = buffer.erase(it);
    } else {
      break;
    }
  }

  if (is_last && first_unassembled_index >= is_last_index) {
    output.close();
  }
}

uint64_t Reassembler::bytes_pending() const
{
  return buffer_size;
}
