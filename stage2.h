// Walter King and William Martin
// CS 4301
// Stage 1

#include <ctime>
#include <fstream>
#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cstdlib>
using namespace std;

const int MAX_SYMBOL_TABLE_SIZE = 256;
const int NUM_KEYWORDS = 23;
const int NUM_SPECCHAR = 12;
const uint MAX_NAME_SIZE = 15;
enum storeType {INTEGER, BOOLEAN, PROG_NAME, UNKNOWN};
enum allocation {YES,NO};
enum modes {VARIABLE, CONSTANT};
char specChar[] = {'=',':',',',';','.','+','-','*','(',')','<','>'};
string keyWords[] = {"program", "const", "var", "integer", "boolean", "begin", "end", "true", "false", "not", "mod", "div", "and", "or", "read", "write", "if", "then", "else", "repeat", "while", "do", "until"};
struct entry //define symbol table entry format
{
	string internalName;
	string externalName;
	storeType dataType;
	modes mode;
	string value;
	allocation alloc;
	int units;
};
vector<entry> symbolTable;
ifstream sourceFile;
ofstream listingFile, objectFile; 
string token;
char charac;
const char END_OF_FILE = '$'; // arbitrary choice
int intCounter;
int boolCounter;
int errorCounter;
bool newLine = true;
int countLine = 0;
int countError = 0;
int countTemp = 0;
int labelCount = 0;
int currentTempNo = -1;
int maxTempNo = -1;
vector<string> operandStk;
vector<string> operatorStk;
string AReg;
int beginEndCount = 0;

void CreateListingHeader();
void Parser();
void CreateListingTrailer();
void PrintSymbolTable();
void Prog();
void ProgStmt();
void Consts();
void Vars();
void BeginEndStmt();
void ConstStmts();
void VarStmts();
string Ids();
void Insert(string externalName,storeType inType, modes inMode, string inValue, allocation inAlloc, int inUnits);
storeType WhichType(string name);
string WhichValue(string name);
string NextToken();
char NextChar();
bool isNonKeyId(string str);
bool isSpecChar(char str);
bool isInteger(string str);
bool multiplyDefined(string str);
bool isKeyWord(string str);
bool isLit(string str);
storeType getDataType(string str);
string getValue(string str);
string GenInternalName(storeType s);
bool hasValue(string str);
void processError(string error);
string getStoreType(storeType s);
string getAllocation(allocation a);
string getModes(modes m);
void PushOperator(string str);
void PushOperand(string str);
string PopOperator();
string PopOperand();
void isInTable(string str);
void ExpressStmt();
void ExpressesStmt();
void TermStmt();
void TermsStmt();
void FactorStmt();
void FactorsStmt();
void PartStmt();
void ExecStmt();
void AssignStmt();
void ReadStmt();
void ReadList();
void WriteStmt(); 
void WriteList();
void IfStmt();
void ElsePt();
void WhileStmt();
void RepeatStmt();
void NullStmt();
bool isAdLvlOp(string str);
bool isBoolean(string str);
bool isRelOp(string str);
bool isMultLvlOp(string str);
void validateSpecChar(char c);
void Code(string op, string operand1 = "", string operand2 = "");
void EmitStartCode();
void EmitEndCode();
void EmitReadCode(string operand1);
void EmitWriteCode(string operand1);
void EmitAdditionCode(string operand1, string operand2);
void EmitSubtractionCode(string operand1, string operand2);
void EmitNegCode(string operand1);
void EmitNotCode(string operand1);
void EmitMultiplicationCode(string operand1,string operand2);
void EmitDivisionCode(string operand1, string operand2);
void EmitModCode(string operand1, string operand2);
void EmitAndCode(string operand1, string operand2);
void EmitOrCode(string operand1, string operand2);
void EmitLTCode(string operand1, string operand2);
void EmitGTCode(string operand1, string operand2);
void EmitLTECode(string operand1, string operand2);
void EmitGTECode(string operand1, string operand2);
void EmitNETCode(string operand1, string operand2);
void EmitEqualsCode(string operand1, string operand2);
void EmitAssignCode(string operand1, string operand2);
void writeToObject(string label = "", string instruct= "", string arg= "", string signVal= "", string comment= "");
storeType getTableEntryType(string str);
modes getTableEntryMode(string str);
string getTableEntryInName(string str);
string getTableEntryExtName(string str);
string getTableEntryValue(string str);
string GetTemp();
void FreeTemp();
bool isTemp(string s);
void setDataType(string s, storeType t);
entry& genTableEntry(string s);
entry& genExtTableEntry(string s);
string getLabel();
string getFalse();
string getTrue();
string getZero();
void EmitThenCode(string operand);
void EmitElseCode(string operand);
void EmitPostIfCode(string operand);
void EmitWhileCode();
void EmitDoCode(string operand);
void EmitPostWhileCode(string operand1, string operand2);
void EmitRepeatCode();
void EmitUntilCode(string operand1, string operand2);
void PutBackToken(string str);
//delete me
void debug(string s);
//delete me

int main(int argc, char **argv)
{
	//this program is the stage0 compiler for Pascallite. It will accept
	//input from argv[1], generating a listing to argv[2], and object code to
	//argv[3]
	sourceFile.open(argv[1]);
	listingFile.open(argv[2]);
	objectFile.open(argv[3]);
	CreateListingHeader();
	Parser();
	CreateListingTrailer();
	return 0;
}

void CreateListingHeader()
{
	time_t now = time (NULL);
	listingFile << "STAGE1:  WALTER KING, WILLIAM MARTIN       " << ctime(&now) << endl;
	listingFile << "LINE NO.              " << "SOURCE STATEMENT" << endl << endl;
	//line numbers and source statements should be aligned under the headings
}

void Parser()
{
	NextChar(); 
	//charac must be initialized to the first character of the source file 
	if(NextToken() != "program")
		processError(" keyword \"program\" expected");
		//a call to NextToken() has two effects
		// (1) the variable, token, is assigned the value of the next token
		// (2) the next token is read from the source file in order to make
		// the assignment. The value returned by NextToken() is also 
		// the next token.
	Prog();
	//parser implements the grammar rules, calling first rule
}
void CreateListingTrailer()
{
	listingFile << "\nCOMPILATION TERMINATED      "<< countError <<" ERRORS ENCOUNTERED" << endl;
} 

void PrintSymbolTable()
{
	for(uint i = 0; i < symbolTable.size(); ++i){
		if(symbolTable[i].alloc == YES)
		{
			stringstream sstemp;
			if(symbolTable[i].value[0] == '-')
				sstemp << '-' << right << setfill('0') << setw(3) << symbolTable[i].value.substr(1);
			else
				sstemp << right << setfill('0') << setw(4) << symbolTable[i].value;	
			if(symbolTable[i].mode == CONSTANT)
				writeToObject(symbolTable[i].internalName,"DEC",sstemp.str(),"",symbolTable[i].externalName);
			else
				writeToObject(symbolTable[i].internalName,"BSS","0001","",symbolTable[i].externalName);
			
		}
	}
}

void Prog() //token should be "program"
{ 
	if (token != "program")
		processError( " keyword \"program\" expected");
	ProgStmt();
	if (token == "const") Consts();
	if (token == "var") Vars();
	if (token != "begin")
		processError(" keyword \"begin\" expected");
	BeginEndStmt();
	if (token[0] != END_OF_FILE)
		processError(" no text may follow \".\"");
}

