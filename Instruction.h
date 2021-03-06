#ifndef __INSTRUCTION__
#define __INSTRUTION__

#include "Memory.h"

using namespace std;

typedef enum 
  {
    ADD,
    SUB,
    ADDI,
    ADDU,
    SUBU,
    ADDIU,
    SLT,
    SLTI,
    SLTU,
    SLTIU,
    MULT,
    MULTU,
    DIV,
    DIVU,
    MFHI,
    MFLO,
    LUI,
    AND,
    OR,
    ANDI,
    ORI,
    NOR,
    XOR,
    XORI,
    SLL,
    SLLV,
    SRL,
    SRLV,
    SRA,
    SRAV,
    LW,
    LH,
    SW,
    LB,
    LBU,
    SB,
    BEQ,
    BNE,
    J,
    JR,
    JAL,
  } MipsInst_t;

typedef enum{
    MOVE,
    CLEAR,
    NOT,
    LA,
    LI,
    B,
    BAL,
    BGT,
    BLT,
    BGE,
    BLE,
    BGTU,
    BGTZ,
    BEQZ,
    REM,
} PseudoMipsInst_t;
  
// For decoding instructions and operands
typedef enum 
  {
    OP_NONE,
    OP_Register,
    OP_Constant,
    OP_ConstantOffset,  // Memory address offset
    OP_InstructionTag,  // Tag for Instruction Memory
    OP_AddressTag       // Tag for Data Memory
  } OperandType_t;
typedef enum
	{
    MF_NONE,
		MF_HI,
		MF_LO
	} MemoryFilter_t;

class InstLookup 
{
public:
  InstLookup(): st("ADD"), instruction(ADD) {}
  InstLookup(std::string st0, MipsInst_t i0)
    : st(st0), instruction(i0) {}
  InstLookup(std::string st0, PseudoMipsInst_t i0): st(st0), pseudoInstruction(i0) {}
 public:
  std::string st;
  MipsInst_t      instruction;
  PseudoMipsInst_t pseudoInstruction;
};

class InstOperand 
{
public:
  InstOperand() : opType(OP_NONE), addressInt(0), filterPrefix(MF_NONE) {}
  InstOperand(const std::string& st, OperandType_t);
  OperandType_t opType;
  unsigned      RegNum;   // If OP_Register
  int			      Const;    // If OP_Constant
  std::string   address;  // If address tag
  unsigned      addressInt;  // The look'ed up value of addrss tag "Address"
  static string registerPrefix[];
	MemoryFilter_t filterPrefix; //Filter the value: either HI or LO
};

class Instruction {
public:
  Instruction(const std::string& st,Memory& m0,Memory& i0,unsigned lineNum);
public: 
  Memory&       data;
	Memory&				inst;
  InstLookup    instruction;
  InstOperand   operands[3];
  static string registerMap[];
  unsigned      sourceLine;
public:
  static unsigned getRegisterFromCode(std::string&); //Decodes a register name to it's value
  void Parse(const std::string&); // Parse the text rep. of an inst
  void LookupOpcode(const std::string&);
  unsigned GetReg(unsigned);                 // Get register value from operand
  unsigned GetConst(unsigned);               // Get constant value from operand
  std::string GetAddress(unsigned);
  OperandType_t GetOptype(unsigned);
  bool containsLabel(); //Check for an instruction Label
  
};
class PseudoInstruction:Instruction {
public:
  Instruction derived[];
  PseudoMipsInst_t pseudoInstruction;
};

#endif
