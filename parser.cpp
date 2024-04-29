/* Implementation of Recursive-Descent Parser
 * for the SFort95 Language
 * parser(SP24).cpp
 * Programming Assignment 2
 * Spring 2024
*/

#include <string>
#include <map>
#include <stack>

#include "parserInterp.h"
#include "val.h"
#include "lex.cpp"
#include "eval.cpp"

using namespace std;

map<string, bool> defVar;
map<string, Token> SymTable;
map<string, Value> AssignTable;

queue<Value>* valQue;

stack<Value> operands;
stack<string> operators;

namespace Parser {
	bool pushed_back = false;
	LexItem	pushed_token;

	static LexItem GetNextToken(istream& in, int& line) {
		if( pushed_back ) {
			pushed_back = false;
			return pushed_token;
		}
		return getNextToken(in, line);
	}

	static void PushBackToken(LexItem & t) {
		if( pushed_back ) {
			abort();
		}
		pushed_back = true;
		pushed_token = t;	
	}

    void clearQueue() {
        while (!(*valQue).empty())
        {
            Value nextVal = (*valQue).front();
            cout << nextVal;
            valQue->pop();
        }
    }

}

static int error_count = 0;

int ErrCount() {
    return error_count;
}

void ParseError(int line, string msg) {
	++error_count;
	cout << line << ": " << msg << endl;
}

bool assignToVariable(string varname, Token vartype, Value v, int line) {

    //type checking
    switch (vartype) {
        case CHARACTER: //variable would have been assigned a default value
            if (v.IsString()) {
                int variableLength = AssignTable[varname].GetString().length();

                if (v.GetstrLen() > variableLength) {
                    //truncates to variable length
                    v.SetString(v.GetString().substr(0, variableLength));
                } else if (v.GetstrLen() < variableLength) {
                    //pads to variable length
                    v.SetString(v.GetString() + string(variableLength - v.GetstrLen(), ' '));
                }
                //all checks passed, make the assignment in memory
                AssignTable[varname] = v;
            } else {
                ParseError(line, "Invalid typing for variable assignment");
                return false;
            }
            break;
        case INTEGER:
            if (v.IsInt()) {
                AssignTable[varname] = v;
            } else if (v.IsReal()) {
                //implicit type casting double --> int
                AssignTable[varname] = Value((int) v.GetReal());
            } else {
                ParseError(line, "Invalid typing for variable assignment");
            }
            break;
        case REAL:
            if (v.IsInt()) {
                //implicit type casting int --> double
                AssignTable[varname] = Value((double) v.GetInt());
            } else if (v.IsReal()) {
                AssignTable[varname] = v;
            } else {
                ParseError(line, "Invalid typing for variable assignment");
                return false;
            }
            break;
        case BCONST:
            if (v.IsBool()) {
                AssignTable[varname] = v;
            } else {
                ParseError(line, "Invalid typing for variable assignment");
                return false;
            }
            break;
        default:
            ParseError(line,"VARIABLE TYPE (" + to_string(((int) vartype)) + ") NOT RECOGNIZED FOR ASSIGNMENT");
            return false;
    }
    return true;
}

//Program is: Prog = PROGRAM IDENT {Decl} {Stmt} END PROGRAM IDENT
bool Prog(istream& in, int& line)
{
	bool dl = false, sl = false;
	LexItem tok = Parser::GetNextToken(in, line);
		
	if (tok.GetToken() == PROGRAM) {
		tok = Parser::GetNextToken(in, line);
		if (tok.GetToken() == IDENT) {
			dl = Decl(in, line);
			if( !dl  )
			{
				ParseError(line, "Incorrect Declaration in Program");
				return false;
			}
			sl = Stmt(in, line);
			if( !sl  )
			{
				ParseError(line, "Incorrect Statement in program");
				return false;
			}	
			tok = Parser::GetNextToken(in, line);
			
			if (tok.GetToken() == END) {
				tok = Parser::GetNextToken(in, line);
				
				if (tok.GetToken() == PROGRAM) {
					tok = Parser::GetNextToken(in, line);
					
					if (tok.GetToken() == IDENT) {
						cout << "(DONE)" << endl;
						return true;
					}
					else
					{
						ParseError(line, "Missing Program Name");
						return false;
					}	
				}
				else
				{
					ParseError(line, "Missing PROGRAM at the End");
					return false;
				}	
			}
			else
			{
				ParseError(line, "Missing END of Program");
				return false;
			}	
		}
		else
		{
			ParseError(line, "Missing Program name");
			return false;
		}
	}
	else if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	
	else
	{
		ParseError(line, "Missing Program keyword");
		return false;
	}
}

