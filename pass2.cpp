#include "pass1.cpp"

using namespace std;

/*Declaring variables in global space*/
ifstream intermediateFile;
ofstream errorFile,objectFile,ListingFile;

ofstream printtab ;
string writestring ;

bool isComment;
string label,opcode,operand,comment;
string operand1,operand2;

int line_number,blockNumber,address,startAdd;
string objectCode, writeData, currentRecord, modificationRecord="M^", endRecord, write_R_Data, write_D_Data,currentSectName="DEFAULT";
int sectionCounter=0;
int program_section_length=0;

int program_counter, base_register_value, currentTextRecordLength;
bool nobase;
/*Declaration ends*/

string readTillTab(string data,int& index){
  string tempBuffer = "";

  while(index<data.length() && data[index] != '\t'){
    tempBuffer += data[index];
    index++;
  }
  index++;
  if(tempBuffer==" "){
    tempBuffer="-1" ;
  }
  return tempBuffer;
}
bool readIntermediateFile(ifstream& readFile,bool& isComment, int& line_number, int& address, int& blockNumber, string& label, string& opcode, string& operand, string& comment){
  string fileLine="";
  string tempBuffer="";
  bool tempStatus;
  int index=0;
  if(!getline(readFile, fileLine)){
    return false;
  }
  line_number = stoi(readTillTab(fileLine,index));

  isComment = (fileLine[index]=='.')?true:false;
  if(isComment){
    readFirstNonWhiteSpace(fileLine,index,tempStatus,comment,true);
    return true;
  }
  address = stringHexToInt(readTillTab(fileLine,index));
  tempBuffer = readTillTab(fileLine,index);
  if(tempBuffer == " "){
    blockNumber = -1;
  }
  else{
    blockNumber = stoi(tempBuffer);
  }
  label = readTillTab(fileLine,index);
  if(label=="-1"){
    label=" " ;
  }
  opcode = readTillTab(fileLine,index);
  if(opcode=="BYTE"){
    readByteOperand(fileLine,index,tempStatus,operand);
  }
  else{
    operand = readTillTab(fileLine,index);
    if(operand=="-1"){
      operand=" " ;
    }
    if(opcode=="CSECT"){
      return true ;
    }
  }
  readFirstNonWhiteSpace(fileLine,index,tempStatus,comment,true);  
  return true;
  
}