void ProgStmt() //token should be "program"
{ 
	string x;
	if (token != "program")
		processError( " keyword \"program\" expected");
	x = NextToken();
	if (!isNonKeyId(token))
		processError( " program name expected");
	if (NextToken() != ";")
		processError( " semicolon expected");
	NextToken();
	Insert(x,PROG_NAME,CONSTANT,x,NO,0);
    Code("begin");
}

void Consts() //token should be "const"
{ 
	if (token != "const")
		processError( " keyword \"const\" expected");
	if (!isNonKeyId(NextToken()))
		processError( " non-keyword identifier must follow \"const\"");
	ConstStmts();
}

void Vars() //token should be "var"
{ 
	if (token != "var")
		processError( " keyword \"var\" expected");
	if (!isNonKeyId(NextToken()))
		processError( " non-keyword identifier must follow \"var\"");
	VarStmts();
}

void ConstStmts() //token should be NON_KEY_ID
{ 
	string x,y;
	if (!isNonKeyId(token))
		processError( " non-keyword identifier expected");
	x = token;
	if (NextToken() != "=")
		processError( " \"=\" expected");
	y = NextToken();
	if (y != "+" && y != "-" && y != "not" && !isNonKeyId(y) && y != "true" && y != "false" && !isInteger(y))
		processError( " token to right of \"=\" illegal");
	if (y == "+" || y == "-")
	{
		if(!isInteger(NextToken()))
			processError( " integer expected after sign");
		y = y + token;
	}
	if (y == "not")
	{
		NextToken();
		if (isNonKeyId(token)) 
		{
			if (!multiplyDefined(token)) 
			{
				processError(" undefined symbol " + token);
			}
		}
		else
		{
			if(!isBoolean(token))
				processError(" boolean expected after not");
		}
			
		if(token == "true")
			y = "false";
		else
			y = "true";
	}
	if (NextToken() != ";")
		processError( " semicolon expected");
	Insert(x,WhichType(y),CONSTANT,WhichValue(y),YES,1);
	if (NextToken() != "begin" && token != "var" && !isNonKeyId(token))
		processError( " non-keyword identifier,\"begin\", or \"var\" expected");
	if (isNonKeyId(token))
		ConstStmts();
}

void VarStmts() //token should be NON_KEY_ID
{ 
	string x,y,temp;
	storeType varType;
	temp = "";
	if (!isNonKeyId(token))
		processError( " non-keyword identifier expected");
	x = Ids();
	if (token != ":")
		processError( " \":\" expected");
	if(NextToken() != "integer" && token != "boolean")
		processError( " illegal type follows \":\"");
	y = token;
	if(y == "boolean")
		varType = BOOLEAN;
	else if(y == "integer")
		varType = INTEGER;
	if(NextToken() != ";")
		processError( " semicolon expected");
	for(uint i = 0; i < x.size(); ++i)
	{
		if(x[i] == ',')
		{
			Insert(temp,varType,VARIABLE,"",YES,1);
			temp = "";
		}
		else
			temp += x[i];
	}	
	Insert(temp,varType,VARIABLE,"",YES,1);
	if (NextToken() != "begin" && !isNonKeyId(token))
		processError( " non-keyword identifier or \"begin\" expected");
	if (isNonKeyId(token))
		VarStmts();
}

void BeginEndStmt() //token should be "begin"
{ 
	if (token != "begin")
		processError( " keyword \"begin\" expected");
	++beginEndCount;
	while(NextToken() != "end")
		ExecStmt();		
	if (token != "end")
		processError( " keyword \"end\" expected");
	NextToken();
	--beginEndCount;
	if (!(token == "." || token == ";"))
		processError( " period or semi-colon expected");
/*    if(token == ";")
        NextToken();*/
    if(token == "." && beginEndCount == 0){
        Code("end");
        NextToken();
    }
    else if(token == "." && beginEndCount != 0)
        processError( " improper balance between begin and end");
	
}

void ExecStmt()
{
    debug(token + "in EXECSTMT()");
	if (isNonKeyId(token))
	{
		if(multiplyDefined(token))
			AssignStmt();
		else
			processError(" undefined variable");
	}
	else if (token == "read")
		ReadStmt();
	else if (token == "write")
		WriteStmt();
	else if (token == "if")
		IfStmt();
	else if (token == "while")
		WhileStmt();
	else if (token == "repeat")
		RepeatStmt();
	else if (token == "begin")
		BeginEndStmt();
	else if (token == ";")
		NullStmt();		
	else
		processError(" non-keyword id or read or write expected");
}

void AssignStmt() 
{
	if (!isNonKeyId(token))
		processError( " non-keyword identifier expected");
	PushOperand(token);//first operand
	if (NextToken() != ":=")
		processError( " \":=\" expected");
	PushOperator(token);
	NextToken();
	ExpressStmt();
	if(token != ";")
		processError(" Expected ';'");
	Code(PopOperator(),PopOperand(),PopOperand());
}

void ExpressStmt()
{
	TermStmt();
	ExpressesStmt();
}

void ExpressesStmt()
{
	if(!isRelOp(token))
		return;
	else
	{
		PushOperator(token);
		NextToken();
		TermStmt();
		Code(PopOperator(), PopOperand(),PopOperand());
	}
}

void TermStmt()
{
	FactorStmt();
	TermsStmt();
}

void TermsStmt()
{
	if(!isAdLvlOp(token))
		return;
	while(isAdLvlOp(token))
	{
		PushOperator(token);
		NextToken();
		FactorStmt();
		Code(PopOperator(), PopOperand(),PopOperand());
	}
}

void FactorStmt()
{
	PartStmt();
	FactorsStmt();
}

void FactorsStmt()
{
	if(!isMultLvlOp(token))
		return;
	while(isMultLvlOp(token))
	{
		PushOperator(token);
		NextToken();
		PartStmt();
		Code(PopOperator(), PopOperand(),PopOperand());
	}
}

void PartStmt()
{
	string temp = token;
	if(token == "+" || token == "-")
	{
		if (token == "+")
			PushOperator("pos");
		else
			PushOperator("neg");
		NextToken();
		if(token == "(")
		{
			NextToken();
			ExpressStmt();
			if(token != ")")
				processError(" expected \")\"");
			Code(PopOperator(),PopOperand());
			NextToken();
		}
		else if(isInteger(token))
		{
			PopOperator();
			if(temp == "+")
			{
				PushOperand(token);
				NextToken();
			}
			else
			{
				PushOperand("-"+token);
				NextToken();
			}
		}
		else if(isNonKeyId(token))
		{
			PushOperand(token);
			Code(PopOperator(),PopOperand());
			NextToken();
		}
		else
			processError(" Expected '(', INTEGER, or NON-KEY-ID");
	}
	else if(token == "(")
	{
		NextToken();
		ExpressStmt();
		if(token != ")")
			processError(" expected \")\"");
		NextToken();
	}
	else if(isNonKeyId(token) || isInteger(token))
	{
		PushOperand(token);
		NextToken();
	}
	else if(token == "not")
	{
		PushOperator(token);
		NextToken();
		if(token == "(")
		{
			NextToken();
			ExpressStmt();
			if(token != ")")
				processError(" expected \")\"");
			NextToken();
			Code(PopOperator(),PopOperand());
		}
		else if(isNonKeyId(token))
		{
			PushOperand(token);
			NextToken();	
			Code(PopOperator(),PopOperand());			
		}
		
		else if (isBoolean(token))
		{
			PopOperator();
			//("whatiship");
			PushOperand("not " + token);
			NextToken();
		}
	}
	else if(isBoolean(token))
	{
		PushOperand(token);
		NextToken();
	}
	else
		processError(" expected 'not', 'true', 'false', '(', '+', '-', integer, or non-keyword id; found " + token);
}

