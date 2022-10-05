#include<iostream>
#include<map>
#include<string>

using namespace std;

struct struct_opcode{
    string opcode;
    int format;
    char exists;
    struct_opcode(){
      opcode="undefined";
      format=0;
      exists='n';
    }
};
struct struct_literal{
    string value;
    string address;
    char exists;
    int blockNumber = 0;
    struct_literal(){
      value="";
      address="?";
      blockNumber = 0;
      exists='n';
    }
};
struct struct_label{
     string address;
     string name;
     int relative;
     int blockNumber;
     char exists;
     struct_label(){
       name="undefined";
       address="0";
       blockNumber = 0;
       exists='n';
       relative = 0;
     }
};
struct struct_blocks{
     string startAdd;
     string name;
     string LOCCTR;
     int number;
     char exists;
     struct_blocks(){
       name="undefined";
       startAdd="?";
       exists='n';
       number = -1;
       LOCCTR = "0";
     }
};


struct struct_register{
     char num;
     char exists;
     struct_register(){
       num = 'F';
       exists='n';
     }
};

typedef map<string,struct_label> SYMBOL_TABLE_TYPE;
typedef map<string,struct_opcode> OPCODE_TABLE_TYPE;
typedef map<string,struct_register> REG_TABLE_TYPE;
typedef map<string,struct_literal> LIT_TABLE_TYPE;
typedef map<string,struct_blocks> BLOCK_TABLE_TYPE;

SYMBOL_TABLE_TYPE SYMTAB;
OPCODE_TABLE_TYPE OP_table;
REG_TABLE_TYPE REG_table;
LIT_TABLE_TYPE Literals_Table;
BLOCK_TABLE_TYPE BLOCKS;

