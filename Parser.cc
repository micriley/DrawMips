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
		ParseLabel(tokens);
//    if(tokens[0] == ".alloc")
//      Alloc(data,inst);
//    if(tokens[0] == ".word")
//      Words(data,inst);
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
  ostringstream s;
  for(unsigned i = labelNextLine ? 1 : 0; i < tokens.size(); ++i)
  {
    s << tokens[i];
    if(i != tokens.size() - 1)
      s << " ";
  }
  ReadContent(inst);
  inst.memory.back().SetContentsGeneric(s.str()); //Might have to mess with this some.
  return;
}
void Parser::ReadDataContent(Memory& data)
{
	std::string targetToken("");
	if(tokens.size() == 1)
	{
		if(!isNumber(tokens[0])) Parser::exitWithOutput("Found a singular data token that was Not a Number.");
		targetToken = tokens[0];
	}
	if(tokens.size() == 2)
	{
		if((isNumber(tokens[0]) && isNumber(tokens[1])) || (!isNumber(tokens[0]) && !isNumber(tokens[1])))
		{
			Parser::exitWithOutput("Found 2 data tokens and expected a key + a number value");
		}
		if(isNumber(tokens[0]))
		{
			targetToken = tokens[0];
			nextLabel = tokens[1];
		}
		else
		{
			targetToken = tokens[1];
			nextLabel = tokens[0];
		}
	}
  if(tokens.size() > 2)
  {
		if(isNumber(tokens[0]))
		{
	    Parser::exitWithOutput("Found more than 2 tokens on a data line and expected the first token to be a key-label");
		}
		labelNextLine = true;
		nextLabel = tokens[0];
    int allocPos = contains(tokens, std::string(".alloc"));
		int wordPos  = contains(tokens, std::string(".word"));
		if(allocPos != -1){ Alloc(data, allocPos); return; } // The Return is a concession to complex alloc words. If you do that then this parser won't handle it
		if(wordPos != -1){ Words(data,wordPos); return; }
  }
  ReadContent(data);
  data.memory.back().SetContentsGeneric(targetToken);
  FindLabel(data);
}

void Parser::ReadContent(Memory& memTarget)
{
  memTarget.AddEmptyMemBlock(lineNum);
  //Set the next line with a label for the view and the model
  FindLabel(memTarget);
}

void Parser::Alloc(Memory& data, unsigned allocPos)
{
  switch(state)
  {
    case P_INST:
    case P_DATA:
      break;
    case P_INST_HEADER:
			exitWithOutput("Found an .alloc tag, but expecting a full .text header line.");
    case P_DATA_HEADER:
      exitWithOutput("Found an .alloc tag, but expecting a full .data header line.");
      break;
    case P_NONE:
      exitWithOutput("Found an .alloc tag, but we're not in a .data or .text block.");
      break;
  }
	if(tokens.size() < allocPos + 1)
		exitWithOutput("Found an .alloc tag, but missing the number of words to allocate");
  if(!isNumber(tokens[allocPos + 1]))
  {
    ostringstream s;
    s << tokens[allocPos + 1] << " is not a number. Please use the format .alloc [integer].";
    exitWithOutput(s.str());
    return;
  }
  unsigned n = atoi(tokens[allocPos + 1].c_str());
  for(unsigned i = 0; i < n; ++i)
  {
    data.AddEmptyMemBlock(lineNum);
    FindLabel(data);
  }
}

void Parser::Words(Memory& data, unsigned wordsPos)
{
  switch(state)
  {
    case P_INST:
      break;
    case P_DATA:
      break;
    case P_INST_HEADER:
			exitWithOutput("Found a .words tag, but expecting a full .text header line.");
    case P_DATA_HEADER:
      exitWithOutput("Found a .words tag, but expecting a full .data header line.");
      break;
    case P_NONE:
      exitWithOutput("Found a .words tag, but we're not in a .data or .text block.");
      break;
  }
  for(unsigned i = wordsPos + 1; i < tokens.size(); ++i)
  {
    if(!isNumber(tokens[i]))
    {
      ostringstream s;
      s << tokens[i] << " is not a number. Please only use integers when defining words.";
      exitWithOutput(s.str().c_str());
      return;
    }
    data.AddEmptyMemBlock(lineNum);
    data.GetBack()->SetContentsGeneric(tokens[i]);
    FindLabel(data);
  }
}

void Parser::SetupHeader(Memory& memTarget)
{
	if(tokens.size() > 2)
	{
	  memTarget.firstAddress = atoi(tokens[1].c_str());
	  memTarget.addressAdder = atoi(tokens[2].c_str());
	}
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

int Parser::contains(StringVec_t& toks, std::string str)
{
	for(unsigned i = 0; i < toks.size(); i++)
	{
		if(toks[i].compare(str) == 0) return i;
	}
	return -1;
}
