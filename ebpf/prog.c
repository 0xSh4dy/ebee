#include <bpf/bpf_helpers.h>
#include <linux/bpf.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/ipv6.h>

SEC("xdp")
int check_packets(struct xdp_md *ctx) {
  void *data = (void *)(long)ctx->data;
  void *data_end = (void *)(long)ctx->data_end;

  struct ethhdr *eth = data;
  unsigned long ip_header_offset = sizeof(*eth);

  if (data + ip_header_offset > data_end) {
    return XDP_PASS;
  }
  unsigned short h_proto = eth->h_proto;

  if (h_proto == htons(ETH_P_IP)) {
    void *ip_data = data + ip_header_offset;
    struct iphdr *ip_header = ip_data;
    
    if ((void *)&ip_header[1] > data_end) {
      return 0;
    }
    if(ip_header->protocol == IPPROTO_UDP){
        
    }
  }
  return XDP_PASS;
}