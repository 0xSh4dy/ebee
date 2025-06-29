#include "instructions.h"

#include <iostream>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>

int main(){
    llvm::LLVMContext ctx;
    auto module = std::make_unique<llvm::Module>("shadie_module",ctx);
    ebee::Instruction *x = new ebee::Xor();
    x->build(ctx,module.get());
    return 0;
}