#pragma once

#include "reassembler.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"

class TCPReceiver
{
protected:
  Wrap32 ISN;       //flag ISN
  Wrap32 fin;       //flag fin
  bool is_set_ISN;   //is set ISN
  bool is_end;      //is end?
public:
  /*
   * The TCPReceiver receives TCPSenderMessages, inserting their payload into the Reassembler
   * at the correct stream index.
   */
  void receive( TCPSenderMessage message, Reassembler& reassembler, Writer& inbound_stream );

  /* The TCPReceiver sends TCPReceiverMessages back to the TCPSender. */
  TCPReceiverMessage send( const Writer& inbound_stream ) const;
  TCPReceiver():ISN(0),fin(0),is_set_ISN(0),is_end(false){}
};
