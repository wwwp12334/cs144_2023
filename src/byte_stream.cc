#include <stdexcept>

#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ), buffer(), is_stop(false), 
                                          is_err(false), writeByte(0), readByte(0) {}

void Writer::push( string data )
{
  // Your code here.
  if (is_stop || is_err)
    return ;

  int numByte = min(data.size(), capacity_ - buffer.size());
  for (int i = 0; i < numByte; ++i) {
    buffer.push(data[i]);
  }

  writeByte += numByte;
}

void Writer::close()
{
  // Your code here.
  is_stop = true;
}

void Writer::set_error()
{
  // Your code here.
  is_err = true;
}

bool Writer::is_closed() const
{
  // Your code here.
  return is_stop;
}

uint64_t Writer::available_capacity() const
{
  // Your code here.
  return capacity_ - buffer.size();
}

uint64_t Writer::bytes_pushed() const
{
  // Your code here.
  return writeByte;
}

string_view Reader::peek() const
{
  // Your code here.
  return { &buffer.front(), 1};
}

bool Reader::is_finished() const
{
  // Your code here.
  if (is_stop && buffer.size() == 0)
    return true;
  else
    return false;
}

bool Reader::has_error() const
{
  // Your code here.
  return is_err;
}

void Reader::pop( uint64_t len )
{
  // Your code here.
  int numByte = min(len, buffer.size());
  for (int i = 0; i < numByte; ++i) {
    buffer.pop();
  }

  readByte += len;
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  return buffer.size();
}

uint64_t Reader::bytes_popped() const
{
  // Your code here.
  return readByte;
}