//Decl ::= Type :: VarList 
//Type ::= INTEGER | REAL | CHARARACTER [(LEN = ICONST)] 
bool Decl(istream& in, int& line) {
	bool status = false;
	LexItem tok;
	string strLen;
	
	LexItem type = Parser::GetNextToken(in, line);
	
	if(type == INTEGER || type == REAL || type == CHARACTER ) {
		tok = type;
		
		tok = Parser::GetNextToken(in, line);
		if (tok.GetToken() == DCOLON) {
			status = VarList(in, line, type);
			
			if (status) {
				status = Decl(in, line);
				if(!status) {
					ParseError(line, "Declaration Syntactic Error.");
					return false;
				}
				return status;
			}
			else {
				ParseError(line, "Missing Variables List.");
				return false;
			}
		}
		else if(type == CHARACTER && tok.GetToken() == LPAREN) {
			tok = Parser::GetNextToken(in, line);
			
			if(tok.GetToken() == LEN) {
				tok = Parser::GetNextToken(in, line);
				
				if(tok.GetToken() == ASSOP) {
					tok = Parser::GetNextToken(in, line);
					
					if(tok.GetToken() == ICONST) {
						strLen = tok.GetLexeme();
						
						tok = Parser::GetNextToken(in, line);
						if(tok.GetToken() == RPAREN) {
							tok = Parser::GetNextToken(in, line);
							if(tok.GetToken() == DCOLON) {
								status = VarList(in, line, type, stoi(strLen));
								
								if (status) {
									cout << "Definition of Strings with length of " << strLen << " in declaration statement." << endl;
									status = Decl(in, line);
									if(!status)
									{
										ParseError(line, "Declaration Syntactic Error.");
										return false;
									}
									return status;
								}
								else
								{
									ParseError(line, "Missing Variables List.");
									return false;
								}
							}
						}
						else
						{
							ParseError(line, "Missing Right Parenthesis for String Length definition.");
							return false;
						}
					
					}
					else
					{
						ParseError(line, "Incorrect Initialization of a String Length");
						return false;
					}
				}
			}
		}
		else
		{
			ParseError(line, "Missing Double Colons");
			return false;
		}
			
	}
		
	Parser::PushBackToken(type);
	return true;
}//End of Decl

//Stmt ::= AssigStmt | BlockIfStmt | PrintStmt | SimpleIfStmt
bool Stmt(istream& in, int& line) {
    resetStacks(operands, operators); //we expect to call expr once in good exec

    bool status;
	
	LexItem t = Parser::GetNextToken(in, line);

	switch( t.GetToken() ) {

	case PRINT:
		status = PrintStmt(in, line);
		
		if(status)
			status = Stmt(in, line);
		break;

	case IF:
		status = BlockIfStmt(in, line);
		if(status)
			status = Stmt(in, line);
		break;

	case IDENT:
		Parser::PushBackToken(t);
        status = AssignStmt(in, line);
		if(status)
			status = Stmt(in, line);
		break;
		
	
	default:
		Parser::PushBackToken(t);
		return true;
	}

	return status;
}//End of Stmt

bool SimpleStmt(istream& in, int& line) {
	bool status;
	
	LexItem t = Parser::GetNextToken(in, line);
	
	switch( t.GetToken() ) {

	case PRINT:
		status = PrintStmt(in, line);
		
		if(!status)
		{
			ParseError(line, "Incorrect Print Statement");
			return false;
		}	
		cout << "Print statement in a Simple If statement." << endl;
		break;

	case IDENT:
		Parser::PushBackToken(t);
        status = AssignStmt(in, line);
		if(!status)
		{
			ParseError(line, "Incorrect Assignent Statement");
			return false;
		}
		cout << "Assignment statement in a Simple If statement." << endl;
			
		break;
		
	
	default:
		Parser::PushBackToken(t);
		return true;
	}

	return status;
}//End of SimpleStmt

