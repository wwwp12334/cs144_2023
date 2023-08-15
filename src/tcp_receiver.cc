#include "tcp_receiver.hh"
#include <iostream>
#include <unistd.h>

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message, Reassembler& reassembler, Writer& inbound_stream )
{
 if (message.SYN) {
    //cout<<"wp"<<endl;
    zero_point = message.seqno;
    //sleep(1);
  }

  if (!zero_point.has_value()) {
    return ;
  }

  //cout<<"pp"<<endl;

  uint64_t absolut_index = message.seqno.unwrap(zero_point.value(), inbound_stream.bytes_pushed() + 1);
  uint64_t first_index = message.SYN ? 0 : absolut_index - 1;

  reassembler.insert(first_index, message.payload.release(), message.FIN, inbound_stream);
}

TCPReceiverMessage TCPReceiver::send( const Writer& inbound_stream ) const
{
  // Your code here.
  TCPReceiverMessage receiver {};
 // cout<<"pp1"<<endl;
  receiver.window_size = inbound_stream.available_capacity() < UINT16_MAX ? inbound_stream.available_capacity() : UINT16_MAX;
  cout<<"ava: "<<inbound_stream.available_capacity()<<endl;

  if (!zero_point.has_value()) {
 //   cout<<"pp2"<<endl;
    return receiver;
  }
//cout<<"pp3"<<endl;
  receiver.ackno = Wrap32::wrap(inbound_stream.bytes_pushed() + 1 + inbound_stream.is_closed(), zero_point.value());
  
  cout<<receiver.window_size<<endl;

  return receiver;
}
