#include <stdexcept>
#include <iostream>

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
  //cout<<"buffer.size()"<<buffer.size()<<endl;
  //cout<<"cap"<<capacity_<<endl;
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
/*#include <stdexcept>

#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

void Writer::push( string data )
{
  if ( available_capacity() == 0 || data.empty() ) {
    return;
  }
  auto const n = min( available_capacity(), data.size() );
  if ( n < data.size() ) {
    data = data.substr( 0, n );
  }
  data_queue_.push_back( std::move( data ) );
  view_queue_.emplace_back( data_queue_.back().c_str(), n);
  num_bytes_buffered_ += n;
  num_bytes_pushed_ += n;
}

void Writer::close()
{
  is_closed_ = true;
}

void Writer::set_error()
{
  has_error_ = true;
}

bool Writer::is_closed() const
{
  return is_closed_;
}

uint64_t Writer::available_capacity() const
{
  return capacity_ - num_bytes_buffered_;
}

uint64_t Writer::bytes_pushed() const
{
  return num_bytes_pushed_;
}

string_view Reader::peek() const
{
  if ( view_queue_.empty() ) {
    return {};
  }
  return view_queue_.front();
}

bool Reader::is_finished() const
{
  return is_closed_ && num_bytes_buffered_ == 0;
}

bool Reader::has_error() const
{
  return has_error_;
}

void Reader::pop( uint64_t len )
{
  auto n = min( len, num_bytes_buffered_ );
  while ( n > 0 ) {
    auto sz = view_queue_.front().size();
    if ( n < sz ) {
      view_queue_.front().remove_prefix( n );
      num_bytes_buffered_ -= n;
      num_bytes_popped_ += n;
      return;
    }
    view_queue_.pop_front();
    data_queue_.pop_front();
    n -= sz;
    num_bytes_buffered_ -= sz;
    num_bytes_popped_ += sz;
  }
}

uint64_t Reader::bytes_buffered() const
{
  return num_bytes_buffered_;
}

uint64_t Reader::bytes_popped() const
{
  return num_bytes_popped_;
}*/