//VarList ::= Var [= Expr] {, Var [= Expr]}
bool VarList(istream& in, int& line, LexItem& idtok, int strLen) { //idtok is varType
	bool status = false, exprstatus = false;
	string identstr;

	LexItem tok = Parser::GetNextToken(in, line);
	if(tok == IDENT) {
		
		identstr = tok.GetLexeme();
		if (!(defVar.find(identstr)->second)) {  //variable doesn't exist in map?
            SymTable[identstr] = idtok.GetToken();
			defVar[identstr] = true;

            if(idtok == CHARACTER) { //initialized as blanks for strLen referencing cross-scope
                AssignTable[identstr] = string(strLen, ' ');
            }

		} else {
			ParseError(line, "Variable Redefinition");
			return false;
		}
		
	} else {
		ParseError(line, "Missing Variable Name");
		return false;
	}
		
	tok = Parser::GetNextToken(in, line);
	if(tok == ASSOP) {

        //reset stacks
        resetStacks(operands, operators);

        Value returned;
		exprstatus = Expr(in, line, returned);

		if(!exprstatus) {
			ParseError(line, "Incorrect initialization for a variable.");
			return false;
		}

        bool vstatus = assignToVariable(identstr, idtok.GetToken(), returned, line);
		if(!vstatus)
            return false;

		cout<< "Initialization of the variable " << identstr <<" in the declaration statement." << endl;
		tok = Parser::GetNextToken(in, line);
		
		if (tok == COMMA) {
            status = VarList(in, line, idtok);
        } else {
			Parser::PushBackToken(tok);
			return true;
		}
	}
	else if (tok == COMMA) {
		status = VarList(in, line, idtok);
	} else if(tok == ERR) {
		ParseError(line, "Unrecognized Input Pattern");
		return false;
	} else {
		Parser::PushBackToken(tok);
		return true;
	}
	
	return status;
	
}//End of VarList
	


//PrintStmt:= PRINT *, ExprList
bool PrintStmt(istream& in, int& line) {
    LexItem t;
    valQue = new queue<Value>;


    t = Parser::GetNextToken(in, line);

    if( t != DEF ) {

        ParseError(line, "Print statement syntax error.");
        return false;
    }
    t = Parser::GetNextToken(in, line);

    if( t != COMMA ) {

        ParseError(line, "Missing Comma.");
        return false;
    }
    bool ex = ExprList(in, line);

    if( !ex ) {
        ParseError(line, "Missing expression after Print Statement");
        return false;
    }

    Parser::clearQueue();

    cout << endl;
    return ex;
}//End of PrintStmt

//BlockIfStmt:= if (Expr) then {Stmt} [Else Stmt]
//SimpleIfStatement := if (Expr) Stmt
bool BlockIfStmt(istream& in, int& line) {
	bool ex=false, status;
	static int nestlevel = 0;
	int level;
	LexItem t;
	
	t = Parser::GetNextToken(in, line);
	if( t != LPAREN ) {
		
		ParseError(line, "Missing Left Parenthesis");
		return false;
	}

    Value returned;
	ex = RelExpr(in, line, returned);

    if( !ex ) {
		ParseError(line, "Missing if statement condition");
		return false;
	}
	
	t = Parser::GetNextToken(in, line);
	if(t != RPAREN ) {
		
		ParseError(line, "Missing Right Parenthesis");
		return false;
	}
	
	t = Parser::GetNextToken(in, line);
	if(t != THEN)
	{
		Parser::PushBackToken(t);
		
		status = SimpleStmt(in, line);
		if(status) //this is SimpleIfStmt :(
		{
			return true;
		}
		else
		{
			ParseError(line, "If-Stmt Syntax Error");
			return false;
		}
		
	}

    nestlevel++;
	level = nestlevel;
	status = Stmt(in, line);

	if(!status)
	{
		ParseError(line, "Missing Statement for If-Stmt Then-Part");
		return false;
	}
	t = Parser::GetNextToken(in, line);
	if( t == ELSE ) {
		status = Stmt(in, line);
		if(!status)
		{
			ParseError(line, "Missing Statement for If-Stmt Else-Part");
			return false;
		}
		else
		  t = Parser::GetNextToken(in, line);
		
	}

	if(t != END ) {
		
		ParseError(line, "Missing END of IF");
		return false;
	}
	t = Parser::GetNextToken(in, line);
	if(t == IF ) {
		cout << "End of Block If statement at nesting level " << level << endl;
		return true;
	}	
	
	Parser::PushBackToken(t);
	ParseError(line, "Missing IF at End of IF statement");
	return false;
}