string createObjectCodeFormat34(){
  string object_code;
  int halfBytes;
  halfBytes = (getFlagFormat(opcode)=='+')?5:3;

  if(getFlagFormat(operand)=='#'){//Immediate
    if(operand.substr(operand.length()-2,2)==",X"){//Error handling for Immediate with index based
      writeData = "Line: "+to_string(line_number)+" Index based addressing not supported with Indirect addressing";
      writeToFile(errorFile,writeData);
      object_code = intToStringHex(stringHexToInt(OP_table[getRealOpcode(opcode)].opcode)+1,2);
      object_code += (halfBytes==5)?"100000":"0000";
      return object_code;
    }

    string temp_operand = operand.substr(1,operand.length()-1);
    if(if_all_num(temp_operand)||((SYMTAB[temp_operand].exists=='y')&&(SYMTAB[temp_operand].relative==0))){//Immediate integer value
      int immediateValue;

      if(if_all_num(temp_operand)){
        immediateValue = stoi(temp_operand);
      }
      else{
        immediateValue = stringHexToInt(SYMTAB[temp_operand].address);
      }
      /*Process Immediate value*/
      if(immediateValue>=(1<<4*halfBytes)){//Can't fit it
        writeData = "Line: "+to_string(line_number)+" Immediate value exceeds format limit";
        writeToFile(errorFile,writeData);
        object_code = intToStringHex(stringHexToInt(OP_table[getRealOpcode(opcode)].opcode)+1,2);
        object_code += (halfBytes==5)?"100000":"0000";
      }
      else{
        object_code = intToStringHex(stringHexToInt(OP_table[getRealOpcode(opcode)].opcode)+1,2);
        object_code += (halfBytes==5)?'1':'0';
        object_code += intToStringHex(immediateValue,halfBytes);
      }
      return object_code;
    }
    else if(SYMTAB[temp_operand].exists=='n') {
     
      if(halfBytes==3) { 
      writeData = "Line "+to_string(line_number);
     if(halfBytes==3){
         writeData+= " : Invalid format for external reference " + temp_operand; 
      } else{ 
         writeData += " : Symbol doesn't exists. Found " + temp_operand;
       } 
      writeToFile(errorFile,writeData);
      object_code = intToStringHex(stringHexToInt(OP_table[getRealOpcode(opcode)].opcode)+1,2);
      object_code += (halfBytes==5)?"100000":"0000";
      return object_code;
    }
   
    }
    else{
      int operandAddress = stringHexToInt(SYMTAB[temp_operand].address) + stringHexToInt(BLOCKS[BLocksNumToName[SYMTAB[temp_operand].blockNumber]].startAdd);

      /*Process Immediate symbol value*/
      if(halfBytes==5){/*If format 4*/
        object_code = intToStringHex(stringHexToInt(OP_table[getRealOpcode(opcode)].opcode)+1,2);
        object_code += '1';
        object_code += intToStringHex(operandAddress,halfBytes);

        /*add modifacation record here*/
        modificationRecord += "M^" + intToStringHex(address+1,6) + '^';
        modificationRecord += (halfBytes==5)?"05":"03";
        modificationRecord += '\n';

        return object_code;
      }

      /*Handle format 3*/
      program_counter = address + stringHexToInt(BLOCKS[BLocksNumToName[blockNumber]].startAdd);
      program_counter += (halfBytes==5)?4:3;
      int relativeAddress = operandAddress - program_counter;

      /*Try PC based*/
      if(relativeAddress>=(-2048) && relativeAddress<=2047){
        object_code = intToStringHex(stringHexToInt(OP_table[getRealOpcode(opcode)].opcode)+1,2);
        object_code += '2';
        object_code += intToStringHex(relativeAddress,halfBytes);
        return object_code;
      }

      /*Try base based*/
      if(!nobase){
        relativeAddress = operandAddress - base_register_value;
        if(relativeAddress>=0 && relativeAddress<=4095){
          object_code = intToStringHex(stringHexToInt(OP_table[getRealOpcode(opcode)].opcode)+1,2);
          object_code += '4';
          object_code += intToStringHex(relativeAddress,halfBytes);
          return object_code;
        }
      }

      /*Use direct addressing with no PC or base*/
      if(operandAddress<=4095){
        object_code = intToStringHex(stringHexToInt(OP_table[getRealOpcode(opcode)].opcode)+1,2);
        object_code += '0';
        object_code += intToStringHex(operandAddress,halfBytes);

        /*add modifacation record here*/
        modificationRecord += "M^" + intToStringHex(address+1+stringHexToInt(BLOCKS[BLocksNumToName[blockNumber]].startAdd),6) + '^';
        modificationRecord += (halfBytes==5)?"05":"03";
        modificationRecord += '\n';

        return object_code;
      }
    }
  }
  else if(getFlagFormat(operand)=='@'){
    string temp_operand = operand.substr(1,operand.length()-1);
    if(temp_operand.substr(temp_operand.length()-2,2)==",X" || SYMTAB[temp_operand].exists=='n'){//Error handling for Indirect with index based
  
      if(halfBytes==3) {
      writeData = "Line "+to_string(line_number);
    
      writeToFile(errorFile,writeData);
      object_code = intToStringHex(stringHexToInt(OP_table[getRealOpcode(opcode)].opcode)+2,2);
      object_code += (halfBytes==5)?"100000":"0000";
      return object_code;
    }
}
    int operandAddress = stringHexToInt(SYMTAB[temp_operand].address) + stringHexToInt(BLOCKS[BLocksNumToName[SYMTAB[temp_operand].blockNumber]].startAdd);
    program_counter = address + stringHexToInt(BLOCKS[BLocksNumToName[blockNumber]].startAdd);
    program_counter += (halfBytes==5)?4:3;

    if(halfBytes==3){
      int relativeAddress = operandAddress - program_counter;
      if(relativeAddress>=(-2048) && relativeAddress<=2047){
        object_code = intToStringHex(stringHexToInt(OP_table[getRealOpcode(opcode)].opcode)+2,2);
        object_code += '2';
        object_code += intToStringHex(relativeAddress,halfBytes);
        return object_code;
      }

      if(!nobase){
        relativeAddress = operandAddress - base_register_value;
        if(relativeAddress>=0 && relativeAddress<=4095){
          object_code = intToStringHex(stringHexToInt(OP_table[getRealOpcode(opcode)].opcode)+2,2);
          object_code += '4';
          object_code += intToStringHex(relativeAddress,halfBytes);
          return object_code;
        }
      }

      if(operandAddress<=4095){
        object_code = intToStringHex(stringHexToInt(OP_table[getRealOpcode(opcode)].opcode)+2,2);
        object_code += '0';
        object_code += intToStringHex(operandAddress,halfBytes);

        /*add modifacation record here*/
        modificationRecord += "M^" + intToStringHex(address+1+stringHexToInt(BLOCKS[BLocksNumToName[blockNumber]].startAdd),6) + '^';
        modificationRecord += (halfBytes==5)?"05":"03";
        modificationRecord += '\n';

        return object_code;
      }
    }
    else{//No base or pc based addressing in format 4
      object_code = intToStringHex(stringHexToInt(OP_table[getRealOpcode(opcode)].opcode)+2,2);
      object_code += '1';
      object_code += intToStringHex(operandAddress,halfBytes);

      /*add modifacation record here*/
      modificationRecord += "M^" + intToStringHex(address+1+stringHexToInt(BLOCKS[BLocksNumToName[blockNumber]].startAdd),6) + '^';
      modificationRecord += (halfBytes==5)?"05":"03";
      modificationRecord += '\n';

      return object_code;
    }

    writeData = "Line: "+to_string(line_number);
    writeData += "Can't fit into program counter based or base register based addressing.";
    writeToFile(errorFile,writeData);
    object_code = intToStringHex(stringHexToInt(OP_table[getRealOpcode(opcode)].opcode)+2,2);
    object_code += (halfBytes==5)?"100000":"0000";

    return object_code;
  }
  else if(getFlagFormat(operand)=='='){//Literals
    string temp_operand = operand.substr(1,operand.length()-1);

    if(temp_operand=="*"){
      temp_operand = "X'" + intToStringHex(address,6) + "'";
      /*Add modification record for value created by operand `*` */
      modificationRecord += "M^" + intToStringHex(stringHexToInt(Literals_Table[temp_operand].address)+stringHexToInt(BLOCKS[BLocksNumToName[Literals_Table[temp_operand].blockNumber]].startAdd),6) + '^';
      modificationRecord += intToStringHex(6,2);
      modificationRecord += '\n';
    }

    if(Literals_Table[temp_operand].exists=='n'){
      writeData = "Line "+to_string(line_number)+" : Symbol doesn't exists. Found " + temp_operand;
      writeToFile(errorFile,writeData);

      object_code = intToStringHex(stringHexToInt(OP_table[getRealOpcode(opcode)].opcode)+3,2);
      object_code += (halfBytes==5)?"000":"0";
      object_code += "000";
      return object_code;
    }

    int operandAddress = stringHexToInt(Literals_Table[temp_operand].address) + stringHexToInt(BLOCKS[BLocksNumToName[Literals_Table[temp_operand].blockNumber]].startAdd);
    program_counter = address + stringHexToInt(BLOCKS[BLocksNumToName[blockNumber]].startAdd);
    program_counter += (halfBytes==5)?4:3;

    if(halfBytes==3){
      int relativeAddress = operandAddress - program_counter;
      if(relativeAddress>=(-2048) && relativeAddress<=2047){
        object_code = intToStringHex(stringHexToInt(OP_table[getRealOpcode(opcode)].opcode)+3,2);
        object_code += '2';
        object_code += intToStringHex(relativeAddress,halfBytes);
        return object_code;
      }

      if(!nobase){
        relativeAddress = operandAddress - base_register_value;
        if(relativeAddress>=0 && relativeAddress<=4095){
          object_code = intToStringHex(stringHexToInt(OP_table[getRealOpcode(opcode)].opcode)+3,2);
          object_code += '4';
          object_code += intToStringHex(relativeAddress,halfBytes);
          return object_code;
        }
      }

      if(operandAddress<=4095){
        object_code = intToStringHex(stringHexToInt(OP_table[getRealOpcode(opcode)].opcode)+3,2);
        object_code += '0';
        object_code += intToStringHex(operandAddress,halfBytes);

        /*add modifacation record here*/
        modificationRecord += "M^" + intToStringHex(address+1+stringHexToInt(BLOCKS[BLocksNumToName[blockNumber]].startAdd),6) + '^';
        modificationRecord += (halfBytes==5)?"05":"03";
        modificationRecord += '\n';

        return object_code;
      }
    }
    else{//No base or pc based addressing in format 4
      object_code = intToStringHex(stringHexToInt(OP_table[getRealOpcode(opcode)].opcode)+3,2);
      object_code += '1';
      object_code += intToStringHex(operandAddress,halfBytes);

      /*add modifacation record here*/
      modificationRecord += "M^" + intToStringHex(address+1+stringHexToInt(BLOCKS[BLocksNumToName[blockNumber]].startAdd),6) + '^';
      modificationRecord += (halfBytes==5)?"05":"03";
      modificationRecord += '\n';

      return object_code;
    }

    writeData = "Line: "+to_string(line_number);
    writeData += "Can't fit into program counter based or base register based addressing.";
    writeToFile(errorFile,writeData);
    object_code = intToStringHex(stringHexToInt(OP_table[getRealOpcode(opcode)].opcode)+3,2);
    object_code += (halfBytes==5)?"100":"0";
    object_code += "000";

    return object_code;
  }
  else{/*Handle direct addressing*/
    int xbpe=0;
    string temp_operand = operand;
    if(operand.substr(operand.length()-2,2)==",X"){
      temp_operand = operand.substr(0,operand.length()-2);
      xbpe = 8;
    }


    if(SYMTAB[temp_operand].exists=='n') {
      /*****/
      if(halfBytes==3) { /*****/


      writeData = "Line "+to_string(line_number);
     
      writeToFile(errorFile,writeData);
      object_code = intToStringHex(stringHexToInt(OP_table[getRealOpcode(opcode)].opcode)+3,2);
      object_code += (halfBytes==5)?(intToStringHex(xbpe+1,1)+"00"):intToStringHex(xbpe,1);
      object_code += "000";
      return object_code;
    }

   
    }
else{

   int operandAddress = stringHexToInt(SYMTAB[temp_operand].address) + stringHexToInt(BLOCKS[BLocksNumToName[SYMTAB[temp_operand].blockNumber]].startAdd);
    program_counter = address + stringHexToInt(BLOCKS[BLocksNumToName[blockNumber]].startAdd);
    program_counter += (halfBytes==5)?4:3;

    if(halfBytes==3){
      int relativeAddress = operandAddress - program_counter;
      if(relativeAddress>=(-2048) && relativeAddress<=2047){
        object_code = intToStringHex(stringHexToInt(OP_table[getRealOpcode(opcode)].opcode)+3,2);
        object_code += intToStringHex(xbpe+2,1);
        object_code += intToStringHex(relativeAddress,halfBytes);
        return object_code;
      }

      if(!nobase){
        relativeAddress = operandAddress - base_register_value;
        if(relativeAddress>=0 && relativeAddress<=4095){
          object_code = intToStringHex(stringHexToInt(OP_table[getRealOpcode(opcode)].opcode)+3,2);
          object_code += intToStringHex(xbpe+4,1);
          object_code += intToStringHex(relativeAddress,halfBytes);
          return object_code;
        }
      }

      if(operandAddress<=4095){
        object_code = intToStringHex(stringHexToInt(OP_table[getRealOpcode(opcode)].opcode)+3,2);
        object_code += intToStringHex(xbpe,1);
        object_code += intToStringHex(operandAddress,halfBytes);

        /*add modifacation record here*/
        modificationRecord += "M^" + intToStringHex(address+1+stringHexToInt(BLOCKS[BLocksNumToName[blockNumber]].startAdd),6) + '^';
        modificationRecord += (halfBytes==5)?"05":"03";
        modificationRecord += '\n';

        return object_code;
      }
    }
    else{//No base or pc based addressing in format 4
      object_code = intToStringHex(stringHexToInt(OP_table[getRealOpcode(opcode)].opcode)+3,2);
      object_code += intToStringHex(xbpe+1,1);
      object_code += intToStringHex(operandAddress,halfBytes);

      /*add modifacation record here*/
      modificationRecord += "M^" + intToStringHex(address+1+stringHexToInt(BLOCKS[BLocksNumToName[blockNumber]].startAdd),6) + '^';
      modificationRecord += (halfBytes==5)?"05":"03";
      modificationRecord += '\n';

      return object_code;
    }

    writeData = "Line: "+to_string(line_number);
    writeData += "Can't fit into program counter based or base register based addressing.";
    writeToFile(errorFile,writeData);
    object_code = intToStringHex(stringHexToInt(OP_table[getRealOpcode(opcode)].opcode)+3,2);
    object_code += (halfBytes==5)?(intToStringHex(xbpe+1,1)+"00"):intToStringHex(xbpe,1);
    object_code += "000";

    return object_code;
  }}
  return "AB";
}

