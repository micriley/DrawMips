#include <iostream>
#include <limits>

#include "Instruction.h"
#include "Computer.h"

using namespace std;

InstLookup instructionSet[] = {
  InstLookup(string("ADD"),    ADD),
  InstLookup(string("ADDU"),    ADDU),
  InstLookup(string("SUB"),    SUB),
  InstLookup(string("SUBU"),    SUBU),
  InstLookup(string("ADDI"),    ADDI),
  InstLookup(string("ADDIU"),    ADDIU),
  InstLookup(string("MULT"),    MULT),
  InstLookup(string("MULTU"),    MULTU),
  InstLookup(string("DIV"),    DIV),
  InstLookup(string("DIVU"),    DIVU),
  InstLookup(string("LW"),    LW),
  InstLookup(string("LH"),    LW),
  InstLookup(string("LB"),    LB),
  InstLookup(string("LBU"),    LBU),
  InstLookup(string("SW"),    SW),
  InstLookup(string("SB"),    SB),
  InstLookup(string("LUI"),    LUI),
  InstLookup(string("MFHI"),    MFHI),
  InstLookup(string("MFLO"),    MFLO),
  InstLookup(string("AND"),    AND),
  InstLookup(string("ANDI"),    ANDI),
  InstLookup(string("OR"),    OR),
  InstLookup(string("ORI"),    ORI),
  InstLookup(string("XOR"),    XOR),
  InstLookup(string("NOR"),    NOR),
  InstLookup(string("SLT"),    SLT),
  InstLookup(string("SLTI"),    SLTI),
  InstLookup(string("SLL"),    SLL),
  InstLookup(string("SLLV"),    SLLV),
  InstLookup(string("SRL"),    SRL),
  InstLookup(string("SRLV"),    SRLV),
  InstLookup(string("SRA"),    SRA),
  InstLookup(string("BEQ"),    BEQ),
  InstLookup(string("BNE"),    BNE),
  InstLookup(string("J"),    J),
  InstLookup(string("JR"),    JR), //JR 31 = new HALT
  InstLookup(string("JAL"),    JAL),
};

InstLookup psuedoInstructionSet[] = {
  InstLookup(string("MOVE"),    MOVE),
  InstLookup(string("CLEAR"),    CLEAR),
  InstLookup(string("NOT"),    NOT),
  InstLookup(string("LA"),    LA),
  InstLookup(string("LI"),    LI),
  InstLookup(string("B"),    B),
  InstLookup(string("BAL"),    BAL),
  InstLookup(string("BGT"),    BGT),
  InstLookup(string("BLT"),    BLT),
  InstLookup(string("BGE"),    BGE),
  InstLookup(string("BLE"),    BLE),
  InstLookup(string("BGTU"),    BGTU),
  InstLookup(string("BGTZ"),    BGTZ),
  InstLookup(string("BEQZ"),    BEQZ),
  InstLookup(string("REM"),    REM)
};

string Instruction::registerMap[] = {"zero","at","a0","a1","a2","a3","t0","t1","t2","t3","t4","t5","t6","t7","s0","s1","s2","s3","s4","s5","s6","s7","t8","t9","k0","k1","gp","sp","fp","ra"};
char operandSeperators[] = {' ',',','(',')'};

//Utility function for determining numbers in string
static bool isNumber(const std::string& st)
{
    std::string::const_iterator it = st.begin();
    if(*it == '-') ++it;
    while (it != st.end() && std::isdigit(*it)) ++it;
    return !st.empty() && it == st.end();
}

//Reads an operand and intiates an InstOperand from it
InstOperand::InstOperand(const string& st,OperandType_t type=OP_NONE)
{
  //If nothing is set, OP_None must be the case
  opType = type;
  string st0 = st;
  if(st0[0] == '$')
  {
    st0 = st0.substr(1);
    opType = OP_Register;
  }
  else if(st0[0] == '@')
  {
    st0 = st0.substr(1);
    opType = OP_InstructionTag;
  }
  else if(isNumber(st0))
  {
    opType = OP_Constant;
  }
  else
  {
    opType = OP_AddressTag;
  }
  switch(opType)
  {
    case(OP_Register):
      if(!isNumber(st0))
        RegNum = Instruction::getRegisterFromCode(st0);
      else
        RegNum = atoi(st0.c_str());
    break;
    case(OP_Constant):
      Const = atoi(st0.c_str());
    break;
    case(OP_ConstantOffset):
    break;
    case(OP_InstructionTag):
      Address = st0;
    break;

    case(OP_AddressTag):
      Address = st0;
    break;
    case(OP_NONE):
    break;
  }
}