void ReadStmt()
{
	if(token != "read")
		processError(" expected keyword \"read\"");
	ReadList();
	if(token != ";")
		processError(" expected \";\"");
}

void ReadList()
{
	string x,temp;
	if(NextToken() != "(")
		processError(" expected \"(\"");
	NextToken();
	x = Ids();
	temp = "";
	for(uint i = 0; i < x.size(); ++i)
	{
		if(x[i] == ',')
		{
			Code("read",temp);
			temp = "";
		}
		else
			temp += x[i];
	}
	Code("read",temp);
	if(token != ")")
		processError(" expected \")\"");
	NextToken();
}

void WriteStmt()
{

	if(token != "write")
		processError(" expected keyword \"write\"");
	WriteList();
	if(token != ";")
		processError(" expected \";\"");
}

void WriteList()
{
	string x,temp;
	if(NextToken() != "(")
		processError(" expected \"(\"");
	NextToken();
	x = Ids();
	temp = "";
	for(uint i = 0; i < x.size(); ++i)
	{
		if(x[i] == ',')
		{
			Code("write",temp);
			temp = "";
		}
		else
			temp += x[i];
	}
	Code("write",temp);
	if(token != ")")
		processError(" expected \")\"");
	NextToken();
}

void IfStmt()
{
    string x;
	if (token != "if")
		processError( " keyword \"if\" expected");
    debug(token + " in IfStmt() --1--");
	NextToken();
	ExpressStmt();
    debug(token + " in IfStmt() --2--");
	if (token != "then")
		processError( " keyword \"then\" expected");
	Code("then", PopOperand());
	NextToken();
	ExecStmt();
    debug(token + " in IfStmt() --3--");
    x = token;
    if(NextToken() == "else")
        ElsePt();
    else
    {
        x = x + "" + token; 
        PutBackToken(x);
    }
    Code("postIf",PopOperand());
    debug(x);
    debug(token + " in IfStmt() --4--");
}

void ElsePt()
{
    debug(token + " in ElsePt() --1--");
	Code("else",PopOperand());
	NextToken();
	ExecStmt();
    debug(token + " in ElsePt() --2--");
}

void WhileStmt()
{
	if (token != "while")
		processError( " keyword \"while\" expected");
	Code("while");
    NextToken();
	ExpressStmt();
	if (token != "do")
		processError( " keyword \"do\" expected");
	Code("do",PopOperand());
    NextToken();
	ExecStmt();
	Code("postWhile",PopOperand(),PopOperand());
}

void RepeatStmt()
{
	if (token != "repeat")
		processError( " keyword \"repeat\" expected");
	Code("repeat");
    NextToken();
	ExecStmt();
    //debug(token + "in repeatStmt()");
	if (NextToken() != "until")
		processError( " keyword \"until\" expected");
    NextToken();
	ExpressStmt();
	Code("until",PopOperand(),PopOperand());
}

void NullStmt()
{
    //debug("in NullStmt()");
		return;
}

string Ids() //token should be NON_KEY_ID
{ 
	string temp,tempString;
	if (!isNonKeyId(token))
		processError( " non-keyword identifier expected");
	tempString = token;
	temp = token;
	if(NextToken() == ",")
	{
		if (!isNonKeyId(NextToken()))
		processError( " non-keyword identifier expected");
		tempString = temp + "," + Ids();
	}
	return tempString;
}

void Insert(string externalName, storeType inType, modes inMode, string inValue,
allocation inAlloc, int inUnits)
//create symbol table entry for each identifier in list of external names
//Multiply inserted names are illegal
{
	if (externalName.size() > MAX_NAME_SIZE)
		externalName = externalName.substr(0,MAX_NAME_SIZE);
	string name = externalName;
		if(multiplyDefined(name))
			processError(" multiple name definition");
		if(isKeyWord(name) && name != "true" && name != "false")
			processError(" illegal use of keyword");
		if(symbolTable.size() >= 256)
			processError(" table entry exceeds max allowable entries");
		else //create table entry
		{
			if(isupper(name[0])) 
			{
				entry e = {name.substr(0,4), name, inType, inMode, inValue, inAlloc, inUnits};
				symbolTable.push_back(e);
			}
			else
			{
				entry e = {GenInternalName(inType), name, inType, inMode, inValue, inAlloc, inUnits};
				symbolTable.push_back(e);
			}
		}
}

storeType WhichType(string name) //tells which data type a name has
{ 
	storeType dataType;
	if(isLit(name))
	{	
		if(name == "true")
			dataType = BOOLEAN;
		else if(name == "false")
			dataType = BOOLEAN;
		else if(name == "not true")
			dataType = BOOLEAN;
		else if(name == "not false")
			dataType = BOOLEAN;	
		else 
			dataType = INTEGER;
	}
	else //name is an identifier and hopefully a constant
	{
		if(multiplyDefined(name)) 
			dataType = getDataType(name);
		else 
			processError(" reference to undefined constant");
	}
	return dataType;
}

string WhichValue(string name) //tells which value a name has
{ 
	string value = "";
	if(isLit(name))
	{
		value = name;
		if (name[name.size()-1] == 'e')
			if(name == "true")
				value = "1";
			else if(name == "not true")
				value = "0";
			else if(name == "not false")
				value = "1";
			else
				value = "0";
	}
	else //name is an identifier and hopefully a constant
	{	
		if(multiplyDefined(name) && hasValue(name))
			value = getValue(name);
		else
			processError(" reference to undefined constant");
	}
	return value;
}

string NextToken() //returns the next token or end of file marker
{
	token = "";
	while (token == "")
	{
		if(charac == '{')  //process comment
		{
			while (NextChar() != END_OF_FILE && charac != '}');
			if (charac==END_OF_FILE)
				processError(" unexpected end of file");
			else
				NextChar();
		}
		else if(charac == '}')
		{		
			processError(" '}' cannot begin token");
		}
		else if(isspace(charac))
		{
			NextChar();
		}
		else if(isSpecChar(charac))
		{			
			token = charac;
			NextChar();
			if(token == ":" || token == "<" || token == ">")
				validateSpecChar(charac);
		}
		else if (isalpha(charac) && islower(charac))
		{
			token = charac;
			while ((isalpha(NextChar()) && islower(charac)) || isdigit(charac) ||charac == '_')		
				token += charac;
			if(token[token.size() - 1] == '_')
				processError(" token cannot end with _");
		}
		else if(isdigit(charac))
		{
			token = charac;
			while (isdigit(NextChar()))
				token+=charac;
		}
		else if(charac == END_OF_FILE)
			token = charac;
		else
			processError(" illegal symbol");
	}
	return token;
}

char NextChar() //returns the next character or end of file marker
{
	char c;
	sourceFile.get(c);
	if(sourceFile.eof()) //end of file 
	{
		charac = END_OF_FILE; //use a special character to designate end of file
		return charac;
	}
	else
		charac = c;
	if(newLine)
	{
		newLine = false;
		listingFile << right << setw(5) << ++countLine << "|";
	}	
	if (charac == '\n')
		newLine = true;
	
	//print to listing file (starting new line if necessary)
	listingFile << charac;
	return charac;
}

