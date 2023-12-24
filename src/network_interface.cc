#include "network_interface.hh"

#include "arp_message.hh"
#include "ethernet_frame.hh"

using namespace std;

// ethernet_address: Ethernet (what ARP calls "hardware") address of the interface
// ip_address: IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface( const EthernetAddress& ethernet_address, const Address& ip_address )
  : ethernet_address_( ethernet_address )
  , ip_address_( ip_address )
  , ip_grams()
  , ethernet_frame()
  , ip_map_mac()
  , arp_time()
  , MAP_TTL( 30000 )
  , ARP_TTL( 5000 )
{
  cerr << "DEBUG: Network interface has Ethernet address " << to_string( ethernet_address_ ) << " and IP address "
       << ip_address.ip() << "\n";
}

// dgram: the IPv4 datagram to be sent
// next_hop: the IP address of the interface to send it to (typically a router or default gateway, but
// may also be another host if directly connected to the same network as the destination)

// Note: the Address type can be converted to a uint32_t (raw 32-bit IP address) by using the
// Address::ipv4_numeric() method.
void NetworkInterface::send_datagram( const InternetDatagram& dgram, const Address& next_hop )
{
  if ( ip_map_mac.contains( next_hop.ipv4_numeric() ) ) {
    EthernetFrame ethe_frame_;
    // find ip
    ethe_frame_.header.type = EthernetHeader::TYPE_IPv4;
    // source address &&dst address
    ethe_frame_.header.src = ethernet_address_;
    ethe_frame_.header.dst = ip_map_mac[next_hop.ipv4_numeric()].first;
    // operate data
    ethe_frame_.payload = serialize( dgram );
    // push
    ethernet_frame.push_back( ethe_frame_ );
  } else {
    if ( !arp_time.contains( next_hop.ipv4_numeric() ) ) {
      ARPMessage arp_msg;
      // arp request
      arp_msg.opcode = ARPMessage::OPCODE_REQUEST;
      arp_msg.sender_ethernet_address = ethernet_address_;
      arp_msg.sender_ip_address = ip_address_.ipv4_numeric();
      // target ip
      arp_msg.target_ip_address = next_hop.ipv4_numeric();
      // mac frame
      EthernetFrame ethe_frame;
      ethe_frame.header.type = EthernetHeader::TYPE_ARP;
      ethe_frame.header.src = ethernet_address_;
      ethe_frame.header.dst = ETHERNET_BROADCAST;
      // serualizer
      ethe_frame.payload = serialize( arp_msg );
      // save next_ip and data
      ip_grams[next_hop.ipv4_numeric()].push_back( dgram );
      arp_time.emplace( next_hop.ipv4_numeric(), 0 );
      // push
      ethernet_frame.push_back( ethe_frame );
    }
  }

  (void)dgram;
  (void)next_hop;
}

// frame: the incoming Ethernet frame
optional<InternetDatagram> NetworkInterface::recv_frame( const EthernetFrame& frame )
{
  if ( frame.header.dst != ethernet_address_ && frame.header.dst != ETHERNET_BROADCAST ) {
    return nullopt;
  }

  if ( frame.header.type == EthernetHeader::TYPE_IPv4 ) {
    InternetDatagram ip_gram;
    if ( parse( ip_gram, frame.payload ) )
      return ip_gram;
    return nullopt;
  } else if ( frame.header.type == EthernetHeader::TYPE_ARP ) {
    ARPMessage arp_gram;
    if ( parse( arp_gram, frame.payload ) ) {
      ip_map_mac.insert( { arp_gram.sender_ip_address, { arp_gram.sender_ethernet_address, 0 } } );
      if ( arp_gram.opcode == ARPMessage::OPCODE_REQUEST ) {
        if ( arp_gram.target_ip_address == ip_address_.ipv4_numeric() ) {
          // product arp reply
          ARPMessage reply_grame;
          reply_grame.opcode = ARPMessage::OPCODE_REPLY;
          reply_grame.sender_ethernet_address = ethernet_address_;
          reply_grame.sender_ip_address = ip_address_.ipv4_numeric();
          reply_grame.target_ethernet_address = arp_gram.sender_ethernet_address;
          reply_grame.target_ip_address = arp_gram.sender_ip_address;

          // produce mac frame

          EthernetFrame reply_eth_frame;
          reply_eth_frame.header.type = EthernetHeader::TYPE_ARP;
          reply_eth_frame.header.src = reply_grame.sender_ethernet_address;
          reply_eth_frame.header.dst = reply_grame.target_ethernet_address;
          reply_eth_frame.payload = serialize( reply_grame );

          ethernet_frame.push_back( reply_eth_frame );
        }
      } else if ( arp_gram.opcode == ARPMessage::OPCODE_REPLY ) {
        ip_map_mac.insert( { arp_gram.sender_ip_address, { arp_gram.sender_ethernet_address, 0 } } );

        auto& ip_gram = ip_grams[arp_gram.sender_ip_address];

        for ( auto& dgram : ip_gram ) {
          send_datagram( dgram, Address::from_ipv4_numeric( arp_gram.sender_ip_address ) );
        }
        ip_grams.erase( arp_gram.sender_ip_address );
      }
    }
  }
  (void)frame;
  return nullopt;
}

// ms_since_last_tick: the number of milliseconds since the last call to this method
void NetworkInterface::tick( const size_t ms_since_last_tick )
{
  for ( auto it = ip_map_mac.begin(); it != ip_map_mac.end(); ) {
    it->second.second += ms_since_last_tick;
    if ( it->second.second >= MAP_TTL ) {
      it = ip_map_mac.erase( it );
    } else
      it++;
  }

  for ( auto it = arp_time.begin(); it != arp_time.end(); ) {
    it->second += ms_since_last_tick;
    if ( it->second >= ARP_TTL ) {
      it = arp_time.erase( it );
    } else
      it++;
  }
  (void)ms_since_last_tick;
}

optional<EthernetFrame> NetworkInterface::maybe_send()
{
  if ( ethernet_frame.empty() ) {
    return nullopt;
  }
  auto frame = ethernet_frame.front();
  ethernet_frame.pop_front();
  return frame;
}
