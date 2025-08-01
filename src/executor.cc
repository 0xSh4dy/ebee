#include "executor.h"
#include <stack>

namespace ebee {
void Executor::Execute(const std::string &input) {
  std::vector<std::string> instrs = split_string(input);
  std::string function_name = instrs[0];
  auto itr = instruction_map_.find(function_name);
  if (itr == instruction_map_.end()) {
    throw std::runtime_error("cannot execute " + function_name);
  }
  InstructionType instr_type = itr->second;
  int op_mode = instrs.size();

  if (op_mode == 3) {
    llvm::orc::ExecutorAddr addr = GetExecutorAddr(function_name);

    int op1 = std::stoi(instrs[1]);
    int op2 = std::stoi(instrs[2]);
    auto *fn = reinterpret_cast<int (*)(int, int)>(addr.getValue());
    int res = fn(op1, op2);
    stack_.push(res);
  } else if (op_mode == 2) {
    switch (instr_type) {
    case kPush:
      stack_.push(std::stoi(instrs[1]));
      break;
    }
  } else if (op_mode == 1) {
    switch (instr_type) {
    case kCmp: {
      llvm::orc::ExecutorAddr addr = GetExecutorAddr(function_name);

      auto *fn = reinterpret_cast<int (*)(int, int)>(addr.getValue());
      if (!fn) {
        throw std::runtime_error("couldn't find the handler");
      }
      int val1 = stack_.top();
      stack_.pop();
      int val2 = stack_.top();
      stack_.pop();
      stack_.push(fn(val1, val2) & 0xff);
      break;
    }
    case kExit: {
      bool status = stack_.top();
      stack_.pop();
      if (!status) {
        has_errors_ = true;
      }
      break;
    }
    case kPop:
      stack_.pop();
      break;
    case kAnd:
    case kOr:
    case kAdd:
    case kSub:
    case kMul:
    case kDiv:
    case kXor:
    case kLshift:
    case kRshift: {
      llvm::orc::ExecutorAddr addr = GetExecutorAddr(function_name);
      auto *fn = reinterpret_cast<int (*)(int, int)>(addr.getValue());
      if (!fn) {
        throw std::runtime_error("couldn't find the handler");
      }
      int val1 = stack_.top();
      stack_.pop();
      int val2 = stack_.top();
      stack_.pop();
      stack_.push(fn(val1, val2));
      break;
    }
    default:
      throw std::runtime_error("cannot execute unknown instruction");
    }
  }
}
} // namespace ebee