bool Var(istream& in, int& line, LexItem& idtok) {
	string identstr;
	
	LexItem tok = Parser::GetNextToken(in, line);
	
	if (tok == IDENT){
		identstr = tok.GetLexeme();
        //proxy for piping cross-scope variable data

        if (!(defVar.find(identstr)->second)) {
            ParseError(line, "Undeclared Variable");
            return false;
        }

        idtok = LexItem(SymTable.at(identstr), identstr, line);

		return true;
	}
	else if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	return false;
}

bool AssignStmt(istream& in, int& line) {
	
	bool varstatus = false, status = false;
	LexItem t;

    LexItem variable;
    /*
     * Variable
     * Token: VARTYPE
     * Lexeme: VARNAME
     * Line: Line referenced
     */
    varstatus = Var(in, line, variable); //will update variable to include name and type

	if (varstatus){
		t = Parser::GetNextToken(in, line);
		
		if (t == ASSOP){

            Value returned;
			status = Expr(in, line, returned);

			if(!status) {
				ParseError(line, "Missing Expression in Assignment Statment");
				return false;
			}

            varstatus = assignToVariable(variable.GetLexeme(),variable.GetToken(), returned, line);
            if(!varstatus)
                return false;

		}

		else if(t.GetToken() == ERR){
			ParseError(line, "Unrecognized Input Pattern");
			cout << "(" << t.GetLexeme() << ")" << endl;
			return false;
		}

		else {
			ParseError(line, "Missing Assignment Operator");
			return false;
		}
	}
	else {
		ParseError(line, "Missing Left-Hand Side Variable in Assignment statement");
		return false;
	}
    return true;
}//End of AssignStmt

//ExprList:= Expr {,Expr}
bool ExprList(istream& in, int& line) {
    bool status = false;

    Value returned;
    status = Expr(in, line, returned);

    if(!status){
        ParseError(line, "Missing Expression");
        return false;
    }

    valQue->push(returned);
    LexItem tok = Parser::GetNextToken(in, line);

    if (tok == COMMA) {
        status = ExprList(in, line);

    }

    else if(tok.GetToken() == ERR){
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << tok.GetLexeme() << ")" << endl;
        return false;
    }

    else{
        Parser::PushBackToken(tok);
        return true;
    }

    return status;
}

bool RelExpr(istream& in, int& line, Value& retVal) {
	
	bool t1 = Expr(in, line, retVal);
	LexItem tok;
	
	if( !t1 ) {
		return false;
	}
	
	tok = Parser::GetNextToken(in, line);
	if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}

	if ( tok == EQ || tok == LTHAN || tok == GTHAN) {
        addOperatorAndEvaluateStacks(operands, operators, tok.GetLexeme());

        t1 = Expr(in, line, retVal);
		if( !t1 ) {
			ParseError(line, "Missing operand after operator");
			return false;
		}
		
	}
	
	return true;
}

bool Expr(istream& in, int& line, Value& retVal) {
	
	bool t1 = MultExpr(in, line, retVal);
	LexItem tok;
	
	if( !t1 ) {
		return false;
	}
	
	tok = Parser::GetNextToken(in, line);
	if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}

    while ( tok == PLUS || tok == MINUS || tok == CAT) {
        addOperatorAndEvaluateStacks(operands, operators, tok.GetLexeme());

        t1 = MultExpr(in, line, retVal);
		if( !t1 ) {
			ParseError(line, "Missing operand after operator");
			return false;
		}
		
		tok = Parser::GetNextToken(in, line);
		if(tok.GetToken() == ERR) {
			ParseError(line, "Unrecognized Input Pattern");
			cout << "(" << tok.GetLexeme() << ")" << endl;
			return false;
		}		
		
	}

	Parser::PushBackToken(tok);
    evaluateStackRemains(operands, operators);
    if(!operands.empty())
        retVal = operands.top(); // might not need, will see l8r
	return true;
}

