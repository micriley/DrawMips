//Parses a file for both the data and inst memory array
// Michael K. Riley, Georgia Tech, Fall 2014

#include <string.h>
#include <stdlib.h>

#include "Parser.h"

using namespace std;
Parser::Parser(const string& fileName)
{
  file = fileName;
  ifstream ifs(fileName.c_str());
  if (!ifs)
    {
      Parser::exitWithOutput("Can't open file");
    }
  labelNextLine = false;
}

void Parser::ReadMemoryContents(Memory& data, Memory& inst)
{
  //Intialize start state
  lineNum = 0;
  state = P_NONE;
  string label;
  //Open file
  ifstream ifs(file.c_str());
  while(ifs)
  {
    getline(ifs,line); // Get to next line
    state = UpdateState(line);
    lineNum++;
    ParseLine(line, tokens);
    if(tokens.empty()) continue;
    if(ParseLabel(tokens)) continue;
    if(tokens[0] == ".alloc")
      Alloc(data,inst);
    if(tokens[0] == ".word")
      Words(data,inst);
    if(state == P_INST)
      ReadInstContent(inst);
    if(state == P_DATA)
      ReadDataContent(data);
    if(state == P_INST_HEADER)
      SetupHeader(inst);
    if(state  == P_DATA_HEADER)
      SetupHeader(data);
  }
}
Parse_t Parser::UpdateState(const string& line0)
{
  if(state == P_INST_HEADER)
    return P_INST;
  if(state == P_DATA_HEADER)
    return P_DATA;
  if(strstr(line0.c_str(),".text"))
    return P_INST_HEADER;
  if(strstr(line0.c_str(),".data"))
    return P_DATA_HEADER;
  return state;
}
// Static methods
//Makes a string vector of different tokens. We assume they're delinated by ' '
void Parser::ParseLine(const string& st, StringVec_t& tokens)
{ // First look for '#' for comments. Cut off and ignore the contents
  tokens.erase(tokens.begin(),tokens.end()); //Resets the token vector
  string st0 = st;
  string::size_type k = st0.find_first_of("#");
  if (k != string::npos) st0 = st0.substr(0, k);
  
  string::size_type i = 0;
  while ( true )
  {
    // Cut off leading spaces
    while(i < st0.length() && st0[i] == ' ') i++;
    if (i == st0.length()) return;
    string::size_type k = st0.find_first_of(" ", i);
    if (k == string::npos)
    {
      tokens.push_back(st0.substr(i, string::npos));
      return;
    }
    else
    {
      tokens.push_back(st0.substr(i, k - i));
      i = k + 1;
      if (i == st0.length()) return;
    }
  }
}

bool Parser::ParseLabel(StringVec_t tokens)
{
  if(!tokens[0].empty())
  {
    if(tokens[0][0] == '@')
    {
      if(state == P_NONE)
      {
        stringstream ss;
        ss << "Found label " << tokens[0].substr(1, tokens[0].length()) << " with no memory context." << endl << "Please make sure to include all labels within a .data or .text block";
        Parser::exitWithOutput(ss.str());
        return false; //Unncessary, but being concise.
      }     
      labelNextLine = true;
      nextLabel = tokens[0].substr(1,tokens[0].length());
      return true;
    }
  }
  return false;
}
void Parser::ReadInstContent(Memory& inst)
{
  if(tokens.size() > 4)
    Parser::exitWithOutput("Found more than 4 tokens on a text line. Please format your lines as [Instruction/Mandatory] [Output Register][Param1/Mandatory] [Param2/Optional]");
  ReadContent(inst);
  ostringstream s;
  for(unsigned i = 0; i< tokens.size(); ++i)
  {
    s << tokens[i];
    if(i != tokens.size() - 1)
      s << " ";
  }
  inst.memory.back().SetContentsGeneric(s.str()); //Might have to mess with this some.
  return;
}
void Parser::ReadDataContent(Memory& data)
{
  if(tokens.size() > 2)
  {
    Parser::exitWithOutput("Found more than 2 tokens on a data line. Please format your lines as [Value/Mandatory] [Label/Optional]");
  return;
  }
  ReadContent(data);
  data.memory.back().SetContentsGeneric(tokens[0]);
  if(tokens.size() > 1)
  {
    data.memory.back().SetLabel(tokens[1]);
  }
}

