#include "network_interface.hh"

#include "arp_message.hh"
#include "ethernet_frame.hh"

using namespace std;

// ethernet_address: Ethernet (what ARP calls "hardware") address of the interface
// ip_address: IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface( const EthernetAddress& ethernet_address, const Address& ip_address )
  : ethernet_address_( ethernet_address ), ip_address_( ip_address )
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
  uint32_t target_ip = next_hop.ipv4_numeric();
  if (ip_and_mac.count(target_ip)) {
    EthernetFrame eframe { { ip_and_mac[target_ip].first, ethernet_address_, EthernetHeader::TYPE_IPv4}, serialize(dgram)};
    out_frames.push(eframe);
  } else {
    if (!arp_timer.count(target_ip)) {
        ARPMessage arpmsg;
        arpmsg.opcode = ARPMessage::OPCODE_REQUEST;
        arpmsg.sender_ip_address = ip_address_.ipv4_numeric();
        arpmsg.sender_ethernet_address = ethernet_address_;
        arpmsg.target_ip_address = next_hop.ipv4_numeric();

        EthernetFrame eframe { { ETHERNET_BROADCAST, ethernet_address_, EthernetHeader::TYPE_ARP}, serialize(arpmsg)};
        out_frames.push(eframe);
        arp_timer.emplace(next_hop.ipv4_numeric(), 0);
        waited_dagrams.insert( { target_ip, { dgram}});
    } else {
        waited_dagrams[target_ip].push_back(dgram);
    }
  }
}

// frame: the incoming Ethernet frame
optional<InternetDatagram> NetworkInterface::recv_frame( const EthernetFrame& frame )
{
  if (frame.header.dst != ethernet_address_ && frame.header.dst != ETHERNET_BROADCAST)
    return {};

  if (frame.header.type == EthernetHeader::TYPE_IPv4) {
    InternetDatagram data;
    if (parse(data, frame.payload))
      return data;
  } else if (frame.header.type == EthernetHeader::TYPE_ARP) {
    ARPMessage arpmsg;
    if (parse(arpmsg, frame.payload)) {
      ip_and_mac.insert( { arpmsg.sender_ip_address, {arpmsg.sender_ethernet_address, 0}});
      if (arpmsg.opcode == ARPMessage::OPCODE_REPLY) {
        vector<InternetDatagram> datas = waited_dagrams[arpmsg.sender_ip_address];
        for (InternetDatagram data : datas) {
          send_datagram(data, Address::from_ipv4_numeric(arpmsg.sender_ip_address));
        }

        waited_dagrams.erase(arpmsg.sender_ip_address);
      } else if (arpmsg.opcode == ARPMessage::OPCODE_REQUEST) {
        if (arpmsg.target_ip_address == ip_address_.ipv4_numeric()) {
          ARPMessage reply_msg;
          reply_msg.opcode = ARPMessage::OPCODE_REPLY;
          reply_msg.sender_ip_address = ip_address_.ipv4_numeric();
          reply_msg.sender_ethernet_address = ethernet_address_;
          reply_msg.target_ip_address = arpmsg.sender_ip_address;
          reply_msg.target_ethernet_address = arpmsg.sender_ethernet_address;

          EthernetFrame reply_frame { { arpmsg.sender_ethernet_address, ethernet_address_, EthernetHeader::TYPE_ARP}, serialize(reply_msg)};
          out_frames.push(reply_frame);
        }
      }
    }
  }

  return {};
}

// ms_since_last_tick: the number of milliseconds since the last call to this method
void NetworkInterface::tick( const size_t ms_since_last_tick )
{
  static const size_t IP_MAP_TTL = 30000;
  static const size_t ARP_TTL = 5000;

  for (auto it = ip_and_mac.begin(); it != ip_and_mac.end(); ) {
    it->second.second += ms_since_last_tick;
    if (it->second.second >= IP_MAP_TTL) {
      it = ip_and_mac.erase(it);
    } else {
      ++it;
    }
  }

  for (auto it = arp_timer.begin(); it != arp_timer.end(); ) {
    it->second += ms_since_last_tick;
    if (it->second >= ARP_TTL) {
      it = arp_timer.erase(it);
    } else {
      ++it;
    }
  }
}

optional<EthernetFrame> NetworkInterface::maybe_send()
{
  if (!out_frames.empty()) {
    auto frame = out_frames.front();
    out_frames.pop();

    return frame;
  }

  return {};
}