bool MultExpr(istream& in, int& line, Value& retVal) {
	
	bool t1 = TermExpr(in, line, retVal);
	LexItem tok;
	
	if( !t1 ) {
		return false;
	}
	
	tok	= Parser::GetNextToken(in, line);
	if(tok.GetToken() == ERR) {
			ParseError(line, "Unrecognized Input Pattern");
			cout << "(" << tok.GetLexeme() << ")" << endl;
			return false;
	}

	while ( tok == MULT || tok == DIV  ) {
        addOperatorAndEvaluateStacks(operands, operators, tok.GetLexeme());

        t1 = TermExpr(in, line, retVal);
		
		if( !t1 ) {
			ParseError(line, "Missing operand after operator");
			return false;
		}
		
		tok	= Parser::GetNextToken(in, line);
		if(tok.GetToken() == ERR){
			ParseError(line, "Unrecognized Input Pattern");
			cout << "(" << tok.GetLexeme() << ")" << endl;
			return false;
		}
		
	}
	Parser::PushBackToken(tok);
	return true;
}

bool TermExpr(istream& in, int& line, Value& retVal) {
	
	bool t1 = SFactor(in, line, retVal);
	LexItem tok;

	if( !t1 ) {
		return false;
	}
	
	tok	= Parser::GetNextToken(in, line);
	if(tok.GetToken() == ERR){
			ParseError(line, "Unrecognized Input Pattern");
			cout << "(" << tok.GetLexeme() << ")" << endl;
			return false;
	}

	while (tok == POW) {
        addOperatorAndEvaluateStacks(operands, operators, tok.GetLexeme());

        t1 = SFactor(in, line, retVal);
		
		if( !t1 ) {
			ParseError(line, "Missing exponent operand");
			return false;
		}
		
		tok	= Parser::GetNextToken(in, line);
		if(tok.GetToken() == ERR){
			ParseError(line, "Unrecognized Input Pattern");
			cout << "(" << tok.GetLexeme() << ")" << endl;
			return false;
		}
		
	}
	Parser::PushBackToken(tok);
	return true;
}

bool SFactor(istream& in, int& line, Value& retVal) {
	LexItem t = Parser::GetNextToken(in, line);
	
	bool status;
	int sign = 0;

	if(t == MINUS ) {
		sign = -1;
	} else if(t == PLUS) {
		sign = 1;
	} else {
        Parser::PushBackToken(t);
    }
		
	status = Factor(in, line, sign, retVal);
	return status;
}

bool Factor(istream& in, int& line, int sign, Value& retVal) {
    string signage = (sign >= 0 ? "" : "-");

	LexItem tok = Parser::GetNextToken(in, line);

    Value lexValue;
	if( tok == IDENT ) { //Variable, check if exists then pipe value into stack buffer
		string lexeme = tok.GetLexeme();
		if (!(defVar.find(lexeme)->second)) {
			ParseError(line, "Using Undefined Variable");
			return false;	
		} else if(AssignTable.find(lexeme) == AssignTable.end()) { //iterated wto finding
            ParseError(line, "Using Unassigned Variable (" + lexeme + ")");
            return false;
        }
        lexValue = AssignTable[lexeme];
        addOperandAndEvaluateStacks(operands, operators, lexValue);
        return true;
	}

	else if( tok == ICONST ) {
        lexValue = Value(stoi(signage + tok.GetLexeme()));
        addOperandAndEvaluateStacks(operands, operators, lexValue);
		return true;
	}
	else if( tok == SCONST ) {
        if(signage == "-") {
            ParseError(line, "String value cannot be negative");
            return false;
        }
        lexValue = Value(tok.GetLexeme());
        addOperandAndEvaluateStacks(operands, operators, lexValue);
		return true;
	}
	else if( tok == RCONST ) {
        lexValue = Value(stod(signage + tok.GetLexeme()));
        addOperandAndEvaluateStacks(operands, operators, lexValue);
		return true;
	}
	else if( tok == LPAREN ) {
        addOperatorAndEvaluateStacks(operands, operators, "(");

		bool ex = Expr(in, line, retVal);

        if( !ex ) {
			ParseError(line, "Missing expression after (");
			return false;
		}

		if( Parser::GetNextToken(in, line) == RPAREN ) {
            addOperatorAndEvaluateStacks(operands, operators, ")");
            return ex;
        }
		else {
			Parser::PushBackToken(tok);
			ParseError(line, "Missing ) after expression");
			return false;
		}
	}
	else if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	
	return false;
}



