#include "router.hh"

#include <iostream>
#include <limits>

using namespace std;

// route_prefix: The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
// prefix_length: For this route to be applicable, how many high-order (most-significant) bits of
//    the route_prefix will need to match the corresponding bits of the datagram's destination address?
// next_hop: The IP address of the next hop. Will be empty if the network is directly attached to the router (in
//    which case, the next hop address should be the datagram's final destination).
// interface_num: The index of the interface to send the datagram out on.
void Router::add_route( const uint32_t route_prefix,
                        const uint8_t prefix_length,
                        const optional<Address> next_hop,
                        const size_t interface_num )
{
  cerr << "DEBUG: adding route " << Address::from_ipv4_numeric( route_prefix ).ip() << "/"
       << static_cast<int>( prefix_length ) << " => " << ( next_hop.has_value() ? next_hop->ip() : "(direct)" )
       << " on interface " << interface_num << "\n";
  route_table.emplace(make_pair(route_prefix,prefix_length),route_data{next_hop,interface_num});

  (void)route_prefix;
  (void)prefix_length;
  (void)next_hop;
  (void)interface_num;
}

void Router::route() 
{
  for(auto&& interface:interfaces_){
    std::optional<InternetDatagram> ip_dgram;
    while(true)
    {
      ip_dgram=interface.maybe_receive();
      if(ip_dgram==nullopt)
        break;
      if(ip_dgram.value().header.ttl>0)
        ip_dgram.value().header.ttl--;
      if(ip_dgram.value().header.ttl<=0)
        continue;
      ip_dgram.value().header.compute_checksum();

      //start match
      bool is_match_route=false;
      std::pair<std::pair<uint32_t,uint8_t>,route_data> route_best;
      if(route_table.size()==0) continue;

      bool has_route_defult=false;
      std::pair<std::pair<uint32_t,uint8_t>,route_data> route_defult;

      for(auto&& route:route_table){
        uint8_t len=32-route.first.second;
        if(len==32){
          has_route_defult=true;
          route_defult=route;
          continue;
        }
        if((route.first.first>>len)==(ip_dgram->header.dst>>len)){
          if(route.first.second>=route_best.first.second){
            is_match_route=true;
            route_best=route; 
          }  
        }
      }
      if(!is_match_route){
        if(has_route_defult){
          route_best=route_defult;
          is_match_route=true;
        }else{
          continue;
        }
      }

      interfaces_[route_best.second.interface_num]
      .send_datagram(ip_dgram.value(),
                     route_best.second.next_hop.value_or(Address::from_ipv4_numeric(ip_dgram.value().header.dst)));
    }
  }
}