void writeTextRecord(bool lastRecord=false){
  if(lastRecord){
    if(currentRecord.length()>0){//Write last text record
      writeData = intToStringHex(currentRecord.length()/2,2) + '^' + currentRecord;
      writeToFile(objectFile,writeData);
      currentRecord = "";
    }
    return;
  }
  if(objectCode != ""){
    if(currentRecord.length()==0){
      writeData = "T^" + intToStringHex(address+stringHexToInt(BLOCKS[BLocksNumToName[blockNumber]].startAdd),6) + '^';
      writeToFile(objectFile,writeData,false);
    }
    //What is objectCode length > 60
    if((currentRecord + objectCode).length()>60){
      //Write current record
      writeData = intToStringHex(currentRecord.length()/2,2) + '^' + currentRecord;
      writeToFile(objectFile,writeData);

      //Initialize new text currentRecord
      currentRecord = "";
      writeData = "T^" + intToStringHex(address+stringHexToInt(BLOCKS[BLocksNumToName[blockNumber]].startAdd),6) + '^';
      writeToFile(objectFile,writeData,false);
    }

    currentRecord += objectCode;
  }
  else{
    /*Assembler directive which doesn't result in address genrenation*/
    if(opcode=="START"||opcode=="END"||opcode=="BASE"||opcode=="NOBASE"||opcode=="LTORG"||opcode=="ORG"||opcode=="EQU"){
      /*DO nothing*/
    }
    else{
      //Write current record if exists
      if(currentRecord.length()>0){
        writeData = intToStringHex(currentRecord.length()/2,2) + '^' + currentRecord;
        writeToFile(objectFile,writeData);
      }
      currentRecord = "";
    }
  }
}

