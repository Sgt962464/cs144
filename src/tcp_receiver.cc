#include "tcp_receiver.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message, Reassembler& reassembler, Writer& inbound_stream )
{
  // Your code here.
  if(message.SYN==true){                //set connect
    ISN=Wrap32(message.seqno);
    is_set_ISN=true;
    message.seqno=message.seqno+1;
  }

  if(message.FIN==true){
    is_end=true;
    fin=Wrap32(message.seqno+message.payload.size());
  }else{
    is_end=false;
  }

  if(is_set_ISN==true){
    reassembler.insert(message.seqno.unwrap(ISN,inbound_stream.bytes_pushed())-1,
                       message.payload,
                       is_end,
                       inbound_stream);
  }
  (void)message;
  (void)reassembler;
  (void)inbound_stream;
}  

TCPReceiverMessage TCPReceiver::send( const Writer& inbound_stream ) const
{
  // Your code here.
  TCPReceiverMessage tcpreceivermessage;
  Wrap32 ackno=Wrap32::wrap((inbound_stream.bytes_pushed()+1),ISN);
  uint64_t cur_siez=inbound_stream.available_capacity();
  if(is_set_ISN==true){
    tcpreceivermessage.ackno=ackno==fin?ackno+1:ackno;
  }
  tcpreceivermessage.window_size=cur_siez>UINT16_MAX?UINT16_MAX:cur_siez;
  (void)inbound_stream;
  return tcpreceivermessage;
}
