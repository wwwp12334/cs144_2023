#pragma once

#include "byte_stream.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"
#include <queue>

using namespace std;

class Timer{
private:
  uint64_t initial_RTO_ms;
  uint64_t current_RTO_ms;
  uint64_t time_ms {0};
  bool running {false};

public:

  explicit Timer(uint64_t init_ROT) : initial_RTO_ms(init_ROT), current_RTO_ms(init_ROT) {}

  void stop() {
    running = false;
  }

  void start() {
    time_ms = 0;
    running = true;
  }

  void tick(uint64_t ms_since_last_tick) {
    if (running) {
      time_ms += ms_since_last_tick;
    }
  }

  bool is_running() {
    return running;
  }

  bool is_expired() {
    return running && time_ms >= current_RTO_ms;
  }

  void doubleRTO() {
    current_RTO_ms *= 2;
  }

  void resetRTO() {
    current_RTO_ms = initial_RTO_ms;
  }

};

class TCPSender
{
  Wrap32 isn_;
  uint64_t initial_RTO_ms_;//超时时长

  bool syn_ {false};
  bool fin_ {false};
  uint64_t retransmit_cnt_ {0};

  uint64_t acked_seqno {0};
  uint64_t next_seqno {0};
  uint64_t window_size {1};
  uint64_t outstanding_cnt {0};

  queue<TCPSenderMessage> outstanding_segments {};
  queue<TCPSenderMessage> queued_segments {};

  Timer timer {initial_RTO_ms_};

public:
  /* Construct TCP sender with given default Retransmission Timeout and possible ISN */
  TCPSender( uint64_t initial_RTO_ms, std::optional<Wrap32> fixed_isn );

  /* Push bytes from the outbound stream */
  void push( Reader& outbound_stream );

  /* Send a TCPSenderMessage if needed (or empty optional otherwise) */
  std::optional<TCPSenderMessage> maybe_send();

  /* Generate an empty TCPSenderMessage */
  TCPSenderMessage send_empty_message() const;

  /* Receive an act on a TCPReceiverMessage from the peer's receiver */
  void receive( const TCPReceiverMessage& msg );

  /* Time has passed by the given # of milliseconds since the last time the tick() method was called. */
  void tick( uint64_t ms_since_last_tick );

  /* Accessors for use in testing */
  uint64_t sequence_numbers_in_flight() const;  // How many sequence numbers are outstanding?
  uint64_t consecutive_retransmissions() const; // How many consecutive *re*transmissions have happened?
};