void writeEndRecord(bool write=true){
  if(write){
    if(endRecord.length()>0){
      writeToFile(objectFile,endRecord);
     
    }
    else{
      writeEndRecord(false);
    }
  }
  if((operand==""||operand==" ")&&currentSectName=="DEFAULT"){//If no operand of END
    endRecord = "E^" + intToStringHex(startAdd,6);
  }else if(currentSectName!="DEFAULT"){
    endRecord = "E";
  }
  else{//Make operand on end firstExecutableAddress
    int firstExecutableAddress;
   
      firstExecutableAddress = stringHexToInt(SYMTAB[firstExecutable_Sec].address);
    
    endRecord = "E^" + intToStringHex(firstExecutableAddress,6)+"\n";
  }
  
}

void pass2(){
  string tempBuffer;
  intermediateFile.open("intermediate_"+fileName);//begin
  if(!intermediateFile){
    cout << "Unable to open file: intermediate_"<<fileName<<endl;
    exit(1);
  }
  getline(intermediateFile, tempBuffer); // Discard heading line

  objectFile.open("object_"+fileName);
  if(!objectFile){
    cout<<"Unable to open file: object_"<<fileName<<endl;
    exit(1);
  }

  ListingFile.open("listing_"+fileName);
  if(!ListingFile){
    cout<<"Unable to open file: listing_"<<fileName<<endl;
    exit(1);
  }
  writeToFile(ListingFile,"Line\tAddress\tLabel\tOPCODE\tOPERAND\tObjectCode\tComment");

  errorFile.open("error_"+fileName,fstream::app);
  if(!errorFile){
    cout<<"Unable to open file: error_"<<fileName<<endl;
    exit(1);
  }
  writeToFile(errorFile,"\n\n************PASS2************");
   /*Intialize global variables*/
  objectCode = "";
  currentTextRecordLength=0;
  currentRecord = "";
  modificationRecord = "";
  blockNumber = 0;
  nobase = true;

  readIntermediateFile(intermediateFile,isComment,line_number,address,blockNumber,label,opcode,operand,comment);
  while(isComment){//Handle with previous comments
    writeData = to_string(line_number) + "\t" + comment;
    writeToFile(ListingFile,writeData);
    readIntermediateFile(intermediateFile,isComment,line_number,address,blockNumber,label,opcode,operand,comment);
  }

  if(opcode=="START"){
    startAdd = address;
    writeData = to_string(line_number) + "\t" + intToStringHex(address) + "\t" + to_string(blockNumber) + "\t" + label + "\t" + opcode + "\t" + operand + "\t" + objectCode +"\t" + comment;
    writeToFile(ListingFile,writeData);
  }
  else{
    label = "";
    startAdd = 0;
    address = 0;
  }
    program_section_length = program_length ;
  writeData = "H^"+expandString(label,6,' ',true)+'^'+intToStringHex(address,6)+'^'+intToStringHex(program_section_length,6);
  writeToFile(objectFile,writeData);
 
  readIntermediateFile(intermediateFile,isComment,line_number,address,blockNumber,label,opcode,operand,comment);
  currentTextRecordLength  = 0;

  while(opcode!="END"){
      while(opcode!="END"){
    if(!isComment){
      if(OP_table[getRealOpcode(opcode)].exists=='y'){
        if(OP_table[getRealOpcode(opcode)].format==1){
          objectCode = OP_table[getRealOpcode(opcode)].opcode;
        }
        else if(OP_table[getRealOpcode(opcode)].format==2){
          operand1 = operand.substr(0,operand.find(','));
          operand2 = operand.substr(operand.find(',')+1,operand.length()-operand.find(',') -1 );

          if(operand2==operand){//If not two operand i.e. a
            if(getRealOpcode(opcode)=="SVC"){
              objectCode = OP_table[getRealOpcode(opcode)].opcode + intToStringHex(stoi(operand1),1) + '0';
            }
            else if(REG_table[operand1].exists=='y'){
              objectCode = OP_table[getRealOpcode(opcode)].opcode + REG_table[operand1].num + '0';
            }
            else{
              objectCode = getRealOpcode(opcode) + '0' + '0';
              writeData = "Line: "+to_string(line_number)+" Invalid Register name";
              writeToFile(errorFile,writeData);
            }
          }
          else{//Two operands i.e. a,b
            if(REG_table[operand1].exists=='n'){
              objectCode = OP_table[getRealOpcode(opcode)].opcode + "00";
              writeData = "Line: "+to_string(line_number)+" Invalid Register name";
              writeToFile(errorFile,writeData);
            }
            else if(getRealOpcode(opcode)=="SHIFTR" || getRealOpcode(opcode)=="SHIFTL"){
              objectCode = OP_table[getRealOpcode(opcode)].opcode + REG_table[operand1].num + intToStringHex(stoi(operand2),1);
            }
            else if(REG_table[operand2].exists=='n'){
              objectCode = OP_table[getRealOpcode(opcode)].opcode + "00";
              writeData = "Line: "+to_string(line_number)+" Invalid Register name";
              writeToFile(errorFile,writeData);
            }
            else{
              objectCode = OP_table[getRealOpcode(opcode)].opcode + REG_table[operand1].num + REG_table[operand2].num;
            }
          }
        }
        else if(OP_table[getRealOpcode(opcode)].format==3){
          if(getRealOpcode(opcode)=="RSUB"){
            objectCode = OP_table[getRealOpcode(opcode)].opcode;
            objectCode += (getFlagFormat(opcode)=='+')?"000000":"0000";
          }
          else{
            objectCode = createObjectCodeFormat34();
          }
        }
      }//If opcode in OP_table
      else if(opcode=="BYTE"){
        if(operand[0]=='X'){
          objectCode = operand.substr(2,operand.length()-3);
        }
        else if(operand[0]=='C'){
          objectCode = stringToHexString(operand.substr(2,operand.length()-3));
        }
      }
      else if(label=="*"){
        if(opcode[1]=='C'){
          objectCode = stringToHexString(opcode.substr(3,opcode.length()-4));
        }
        else if(opcode[1]=='X'){
          objectCode = opcode.substr(3,opcode.length()-4);
        }
      }
      else if(opcode=="WORD"){
        objectCode = intToStringHex(stoi(operand),6);
      }
      else if(opcode=="BASE"){
        if(SYMTAB[operand].exists=='y'){
          base_register_value = stringHexToInt(SYMTAB[operand].address) + stringHexToInt(BLOCKS[BLocksNumToName[SYMTAB[operand].blockNumber]].startAdd);
          nobase = false;
        }
        else{
          writeData = "Line "+to_string(line_number)+" : Symbol doesn't exists. Found " + operand;
          writeToFile(errorFile,writeData);
        }
        objectCode = "";
      }
      else if(opcode=="NOBASE"){
        if(nobase){//check if assembler was using base addressing
          writeData = "Line "+to_string(line_number)+": Assembler wasn't using base addressing";
          writeToFile(errorFile,writeData);
        }
        else{
          nobase = true;
        }
        objectCode = "";
      }
      else{
        objectCode = "";
      }
      //Write to text record if any generated
      writeTextRecord();

      if(blockNumber==-1 && address!=-1){
        writeData = to_string(line_number) + "\t" + intToStringHex(address) + "\t" + " " + "\t" + label + "\t" + opcode + "\t" + operand + "\t" + objectCode +"\t" + comment;
      }
      else if(address==-1){

        writeData = to_string(line_number) + "\t" + " " + "\t" + " " + "\t" + label + "\t" + opcode + "\t" + operand + "\t" + objectCode +"\t" + comment;
      } 
     
      else{ writeData = to_string(line_number) + "\t" + intToStringHex(address) + "\t" + to_string(blockNumber) + "\t" + label + "\t" + opcode + "\t" + operand + "\t" + objectCode +"\t" + comment;
      }
    }//if not comment
    else{
      writeData = to_string(line_number) + "\t" + comment;
    }
    writeToFile(ListingFile,writeData);//Write listing line
    readIntermediateFile(intermediateFile,isComment,line_number,address,blockNumber,label,opcode,operand,comment);//Read next line
    objectCode = "";
  }//while opcode not end

  writeTextRecord();

  //Save end record
  writeEndRecord(false);
}
  
if(opcode!="CSECT"){
  while(readIntermediateFile(intermediateFile,isComment,line_number,address,blockNumber,label,opcode,operand,comment)){
    if(label=="*"){
      if(opcode[1]=='C'){
        objectCode = stringToHexString(opcode.substr(3,opcode.length()-4));
      }
      else if(opcode[1]=='X'){
        objectCode = opcode.substr(3,opcode.length()-4);
      }
      writeTextRecord();
    }
    writeData = to_string(line_number) + "\t" + intToStringHex(address) + "\t" + to_string(blockNumber) + label + "\t" + opcode + "\t" + operand + "\t" + objectCode +"\t" + comment;
    writeToFile(ListingFile,writeData);
  }
}   
  
writeTextRecord(true);
if(!isComment){
  
  writeToFile(objectFile,modificationRecord,false);//Write modification record
  writeEndRecord(true);//Write end record
  currentSectName=label;
  modificationRecord="";
}

  readIntermediateFile(intermediateFile,isComment,line_number,address,blockNumber,label,opcode,operand,comment);//Read next line
    objectCode = "";

}
//Function end

