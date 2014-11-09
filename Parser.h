#ifndef __PARSER__
#define __PARSER__

#include <sstream>
#include <iostream>
#include <fstream>
#include <string>

#include <vector>
#include <map>
#include <qrect.h>

#include "Memory.h"

typedef enum {
  P_NONE,
  P_DATA,
  P_DATA_HEADER,
  P_INST,
  P_INST_HEADER } Parse_t;


class Parser{
public:
  std::string file;
  StringVec_t tokens;
  Parse_t state;
  std::string line;
  unsigned lineNum;
  bool labelNextLine;
  std::string nextLabel;
public:
  Parser(const std::string& fileName);
  void ReadMemoryContents(Memory& data,Memory& inst);
  Parse_t UpdateState(const std::string& line0); //Figure out the new state from  the next line
  bool ParseLabel(StringVec_t tokens);
  void ReadInstContent(Memory& inst);
  void ReadDataContent(Memory& data);
  void ReadContent(Memory& memTarget);
  void Alloc(Memory& data,Memory& inst);
  void Words(Memory& data,Memory& inst);
  void SetupHeader(Memory& memTarget);
  void FindLabel(Memory& memTarget);
  unsigned FindAddressTag(const std::string&);
  void ParseLine(const std::string&, StringVec_t&);
  void exitWithOutput(const std::string reason);
  bool isNumber(std::string& str);
};

#endif
