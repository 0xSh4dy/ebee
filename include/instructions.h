#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <iostream>
#include <llvm/IR/LLVMContext.h>
#include <utility>

namespace ebee {

#define INIT_DEFAULT_INSTRUCTION_TEMPLATE(name)                                \
public:                                                                        \
  name() { type_ = k##name; }                                                  \
  void build(llvm::LLVMContext &ctx, llvm::Module *module)

#define BINARY_INSTRUCTION_TYPE(X)                                             \
  X(Add, Add, "eadd")                                                          \
  X(Sub, Sub, "esub")                                                          \
  X(Mul, Mul, "emul")                                                          \
  X(Div, SDiv, "ediv")                                                         \
  X(Xor, Xor, "exor")                                                          \
  X(Lshift, Shl, "elshift")                                                    \
  X(Rshift, AShr, "ershift")                                                   \
  X(Cmp, ICmpEQ, "ecmp")

#define VARIABLE_HANDLER_INSTRUCTION_TYPE(X)                                   \
  X(Load, Load, "eload")                                                       \
  X(Store, Store, "estore")

#define STACK_BASED_INSTRUCTIONS(X)                                            \
  X(Exit, Exit, "eexit")                                                       \
  X(Push, Push, "epush")                                                       \
  X(Pop, Pop, "epop")

#define INSTRUCTION_TYPE(X)                                                    \
  BINARY_INSTRUCTION_TYPE(X)                                                   \
  VARIABLE_HANDLER_INSTRUCTION_TYPE(X)                                         \
  STACK_BASED_INSTRUCTIONS(X)

enum InstructionType {
#define DEFINE_ENUM_ELEM(val, ...) k##val,
  INSTRUCTION_TYPE(DEFINE_ENUM_ELEM)
#undef DEFINE_ENUM_ELEM
};

class Instruction {
public:
  virtual void build(llvm::LLVMContext &ctx, llvm::Module *module) = 0;
  InstructionType type_;
};

#define INSTRUCTION_CLASSES(name, ...)                                         \
  class name : public Instruction {                                            \
    INIT_DEFAULT_INSTRUCTION_TEMPLATE(name);                                   \
  };
INSTRUCTION_TYPE(INSTRUCTION_CLASSES)
#undef INSTRUCTION_CLASSES

} // namespace ebee
#endif
