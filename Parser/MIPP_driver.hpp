#ifndef __MCDRIVER_HPP__
#define __MCDRIVER_HPP__ 1
 
#include <string>
#include "mc_scanner.hpp"
#include "mc_parser.tab.hh"
#include "../Instruction.h"
namespace MIPP{

class MIPP_Driver{
public:
MIPP_Driver() :
parser( nullptr ),
scanner( nullptr ){};
instructionSet( nullptr ),

virtual ~MC_Driver();
 
void parse( const char *filename );
 
std::ostream& print(std::ostream &stream);
private:
Instrucion *instructionSet;
MIPP::MIPP_Parser *parser;
MIPP::MIPP_Scanner *scanner;
};
 
} /* end namespace MC */
#endif /* END __MCDRIVER_HPP__ */