bool isNonKeyId(string str)
{
	for(int i = 0; i < NUM_KEYWORDS; ++i)
	{
		if(str == keyWords[i])
			return false;
	}

	if(!(isalpha(str[0]) && islower(str[0])))
		return false;
			
	for(uint i=0; i < str.size(); ++i)
	{	
		if(!((isalpha(str[i]) && islower(str[i])) || isdigit(str[i]) || str[i] == '_'))
			return false;
	}
	if(str[str.size() - 1] == '_')
	{
		processError( " token cannot end with _");
		return false;
	}
	return true;
}

bool isSpecChar(char c)
{
	for(int i = 0; i < NUM_SPECCHAR; ++i)
	{
		if(c == specChar[i])
		{			
			return true;
		}
	}
	return false;
}

bool isInteger(string str)
{
	for(uint i =0; i < str.size(); ++i)
		if(!isdigit(str[i]))
			return false;
	return true;
}

bool isBoolean(string str)
{
	if(str == "true" || str == "false")
		return true;
	else
		return false;
}

bool multiplyDefined(string str)
{
	if (str.size() > MAX_NAME_SIZE)
		str = str.substr(0,MAX_NAME_SIZE);	
	for(uint i = 0; i < symbolTable.size(); ++i)
		if(symbolTable[i].externalName == str)
			return true;
	
	return false;
}

bool isKeyWord(string str)
{
	for(int i = 0; i < NUM_KEYWORDS; ++i)
		if(str == keyWords[i])
			return true;
	return false;
}

bool isLit(string str)
{
	if(str[0] == '+' || str[0] == '-' || isInteger(str))
		if(isInteger(str.substr(1, str.size()-1)))
			return true;
	if(str == "true" || str == "false" || str == "not true" || str == "not false")
			return true;
	return false;
}

storeType getDataType(string str)
{
	for(uint i = 0; i < symbolTable.size(); ++i)
		if(symbolTable[i].externalName == str)
			return symbolTable[i].dataType;
	return PROG_NAME;
}

string getValue(string str)
{
	if (str.size() > MAX_NAME_SIZE)
		str = str.substr(0,MAX_NAME_SIZE);	
	for(uint i = 0; i < symbolTable.size(); ++i)
		if(symbolTable[i].externalName == str)
		   return symbolTable[i].value;
	return str;
}

string GenInternalName(storeType s)
{
	string str;
	stringstream temp;
	if(s == UNKNOWN)
	{
		return "T" + currentTempNo;
	}
	else if(s == BOOLEAN)
	{
		temp << boolCounter;
		str = "B" + temp.str();
		++boolCounter;
		return str;
	}
	else if(s == INTEGER)
	{
		temp << intCounter;
		str = "I" + temp.str();
		++intCounter;
		return str;
	}
	else
	{
		return "P0";
	}
}

string currentInternalName(storeType s)
{
	string str;
	stringstream temp;
	if(s == UNKNOWN)
	{
		return "T" + currentTempNo;
	}
	else if(s == BOOLEAN)
	{
		temp << boolCounter;
		str = "B" + temp.str();
		return str;
	}
	else if(s == INTEGER)
	{
		temp << intCounter;
		str = "I" + temp.str();
		return str;
	}
	else
	{
		return "P0";
	}
}

bool hasValue(string str)
{
	entry e;
	for(uint i = 0; i < symbolTable .size(); ++i)
	{
		if(symbolTable[i].externalName == str)
			e = symbolTable[i];
	}
	if(e.value != "")
		return true;
	return false;
}

void processError(string error)
{
   listingFile << "\nError: Line " << countLine << ":" << error << endl;
   ++countError;
   CreateListingTrailer();
   exit(EXIT_SUCCESS);
}

string getStoreType(storeType s)
{
	switch(s)
	{
		case 0:  return "INTEGER";
		case 1:  return "BOOLEAN";
		default: return "PROG_NAME";
	}
}

string getAllocation(allocation a)
{
	switch(a)
	{
		case 0:  return "YES";
		default: return "NO";
	}
}

void validateSpecChar(char c)
{
	if(c == ' ' || c == '\n' || c == 'b'|| c == 'i')
		;
	else if(token == ":" && c == '=')
	{
		token += c;
		NextChar();
	}
	else if(token == "<" && c == '>')
	{
		token += c;
		NextChar();
	}
	else if(token == "<" && c == '=')
	{
		token += c;
		NextChar();
	}
	else if(token == ">" && c == '=')
	{
		token += c;
		NextChar();
	}
	//else
		//processError(" invalid token");
}

string getModes(modes m)
{
	switch(m)
	{
		case 0:  return "VARIABLE";
		default: return "CONSTANT";
	}
}

void PushOperator(string str)
{
	operatorStk.push_back(str);
}

void PushOperand(string str)
{

	
	//Check if string is literal
	//allocate some dat memory for literal
	if(isLit(str))
	{
		for(uint i = 0; i < symbolTable.size(); ++i)
		{
			if(WhichValue(str) == symbolTable[i].value && WhichType(str) == symbolTable[i].dataType && symbolTable[i].mode == CONSTANT)
			{
				//debug(symbolTable[i].internalName);
				operandStk.push_back(symbolTable[i].internalName);
				return;
			}
		}
		string value = str;
		str = currentInternalName(WhichType(value));
		//Insert(str, WhichType(value), CONSTANT, value, YES, 1);
		Insert(value, WhichType(value), CONSTANT, WhichValue(value), YES, 1);
	}
	if(isNonKeyId(str)) // must be non-key id
	{
		//First check if string is in table
		if(!multiplyDefined(str))
			processError(" reference to undefined symbol " + str);
		str = getTableEntryInName(str);		
	}
	//debug(str);
	operandStk.push_back(str);
	return;
}

string PopOperator()
{
	if(!operatorStk.empty())
	{
		string temp = operatorStk.back();
		operatorStk.pop_back();		
		return temp;
	}
	else
	{
		processError(" operator stack underflow");
		return "BOOM-SHAKA-LAKA";
	}
}

string PopOperand()
{
	if(!operandStk.empty())
	{
		string temp = operandStk.back();
		operandStk.pop_back();
		return temp;
		
	}
	else
	{
		processError(" operand stack underflow");
		return "BOOM-SHAKA-LAKA";
	}
}

bool isAdLvlOp(string str)
{
	if(str == "+" || str == "-" || str == "or")
		return true;
	return false;
}

bool isRelOp(string str)
{
	if(str == "=" || str == "<>" || str == ">" || str == ">=" || str == "<" || str == "<=")
		return true;
	return false;
}

bool isMultLvlOp(string str)
{
	if(str == "*" || str == "div" || str == "mod" || str == "and")
		return true;
	return false;
}

