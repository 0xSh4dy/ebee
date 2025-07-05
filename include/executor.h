#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "instructions.h"
#include "utils.h"

#include <iostream>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Transforms/Utils/Cloning.h>
#include <sstream>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

namespace ebee {
class Executor {
public:
#define DEFINE_INSTR_HANDLERS(op, handler, name)                               \
  case k##op: {                                                                \
    Instruction *instr = new op();                                             \
    instr->build(ctx_, module_.get());                                         \
    instructions_.push_back(instr);                                            \
    break;                                                                     \
  }

  inline bool HasErrors() const noexcept { return has_errors_; }
  
  Executor() {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    module_ = std::make_unique<llvm::Module>("shadie_module", ctx_);
    jit_builder_ = llvm::orc::LLJITBuilder();
    auto expected_jit = jit_builder_.create();

    if (!expected_jit) {
      llvm::logAllUnhandledErrors(expected_jit.takeError(), llvm::errs(),
                                  "JIT creation failed: ");
      std::exit(1);
    }
    jit_ = std::move(*expected_jit);

    for (auto &itr : instruction_map_) {
      InstructionType instr_type = itr.second;
      switch (instr_type) {
        INSTRUCTION_TYPE(DEFINE_INSTR_HANDLERS)
      default:
        throw std::runtime_error("invalid instruction found, leaving");
      }
    }
    if (auto err = jit_->addIRModule(llvm::orc::ThreadSafeModule(
            llvm::CloneModule(*module_.get()),
            std::make_unique<llvm::LLVMContext>()))) {
      throw std::runtime_error("Failed to add IR module for JIT compilation: " +
                               llvm::toString(std::move(err)));
    }
  }

#undef DEFINE_INSTR_HANDLERS

  void Execute(const std::string &input);
  std::vector<Instruction *> GetInstructions() const { return instructions_; }

private:
  bool has_errors_ = false;
  std::stack<int> stack_;
  llvm::LLVMContext ctx_;
  llvm::orc::LLJITBuilder jit_builder_;
  std::unique_ptr<llvm::orc::LLJIT> jit_;
  std::unique_ptr<llvm::Module> module_;
  std::vector<Instruction *> instructions_;
  const std::unordered_map<std::string, InstructionType> instruction_map_ = {
#define DEFINE_INSTR_TYPE(op, handler, name) {name, k##op},
      INSTRUCTION_TYPE(DEFINE_INSTR_TYPE)
#undef DEFINE_INSTR_TYPE
  };

  llvm::orc::ExecutorAddr GetExecutorAddr(const std::string &function_name) {
    llvm::orc::ExecutorAddr sym = jit_->lookup(function_name).get();
    if (!sym) {
      throw std::runtime_error("Function not found in JIT: " + function_name);
    }
    return sym;
  }
}; // namespace ebee
} // namespace ebee

#endif