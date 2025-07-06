#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <iostream>
#include <llvm/IR/LLVMContext.h>
#include <array>
#include <utility>

#define EPUSH "172ed1de-9170-4783-b033-dec4596e5ada"
#define EADD "1783f882-67bc-40b7-93a2-892961497772"
#define ESUB "1f6683a7-4478-494e-9e9f-f679491b5344"
#define EMUL "48cc495b-5b7a-443c-abf9-9b0ae63bd723"
#define EDIV "7aaf6c46-6861-4dcf-b530-4b6937d4aaeb"
#define EAND "5066f217-6223-487e-8e05-96eb1229a680"
#define EOR "439b9ed2-ce85-4880-b8aa-9e2081c76769"
#define EXOR "470f2241-b469-468b-afeb-d5e5309ca6c8"
#define ELSHIFT "dfb44eec-1e5f-4f4b-94b4-e1146181dd13"
#define ERSHIFT "e5ddbc48-c872-4b57-ada7-d7e6d80f55cb"
#define EPOP "6f3bd414-52fb-464f-ad55-f7ea266f513d"
#define ELOAD "4389e2c6-f34b-4b2b-8df4-20c21105585c"
#define ESTORE "13025381-b3b9-4bd9-92ec-8b0e4a71f863"
#define ECMP "57479ca5-787a-420f-b374-9eb0108deb64"
#define EEXIT "57219a35-4dc4-4f5a-aaab-f17bad611232"

namespace ebee {

#define INIT_DEFAULT_INSTRUCTION_TEMPLATE(name)                                \
public:                                                                        \
  name() { type_ = k##name; }                                                  \
  void build(llvm::LLVMContext &ctx, llvm::Module *module)

#define BINARY_INSTRUCTION_TYPE(X)                                             \
  X(Add, Add, EADD)                                                          \
  X(Sub, Sub, ESUB)                                                          \
  X(Mul, Mul, EMUL)                                                          \
  X(Div, SDiv, EDIV)                                                         \
  X(Xor, Xor, EXOR)                                                          \
  X(And, And, EAND)                                                          \
  X(Or, Or, EOR)                                                             \
  X(Lshift, Shl, ELSHIFT)                                                    \
  X(Rshift, AShr, ERSHIFT)                                                   \
  X(Cmp, ICmpEQ, ECMP)

#define VARIABLE_HANDLER_INSTRUCTION_TYPE(X)                                   \
  X(Load, Load, ELOAD)                                                       \
  X(Store, Store, ESTORE)

#define STACK_BASED_INSTRUCTIONS(X)                                            \
  X(Exit, Exit, EEXIT)                                                       \
  X(Push, Push, EPUSH)                                                       \
  X(Pop, Pop, EPOP)

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