unsigned Instruction::getRegisterFromCode(string& st){
  for(unsigned i=0; i < (sizeof(registerMap)/sizeof(registerMap[0]));i++)
  {
    if(st.compare(registerMap[i]) == 0)
      return i;
  }
  cout << "No such instruction called" << st << endl;
  exit(1);
  return 33;//Change this to a variable later
}

Instruction::Instruction(const string& st,Memory& m0,unsigned lineNum): m(m0), sourceLine(lineNum)
{
  Parse(st);
}

void Instruction::Parse(const string& st)
{
  //cout << "Parsing instruction " << st << endl;
  string::size_type i = 0;
  char seperator;
  //Parse the Operands
  for (int j = 0; j < 4; ++j)
    { // Opcode and up to 3 operands
      string st0; // Next substring
      string::size_type k = string::npos;
      //If the last seperator was (, then it's a constant value we need to find
      if(seperator == '(')
      {
        string::size_type n = st.find_first_of(')');
        if(n == string::npos)
        {
          cout << "Expecting a matching ) in instruction line:" << sourceLine << endl;
          exit(1);
        }
        st0 = st.substr(i,n-i);
      }
      else
      {
        for(unsigned l = 0;l<(sizeof(operandSeperators)/sizeof(*operandSeperators));l++)
        {
          string::size_type o = st.find_first_of(operandSeperators[l], i);
          if(o<k)
          {
            k = o;
            seperator = operandSeperators[l];
          }
        }
        if (k == string::npos) st0 = st.substr(i, string::npos);
        else                   st0 = st.substr(i, k - i);
      }
      //Matching Const Value
      //cout << "st0 " << st0 << endl;
      if (j == 0)
        { // Opcode
          LookupOpcode(st0);
        }
      else
        { // Operand
          operands[j - 1] = InstOperand(st0);
        }
      if (k == string::npos) break;
      i = k + 1;
    }
  //cout << "Instruction is " << instruction
  //     << " (" << instructionSet[instruction].st << ") " << endl;
#ifdef VERBOSE_DEBUG
  cout << "Instruction is " << instruction
       << " (" << instructionSet[instruction].st << ") ";
  for (int i = 0; i < 3; ++i)
    {
      cout << "Operand Type " << operands[i].opType << " ";
      switch(operands[i].opType) {
      case OP_None:
        cout << "None";
        break;
      case OP_Register:
        cout << "Register " << operands[i].RegNum;
        break;
      case OP_Constant:
        cout << "Constant " << operands[i].Const;
        break;
      case OP_AddressTag:
        cout << "Address " << operands[i].Address;
        break;
        }
      cout << endl;
    }
#endif
  // Convert the address tags to constant addresses
  for (int i = 0; i < 3; ++i)
    {
      InstOperand& operand = operands[i];
      if (operand.opType == OP_AddressTag)
        {
          string tag = operand.Address;
          operand.Const = m.FindAddressTag(tag);
          operand.opType = OP_Constant;
        }
    }
}

void Instruction::LookupOpcode(const string& st)
{
  string st0 = st;
  for(unsigned i = 0; st[i]!='\0';i++)
    st0[i] = toupper(st[i]);
  for(unsigned i = 0; i < (sizeof(instructionSet)/sizeof(instructionSet[0])); i++)
    {
      if (instructionSet[i].st.empty())
        { // Not found
          cout << "Uknown instruction " << st << endl;
          exit(1);
        }
      if (instructionSet[i].st == st0)
        { // Found it
          instruction = instructionSet[i];
          return;
        }
    }
}

unsigned Instruction::GetReg(unsigned opnum)
{ // Get the register value from the specified operand
  if (operands[opnum].opType == OP_NONE)
  {
    return 0;
  }
  if (operands[opnum].opType != OP_Register)
    {
      cout << "OOps, expected register type for operand " << opnum
           << " found opType " << operands[opnum].opType 
           << " on line number " << sourceLine
           << endl;
      exit(1);
    }
  return operands[opnum].RegNum;
}

unsigned Instruction::GetConst(unsigned opnum)
{ // Get the register value from the specified operand
  if (operands[opnum].opType != OP_Constant)
    {
      cout << "Oops, expected const type for operand " << opnum
           << " found " << operands[opnum].opType << endl;
      exit(1);
    }
  return operands[opnum].Const;
}

std::string Instruction::GetAddress(unsigned opnum)
{
  return operands[opnum].Address;
}

OperandType_t Instruction::GetOptype(unsigned opnum)
{
  return operands[opnum].opType;
}

bool Instruction::containsLabel()
{
  return operands[0].opType == OP_AddressTag || operands[1].opType == OP_AddressTag || operands[2].opType == OP_AddressTag;
}
