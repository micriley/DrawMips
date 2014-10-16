%skeleton "lalr1.cc"
%require "3.0"
%debug
%defines
%define api.namespace {MC}
%define parser_class_name {MC_Parser}
 
%code requires{
namespace MIPP {
class MIPP_Driver;
class MIPP_Scanner;
}
}
 
%lex-param { MIPP_Scanner &scanner }
%parse-param { MIPP_Scanner &scanner }
 
%lex-param { MIPP_Driver &driver }
%parse-param { MIPP_Driver &driver }
 
%code{
#include <iostream>
#include <cstdlib>
#include <fstream>
/* include for all driver functions */
#include "MIPP_driver.hpp"
/* this is silly, but I can't figure out a way around */
static int yylex(MC::MC_Parser::semantic_type *yylval,
MIPP::MIPP_Scanner &scanner,
MIPP::MIPP_Driver &driver);
}
 
/* token types */
%union {
std::string *sval;
}
 
%token END 0 "end of file"
%token UPPER
%token LOWER
%token <sval> WORD
%token NEWLINE
%token CHAR
%token NUMBER
 
 
/* destructor rule for <sval> objects */
%destructor { if ($$) { delete ($$); ($$) = nullptr; } } <sval>
 
 
%%
 
list_option : END | list END;
 
list
: item
| list item
;
 
item
: UPPER { driver.add_upper(); }
| LOWER { driver.add_lower(); }
| WORD { driver.add_word( *$1 ); }
| NEWLINE { driver.add_newline(); }
| CHAR { driver.add_char(); }
| NUMBER { driver.add_number(); }
| REGISTER { driver.add_op_register(); }
;
 
%%
 
 
void
MIPP::MIPP_Parser::error( const std::string &err_message )
{
std::cerr << "Error: " << err_message << "\n";
}
 
 
/* include for access to scanner.yylex */
#include "mipp_scanner.hpp"
static int
yylex( MIPP::MIPP_Parser::semantic_type *yylval,
MIPP::MIPP_Scanner &scanner,
MIPP::MIPP_Driver &driver )
{
return( scanner.yylex(yylval) );
}