void Code(string op, string operand1, string operand2)
{
		if(op == "begin")
			EmitStartCode();
		else if(op == "end")    
			EmitEndCode();
		else if(op == "read")    
			EmitReadCode(operand1);
		else if(op == "write")   
			EmitWriteCode(operand1);
		else if(op == "+")       
			EmitAdditionCode(operand1, operand2);
		else if(op == "-")       
			EmitSubtractionCode(operand1, operand2);
		else if(op == "pos")
			PushOperand(operand1);
		else if(op == "neg")     
			EmitNegCode(operand1);
		else if(op == "not")     
			EmitNotCode(operand1);
		else if(op == "*")       
			EmitMultiplicationCode(operand1, operand2);
		else if(op == "div")     
			EmitDivisionCode(operand1, operand2);
		else if(op == "mod")     
			EmitModCode(operand1, operand2);
		else if(op == "and")     
			EmitAndCode(operand1, operand2);
		else if(op == "or")	    
			EmitOrCode(operand1, operand2);
		else if(op == "<")	    
			EmitLTCode(operand1, operand2);
		else if(op == ">")	    
			EmitGTCode(operand1, operand2);
		else if(op == "<=")	    
			EmitLTECode(operand1, operand2);
		else if(op == ">=")	   
			EmitGTECode(operand1, operand2);
		else if(op == "<>")	   
			EmitNETCode(operand1, operand2);
		else if(op == "=")      
			EmitEqualsCode(operand1, operand2);
		else if(op == ":=")     
			EmitAssignCode(operand1, operand2);
		else if(op == "then")     
			EmitThenCode(operand1);	
		else if(op == "else")     
			EmitElseCode(operand1);
		else if(op == "while")     
			EmitWhileCode();
		else if(op == "do")     
			EmitDoCode(operand1);
		else if(op == "postIf")     
			EmitPostIfCode(operand1);
		else if(op == "postWhile")     
			EmitPostWhileCode(operand1, operand2);
		else if(op == "repeat")     
			EmitRepeatCode();
		else if(op == "until")     
			EmitUntilCode(operand1, operand2);
		else 
			processError(" undefined operation");
}

void EmitStartCode()
{
	writeToObject("STRT","NOP","","",symbolTable[0].externalName + " - King & Martin");
}

void EmitEndCode()
{
	writeToObject("","HLT");
	PrintSymbolTable();
	writeToObject("","END","STRT");
}

void EmitReadCode(string operand1)
{
	if(!multiplyDefined(operand1))
		processError( " variable not declared");
		
	if(genExtTableEntry(operand1).mode == CONSTANT)
		processError( " reading in of read-only location " + operand1);
	writeToObject("","RDI", getTableEntryInName(operand1),"","read(" + getTableEntryExtName(operand1) + ")");
}

void EmitWriteCode(string operand1)
{
	if(!multiplyDefined(operand1))
		processError( " variable not declared");
	writeToObject("","PRI",getTableEntryInName(operand1),"","write("+ getTableEntryExtName(operand1) +")");
}

void EmitAdditionCode(string operand1, string operand2)
{
	////("--EmitAdditionCode: " + op2 + " + " + op1);
	entry op1 = genTableEntry(operand1);
	entry op2 = genTableEntry(operand2);
	
	if (op1.dataType != INTEGER || op2.dataType != INTEGER) 
	{
			processError(" illegal type");
	}
	
	if (isTemp(AReg) && AReg != operand1 && AReg != operand2) 
	{
		// FreeTemp();
		writeToObject("", "STA", AReg,"", "deassign AReg");
		genTableEntry(AReg).alloc = YES;
		AReg = "";
	}
	if (!isTemp(AReg) && AReg != operand1 && AReg != operand2) 
	{
		AReg = "";
	}
	if (AReg != operand1 && AReg != operand2) 
	{
		writeToObject("", "LDA", operand2,"","");
		AReg = operand2;
	}
	
	if (AReg == op2.internalName)
		writeToObject("", "IAD", operand1, "",op2.externalName + " + " + op1.externalName);
	else if (AReg == op1.internalName)
		writeToObject("", "IAD", operand2,"", op2.externalName + " + " + op1.externalName);
		
		
	if (isTemp(operand1)) 
		FreeTemp();
	if (isTemp(operand2)) 
		FreeTemp();
	
	AReg = GetTemp();
	////("AReg: " + AReg);
	genTableEntry(AReg).dataType = INTEGER;
	PushOperand(AReg);
	setDataType(AReg,INTEGER);
}

void EmitSubtractionCode(string operand1, string operand2)
{
	////("--EmitSubtractionCode: " + op2 + " - " + op1);
	
	entry op1 = genTableEntry(operand1);
	entry op2 = genTableEntry(operand2);
	
	if (op1.dataType != INTEGER || op2.dataType != INTEGER) 
	{
			processError(" illegal type");
	}
	
	if (isTemp(AReg) && AReg != operand2) 
	{
		// FreeTemp();
		writeToObject("", "STA", AReg,"", "deassign AReg");
		genTableEntry(AReg).alloc = YES;
		AReg = "";
	}
	if (!isTemp(AReg) && AReg != operand2) 
	{
		AReg = "";
	}
	if (AReg != operand2) 
	{
		writeToObject("", "LDA", operand2,"", "");
		AReg = operand2;
	}
	
	writeToObject("", "ISB", operand1,"", op2.externalName + " - " + op1.externalName);
	if (isTemp(operand1)) 
	{
		FreeTemp();
	}
	if (isTemp(operand2)) 
	{
		FreeTemp();
	}
	
	AReg = GetTemp();
	////("AReg: " + AReg);
	genTableEntry(AReg).dataType = INTEGER;
	PushOperand(AReg);
	setDataType(AReg,INTEGER);
	// if (AReg[0] == 'T')
}