int main(){
  cout<<"Enter input file Name:";
  cin>>fileName;

  cout<<"\nLoading OP_table"<<endl;
  load_tables();

  cout<<"\nExecuting PASS1 file"<<endl;
  cout<<"Creating new file and Writing  the intermediate file to 'intermediate"<<fileName<<"'"<<endl;
  cout<<"Creating new file and Writing error file to 'error"<<fileName<<"'"<<endl;
  pass1();

cout<<"Creating the SYMBOL TABLE"<<endl;
  printtab.open("tables_"+fileName) ;
  writeToFile(printtab,"**********************************SYMBOL TABLE*****************************\n") ;
    for (auto const& it: SYMTAB) { 
        writestring+=it.first+":-\t"+ "name:"+it.second.name+"\t|"+ "address:"+it.second.address+"\t|"+ "relative:"+intToStringHex(it.second.relative)+" \n" ;
    } 
    writeToFile(printtab,writestring) ;

writestring="" ;
    cout<<"Creating the new  LITERAL TABLE"<<endl;
  
  writeToFile(printtab,"**********************************LITERAL TABLE*****************************\n") ;
    for (auto const& it: Literals_Table) { 
        writestring+=it.first+":-\t"+ "value:"+it.second.value+"\t|"+ "address:"+it.second.address+" \n" ;
    } 
    writeToFile(printtab,writestring) ;
  cout<<"\nExecuting the PASS2 file"<<endl;
  cout<<"Creating new file and Writing object file to 'object"<<fileName<<"'"<<endl;
  cout<<" Creating new file and Writing listing file to 'listing"<<fileName<<"'"<<endl;
  pass2();
}