#include "challenge.h"
#include "instructions.h"
#include <bpf/bpf.h>
#include <bpf/libbpf.h>
#include <csignal>
#include <iostream>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <net/if.h>
#include <unistd.h>

static int ifindex = 0;
static int attach_flags = 0;

struct ebee_event {
  unsigned char payload[20];
  int size;
};

static void handle_signal(int sig) {
  std::cout << "\nSignal received (" << sig << "), detaching XDP program...\n";
  if (ifindex) {
    bpf_xdp_detach(ifindex, attach_flags, NULL);
  }
  exit(0);
}

static int event_handler(void *ctx, void *data, size_t len) {
  std::cout << "Triggered event handler\n";
  struct ebee_event *evt = (struct ebee_event *)data;
  // for(int i=0;i<20;i++){
  //   printf("%02x ",evt->payload[i]);
  // }
  // fflush(stdout);
  // printf("\n");
  std::cout << evt->size << std::endl;
  StartChallenge(evt->payload, evt->size);
  return 0;
}

int Run(int argc, char **argv) {
  const char *if_name = argv[1];
  const char *obj_file = argv[2];
  ifindex = if_nametoindex(if_name);
  attach_flags = 0;

  if (!ifindex) {
    perror("if_nametoindex");
    return 1;
  }

  struct sigaction sa = {};
  sa.sa_handler = handle_signal;
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);

  struct bpf_object *obj;
  struct bpf_program *prog;

  obj = bpf_object__open(obj_file);
  if (!obj) {
    std::cerr << "failed to open the eBPF object file\n";
    return 1;
  }

  if (bpf_object__load(obj)) {
    perror("bpf_object__load");
    std::cerr << "failed to load the eBPF program\n";
    return 1;
  }

  prog = bpf_object__find_program_by_name(obj, "check_packets");
  if (!prog) {
    std::cerr << "failed to open the XDP program\n";
    return 1;
  }

  int prog_fd = bpf_program__fd(prog);
  if (prog_fd < 0) {
    perror("bpf_program__fd");
    std::cerr << "failed to retrieve the file descriptor of the eBPF program\n";
    return 1;
  }

  if (bpf_xdp_attach(ifindex, prog_fd, attach_flags, NULL)) {
    std::cerr << "failed to attach the program to network interface\n";
    return 1;
  }

  int rb_fd = bpf_object__find_map_fd_by_name(obj, "ebee_map");
  if (rb_fd < 0) {
    perror("bpf_object__find_map_fd_by_name");
    std::cerr << "failed to find the file descriptor of ebee_map\n";
    return 1;
  }

  struct ring_buffer *rbuffer =
      ring_buffer__new(rb_fd, event_handler, NULL, NULL);
  if (!rbuffer) {
    std::cerr << "failed to create the ring buffer\n";
    return 1;
  }

  while (true) {
    int e = ring_buffer__consume(rbuffer);
    if (e < 0) {
      std::cerr << "ring_buffer__consume error\n";
      break;
    }
    sleep(1);
  }
  return 0;
}

int main(int argc, char **argv) {
  // if (argc < 3) {
  //   fprintf(stderr, "Usage: %s <ifname> <bpf_obj_file>\n", argv[0]);
  //   return 1;
  // }
  char *buffer = const_cast<char *>("j1tT3D_4ND_c0MmItT3d");
  StartChallenge(reinterpret_cast<unsigned char *>(buffer), 0);
  // return Run(argc,argv);
  return 0;
}