void EmitNegCode(string operand1)
{
	if(getTableEntryType(operand1) != INTEGER)
        processError(" illegal type");
		
	string zero = getZero();
	if(isTemp(AReg))
	{
		genTableEntry(AReg).alloc = YES;
		writeToObject("","STA",AReg,"","deassign AReg");
		AReg = "";
	}
	if(AReg != zero)
	{
		AReg = zero;
		writeToObject("","LDA",AReg,"","");
	}
	if(isTemp(operand1))
		FreeTemp();
	string temp = GetTemp();
	writeToObject("","ISB",getTableEntryInName(operand1),"","-" + getTableEntryExtName(operand1));
	AReg = temp;
	setDataType(temp, INTEGER);
	PushOperand(temp);
}
void EmitNotCode(string operand1)
{
	entry op1 = genTableEntry(operand1);
	if(op1.dataType != BOOLEAN)
		processError(" illegal type");
	if(isTemp(AReg) && AReg != operand1)
	{
		genTableEntry(AReg).alloc = YES;
		writeToObject("","STA",AReg,"","deassign AReg");
		AReg = "";
	}
	else if (!isTemp(AReg) && AReg != operand1)
	{
		AReg = "";
	}
	if(AReg != operand1)
	{
		AReg = operand1;
		writeToObject("","LDA",operand1,"","");
	}
	string label = getLabel();
	writeToObject("","AZJ",label,"", "not " + op1.externalName);
	writeToObject("","LDA",getFalse(),"","");
	writeToObject("","UNJ",label,"+1","");
	writeToObject(label,"LDA",getTrue(),"","");
	if(isTemp(operand1))
		FreeTemp();
	AReg = GetTemp();
	genTableEntry(AReg).dataType = BOOLEAN;
	PushOperand(AReg);	
}
void EmitMultiplicationCode(string operand1, string operand2)
{
	////("--EmitAdditionCode: " + op2 + " + " + op1);
	entry op1 = genTableEntry(operand1);
	entry op2 = genTableEntry(operand2);
	
	if (op1.dataType != INTEGER || op2.dataType != INTEGER) 
	{
			processError(" illegal type");
	}
	
	if (isTemp(AReg) && AReg != operand1 && AReg != operand2) 
	{
		// FreeTemp();
		writeToObject("", "STA", AReg,"", "deassign AReg");
		genTableEntry(AReg).alloc = YES;
		AReg = "";
	}
	if (!isTemp(AReg) && AReg != operand1 && AReg != operand2) 
	{
		AReg = "";
	}
	if (AReg != operand1 && AReg != operand2) 
	{
		writeToObject("", "LDA", operand2,"","");
		AReg = operand2;
	}
	
	if (AReg == op2.internalName)
		writeToObject("", "IMU", operand1, "",op2.externalName + " * " + op1.externalName);
	else if (AReg == op1.internalName)
		writeToObject("", "IMU", operand2,"", op2.externalName + " * " + op1.externalName);
		
		
	if (isTemp(operand1)) 
		FreeTemp();
	if (isTemp(operand2)) 
		FreeTemp();
	
	AReg = GetTemp();
	////("AReg: " + AReg);
	genTableEntry(AReg).dataType = INTEGER;
	PushOperand(AReg);
	setDataType(AReg,INTEGER);	
}
void EmitDivisionCode(string operand1, string operand2)
{
	////("--EmitSubtractionCode: " + op2 + " - " + op1);
	
	entry op1 = genTableEntry(operand1);
	entry op2 = genTableEntry(operand2);
	
	if (op1.dataType != INTEGER || op2.dataType != INTEGER) 
	{
			processError(" illegal type");
	}
	
	if (isTemp(AReg) && AReg != operand2) 
	{
		// FreeTemp();
		writeToObject("", "STA", AReg,"", "deassign AReg");
		genTableEntry(AReg).alloc = YES;
		AReg = "";
	}
	if (!isTemp(AReg) && AReg != operand2) 
	{
		AReg = "";
	}
	if (AReg != operand2) 
	{
		writeToObject("", "LDA", operand2,"", "");
		AReg = operand2;
	}
	
	writeToObject("", "IDV", operand1,"", op2.externalName + " div " + op1.externalName);
	if (isTemp(operand1)) 
	{
		FreeTemp();
	}
	if (isTemp(operand2)) 
	{
		FreeTemp();
	}
	
	AReg = GetTemp();
	genTableEntry(AReg).dataType = INTEGER;
	PushOperand(AReg);
}
void EmitModCode(string operand1, string operand2)
{
	////("--EmitSubtractionCode: " + op2 + " - " + op1);
	
	entry op1 = genTableEntry(operand1);
	entry op2 = genTableEntry(operand2);
	
	if (op1.dataType != INTEGER || op2.dataType != INTEGER) 
	{
			processError(" illegal type");
	}
	
	if (isTemp(AReg) && AReg != operand2) 
	{
		// FreeTemp();
		writeToObject("", "STA", AReg,"", "deassign AReg");
		genTableEntry(AReg).alloc = YES;
		AReg = "";
	}
	if (!isTemp(AReg) && AReg != operand2) 
	{
		AReg = "";
	}
	if (AReg != operand2) 
	{
		writeToObject("", "LDA", operand2,"", "");
		AReg = operand2;
	}
	
	writeToObject("", "IDV", operand1,"", op2.externalName + " mod " + op1.externalName);
	if (isTemp(operand1)) 
	{
		FreeTemp();
	}
	if (isTemp(operand2)) 
	{
		FreeTemp();
	}
	
	AReg = GetTemp();
	genTableEntry(AReg).dataType = INTEGER;
	PushOperand(AReg);
	genTableEntry(AReg).alloc = YES;
	writeToObject("","STQ",AReg,"","store remainder in memory");
	writeToObject("","LDA",AReg,"","load remainder from memory");
}
void EmitAndCode(string operand1, string operand2)
{
////("--EmitAdditionCode: " + op2 + " + " + op1);
	entry op1 = genTableEntry(operand1);
	entry op2 = genTableEntry(operand2);
	
	if (op1.dataType != BOOLEAN || op2.dataType != BOOLEAN) 
	{
			processError(" operator and requires boolean operands");
	}
	
	if (isTemp(AReg) && AReg != operand1 && AReg != operand2) 
	{
		// FreeTemp();
		writeToObject("", "STA", AReg,"", "deassign AReg");
		genTableEntry(AReg).alloc = YES;
		AReg = "";
	}
	if (!isTemp(AReg) && AReg != operand1 && AReg != operand2) 
	{
		AReg = "";
	}
	if (AReg != operand1 && AReg != operand2) 
	{
		writeToObject("", "LDA", operand2,"","");
		AReg = operand2;
	}
	
	if (AReg == op2.internalName)
		writeToObject("", "IMU", operand1, "",op2.externalName + " and " + op1.externalName);
	else if (AReg == op1.internalName)
		writeToObject("", "IMU", operand2,"", op2.externalName + " and " + op1.externalName);
		
		
	if (isTemp(operand1)) 
		FreeTemp();
	if (isTemp(operand2)) 
		FreeTemp();
	
	AReg = GetTemp();
	////("AReg: " + AReg);
	genTableEntry(AReg).dataType = BOOLEAN;
	PushOperand(AReg);
}
void EmitOrCode(string operand1, string operand2)
{
	////("--EmitAdditionCode: " + op2 + " + " + op1);
	entry op1 = genTableEntry(operand1);
	entry op2 = genTableEntry(operand2);
	
	if (op1.dataType != BOOLEAN || op2.dataType != BOOLEAN) 
	{
			processError(" illegal type");
	}
	
	if (isTemp(AReg) && AReg != operand1 && AReg != operand2) 
	{
		// FreeTemp();
		writeToObject("", "STA", AReg,"", "deassign AReg");
		genTableEntry(AReg).alloc = YES;
		AReg = "";
	}
	if (!isTemp(AReg) && AReg != operand1 && AReg != operand2) 
	{
		AReg = "";
	}
	if (AReg != operand1 && AReg != operand2) 
	{
		writeToObject("", "LDA", operand2,"","");
		AReg = operand2;
	}
	
	if (AReg == op2.internalName)
		writeToObject("", "IAD", operand1, "",op2.externalName + " or " + op1.externalName);
	else if (AReg == op1.internalName)
		writeToObject("", "IAD", operand2,"", op2.externalName + " or " + op1.externalName);
	string label = getLabel();
	writeToObject("", "AZJ", label, "+1","");
	writeToObject(label, "LDA", getTrue(), "","");	
	if (isTemp(operand1)) 
		FreeTemp();
	if (isTemp(operand2)) 
		FreeTemp();
	
	AReg = GetTemp();
	////("AReg: " + AReg);
	genTableEntry(AReg).dataType = BOOLEAN;
	PushOperand(AReg);
}
void EmitLTCode(string op1, string op2)
{
//("--EmitLessThanCode: " + op1 + " < " + op2);
	
	entry ent1 = genTableEntry(op1);
	entry ent2 = genTableEntry(op2);
	
	if (ent1.dataType != INTEGER || ent2.dataType != INTEGER) {
			processError("illegal type");
	}
	
	if (isTemp(AReg) && AReg != op2) {
		// FreeTemp();
		writeToObject("", "STA", AReg, "", "deassign AReg");
		genTableEntry(AReg).alloc = YES;
		AReg = "";
	}
	if (!isTemp(AReg) && AReg != op2) {
		AReg = "";
	}
	if (AReg != op2) {
		writeToObject("", "LDA", op2,"", "");
		AReg = op2;
	}
	
	writeToObject("", "ISB", op1,"", ent2.externalName + " < " + ent1.externalName);
	
	
	string label = getLabel();
	writeToObject("", "AMJ", label,"", "");
	writeToObject("", "LDA", getFalse(),"", "");
	writeToObject("", "UNJ", label, "+1", "");
	writeToObject(label, "LDA", getTrue(),"", "");
		
	if (isTemp(op1)) 
		FreeTemp();
	if (isTemp(op2)) 
		FreeTemp();
	
	AReg = GetTemp();
	genTableEntry(AReg).dataType = BOOLEAN;
	PushOperand(AReg);
}	
void EmitGTCode(string op1, string op2)
{
//("--EmitLessThanCode: " + op1 + " < " + op2);
	
	entry ent1 = genTableEntry(op1);
	entry ent2 = genTableEntry(op2);
	
	if (ent1.dataType != INTEGER || ent2.dataType != INTEGER) {
			processError("illegal type");
	}
	
	if (isTemp(AReg) && AReg != op2) {
		// FreeTemp();
		writeToObject("", "STA", AReg,"", "deassign AReg");
		genTableEntry(AReg).alloc = YES;
		AReg = "";
	}
	if (!isTemp(AReg) && AReg != op2) {
		AReg = "";
	}
	if (AReg != op2) {
		writeToObject("", "LDA", op2,"", "");
		AReg = op2;
	}
	
	writeToObject("", "ISB", op1,"", ent2.externalName + " > " + ent1.externalName);
	
	
	string label = getLabel();
	writeToObject("", "AMJ", label,"", "");
	writeToObject("", "AZJ", label,"", "");
	writeToObject("", "LDA", getTrue(), "","");
	writeToObject("", "UNJ", label, "+1", "");
	writeToObject(label, "LDA", getFalse(),"", "");
		
	if (isTemp(op1)) 
		FreeTemp();
	if (isTemp(op2)) 
		FreeTemp();
	
	AReg = GetTemp();
	genTableEntry(AReg).dataType = BOOLEAN;
	PushOperand(AReg);
}
void EmitLTECode(string op1, string op2)
{
//("--EmitLessThanCode: " + op1 + " <= " + op2);
	
	entry ent1 = genTableEntry(op1);
	entry ent2 = genTableEntry(op2);
	
	if (ent1.dataType != INTEGER || ent2.dataType != INTEGER) {
			processError("illegal type");
	}
	
	if (isTemp(AReg) && AReg != op2) {
		// FreeTemp();
		writeToObject("", "STA", AReg,"","deassign AReg");
		genTableEntry(AReg).alloc = YES;
		AReg = "";
	}
	if (!isTemp(AReg) && AReg != op2) {
		AReg = "";
	}
	if (AReg != op2) {
		writeToObject("", "LDA", op2,"", "");
		AReg = op2;
	}
	
	writeToObject("", "ISB", op1, "",ent2.externalName + " <= " + ent1.externalName);
	
	
	string label = getLabel();
	writeToObject("", "AMJ", label,"", "");
	writeToObject("", "AZJ", label,"", "");
	writeToObject("", "LDA", getFalse(),"", "");
	writeToObject("", "UNJ", label, "+1", "");
	writeToObject(label, "LDA", getTrue(),"", "");
		
	if (isTemp(op1)) 
		FreeTemp();
	if (isTemp(op2)) 
		FreeTemp();
	
	AReg = GetTemp();
	genTableEntry(AReg).dataType = BOOLEAN;
	PushOperand(AReg);
}
void EmitGTECode(string op1, string op2)
{
//("--EmitLessThanCode: " + op1 + " < " + op2);
	
	entry ent1 = genTableEntry(op1);
	entry ent2 = genTableEntry(op2);
	
	if (ent1.dataType != INTEGER || ent2.dataType != INTEGER) {
			processError("illegal type");
	}
	
	if (isTemp(AReg) && AReg != op2) {
		// FreeTemp();
		writeToObject("", "STA", AReg,"", "deassign AReg");
		genTableEntry(AReg).alloc = YES;
		AReg = "";
	}
	if (!isTemp(AReg) && AReg != op2) {
		AReg = "";
	}
	if (AReg != op2) {
		writeToObject("", "LDA", op2,"", "");
		AReg = op2;
	}
	
	writeToObject("", "ISB", op1, "",ent2.externalName + " >= " + ent1.externalName);
	
	
	string label = getLabel();
	writeToObject("", "AMJ", label,"", "");
	writeToObject("", "LDA", getTrue(),"","");
	writeToObject("", "UNJ", label, "+1", "");
	writeToObject(label, "LDA", getFalse(),"", "");
		
	if (isTemp(op1)) 
		FreeTemp();
	if (isTemp(op2)) 
		FreeTemp();
	
	AReg = GetTemp();
	genTableEntry(AReg).dataType = BOOLEAN;
	PushOperand(AReg);
}	
void EmitNETCode(string operand1, string operand2)
{
//("--EmitLessThanCode: " + operand1 + " < " + operand2);
	
	entry ent1 = genTableEntry(operand1);
	entry ent2 = genTableEntry(operand2);
	
	if (ent1.dataType != ent2.dataType) {
			processError("illegal type");
	}
	
	if (isTemp(AReg) && AReg != operand2) {
		// FreeTemp();
		writeToObject("", "STA", AReg,"", "deassign AReg");
		genTableEntry(AReg).alloc = YES;
		AReg = "";
	}
	if (!isTemp(AReg) && AReg != operand2) {
		AReg = "";
	}
	if (AReg != operand2) {
		writeToObject("", "LDA", operand2,"", "");
		AReg = operand2;
	}
	
	writeToObject("", "ISB", operand1, "",ent2.externalName + " <> " + ent1.externalName);
	
	
	string label = getLabel();
	writeToObject("", "AZJ", label,"+1", "");
	writeToObject(label, "LDA", getTrue(),"", "");
		
	if (isTemp(operand1)) 
		FreeTemp();
	if (isTemp(operand2)) 
		FreeTemp();
	
	AReg = GetTemp();
	genTableEntry(AReg).dataType = BOOLEAN;
	PushOperand(AReg);
}	
void EmitEqualsCode(string op1, string op2)
{
//("--EmitLessThanCode: " + op1 + " < " + op2);
	
	entry ent1 = genTableEntry(op1);
	entry ent2 = genTableEntry(op2);
	if (ent1.dataType != ent2.dataType) {
			processError("illegal type");
	}
	
	if (isTemp(AReg) && AReg != op1 && AReg != op2) {
		// FreeTemp();
		writeToObject("", "STA", AReg,"", "deassign AReg");
		genTableEntry(AReg).alloc = YES;
		AReg = "";
	}
	if (!isTemp(AReg) && AReg != op1 && AReg != op2) {
		AReg = "";
	}
	if (AReg != op1 && AReg != op2) {
		writeToObject("", "LDA", op2,"", "");
		AReg = op2;
	}
	if (AReg == op2)
		writeToObject("", "ISB", op1,"", ent2.externalName + " = " + ent1.externalName);
	else if (AReg == op1)
		writeToObject("", "ISB", op2,"", ent2.externalName + " = " + ent1.externalName);
	
	
	string label = getLabel();
	writeToObject("", "AZJ", label,"", "");
	writeToObject("", "LDA", getFalse(),"", "");
	writeToObject("", "UNJ", label, "+1" , "");
	writeToObject(label, "LDA", getTrue() ,"", "");
		
	if (isTemp(op1)) 
		FreeTemp();
	if (isTemp(op2)) 
		FreeTemp();
	
	AReg = GetTemp();
	genTableEntry(AReg).dataType = BOOLEAN;
	PushOperand(AReg);
}	