void load_REG_table(){
  REG_table["A"].num='0';
  REG_table["A"].exists='y';

  REG_table["X"].num='1';
  REG_table["X"].exists='y';

  REG_table["L"].num='2';
  REG_table["L"].exists='y';

  REG_table["B"].num='3';
  REG_table["B"].exists='y';

  REG_table["S"].num='4';
  REG_table["S"].exists='y';

  REG_table["T"].num='5';
  REG_table["T"].exists='y';

  REG_table["F"].num='6';
  REG_table["F"].exists='y';

  REG_table["PC"].num='8';
  REG_table["PC"].exists='y';

  REG_table["SW"].num='9';
  REG_table["SW"].exists='y';
}
void load_OP_table(){
  OP_table["ADD"].opcode="18";
  OP_table["ADD"].format=3;
  OP_table["ADD"].exists='y';

  OP_table["ADDF"].opcode="58";
  OP_table["ADDF"].format=3;
  OP_table["ADDF"].exists='y';

  OP_table["ADDR"].opcode="90";
  OP_table["ADDR"].format=2;
  OP_table["ADDR"].exists='y';

  OP_table["AND"].opcode="40";
  OP_table["AND"].format=3;
  OP_table["AND"].exists='y';

  OP_table["CLEAR"].opcode="B4";
  OP_table["CLEAR"].format=2;
  OP_table["CLEAR"].exists='y';

  OP_table["COMP"].opcode="28";
  OP_table["COMP"].format=3;
  OP_table["COMP"].exists='y';

  OP_table["COMPF"].opcode="88";
  OP_table["COMPF"].format=3;
  OP_table["COMPF"].exists='y';

  OP_table["COMPR"].opcode="A0";
  OP_table["COMPR"].format=2;
  OP_table["COMPR"].exists='y';

  OP_table["DIV"].opcode="24";
  OP_table["DIV"].format=3;
  OP_table["DIV"].exists='y';

  OP_table["DIVF"].opcode="64";
  OP_table["DIVF"].format=3;
  OP_table["DIVF"].exists='y';

  OP_table["DIVR"].opcode="9C";
  OP_table["DIVR"].format=2;
  OP_table["DIVR"].exists='y';

  OP_table["FIX"].opcode="C4";
  OP_table["FIX"].format=1;
  OP_table["FIX"].exists='y';

  OP_table["FLOAT"].opcode="C0";
  OP_table["FLOAT"].format=1;
  OP_table["FLOAT"].exists='y';

  OP_table["HIO"].opcode="F4";
  OP_table["HIO"].format=1;
  OP_table["HIO"].exists='y';

  OP_table["J"].opcode="3C";
  OP_table["J"].format=3;
  OP_table["J"].exists='y';

  OP_table["JEQ"].opcode="30";
  OP_table["JEQ"].format=3;
  OP_table["JEQ"].exists='y';

  OP_table["JGT"].opcode="34";
  OP_table["JGT"].format=3;
  OP_table["JGT"].exists='y';

  OP_table["JLT"].opcode="38";
  OP_table["JLT"].format=3;
  OP_table["JLT"].exists='y';

  OP_table["JSUB"].opcode="48";
  OP_table["JSUB"].format=3;
  OP_table["JSUB"].exists='y';

  OP_table["LDA"].opcode="00";
  OP_table["LDA"].format=3;
  OP_table["LDA"].exists='y';

  OP_table["LDB"].opcode="68";
  OP_table["LDB"].format=3;
  OP_table["LDB"].exists='y';

  OP_table["LDCH"].opcode="50";
  OP_table["LDCH"].format=3;
  OP_table["LDCH"].exists='y';

  OP_table["LDF"].opcode="70";
  OP_table["LDF"].format=3;
  OP_table["LDF"].exists='y';

  OP_table["LDL"].opcode="08";
  OP_table["LDL"].format=3;
  OP_table["LDL"].exists='y';

  OP_table["LDS"].opcode="6C";
  OP_table["LDS"].format=3;
  OP_table["LDS"].exists='y';

  OP_table["LDT"].opcode="74";
  OP_table["LDT"].format=3;
  OP_table["LDT"].exists='y';

  OP_table["LDX"].opcode="04";
  OP_table["LDX"].format=3;
  OP_table["LDX"].exists='y';

  OP_table["LPS"].opcode="D0";
  OP_table["LPS"].format=3;
  OP_table["LPS"].exists='y';

  OP_table["MUL"].opcode="20";
  OP_table["MUL"].format=3;
  OP_table["MUL"].exists='y';

  OP_table["MULF"].opcode="60";
  OP_table["MULF"].format=3;
  OP_table["MULF"].exists='y';

  OP_table["MULR"].opcode="98";
  OP_table["MULR"].format=2;
  OP_table["MULR"].exists='y';

  OP_table["NORM"].opcode="C8";
  OP_table["NORM"].format=1;
  OP_table["NORM"].exists='y';

  OP_table["OR"].opcode="44";
  OP_table["OR"].format=3;
  OP_table["OR"].exists='y';

  OP_table["RD"].opcode="D8";
  OP_table["RD"].format=3;
  OP_table["RD"].exists='y';

  OP_table["RMO"].opcode="AC";
  OP_table["RMO"].format=2;
  OP_table["RMO"].exists='y';

  OP_table["RSUB"].opcode="4F";
  OP_table["RSUB"].format=3;
  OP_table["RSUB"].exists='y';

  OP_table["SHIFTL"].opcode="A4";
  OP_table["SHIFTL"].format=2;
  OP_table["SHIFTL"].exists='y';

  OP_table["SHIFTR"].opcode="A8";
  OP_table["SHIFTR"].format=2;
  OP_table["SHIFTR"].exists='y';

  OP_table["SIO"].opcode="F0";
  OP_table["SIO"].format=1;
  OP_table["SIO"].exists='y';

  OP_table["SSK"].opcode="EC";
  OP_table["SSK"].format=3;
  OP_table["SSK"].exists='y';

  OP_table["STA"].opcode="0C";
  OP_table["STA"].format=3;
  OP_table["STA"].exists='y';

  OP_table["STB"].opcode="78";
  OP_table["STB"].format=3;
  OP_table["STB"].exists='y';

  OP_table["STCH"].opcode="54";
  OP_table["STCH"].format=3;
  OP_table["STCH"].exists='y';

  OP_table["STF"].opcode="80";
  OP_table["STF"].format=3;
  OP_table["STF"].exists='y';

  OP_table["STI"].opcode="D4";
  OP_table["STI"].format=3;
  OP_table["STI"].exists='y';

  OP_table["STL"].opcode="14";
  OP_table["STL"].format=3;
  OP_table["STL"].exists='y';

  OP_table["STS"].opcode="7C";
  OP_table["STS"].format=3;
  OP_table["STS"].exists='y';

  OP_table["STSW"].opcode="E8";
  OP_table["STSW"].format=3;
  OP_table["STSW"].exists='y';

  OP_table["STT"].opcode="84";
  OP_table["STT"].format=3;
  OP_table["STT"].exists='y';

  OP_table["STX"].opcode="10";
  OP_table["STX"].format=3;
  OP_table["STX"].exists='y';

  OP_table["SUB"].opcode="1C";
  OP_table["SUB"].format=3;
  OP_table["SUB"].exists='y';

  OP_table["SUBF"].opcode="5C";
  OP_table["SUBF"].format=3;
  OP_table["SUBF"].exists='y';

  OP_table["SUBR"].opcode="94";
  OP_table["SUBR"].format=2;
  OP_table["SUBR"].exists='y';

  OP_table["SVC"].opcode="B0";
  OP_table["SVC"].format=2;
  OP_table["SVC"].exists='y';

  OP_table["TD"].opcode="E0";
  OP_table["TD"].format=3;
  OP_table["TD"].exists='y';

  OP_table["TIO"].opcode="F8";
  OP_table["TIO"].format=1;
  OP_table["TIO"].exists='y';

  OP_table["TIX"].opcode="2C";
  OP_table["TIX"].format=3;
  OP_table["TIX"].exists='y';

  OP_table["TIXR"].opcode="B8";
  OP_table["TIXR"].format=2;
  OP_table["TIXR"].exists='y';

  OP_table["WD"].opcode="DC";
  OP_table["WD"].format=3;
  OP_table["WD"].exists='y';
}

void load_BLOCKS(){
  BLOCKS["DEFAULT"].exists = 'y';
  BLOCKS["DEFAULT"].name = "DEFAULT";
  BLOCKS["DEFAULT"].startAdd = "00000";
  BLOCKS["DEFAULT"].number=0;
  BLOCKS["DEFAULT"].LOCCTR = "0";
}

void load_tables(){
  load_BLOCKS();
  load_OP_table();
  load_REG_table();
}

