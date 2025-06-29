#include "instructions.h"
#include <iostream>
#include <net/if.h>
#include <bpf/bpf.h>
#include <bpf/libbpf.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>

int main(int argc,char **argv){
    // llvm::LLVMContext ctx;
    // auto module = std::make_unique<llvm::Module>("shadie_module",ctx);
    // ebee::Instruction *x = new ebee::Xor();
    // x->build(ctx,module.get());
    if(argc < 3){
        fprintf(stderr, "Usage: %s <ifname> <bpf_obj_file>\n", argv[0]);
        return 1;
    }
    const char *if_name = argv[1];
    const char *obj_file = argv[2];
    int ifindex = if_nametoindex(if_name);

    if(!ifindex){
        perror("if_nametoindex");
        return 1;
    }
    
    int prog_fd;
    struct bpf_object *obj;
    struct bpf_program *prog;

    obj = bpf_object__open(obj_file);
    if(!obj){
        std::cerr<<"failed to open the eBPF object file\n";
        return 1;
    }
    prog = bpf_object__next_program(obj,NULL);
    if(!prog){
        std::cerr<<"failed to open the XDP program\n";
        return 1;
    }
    prog_fd = bpf_program__fd(prog);
    if(prog_fd < 0){
        std::cerr<<"failed to retrieve the file descriptor of the eBPF program\n";
        return 1;
    }
    if(bpf_xdp_attach(ifindex,prog_fd,0,NULL)){
        std::cerr<<"failed to attach the program to network interface\n";
        return 1;
    }
    return 0;
}