//  A compiler from a very simple Pascal-like structured language LL(k)
//  to 64-bit 80x86 Assembly langage
//  Copyright (C) 2019 Pierre Jourlin
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//  
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//  
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

// Build with "make compilateur"


#include <string>
#include <iostream>
#include <cstdlib>

using namespace std;

char current;
char lookedAhead;				// Current car	

enum OPREL {equ, diff, infe, supe, inf, sup, unknown};
int NLookedAhead=0;

void ReadChar(void){
    if(NLookedAhead>0){
        current=lookedAhead;    // Char has already been read
        NLookedAhead--;
    }
    else
        // Read character and skip spaces until 
        // non space character is read
        while(cin.get(current) && (current==' '||current=='\t'||current=='\n'));
}

void LookAhead(void){
    while(cin.get(lookedAhead) && (lookedAhead==' '||lookedAhead=='\t'||lookedAhead=='\n'));
    NLookedAhead++;
}




void Error(string s){
	cerr<< s << endl;
	exit(-1);
}

// ArithmeticExpression := Term {AdditiveOperator Term}
// Term := Digit | "(" ArithmeticExpression ")"
// AdditiveOperator := "+" | "-"
// Digit := "0"|"1"|"2"|"3"|"4"|"5"|"6"|"7"|"8"|"9"


OPREL RationalOperator(void){
	
	if(current!='<'&&current!='>'&&current!='!'&&current!='='){
		return unknown;
	}
	LookAhead();
	if(lookedAhead=='='){

		if(current=='<'){
			ReadChar();
			ReadChar();
			return infe;
		}

		if(current=='='){
			ReadChar();
			ReadChar();
			return equ;
		}

		if(current=='>'){
			ReadChar();
			ReadChar();
			return supe;
		}

		if(current=='!'){
			ReadChar();
			ReadChar();
			return diff;
		}
	}
	else if (current=='='){
		Error("utilisez '==' comme opérateur d'égalité");

	}
	else if(current=='<'){
		ReadChar();
		return inf;
	}
	else if(current=='>'){
		ReadChar();
		return sup;
	}
	else{
		Error("Opérateur rationnel attendu");
	}
	return unknown;
}

void MultiplicativeOperator(void){
	if(current=='*'||current=='/'||current=='%'){
		ReadChar();
	}
	else if (current=='&' ){
		ReadChar();
		if (current=='&'){
			ReadChar();
		}
		else{
			Error("l'opérateur ET s'écrit '&&'");
		}
		
	}
	else
		Error("Opérateur multiplicatif attendu");
}




	
void AdditiveOperator(void){
	if(current=='+'||current=='-'){
		ReadChar();
	}
	else if (current=='|' ){
		ReadChar();
		if (current=='|'){
			ReadChar();
		}
		else{
			Error("l'opérateur de comparaison s'écrit '||'");
		}
		
	}
	else
		Error("Opérateur additif attendu");	   // Additive operator expected
}
		
void Digit(void){
	if((current<'0')||(current>'9'))
		Error("Chiffre attendu");		   // Digit expected
	else{
		cout << "\tpush $"<<current<<endl;
		ReadChar();
	}
}
void Letter(void){
	if((current<'a')||(current>'z'))
		Error("Lettre attendu");
	else{
		cout << "\tpush $"<<current<<endl;
		ReadChar();
	}
}
void Number(void){
	unsigned long long number;
	if((current<'0')||(current>'9'))
		Error("Chiffre attendu");
	else{
		number=current-'0';
	}
	ReadChar();
	while (current>='0' && current<='9'){
		number*+10;
		number=current-'0';
		ReadChar();
	}

}

void Expression(void);
void Factor(void){
	if (current=='('){
		ReadChar();
		Expression();
		if (current!=')'){
			Error("')' était attendu");	
		}
		ReadChar();
	}
	else{
		if (current<='9'||current>='0'){
			Number();
		}
		else{
			if(current>='a'||current<='z'){
				Letter();
			}
			else{
				Error("'(' ou chiffre ou lettre attendue");
			}
		}
	}
}
void Term(void){
	char mulop;
	Factor();
	while (current=='*'||current=='/'||current=='%'){
		mulop=current;
		MultiplicativeOperator();
		Factor();
		cout << "\tpop %rbx"<<endl;
		cout << "\tpop %rax"<<endl;	
		if (mulop=='*'){
			cout<<"\tmulq %rbx"<<endl; //rbx*rax -> rdx:rax
			cout<<"\tpush %rax"<<endl;
			break;
		}
		else if (mulop=='&'){
			cout<<"\tmulq %rbx"<<endl; 
			cout<<"\tpush %rax"<<endl;
			break;
		}
		else if (mulop=='/'){

			cout<<"\tmovq $0, %rdx"<<endl; 
			cout<<"\tdiv %rbx"<<endl; //rdx:rax / rbx  |  quotient -> rax  |  reste -> rdx
			cout<<"\tpush %rax"<<endl;
			break;
		}
		else if (mulop=='%'){
			cout<<"\tmovq $0, %rdx"<<endl;
			cout<<"\tdiv %rbx"<<endl;
			cout<<"\tpush %rdx"<<endl;
			break;
			}
		else{
			Error("Operateur additif attendu");
		}
	}
}


void SimpleExpression(void){
	char adop;
	Term();
	while(current=='+'||current=='-'){
		adop=current;		// Save operator in local variable
		AdditiveOperator();
		Term();
		cout << "\tpop %rbx"<<endl;	// get first operand
		cout << "\tpop %rax"<<endl;	// get second operand
		if(adop=='+')
			cout << "\taddq	%rbx, %rax"<<endl;	// add both operands
		else
			cout << "\tsubq	%rbx, %rax"<<endl;	// substract both operands
		cout << "\tpush %rax"<<endl;			// store result
	}

}

void Expression(void){
	OPREL oprel;
	SimpleExpression();
	if(current=='='||current=='!'||current=='<'||current=='>'){
		oprel=RationalOperator();
	}
}





int main(void){	// First version : Source code on standard input and assembly code on standard output
	// Header for gcc assembler / linker
	cout << "\t\t\t# This code was produced by the CERI Compiler"<<endl;
	cout << "\t.text\t\t# The following lines contain the program"<<endl;
	cout << "\t.globl main\t# The main function must be visible from outside"<<endl;
	cout << "main:\t\t\t# The main function body :"<<endl;
	cout << "\tmovq %rsp, %rbp\t# Save the position of the stack's top"<<endl;

	// Let's proceed to the analysis and code production
	ReadChar();
	ArithmeticExpression();
	ReadChar();
	// Trailer for the gcc assembler / linker
	cout << "\tmovq %rbp, %rsp\t\t# Restore the position of the stack's top"<<endl;
	cout << "\tret\t\t\t# Return from main function"<<endl;
	if(cin.get(current)){
		cerr <<"Caractères en trop à la fin du programme : ["<<current<<"]";
		Error("."); // unexpected characters at the end of program
	}

}
		
			





