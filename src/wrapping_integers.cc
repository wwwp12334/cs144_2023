#include "wrapping_integers.hh"
#include <cmath>
#include <iostream>

using namespace std;

//将绝对序列号→序列号。给定绝对序列号（n）和初始序列号（零点），生成n的（相对）序列号。
Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  // Your code here.
  /*uint64_t tem = zero_point.raw_value_;
  tem += n;
  tem %= pow(2,32);
  */

  return Wrap32 { zero_point.raw_value_ + uint32_t(n & 0xFFFFFFFFULL) };
}

//将序列号→绝对序列号
uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // Your code here.
  uint64_t min_num = 0;
  uint64_t min_result = 0;

  uint64_t power = 1ULL << 32;
  uint64_t i = checkpoint / power;
  if (i != 0)
    --i;
  while (1) {
    uint64_t result = this->raw_value_ + i * power;
    ++i;
    if (result < zero_point.raw_value_) {
      continue;
    }

//printf("check: %ld and raw_value_: %d\n",checkpoint, raw_value_);
//printf("zero: %d\n",zero_point.raw_value_);
    result -= zero_point.raw_value_;
//printf("result: %ld\n", result);
    if (result == checkpoint) {
//printf("s result: %ld\n", result);
      return result;
    } else if (result < checkpoint) {
      min_num = checkpoint - result;
      min_result = result;
      //printf("m result: %ld\n", result);
    } else {
      //result -= checkpoint;
//printf("l result: %ld\n", result);
      if (min_num != 0)
        return result - checkpoint < min_num ? result : min_result;
      else 
        return result;
    }
    
  }


}
