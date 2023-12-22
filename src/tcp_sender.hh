#pragma once

#include "byte_stream.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"

class TCPSender
{
  Wrap32 isn_;
  bool is_send_ISN; /*the flag of ISN*/
  bool is_send_FIN; /*the flag of FIN*/
  uint64_t initial_RTO_ms_;
  int cur_RTO_ms;            // current time
  bool is_set_timer;         // whether start timer
  TCPReceiverMessage recMsg; // the msg of TCPReceiver(ackno,window_size)
  uint64_t abs_seqno;
  size_t pre_window_size;
  std::deque<TCPSenderMessage> outstanding_byte; // send but not ack
  std::deque<TCPSenderMessage> ready_send_byte;  // ready to send;
  size_t outstanding_set_bytes;                  // the bytes of outstanding set
  size_t retransmission_count;                   // the count of restransmission

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
