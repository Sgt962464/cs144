#include "tcp_sender.hh"
#include "tcp_config.hh"

#include <random>

using namespace std;

/* TCPSender constructor (uses a random ISN if none given) */
TCPSender::TCPSender( uint64_t initial_RTO_ms, optional<Wrap32> fixed_isn )
  : isn_( fixed_isn.value_or( Wrap32 { random_device()() } ) )
  , is_send_ISN( false )
  , is_send_FIN( false )
  , initial_RTO_ms_( initial_RTO_ms )
  , cur_RTO_ms( initial_RTO_ms )
  , is_set_timer( false )
  , recMsg()
  , abs_seqno( 0 )
  , pre_window_size( 1 )
  , outstanding_byte()
  , ready_send_byte()
  , outstanding_set_bytes( 0 )
  , retransmission_count( 0 )
{
  recMsg.ackno = isn_;
  recMsg.window_size = 1;
}

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  // Your code here.
  return outstanding_set_bytes;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  // Your code here.
  return retransmission_count;
}

optional<TCPSenderMessage> TCPSender::maybe_send()
{
  // Your code here.
  if ( ready_send_byte.size() == 0 )
    return nullopt;
  TCPSenderMessage msg( ready_send_byte.front() );
  ready_send_byte.pop_front();
  is_set_timer = true;
  return msg;
}

void TCPSender::push( Reader& outbound_stream )
{
  // Your code here.
  while ( outstanding_set_bytes < recMsg.window_size ) {
    // read a msg
    TCPSenderMessage msg;
    // 1.whether have send?
    if ( !is_send_ISN ) {
      is_send_ISN = true;
      msg.SYN = true;
      msg.seqno = isn_;
    } else {
      msg.seqno = Wrap32::wrap( abs_seqno, isn_ );
    }

    // 2. set lenth
    size_t data_len
      = min( min( static_cast<size_t>( recMsg.window_size - outstanding_set_bytes ), TCPConfig::MAX_PAYLOAD_SIZE ),
             static_cast<size_t>( outbound_stream.bytes_buffered() ) );
    // 3. get data
    read( outbound_stream, data_len, msg.payload );
    // 4.
    if ( outbound_stream.is_finished() && msg.sequence_length() + outstanding_set_bytes < recMsg.window_size ) {
      if ( !is_send_FIN ) {
        is_send_FIN = true;
        msg.FIN = true;
      }
    }
    // 5.
    if ( msg.sequence_length() == 0 )
      break;
    else {
      outstanding_byte.push_back( msg );
      ready_send_byte.push_back( msg );
      outstanding_set_bytes += msg.sequence_length();
    }
    // 6.
    abs_seqno += msg.sequence_length();

    (void)outbound_stream;
  }
}

TCPSenderMessage TCPSender::send_empty_message() const
{
  // Your code here.
  TCPSenderMessage msg;
  msg.seqno = Wrap32::wrap( abs_seqno, isn_ );
  return msg;
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  // Your code here.
  recMsg = msg;
  if ( recMsg.window_size == 0 )
    recMsg.window_size = 1;
  pre_window_size = msg.window_size;
  if ( msg.ackno.has_value() ) {
    if ( msg.ackno.value().unwrap( isn_, abs_seqno ) > abs_seqno )
      return;

    while ( outstanding_set_bytes != 0
            && outstanding_byte.front().seqno.unwrap( isn_, abs_seqno ) + outstanding_byte.front().sequence_length()
                 <= msg.ackno.value().unwrap( isn_, abs_seqno ) ) {
      outstanding_set_bytes -= outstanding_byte.front().sequence_length();
      outstanding_byte.pop_front();
      if ( outstanding_set_bytes == 0 ) {
        is_set_timer = false;
      } else {
        is_set_timer = true;
      }
      cur_RTO_ms = initial_RTO_ms_;
      retransmission_count = 0;
    }
  }
  (void)msg;
}

void TCPSender::tick( const size_t ms_since_last_tick )
{
  // Your code here.
  if ( is_set_timer ) {
    cur_RTO_ms -= ms_since_last_tick;
  }
  if ( cur_RTO_ms <= 0 ) {
    // retransmission
    ready_send_byte.push_front( outstanding_byte.front() );
    retransmission_count++;
    // ARQ
    if ( pre_window_size > 0 ) {
      cur_RTO_ms = pow( 2, retransmission_count ) * initial_RTO_ms_;
    } else {
      cur_RTO_ms = initial_RTO_ms_;
    }
  }
  (void)ms_since_last_tick;
}
