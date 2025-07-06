#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#define ETH_P_IP 0x0800

struct ebee_event {
  unsigned char payload[20];
  int size;
};

struct {
  __uint(type, BPF_MAP_TYPE_RINGBUF);
  __uint(max_entries, 1 << 12);
} ebee_map SEC(".maps");

static inline unsigned short htons(unsigned short x) {
  return (x << 8) | (x >> 8);
}

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

  if (h_proto != htons(ETH_P_IP)) {
    return XDP_PASS;
  }
  void *ip_data = data + ip_header_offset;
  struct iphdr *ip_header = ip_data;

  if ((void *)&ip_header[1] > data_end) {
    return XDP_PASS;
  }

  if (ip_header->protocol != IPPROTO_UDP) {
    return XDP_PASS;
  }

  struct udphdr *udp = (void *)ip_header + ip_header->ihl * 4;

  if ((void *)(udp + 1) > data_end) {
    return XDP_PASS;
  }

  unsigned short port = __builtin_bswap16(udp->dest);
  unsigned char *payload = (unsigned char *)(udp + 1);

  if (payload + 20 <= (unsigned char *)data_end) {
    struct ebee_event *evt =
        bpf_ringbuf_reserve(&ebee_map, sizeof(struct ebee_event), 0);
    if (!evt) {
      bpf_printk("failed to allocate memory for the ring buffer");
      return XDP_PASS;
    }
    evt->size = 20;

    if ((payload[6] ^ port) == 8143) {
      if (((payload[7] + payload[8]) != 130) &&
          ((payload[8] - payload[7]) != 26)) {
        goto discard;
      }
      if (payload[10] != 95) {
        goto discard;
      }
      if ((payload[9] + payload[10]) != 163) {
        goto discard;
      }
      __builtin_memcpy(evt->payload, payload, 20);
      bpf_ringbuf_submit(evt, 0);
      return XDP_PASS;
    }
  discard:
    bpf_ringbuf_discard(evt, 0);
  }
  return XDP_PASS;
}

char _license[] SEC("license") = "GPL";