void EmitAssignCode(string operand1, string operand2)
{
	////("--EmitAssignCode: " + op2 + " := " + op1);
	
	entry op1 = genTableEntry(operand1);
	entry op2 = genTableEntry(operand2);
	
	if (op1.dataType != op2.dataType) 
	{
		//("op1: " + operand1 + "    -  op2: " + operand2);
			processError(" illegal type");
	}
	
	if (op2.mode != VARIABLE) 
	{
		processError("symbol on left-hand side of assignment must have a storage mode of VARIABLE");
	}
	if (operand1 == operand2) 
	{
		return;
	}
	// if (genTableEntry(AReg).value != ent1.value) {
	if (AReg != operand1) 
	{
		writeToObject("", "LDA", operand1,"", "");
	}
	writeToObject("", "STA", operand2,"", op2.externalName + " := " + op1.externalName);
	// do - deassign operand1
	// AReg = genTableEntry(op1).value;
	AReg = operand2;
	
	if (isTemp(operand1)) 
	{
		FreeTemp();
	}
}

void writeToObject(string label, string instruct, string arg, string signVal, string comment)
{
	objectFile 	<< left << setw(6) << label << setw(4) << instruct << setw(4) 
				<< arg << setw(5) << signVal << comment << endl;
}

storeType getTableEntryType(string str)
{
	for(uint i = 0; i < symbolTable.size(); ++i)
		if(symbolTable[i].internalName == str)
			return symbolTable[i].dataType;
	return symbolTable[0].dataType;
}

