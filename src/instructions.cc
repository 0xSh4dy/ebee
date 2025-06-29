#include "instructions.h"

#include <iostream>
#include <llvm/IR/IRBuilder.h>

using namespace llvm;

namespace ebee {
#define BINARY_OPERATION_HANDLERS(op, handler, name)                           \
  void op::build(llvm::LLVMContext &ctx, llvm::Module *module) {               \
    IntegerType *int32_type = Type::getInt32Ty(ctx);                           \
    std::vector<Type *> params(2, int32_type);                                 \
    IRBuilder<> ir_builder(ctx);                                               \
    FunctionType *function_type =                                              \
        FunctionType::get(int32_type, params, false);                          \
    Function *function = Function::Create(                                     \
        function_type, Function::ExternalLinkage, name, module);               \
    BasicBlock *entry_block = BasicBlock::Create(ctx, "entry", function);      \
    ir_builder.SetInsertPoint(entry_block);                                    \
    auto args = function->args();                                              \
    auto arg_iter = args.begin();                                              \
    Argument *arg1 = arg_iter++;                                               \
    Argument *arg2 = arg_iter;                                                 \
    Value *result = ir_builder.Create##handler(arg1, arg2);                    \
    ir_builder.CreateRet(result);                                              \
  }
BINARY_INSTRUCTION_TYPE(BINARY_OPERATION_HANDLERS)
#undef BINARY_OPERATION_HANDLERS

#define VARIABLE_OP_HANDLERS(op, handler, name)                                \
  void op::build(llvm::LLVMContext &ctx, llvm::Module *module) {}
VARIABLE_HANDLER_INSTRUCTION_TYPE(VARIABLE_OP_HANDLERS)
#undef VARIABLE_OP_HANDLERS

#define STACK_OP_HANDLERS(op, handler, name)                                   \
  void op::build(llvm::LLVMContext &ctx, llvm::Module *module) {}
STACK_BASED_INSTRUCTIONS(STACK_OP_HANDLERS)
#undef STACK_OP_HANDLERS
void test(llvm::LLVMContext &ctx) { 
  IRBuilder<> ir_builder(ctx); 
  // ir_builder.CreateICmpEQ()
}

} // namespace ebee