void Parser::ReadContent(Memory& memTarget)
{
  memTarget.AddEmptyMemBlock(lineNum);
  //Set the next line with a label for the view and the model
  FindLabel(memTarget);
}

void Parser::Alloc(Memory& data,Memory& inst)
{
  Memory& memTarget = state == P_INST ? inst : data; // hack around because references must be initialized immediately
  switch(state)
  {
    case P_INST:
    case P_DATA:
      break;
    case P_INST_HEADER:
    case P_DATA_HEADER:
      exitWithOutput("Found an .alloc tag, but expecting the header values [firstAddress] [Bytes per address] before defining an alloc.");
      break;
    case P_NONE:
      exitWithOutput("Found an .alloc tag, but we're not in a .data or .text block.");
      break;
  }
  if(!isNumber(tokens[1]))
  {
    ostringstream s;
    s << tokens[1] << " is not a number. Please use the format .alloc [integer].";
    exitWithOutput(s.str());
    return;
  }
  unsigned n = atoi(tokens[1].c_str());
  for(unsigned i = 0; i < n; ++i)
  {
    memTarget.AddEmptyMemBlock(lineNum);
    FindLabel(memTarget);
  }
}

void Parser::Words(Memory& data,Memory& inst)
{
  Memory& memTarget = state == P_INST ? inst : data; // hack around because references must be initialized immediately
  switch(state)
  {
    case P_INST:
      memTarget = inst;
      break;
    case P_DATA:
      memTarget = data;
      break;
    case P_INST_HEADER:
    case P_DATA_HEADER:
      exitWithOutput("Found an .word tag, but expecting the header values [firstAddress] [Bytes per address] before defining an alloc.");
      break;
    case P_NONE:
      exitWithOutput("Found an .word tag, but we're not in a .data or .text block.");
      break;
  }
  for(unsigned i = 1; i < tokens.size(); ++i)
  {
    if(!isNumber(tokens[i]))
    {
      ostringstream s;
      s << tokens[i] << " is not a number. Please only use integers when defining words.";
      exitWithOutput(s.str().c_str());
      return;
    }
    memTarget.AddEmptyMemBlock(lineNum);
    memTarget.GetBack()->SetContentsGeneric(tokens[i]);
    FindLabel(memTarget);
  }
}

void Parser::SetupHeader(Memory& memTarget)
{
  if(tokens.size() < 3)
  {
    exitWithOutput("Found .data or .text tag with no address specifiers. Expecting format [.data/.text] [starting address] [block size]");
    return;
  }
  memTarget.firstAddress = atoi(tokens[1].c_str());
  memTarget.addressAdder = atoi(tokens[2].c_str());
}

void Parser::FindLabel(Memory& memTarget)
{
  if(labelNextLine)
  {
    Memory::labelMap[nextLabel] = *memTarget.GetBack();
    memTarget.GetBack()->SetLabel(nextLabel);
    labelNextLine = false;
  }
  //Else set the line label to the address number
  else
  {
    ostringstream s;
    s << (memTarget.firstAddress + memTarget.addressAdder*memTarget.memory.back().index);
    memTarget.memory.back().SetLabel(s.str());
  }
}

void Parser::exitWithOutput(const string reason)
{
  cout << "Parsing error." << endl;
  cout << "Line number " << lineNum << ": " << line << endl;
  cout << reason << endl;
  exit(1);
}

bool Parser::isNumber(std::string& str)
{
  for(unsigned i = 0;  i < str.length(); ++i)
  {
    if(!isdigit(str[i]))
      return false;
  }
  return true;
}