modes getTableEntryMode(string str)
{
	for(uint i = 0; i < symbolTable.size(); ++i)
		if(symbolTable[i].internalName == str)
			return symbolTable[i].mode;
	return symbolTable[0].mode;
}

string getTableEntryInName(string str)
{
	for(uint i = 0; i < symbolTable.size(); ++i)
		if(symbolTable[i].externalName == str)
			return symbolTable[i].internalName;
	return str;
}

string getTableEntryExtName(string str)
{
	for(uint i = 0; i < symbolTable.size(); ++i)
		if(symbolTable[i].internalName == str)
			return symbolTable[i].externalName;
	return str;
}

string getTableEntryValue(string str)
{
	for(uint i = 0; i < symbolTable.size(); ++i)
		if(symbolTable[i].internalName == str)
			return symbolTable[i].value;
	return str;
}

void FreeTemp()
{
	--currentTempNo;
	if(currentTempNo < -1)
		processError(" compiler error, currentTempNo should be >= -1");
}

string GetTemp()
{
	string temp;
	++currentTempNo;
	stringstream sstemp;
	sstemp << currentTempNo;
	temp = "T" + sstemp.str();
	if(currentTempNo > maxTempNo)
	{
		Insert(temp, UNKNOWN, VARIABLE, "", NO, 1);
		maxTempNo++;
	}
	return temp;
}

void setDataType(string s, storeType t)
{
	for(uint i = 0; i < symbolTable.size(); ++i)
		if(symbolTable[i].internalName == s)
			symbolTable[i].dataType = t;
}

bool isTemp(string s)
{
	for(uint i = 0; i < symbolTable.size(); ++i)
		if(symbolTable[i].internalName == s && symbolTable[i].externalName[0] == 'T' &&  symbolTable[i].externalName[1] != 'R')
			return true;
	return false;
}

entry& genTableEntry(string s)
{
	s = s.substr(0, MAX_NAME_SIZE);
	
	for(uint i = 0; i < symbolTable.size(); ++i)
		if(symbolTable[i].internalName == s)
			return symbolTable[i];
	return symbolTable[0];
}

entry& genExtTableEntry(string s)
{
	s = s.substr(0, MAX_NAME_SIZE);
	
	for(uint i = 0; i < symbolTable.size(); ++i)
		if(symbolTable[i].externalName == s)
			return symbolTable[i];
	return symbolTable[0];
}

string getLabel()
{
	string temp;
	stringstream sstemp;
	sstemp << labelCount;
	temp = "L" + sstemp.str();
	++labelCount;
	return temp;
}

string getFalse()
{
	if(!multiplyDefined("FALSE"))
		Insert("FALSE",BOOLEAN,CONSTANT, WhichValue("false"), YES, 1);
	return "FALS";
}

string getTrue()
{
	if(!multiplyDefined("TRUE"))
		Insert("TRUE",BOOLEAN,CONSTANT, WhichValue("true"), YES, 1);
	return "TRUE";	
}

string getZero()
{
	if(!multiplyDefined("ZERO"))
		Insert("ZERO",INTEGER,CONSTANT, WhichValue("0"), YES, 1);
	return "ZERO";	
}

void debug(string s)
{
	objectFile << s << endl;
}

void EmitThenCode(string operand)
{
	string tempLabel = getLabel();
	writeToObject("","AZJ",tempLabel,"","if false jump to " + tempLabel);
	operandStk.push_back(tempLabel);
	if (isTemp(operand)) 
		FreeTemp();
	AReg = "";
}

void EmitElseCode(string operand)
{
	string tempLabel = getLabel();
	writeToObject("","UNJ",tempLabel,"","jump to end if");
	writeToObject(operand,"NOP","","","else");
	operandStk.push_back(tempLabel);
	AReg = "";
}

void EmitPostIfCode(string operand)
{
	writeToObject(operand,"NOP","","","end if");
	AReg = "";
}

void EmitWhileCode()
{
	string tempLabel = getLabel();
    writeToObject(tempLabel,"NOP","","","while");
    operandStk.push_back(tempLabel);
    AReg = "";
}

void EmitDoCode(string operand)
{
	string tempLabel = getLabel();
    writeToObject("","AZJ",tempLabel,"","do");
    operandStk.push_back(tempLabel);
    if (isTemp(operand)) 
		FreeTemp();
    AReg = "";
}

void EmitPostWhileCode(string operand1, string operand2)
{
	writeToObject("","UNJ",operand2,"","end while");
    writeToObject(operand1,"NOP","","","");
    AReg = "";
}

void EmitRepeatCode()
{
	string tempLabel = getLabel();
	writeToObject(tempLabel,"NOP","","","repeat");
	operandStk.push_back(tempLabel);
	//deassign operands from all registers
     AReg = "";
}

void EmitUntilCode(string operand1, string operand2)
{
	//emit instruction to set the condition code depending on the value of operand1
	
	//emit instruction to branch the value of operand2 if the condition code indicates operand is zero
	writeToObject("","AZJ",operand2,"","until");
	//if operand is a temp the free operand1's name for reuse
	if (isTemp(operand1)) 
		FreeTemp();
	if (isTemp(operand2)) 
		FreeTemp();
	// deassign operands from all registers
     AReg = "";
}

void PutBackToken(string str)
{
    for(uint i = 0; i < str.size(); ++i)
        sourceFile.putback(str[i]);
}
