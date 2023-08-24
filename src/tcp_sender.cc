#include "tcp_sender.hh"
#include "tcp_config.hh"

#include <random>

using namespace std;

/* TCPSender constructor (uses a random ISN if none given) */
TCPSender::TCPSender( uint64_t initial_RTO_ms, optional<Wrap32> fixed_isn )
  : isn_( fixed_isn.value_or( Wrap32 { random_device()() } ) ), initial_RTO_ms_( initial_RTO_ms )
{}

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  // Your code here.
  return outstanding_cnt;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  // Your code here.
  return retransmit_cnt_;
}

optional<TCPSenderMessage> TCPSender::maybe_send()
{
  if (queued_segments.empty())
    return {};

  if (!timer.is_running())
    timer.start();

  TCPSenderMessage msg = queued_segments.front();
  queued_segments.pop();

  return msg;
}

void TCPSender::push( Reader& outbound_stream )
{
  uint64_t currwindow_size = window_size != 0 ? window_size : 1;
  while (outstanding_cnt < currwindow_size) {
    TCPSenderMessage msg;
    msg.seqno = Wrap32::wrap(next_seqno, isn_);

    if (!syn_) {
      msg.SYN = syn_ = true;
      outstanding_cnt += 1;
    }

    uint64_t payload_size = min(TCPConfig::MAX_PAYLOAD_SIZE, currwindow_size - outstanding_cnt);
    read(outbound_stream, payload_size, msg.payload);
    outstanding_cnt += msg.payload.size();

    if (!fin_ && outbound_stream.is_finished() && outstanding_cnt < currwindow_size) {
      fin_ = true;
      msg.FIN = true;
      outstanding_cnt += 1;
    }

    if (msg.sequence_length() == 0)
      break;

    queued_segments.push(msg);
    outstanding_segments.push(msg);
    next_seqno += msg.sequence_length();

    if (msg.FIN || outbound_stream.bytes_buffered() == 0)
      break;
  }
}

TCPSenderMessage TCPSender::send_empty_message() const
{
  // Your code here.
  auto seqno = Wrap32::wrap(next_seqno, isn_);
  return {seqno, false, {}, false};
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  window_size = msg.window_size;
  if (msg.ackno.has_value()) {
    auto ackno = msg.ackno.value().unwrap(isn_, next_seqno);

    if (ackno > next_seqno)
      return;

    acked_seqno = ackno;

    while (!outstanding_segments.empty()) {
      auto front_msg = outstanding_segments.front();
      if (front_msg.sequence_length() + front_msg.seqno.unwrap(isn_, next_seqno) <= acked_seqno) {
        outstanding_segments.pop();
        outstanding_cnt -= front_msg.sequence_length();

        timer.resetRTO();
        if (!outstanding_segments.empty()) {
          timer.start();
        }
        retransmit_cnt_ = 0;
      } else {
        break;
      }
    }

    if (outstanding_segments.empty())
      timer.stop();
  }
}

void TCPSender::tick( const size_t ms_since_last_tick )
{
  timer.tick(ms_since_last_tick);
  if (timer.is_expired()) {
    queued_segments.push(outstanding_segments.front());
    if (window_size != 0) {
      ++retransmit_cnt_;
      timer.doubleRTO();
    }

    timer.start();
  }

}
