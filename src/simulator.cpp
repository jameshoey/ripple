//============================================================================
// Name        : simulator.cpp
// Author      : James Hoey
// Version     :
// Copyright   : Copyright 2018
// Description : Implementation of the simulator - all features including the interface
//============================================================================

#include "simulator.h"
#include "linkedList.h"
#include "fileIO.h"

#include <iostream>
#include <vector>
#include <stack>
#include <stdlib.h>
#include <iterator>
#include <utility>
#include <tr1/unordered_map>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <stdio.h>
#include <string.h>
#include <set>
#include <stack>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iterator>
#include <utility>
#include <tr1/unordered_map>
#include <sstream>
#include <fstream>
#include <iomanip>

#include <time.h>

namespace std {

/*
 * General Constructor, creating an instance of the simulator and initialising all necessary variables
 * Does not load any file or environments, this will be performed later at the request of the user
 * @param
 * @return
 */
simulator::simulator() {
	vector<string> variables, stacknames; //variable and stack names
	varNames = variables;
	stackNames = stacknames;

	std::tr1::unordered_map<myPair,int,pair_hash> origGamma; //gamma - variable environment
	gamma = origGamma;

	std::tr1::unordered_map<int, int> origSigma; //sigma - data store
	sigma = origSigma;

	gs = {&gamma,&sigma}; //a pair of pointers to gamma and sigma - for more efficient parameter passing

	std::tr1::unordered_map<std::string,statement* > origBeta; //beta - while environment
	beta = origBeta;
	we = {&beta}; //a pointer for efficient parameter passing


	std::tr1::unordered_map<std::string,pair<std::string,statement*> > origMu; //mu - procedure environment
	mu = origMu;
	pe = {&mu}; //a pointer for efficient parameter passing

	std::tr1::unordered_map<myPair,std::string,pair_hash> procIden; //procedure identifiers
	procIdentifiers = procIden;
	pi = {&procIdentifiers}; //a pointer for efficient parameter passing

	std::tr1::unordered_map<std::string,set<std::string> > origOmega; //omega - scope environment
	scopeEnviron = origOmega;
	se = {&scopeEnviron}; //a pointer for efficient parameter passing

	std::tr1::unordered_map<std::string, stack<pair<int,int> > > origDelta; //delta - for stacks of ints
	std::tr1::unordered_map<std::string, stack<pair<int,std::string> > > origDeltaAI; //delta - for stacks of strings
	aux = origDelta;
	auxAI = origDeltaAI;
	as = {&aux,&auxAI}; //a pointer for efficient parameter passing

	//Update
	std::tr1::unordered_map<std::string, std::stack<std::stack<int> > > origAIinfo;
	AIinfo = origAIinfo;
	AI = {&AIinfo};
	//End of Update

	std::tr1::unordered_map<std::string,int > nextName1; //environment for storing current version numbers of construct identifiers
	nextNameFunction = nextName1;
	nextName = {&nextNameFunction}; //a pointer for efficient parameter passing

	program = NULL; //pointer to the program - linkedList
	fwdPointer = NULL; //pointer to the current position within the program
	placeInExecution = NULL; //pointer to the current position within the program
	placeInRevEx = NULL; //pointer to the corresponding position within the inverted version of the program

	int dir = 1; //integer indicating the current direction of execution - 1 = forwards, 0 = reverse
	direction = &dir; //pointer to this direction integer for efficient parameter passing

	pp = NULL; //pointer to the parser instance used to read the original version of the program

	recordTrace = true; //boolean indicating if the trace should or should not be recorded
	pointToTrace = new vector<string>(); //initially empty trace of forwards execution
	pointToInvTrace = new vector<string>(); //initially empty trace of reverse execution

	recordRules = true; //boolean indicating if to record the rules executed during forwards execution
	sequenceOfRules = new vector<string>(); //vector of these rule names

	recordRulesInv = true; //boolean indicating if to record the rules executed during reverse execution
	sequenceOfRulesInv = new vector<string>(); //vector of these rule names
}


//--------------------methods added for updated condition simulation


int stringToInt11(string s) { //convert a string to an integer
	return atoi(s.c_str());
}

bool isNumber(string s) {
    for (int i = 0; i < s.length(); i++)
        if (isdigit(s[i]) == false)
            return false;

    return true;
}


void displayTree(expTree *root) {

	if ((root->leftTree == NULL) && (root->rightTree == NULL)) {
		cout << root->element.op << " ";
	}
	else {
		displayTree(root->leftTree);
		cout << root->element.op << " ";
		displayTree(root->rightTree);
	}
}


bool typeOfOp(string op) {
	//return true if this is a boolean operator
	//false if arithmetic

	if ((op == "==") || (op == "!=") || (op == ">") || (op == ">=") || (op == "<") || (op == "<=") || (op == "&&") || (op == "||")) {
		return true;
	}
	else {
		return false;
	}
}


//--------------------end of update


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////// Helper Functions //////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


string intToString11(int i) { //convert an integer to a string
	stringstream ss;
	ss << i;
	string str = ss.str();
	return str;
}

void indent1(int number) { //output an indent - n number of empty spaces
	for (int i = 0; i < number; i++) {
		cout << "  ";
	}
}

void showIden1(stack<int> s) { //display a stack of integers - used to display a stack of identifiers, separated by commas
	while (!s.empty()) {
		if (s.size() == 1) {//last element, so no comma
			cout << s.top();
			s.pop();
		}
		else { //not the last element so include the comma
			cout << s.top() << ", ";
			s.pop();
		}
	}
}

set<statement*> combine(set<statement*> main, set<statement*> toAdd) { //combine two sets of statements pointers (used in getOptions)
	for (std::set<statement*>::iterator it=toAdd.begin(); it!=toAdd.end(); ++it) {
		main.insert(*it);
	}
	return main;
}

std::string reduceARenamedBlockName(string orig) { //get final part of renamed block name (giving the original block name, - the version number)
	string toReturn = orig;

	//remove the last ;
	toReturn.erase(toReturn.size());

	//find the last :
	int j = toReturn.find_last_of(":");
	toReturn.erase(0,j+1);

	//now remove the version of this blockname
	int i = toReturn.find_first_of(".");
	toReturn.erase(i);

	return toReturn;
}

string extractFirstID(string path) { //return the first element of a path
	int i = path.find_first_of(";");
	string toReturn = path.erase(i);
	return toReturn;
}

void linkEnd(linkedList *toLink, statement *toAdd) { //take a linked list of statements - traverse to the end (null pointer) and link to the given statement
	statement *temp = toLink->nextStatement();
	while (temp->next != NULL) {
		temp = temp->next;
	}
	temp->next = toAdd;
}

string stackOfIden(stack<int> iden) { //convert stack of identifiers to a string for printing
	string toReturn;
	int count = iden.size();
	while (!iden.empty()) {
		if (count == 1) {
			toReturn = toReturn + intToString11(iden.top());
			iden.pop();
		}
		else {
			toReturn = toReturn + intToString11(iden.top())+ "," ;
			iden.pop();
			count -= 1;
		}
	}
	return toReturn;
}

std::string reduceToMostDirectBlock(string orig) {
	//remove all preceeding call or construct names
	//and then remove the version number

	string toReturn = orig;

	int i = toReturn.find_first_of(";");
	toReturn.erase(i);


	return toReturn;
}

///UPDATED DISPLAY FUNCTION WITH INDENT

/*
 * When a program has been executed and replaced with skip, this function will output the identifiers alongside the word skip.
 * This is used for displaying programs, showing the use of identifiers at any point.
 */
void displayStatementIdentifiers1(statement *temp) { //uses the oldType to determine the type of statement
	if (temp->oldType == 0) {
		DA *i = static_cast<DA*>(temp);
		cout << " [";
		showIden1(i->idendifiers);
		cout << "]";
	}
	else if (temp->oldType == 1) {
		CA *i = static_cast<CA*>(temp);
		cout << " [";
		showIden1(i->idendifiers);
		cout << "]";
	}
	else if (temp->oldType == 2) {
		//If statement
		IfS *i = static_cast<IfS*>(temp);
		cout << " [";
		showIden1(i->idendifiers);
		cout << "]";
	}
	else if (temp->oldType == 3) {
		WlS *i = static_cast<WlS*>(temp);
		cout << " [";
		showIden1(i->idendifiers);
		cout << "]";
	}
	else if (temp->oldType == 4) {
		//par statement - so no identifiers to display
		cout << " []";
	}
	else if (temp->oldType == 5) {
		//block statement - so no identifiers to display
		cout << " []";
	}
	else if (temp->oldType == 6) {
		VdS *i = static_cast<VdS*>(temp);
		cout << " [";
		showIden1(i->idendifiers);
		cout << "]";
	}
	else if (temp->oldType == 7) {
		PdS *i = static_cast<PdS*>(temp);
		cout << " [";
		showIden1(i->idendifiers);
		cout << "]";
	}
	else if (temp->oldType == 8) {
		PcS *i = static_cast<PcS*>(temp);
		cout << " [";
		showIden1(i->idendifiers);
		cout << "]";
	}
	else if (temp->oldType == 9) {
		PrS *i = static_cast<PrS*>(temp);
		cout << " [";
		showIden1(i->idendifiers);
		cout << "]";
	}
	else if (temp->oldType == 10) {
		VrS *i = static_cast<VrS*>(temp);
		cout << " [";
		showIden1(i->idendifiers);
		cout << "]";
	}
	else if (temp->oldType == 11) {
		//skip statement cannot be the original type of statement
	}
}

/*
 * Function to display a program, starting from a single statement program, until we reach the stopper.
 * Also displays the position when we reach the statement current.
 * USed in environment displays - column indent for alignment and program indent to make the code readable
 * If we are printing a call, we must indent the first output slightly less than others (the size of extra)
 */
void programDisplayIndent(statement *program, statement *stopper, int columnIndent, int programIndent, statement *current,
		bool printingCall, string callName, bool first, int extra) {

	statement *temp = new statement;
	temp = program;

	//bool to indicate whether the current statement to display the current one to be executed
	bool foundCurrent=false;

//	cout << "Looking for Current type = " << current->type << "\n";
//	if (current->type == 0) {
//		DA *ff = static_cast<DA*>(current);
//		cout << ff->full << "\n";
//	}
//	else if (current->type == 1) {
//		CA *ff = static_cast<CA*>(current);
//		cout << ff->full << "\n";
//	}

	//set the indentation for the first iteration - should include the extra parameter
	int currentIndent=columnIndent;
	if (first) {
		currentIndent -= extra;
		first = false;
	}

	//now loop until the end of the program to display is reached
	while (temp != stopper) {

		//now indent the output to match the
		//cout << setw(columnIndent) << "| ";
		//cout << setw(currentIndent) << "| ";
		//indent1(programIndent);

		//check if this is the current statement, meaning we need to indicate this is the current position
		if (temp == current) {
			foundCurrent = true;
		}

		if (temp->type == 0) {
			DA *i = static_cast<DA*>(temp);
			cout << setw(currentIndent) << "| ";
			indent1(programIndent);
			cout << i->varName << " = " << i->newCondition;
			/*if (i->newVal != 0) { cout << i->newVal; }
			else {
				if (intToString11(i->newVal) != i->newValString) { cout << i->newValString; }
				else { cout << i->newVal; }
			}*/
			cout << " (" << i->path << ") [";
			showIden1(i->idendifiers);
			cout << "]";
			if (foundCurrent) {
				cout << " <-------- ";
				foundCurrent = false;
			}

			//if (i->last) {
			//	cout << "LAST\n";
			//}

			cout << "\n";
		}
		else if (temp->type == 1) {
			CA *i = static_cast<CA*>(temp);
			cout << setw(currentIndent) << "| ";
			indent1(programIndent);
			cout << i->varName;
			if (i->inc) { cout << " += "; }
			else { cout << " -= "; }

			/*if (i->newVal != 0) { cout << i->newVal; }
			else {
				if (intToString11(i->newVal) != i->newValString) { cout << i->newValString; }
				else { cout << i->newVal; }
			}*/

			cout << i->newCondition;
			cout << " (" << i->path << ") [";
			showIden1(i->idendifiers);
			cout << "]";
			if (foundCurrent) {
				cout << " <-------- ";
				foundCurrent = false;
			}


			cout << "\n";
		}
		else if (temp->type == 2) {
			IfS *i = static_cast<IfS*>(temp);
			cout << setw(currentIndent) << "| ";
			indent1(programIndent);
			//cout << "if " << i->condID << " (" << i->varName << " " << i->op << " " << i->newVal << ") then";
			cout << "if " << i->condID << " (" << i->newConditionString << ") then ";
			if (foundCurrent && (!i->seen)) {
				cout << " <-------- ";
				foundCurrent = false;
			}
			cout << "\n";

			programDisplayIndent(i->trueBranch->nextStatement(),i,columnIndent,programIndent+1,current,printingCall,callName,false,extra);
			cout << setw(columnIndent) << "| ";
			indent1(programIndent);
			cout  << "else\n";
			programDisplayIndent(i->falseBranch->nextStatement(),i,columnIndent,programIndent+1,current,printingCall,callName,false,extra);
			cout << setw(columnIndent) << "| ";
			indent1(programIndent);
			cout  << "fi (" << i->path << ") [";
			showIden1(i->idendifiers);
			cout << "]";
			if (foundCurrent && (i->seen)) {
				cout << " <-------- ";
				foundCurrent = false;
			}
			cout << "\n";
		}
		else if (temp->type == 3) {
			WlS *i = static_cast<WlS*>(temp);
			cout << setw(currentIndent) << "| ";
			indent1(programIndent);
			//cout << "while " << i->WID << " (" << i->varName << " " << i->op << " " << i->newVal << ")" << " do";
			cout << "while " << i->WID << " (" << i->newConditionString << ") do";
			if (foundCurrent) {
				cout << " <-------- ";
				foundCurrent = false;
			}
			cout << "\n";
			programDisplayIndent(i->loopBody->nextStatement(),i,columnIndent,programIndent+1,current,printingCall,callName,false,extra);

			cout << setw(columnIndent) << "| ";
			indent1(programIndent);
			cout << "elihw (" << i->path << ") [";
			showIden1(i->idendifiers);

			cout << "]\n";
		}
		else if (temp->type == 4) {
			Par *i = static_cast<Par*>(temp);
			cout << setw(currentIndent) << "| ";
			indent1(programIndent);
			cout << "par {\n";
			if (foundCurrent) {
				programDisplayIndent(i->leftSide->nextStatement(),i,columnIndent,programIndent+1,i->currLeft,printingCall,callName,false,extra);
			}
			else {
				programDisplayIndent(i->leftSide->nextStatement(),i,columnIndent,programIndent+1,current,printingCall,callName,false,extra);
			}
			cout << setw(columnIndent) << "| ";
			indent1(programIndent);
			cout << "}\n";
			cout << setw(columnIndent) << "| ";
			indent1(programIndent);
			cout << "{\n";
			if (foundCurrent) {
				programDisplayIndent(i->rightSide->nextStatement(),i,columnIndent,programIndent+1,i->currRight,printingCall,callName,false,extra);
			}
			else {
				programDisplayIndent(i->rightSide->nextStatement(),i,columnIndent,programIndent+1,current,printingCall,callName,false,extra);
			}
			cout << setw(columnIndent) << "| ";
			indent1(programIndent);
			cout << "}";
			if ((foundCurrent) && ((i->currLeft == i) || (i->currRight == i))) {
				cout << " <-------- ";
				foundCurrent = false;
			}
			else if (foundCurrent) {
				foundCurrent = false;
			}
			cout << "\n";
		}
		else if (temp->type == 5) {
			BlS *i = static_cast<BlS*>(temp);
			cout << setw(currentIndent) << "| ";
			indent1(programIndent);
			cout << "begin " << i->bid << " ";
			if (foundCurrent && (!i->seen)) {
				cout << " <-------- ";
				foundCurrent = false;
			}
			cout << "\n";

			programDisplayIndent(i->blockBody->nextStatement(),i,columnIndent,programIndent+1,current,printingCall,callName,false,extra);

			cout << setw(columnIndent) << "| ";
			indent1(programIndent);
			cout << "end [";
			showIden1(i->idendifiers);
			cout << "]";
			if (foundCurrent && (i->seen)) {
				cout << " <-------- ";
				foundCurrent = false;
			}
			cout << "\n";
		}
		else if (temp->type == 6) {
			VdS *i = static_cast<VdS*>(temp);
			cout << setw(currentIndent) << "| ";
			indent1(programIndent);
			cout << "var " << i->varName << " = " << i->value << " (" << i->path << ") [";
			showIden1(i->idendifiers);
			cout << "]";
			if (foundCurrent) {
				cout << " <-------- ";
				foundCurrent = false;
			}
			cout << "\n";
		}
		else if (temp->type == 7) {
			PdS *i = static_cast<PdS*>(temp);
			cout << setw(currentIndent) << "| ";
			indent1(programIndent);
			cout << "proc " << i->procIden << " " << i->procName << " is\n";
			programDisplayIndent(i->procBodyList->nextStatement(),NULL,columnIndent,programIndent+1,current,printingCall,callName,false,extra);

			cout << setw(columnIndent) << "| ";
			indent1(programIndent);
			cout << "corp (" << i->path << ") [";
			showIden1(i->idendifiers);
			cout << "]";
			if (foundCurrent) {
				cout << " <-------- ";
				foundCurrent = false;
			}
			cout << "\n";
		}
		else if (temp->type == 8) {
			PcS *i = static_cast<PcS*>(temp);

			if ((printingCall) && (i->callID == callName)) {
				return;
			}
			cout << setw(currentIndent) << "| ";
			indent1(programIndent);
			cout << "call " << i->callID << " " << i->procName << "(" << i->path << ") [";
			showIden1(i->idendifiers);
			cout << "]";
			if (foundCurrent) {
				cout << " <-------- ";
				foundCurrent = false;
			}
			cout << "\n";
		}
		else if (temp->type == 9) {
			PrS *i = static_cast<PrS*>(temp);
			cout << setw(currentIndent) << "| ";
			indent1(programIndent);
			cout << "remove proc " << i->procIden << " " << i->procName << " is\n";

			programDisplayIndent(i->procBodyList->nextStatement(),NULL,columnIndent,programIndent+1,current,printingCall,callName,false,extra);

			cout << setw(columnIndent) << "| ";
			indent1(programIndent);
			cout << "corp (" << i->path << ") [";
			showIden1(i->idendifiers);
			cout << "]";
			if (foundCurrent) {
				cout << " <-------- ";
				foundCurrent = false;
			}
			cout << "\n";
		}
		else if (temp->type == 10) {
			VrS *i = static_cast<VrS*>(temp);
			cout << setw(currentIndent) << "| ";
			indent1(programIndent);
			cout << "remove var " << i->varName << " = " << i->value << " (" << i->path << ") [";
			showIden1(i->idendifiers);
			cout << "]";
			if (foundCurrent) {
				cout << " <-------- ";
				foundCurrent = false;
			}
			cout << "\n";
		}
		else if (temp->type == 11) {
			cout << setw(currentIndent) << "| ";
			indent1(programIndent);
			cout << "skip";
			displayStatementIdentifiers1(temp);
			if (foundCurrent) {
				cout << " <-------- ";
				foundCurrent = false;
			}
			cout << "\n";
		}
		else if (temp->type == 12) {
			cout << setw(currentIndent) << "| ";
			indent1(programIndent);
			cout << "skip (placeholder)";
			displayStatementIdentifiers1(temp);
			if (foundCurrent) {
				cout << " <-------- ";
				foundCurrent = false;
			}
			cout << "\n";
		}
		else if (temp->type == 99) {
			cout << setw(currentIndent) << "| ";
			indent1(programIndent);
			cout << "abort";
			displayStatementIdentifiers1(temp);
			if (foundCurrent) {
				cout << " <-------- ";
				foundCurrent = false;
			}
			cout << "\n";
		}
		//finally, move the pointer onto the next statement
		temp = temp->next;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////// Forwards Renaming /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//remove the version from a given name - find the first occurrence of ., will split the version number
string removeVersionOfBlockName(string full) {
	string toReturn = full;
	int i = toReturn.find_first_of(".");
	return toReturn.erase(i);
}

//get the first block name from a given path - first element = most direct - split on ;
string getMostDirectBlockName(string full) {
	string toReturn = full;
	int i = toReturn.find_first_of(";");
	return toReturn.erase(i);
}

//take an old path, and replace all occurrences of oldBlockName with newBlockName
string updateSinglePath(string oldPath, string oldBlockName, string newBlockName) {
	string newPath = oldPath; //new path is largely identical to the old version

	int i = oldPath.find(oldBlockName); //find the beginning index of the old block name
	int start, end;

	if (i != string::npos) { //if there is an occurrence of the old block name

		if (i != 0) { //something prior to the beginning of what should be replaced - so step over it
			char c = oldPath.at(i-1);
			if (c == ';') { //if this block name is actually an occurrence - and not simply used in the name of another
				start = i;
				end = i + oldBlockName.size();
				string first = newPath.substr(0,start);
				string second = newPath.substr(end);
				return first + newBlockName + updateSinglePath(second,oldBlockName,newBlockName); //recursively modify after the occurrence
			}
			else { //path only contains this
				return newPath; //return old path (as newpath = old path)
			}
		}
		else { //starts with the block name
			start = i;
			end = i + oldBlockName.size();
			string first = newPath.substr(0,start);
			string second = newPath.substr(end);
			return first + newBlockName + updateSinglePath(second,oldBlockName,newBlockName);
		}
	} //not found - return old path (as newpath = old path)
	else {
		return newPath;
	}
}

//change all paths with a modified block name within a program (body)
void updatePathsWithBlockName(linkedList *body, statement *stopper, string oldBlockName, string newBlockName) {
	statement *temp = new statement;
	temp = body->nextStatement();

	while (temp != stopper) {
		if (temp->type == 0) {
			DA *origSt = static_cast<DA*>(temp);
			string s = updateSinglePath(origSt->path,oldBlockName,newBlockName);
			origSt->path = s;
		}
		else if (temp->type == 1) {
			CA *origSt = static_cast<CA*>(temp);
			string s = updateSinglePath(origSt->path,oldBlockName,newBlockName);
			origSt->path = s;
		}
		else if (temp->type == 2) {
			IfS *origSt = static_cast<IfS*>(temp);
			string s = updateSinglePath(origSt->path,oldBlockName,newBlockName);
			origSt->path = s;
			updatePathsWithBlockName(origSt->trueBranch,origSt,oldBlockName,newBlockName);
			updatePathsWithBlockName(origSt->falseBranch,origSt,oldBlockName,newBlockName);
		}
		else if (temp->type == 3) {
			WlS *origSt = static_cast<WlS*>(temp);
			string s = updateSinglePath(origSt->path,oldBlockName,newBlockName);
			origSt->path = s;
			updatePathsWithBlockName(origSt->loopBody,origSt,oldBlockName,newBlockName);
		}
		else if (temp->type == 4) {
			Par *origSt = static_cast<Par*>(temp);
			updatePathsWithBlockName(origSt->leftSide,origSt,oldBlockName,newBlockName);
			updatePathsWithBlockName(origSt->rightSide,origSt,oldBlockName,newBlockName);
		}
		else if (temp->type == 5) {
			//a nested block - will be renamed itself later on, so only alter path for the previous block name
			BlS *origSt = static_cast<BlS*>(temp);
			updatePathsWithBlockName(origSt->blockBody,origSt,oldBlockName,newBlockName);
		}
		else if (temp->type == 6) {
			VdS *origSt = static_cast<VdS*>(temp);
			string s = updateSinglePath(origSt->path,oldBlockName,newBlockName);
			origSt->path = s;
		}
		else if (temp->type == 7) {
			PdS *origSt = static_cast<PdS*>(temp);
			string s = updateSinglePath(origSt->path,oldBlockName,newBlockName);
			origSt->path = s;

			updatePathsWithBlockName(origSt->procBodyList,NULL,oldBlockName,newBlockName);
		}
		else if (temp->type == 8) {
			PcS *origSt = static_cast<PcS*>(temp);
			string s = updateSinglePath(origSt->path,oldBlockName,newBlockName);
			origSt->path = s;
		}
		else if (temp->type == 9) {
			PrS *origSt = static_cast<PrS*>(temp);
			string s = updateSinglePath(origSt->path,oldBlockName,newBlockName);
			origSt->path = s;

			updatePathsWithBlockName(origSt->procBodyList,NULL,oldBlockName,newBlockName);
		}
		else if (temp->type == 10) {
			VrS *origSt = static_cast<VrS*>(temp);
			string s = updateSinglePath(origSt->path,oldBlockName,newBlockName);
			origSt->path = s;
		}

		temp = temp->next;
	}
}

/*
 * Takes a construct name, and queries the structure containing the next version number with it.
 * This returns the next version of the given construct name, while also updates the entry on the structure maintaining correct behaviour
 */
string getNextVersion(string baseConstructName,nextNames nnf) {

	int nextVal = 0;

	//query the next names environment with the reduced construct name (as passed as parameter)
	std::tr1::unordered_map<std::string,int >::const_iterator got = nnf.nn->find(baseConstructName);
	if (got == nnf.nn->end()) { //error - construct name not found
		std::cout << "ERROR: trying to find the next version of missing construct identifier\n";
	}
	else { //we have the next version number
		nextVal = got->second;
	}

	//now update this value for future use
	((*nnf.nn)[baseConstructName]) = nextVal+1;

	//convert the next version number to a string
	string nextValString = intToString11(nextVal);

	//finally, return the whole construct name with updated version number
	return baseConstructName + "." + nextValString;
}

/*
 * Version of getNextVersion (above) that essentially peeks the next value, which does not update it (as it will not actually be used)
 */
string getNextVersionNotUpdate(string baseConstructName,nextNames nnf) {
	int nextVal = 0;

	std::tr1::unordered_map<std::string,int >::const_iterator got = nnf.nn->find(baseConstructName);
	if (got == nnf.nn->end()) {
		std::cout << "reduced name not found-3";
	}
	else {
		nextVal = got->second;
	}

	string nextValString = intToString11(nextVal);

	return baseConstructName + "." + nextValString;
}

void renameLoopHelperUpdated(statement *body, statement *stopper, nextNames nnf) {

	while (body != stopper) {

		if (body->type == 0) {
			//no change required for DA
		}
		else if (body->type == 1) {
			//no change required for CA
		}
		else if (body->type == 2) {
			IfS *origSt = static_cast<IfS*>(body); //cast the original if statement
			string oldIfName = origSt->condID;
			string baseConstructName = removeVersionOfBlockName(origSt->condID);
			string newIfName = getNextVersion(baseConstructName,nnf);
			origSt->condID = newIfName; //set the updated name of the if - using the next version number

			//now rename the branches of the conditionals
			renameLoopHelperUpdated(origSt->trueBranch->nextStatement(),origSt,nnf);
			renameLoopHelperUpdated(origSt->falseBranch->nextStatement(),origSt,nnf);
		}
		else if (body->type == 3) {
			WlS *origSt = static_cast<WlS*>(body); //cast the original while loop
			string oldLoopName = origSt->WID;
			string baseConstructName = removeVersionOfBlockName(origSt->WID);
			string newLoopName = getNextVersion(baseConstructName,nnf);
			origSt->WID = newLoopName; //update the while loop name to the next version

			//now update the rest of the loop body
			renameLoopHelperUpdated(origSt->loopBody->nextStatement(),origSt,nnf);
		}
		else if (body->type == 4) {
			Par *origSt = static_cast<Par*>(body); //cast the original parallel statement
			//no change made to the parallel itself, just to each side
			renameLoopHelperUpdated(origSt->leftSide->nextStatement(),origSt,nnf);
			renameLoopHelperUpdated(origSt->rightSide->nextStatement(),origSt,nnf);
		}
		else if (body->type == 5) {
			BlS *origSt = static_cast<BlS*>(body); //cast the original block statement
			string oldBlockName = origSt->bid;
			string baseConstructName = removeVersionOfBlockName(origSt->bid);
			string newBlockName = getNextVersion(baseConstructName,nnf);
			origSt->bid = newBlockName; //update the block to its new name, using the next version number

			//now reflect the change of this block name into all paths of its inner statements
			updatePathsWithBlockName(origSt->blockBody,origSt,oldBlockName,newBlockName);

			//now update any constructs in the block body
			renameLoopHelperUpdated(origSt->blockBody->nextStatement(),origSt,nnf);
		}
		else if (body->type == 6) {
			//no change required for VdS
		}
		else if (body->type == 7) {
			PdS *origSt = static_cast<PdS*>(body); //cast original version of the procedure declaration
			string baseConstructName = removeVersionOfBlockName(origSt->procIden);
			string newBlockName = getNextVersionNotUpdate(baseConstructName,nnf);
			origSt->procIden = newBlockName; //set the new name using the next version number

			renameLoopHelperUpdated(origSt->procBody,NULL,nnf);
		}
		else if (body->type == 8) {
			PcS *origSt = static_cast<PcS*>(body); // cast original call statement
			string baseConstructName = removeVersionOfBlockName(origSt->callID);
			string newBlockName = getNextVersion(baseConstructName,nnf);
			origSt->callID = newBlockName; //update the name to the next version
		}
		else if (body->type == 9) {
			PrS *origSt = static_cast<PrS*>(body); //cast original procedure removal
			string baseConstructName = removeVersionOfBlockName(origSt->procIden);
			string newBlockName = getNextVersion(baseConstructName,nnf);
			origSt->procIden = newBlockName; //update the name to the next version

			renameLoopHelperUpdated(origSt->procBody,NULL,nnf);
		}
		else if (body->type == 10) {
			//no change required for VrS
		}

		body = body->next;
	}
}

void renameLoopBodyUpdated(statement *body, statement *stopper, nextNames nnf) {
	//must be called on a while loop - first statement of body = 3
	if (body->type != 3) {
		cout << "Error - renameLoopBodyUpdated must be called on a while loop\n";
		return;
	}

	WlS *origSt = static_cast<WlS*>(body); //cast the original while loop
	string oldLoopName = origSt->WID;
	string baseConstructName = removeVersionOfBlockName(origSt->WID);
	//	string newLoopName = getNextVersion(baseConstructName,nnf);
	//	origSt->WID = newLoopName; //update the while loop name to the next version

	//now update the rest of the loop body
	renameLoopHelperUpdated(origSt->loopBody->nextStatement(),origSt,nnf);
}

void renameProcBody(statement* body, statement* stopper, string callID) {

	//cout << "IN RENAME PROC BODY----------------------------------------type = " << body->type << "\n";
	while (body != stopper) {
		//cout << "type = " << body->type << "\n";
		if (body->type == 2) {
			//cout << "body type is 2 starting here\n";
			IfS *temp = static_cast<IfS*>(body);
			string oldIfName = temp->condID;
			string newBlockNameHere = callID + ":" + oldIfName;
			temp->condID = newBlockNameHere;

			renameProcBody(temp->trueBranch->nextStatement(),temp,callID);
			renameProcBody(temp->falseBranch->nextStatement(),temp,callID);
		}

		else if (body->type == 3) {
			WlS *temp = static_cast<WlS*>(body);
			string oldIfName = temp->WID;
			string newBlockNameHere = callID + ":" + oldIfName;
			temp->WID = newBlockNameHere;

			renameProcBody(temp->loopBody->nextStatement(),temp,callID);
		}
		else if (body->type == 4) {
			Par *temp = static_cast<Par*>(body);
			renameProcBody(temp->leftSide->nextStatement(),temp,callID);
			renameProcBody(temp->rightSide->nextStatement(),temp,callID);
		}
		else if (body->type == 5) {
			BlS *temp = static_cast<BlS*>(body);
			string oldBlockName = temp->bid;
			string newBlockNameHere = callID + ":" + temp->bid;
			temp->bid = newBlockNameHere;

			updatePathsWithBlockName(temp->blockBody,temp,oldBlockName,newBlockNameHere);
			renameProcBody(temp->blockBody->nextStatement(),temp,callID);
		}
		else if (body->type == 7) {
			//cout << "im here\n";
			//proc dec
			PdS *temp = static_cast<PdS*>(body);
			temp->procIden = callID + ":" + temp->procIden;
			//renameProcBody(temp->procBodyList->nextStatement(),NULL,callID);
		}
		else if (body->type == 8) {
			PcS *temp = static_cast<PcS*>(body);

			//spot a recursive call
			if (temp->callID == callID) {
				//this is a recursive call
			}
			else {
				temp->callID = callID + ":" + temp->callID;
			}
		}
		else if (body->type == 9) {
			//cout << "im here\n";
			//proc dec
			PrS *temp = static_cast<PrS*>(body);
			temp->procIden = callID + ":" + temp->procIden;
			//renameProcBody(temp->procBodyList->nextStatement(),NULL,callID);
		}

		body = body->next;
	}

}




//can potentially remove this, identical to function defined above
void linkProcBodyToCall(linkedList *program, statement *toLink) {
	//link end of program to toLink
	statement *temp = new statement;
	temp = program->nextStatement();

	bool finished = false;

	while (!finished) {
		if (temp->next == NULL) {
			//found the end
			temp->next = toLink;
			finished = true;
			//	cout << "found the place to link\n";
		}
		else {
			temp = temp->next;
		}

	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////// Forwards Execution ////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//get the memory location that a variable local to a blockname is bound
int getLoc(std::string varName, std::string blockName, gammaSigma sg) {

	//cout << "getLoc - varname = " << varName << " blockname = " << blockName << "\n";

	//query gamma with the given variable and block name
	std::tr1::unordered_map<myPair,int,pair_hash>::const_iterator got = sg.gamma->find(make_pair(varName,blockName));
	if (got == sg.gamma->end()) {
		//this variable local to the given blockname does not exist - so return -1 indicating an error
		return -1;
	}
	else { //this specific version of a variable has been found - so return it
		int loc = got->second;
		return loc;
	}
}

//get the necessary block name to evaluate varName using the given path - and scope environment
std::string getB(std::string varName, std::string path, scopeEnvironment se) {

	if (path == "lam") { //if the path is lambda, this variable must be global
		return path;
	}

	//go through the path - checking if this block has a local version of the given varName
	bool allPathChecked = false; //flag to indicate we have reached the end of the path
	string nextBlockName = "", nextBlockName1 = "";

	int count = 0;

	while (!allPathChecked) { //while there is a still another block name within the path to check
		count += 1;

		nextBlockName1 = extractFirstID(path); //get this next block name
		nextBlockName = reduceARenamedBlockName(nextBlockName1); //remove the version number

//		cout << count << ": checking " << nextBlockName << ".\n";

		//now check this block name for a local version of the given variable
		std::tr1::unordered_map<std::string,set<std::string> >::const_iterator got = se.se->find(nextBlockName);
		if (got == se.se->end()) {
			cout << "Critical ERROR: getB - block name in path not found in scope environment\n";
		}
		else { //we have the scope information for this block name
			set<std::string> result = got->second;
			bool is_in = result.find(varName) != result.end(); //if this block has a local version of the given variable name
			if (is_in) {
				return nextBlockName1; //this is first match - this block name is used for evaluation - and return
			}
		}
		//only here if the return statement was not hit, so this block name is not the required one
		path = path.erase(0,nextBlockName1.size()+1); //so remove it
		if (path.size() == 0) {
			allPathChecked = true; //if path is now empty, variable must be global - stop the loop
		}
	}

	//if we get here - loop finished - so no match found - variable must be global
//	cout << "--------GOT HERE-------\n";
	return "lam";
}

//get the block name needed to evaluate a procedure name dependent on the given path - using the scope information
std::string getBUpdated(std::string name, std::string path, scopeEnvironment se) {

	if (path == "lam") { //if the path is lamdba - must be global so just return the path - won't actually be possible for procedures unless an error has occurred
		return path;
	}

	//go through the path - checking if this block has a local version of the given varName
	bool allPathChecked = false; //flag for when the path is empty
	string nextBlockName = "", nextBlockName1 = "";

	while (!allPathChecked) { //while the path still has another block name to check
		nextBlockName1 = extractFirstID(path); //extract the blockname
		nextBlockName = reduceARenamedBlockName(nextBlockName1); //remove the version number

		//query the scope environment to retrieve the set of local names this block uses
		std::tr1::unordered_map<std::string,set<std::string> >::const_iterator got = se.se->find(nextBlockName);
		if (got == se.se->end()) { //block not found in the scope environment
			std::cout << "Critical Error: Block name expected to have scope information, but doesn't!\n";
		}
		else { //scope information found
			set<std::string> result = got->second;
			bool is_in = result.find(name) != result.end(); //now check if this block has a local version of the given name
			if (is_in) {
				return nextBlockName1; //if yes, we have found the required block name
			}
		}

		//otherwise, this was not a match, so remove it and continue
		path = path.erase(0,nextBlockName1.size()+1);
		if (path.size() == 0) { //if the path is now empty - we finish the loop
			allPathChecked = true;
		}
	}

	//finally, loop finished without a match - name must be global
	return "lam";
}

///***************************************************************************************************
///*************************Update for new conditions*************************************************
///***************************************************************************************************

int evaluateArithTree(expTree *root, string path, scopeEnvironment se, gammaSigma sg) {

	if ((root->leftTree == NULL) && (root->rightTree == NULL)) {
		//this is where we may need to evaluate a variable name

		 if (isNumber(root->element.op.c_str())) {
			 //this is a leaf and contains a number
			 return stringToInt11(root->element.op);
		 }
		 else {
			 //this is not a number - so must be a variable name - must evaluate it accordingly
			//firstly, evaluate the variable name, first getting the necessary block name and then bound location
			std::string bnForEval = getB(root->element.op,path,se);
			int location = getLoc(root->element.op,bnForEval,sg);

			//get the current value of the variable in question
			int currVal;
			std::tr1::unordered_map<int,int >::const_iterator got2 = sg.sigma->find(location);
			if (got2 == sg.sigma->end()) { //location is not found in sigma
				std::cout << "Error: expected location" << location << "not found in sigma";
				return -1;
			}
			else { //we have the location required
				currVal = got2->second; //set currVal to the current value of the given location
				return currVal;
			}
		 }
	}

	if (root->element.op == "+") {
		return evaluateArithTree(root->leftTree,path,se,sg) + evaluateArithTree(root->rightTree,path,se,sg);
	}
	else if (root->element.op == "-") {
		return evaluateArithTree(root->leftTree,path,se,sg) - evaluateArithTree(root->rightTree,path,se,sg);
	}
	else {
		cout << "Error in evalArithTree\n";
		return -100;
	}
}

bool treeIsLeaf(expTree *root) {
	if ((root->leftTree == NULL) && (root->rightTree == NULL)) {
		return true;
	}
	return false;
}


bool evaluateBoolTreeUpdated(expTree *root, string path, scopeEnvironment se, gammaSigma sg) {

	//variables used to indicate the type of each child branch, and the result of evaluating each
	int leftI = -1, rightI = -1;
	bool leftB = false, rightB = false;

	//indicate which type was evaluated, allowing the correct return type to be used in the correct case
	bool leftWasB = false, rightWasB = false;

	//if each of the children of this node are leafs, they must be evaluated as arithmetic expressions
	if ((treeIsLeaf(root->leftTree)) && (treeIsLeaf(root->rightTree))) {
		leftI = evaluateArithTree(root->leftTree,path,se,sg);
		rightI = evaluateArithTree(root->rightTree,path,se,sg);
		leftWasB = false;
		rightWasB = false;
	}
	else { //both children are not leafs - one could be
		//evaluate each branch depending on its type - firstly the left branch

		if (typeOfOp(root->leftTree->element.op)) {
			leftB = evaluateBoolTreeUpdated(root->leftTree,path,se,sg);
			leftWasB = true;
		}
		else {
			leftI = evaluateArithTree(root->leftTree,path,se,sg);
			leftWasB = false;
		}

		//now evaluate the right branch
		if (typeOfOp(root->rightTree->element.op)) {
			rightB = evaluateBoolTreeUpdated(root->rightTree,path,se,sg);
			rightWasB = true;
		}
		else {
			rightI = evaluateArithTree(root->rightTree,path,se,sg);
			rightWasB = false;
		}
	}


	//finally, perform the necessary evaluation - depending on operator and certain values

	if (root->element.op == "==") {
		if ((leftWasB) && (rightWasB)) {
			return leftB == rightB;
		}
		else if ((!leftWasB) && (!rightWasB)) {
			return leftI == rightI;
		}
		else {
			cout << "some other ordering of left and right expressions\n";
			return false;
		}
	}

	else if (root->element.op == "!=") {
		if ((leftWasB) && (rightWasB)) {
			return leftB != rightB;
		}
		else if ((!leftWasB) && (!rightWasB)) {
			return leftI != rightI;
		}
		else {
			cout << "some other ordering of left and right expressions\n";
			return false;
		}
	}

	else if (root->element.op == ">") {
		if ((leftWasB) && (rightWasB)) {
			return leftB > rightB;
		}
		else if ((!leftWasB) && (!rightWasB)) {
			return leftI > rightI;
		}
		else {
			cout << "some other ordering of left and right expressions\n";
			return false;
		}
	}

	else if (root->element.op == ">=") {
		if ((leftWasB) && (rightWasB)) {
			return leftB >= rightB;
		}
		else if ((!leftWasB) && (!rightWasB)) {
			return leftI >= rightI;
		}
		else {
			cout << "some other ordering of left and right expressions\n";
			return false;
		}
	}

	else if (root->element.op == "<") {
		if ((leftWasB) && (rightWasB)) {
			return leftB < rightB;
		}
		else if ((!leftWasB) && (!rightWasB)) {
			return leftI < rightI;
		}
		else {
			cout << "some other ordering of left and right expressions\n";
			return false;
		}
	}

	else if (root->element.op == "<=") {
		if ((leftWasB) && (rightWasB)) {
			return leftB <= rightB;
		}
		else if ((!leftWasB) && (!rightWasB)) {
			return leftI <= rightI;
		}
		else {
			cout << "some other ordering of left and right expressions\n";
			return false;
		}
	}

	else if (root->element.op == "&&") {
		if ((leftWasB) && (rightWasB)) {
			return leftB && rightB;
		}
		else if ((!leftWasB) && (!rightWasB)) {
			return leftI && rightI;
		}
		else {
			cout << "some other ordering of left and right expressions\n";
			return false;
		}
	}

	else if (root->element.op == "||") {
		if ((leftWasB) && (rightWasB)) {
			return leftB || rightB;
		}
		else if ((!leftWasB) && (!rightWasB)) {
			return leftI || rightI;
		}
		else {
			cout << "some other ordering of left and right expressions\n";
			return false;
		}
	}

	else {
		cout << "hit the false branch meaning incorrect return value\n";
		return false;
	}
}




///***************************************************************************************************
///**************************END OF UPDATE************************************************************
///***************************************************************************************************


/*
 * Updated Commands for a non-atomic evaluation of a condition
 */

bool evalBool(expTree *left, expTree *right, string op) {

	if ((left->element.type == 0) && (right->element.type == 0)) {
		//eval bool expression containing numbers
		if (op == "==") {
			return left->element.resultInt == right->element.resultInt;
		}
		else if (op == "!=") {
			return left->element.resultInt != right->element.resultInt;
		}
		else if (op == "<") {
			return left->element.resultInt < right->element.resultInt;
		}
		else if (op == "<=") {
			return left->element.resultInt <= right->element.resultInt;
		}
		else if (op == ">") {
			return left->element.resultInt > right->element.resultInt;
		}
		else if (op == ">=") {
			return left->element.resultInt >= right->element.resultInt;
		}
		else {
			cout << "Incorrect type of operator for boolean expression containing integer values\n";
			return false;
		}
	}
	else if ((left->element.type == 1) && (right->element.type == 1)) {
		//eval bool expression containing bool values
		if (op == "==") {
			return left->element.resultBool == right->element.resultBool;
		}
		else if (op == "!=") {
			return left->element.resultBool != right->element.resultBool;
		}
		else if (op == "&&") {
			return left->element.resultBool && right->element.resultBool;
		}
		else if (op == "||") {
			return left->element.resultBool || right->element.resultBool;
		}
		else {
			cout << "Incorrect type of operator for boolean expression containing boolean values\n";
			return false;
		}
	}
	else {
		cout << "Types of children do not match\n";
		return false;
	}

}

int evalArith(int left, int right, string op) {
	if (op == "+") {
		return left + right;
	}
	else if (op == "-") {
		return left - right;
	}
	else {
		cout << "Incorrect operator in evalArith function\n";
		return -1;
	}
}

bool childrenEvaluated(expTree *root) {

	if (treeIsLeaf(root)) {
		return false;
	}

	return root->leftTree->element.evaluated && root->rightTree->element.evaluated;
}

void evalStepOfTree(expTree *root,string path, scopeEnvironment se, gammaSigma sg) {

	if (root->element.evaluated) {
		cout << "Incorrectly called step of tree - already finished\n";
		return;
	}

	//otherwise the main root has not yet been evaluated - so we must look further down

	if (childrenEvaluated(root)) {
		//then this root element must be the next to evaluate
		//must evaluate this step - based on the type of operator at this root
		if (typeOfOp(root->element.op)) {
			//true here means a boolean expression
			//how we evaluate is based on the types of the children (if both are arithmetic or both are boolean)
			bool result = evalBool(root->leftTree,root->rightTree,root->element.op);
			root->element.evaluated = true;
			root->element.type = 1;
			root->element.resultBool = result;
		}
		else {
			//false here means an arithmetic expression
			int result = evalArith(root->leftTree->element.resultInt,root->rightTree->element.resultInt,root->element.op);
			root->element.evaluated = true;
			root->element.type = 0;
			root->element.resultInt = result;
		}
	}
	else if (treeIsLeaf(root)) {
		//this is the final level of this part of the tree - and not yet evaluated so evaluate it
		//must be an arithmetic expression (value or variable name) - so evaluate it

		root->element.evaluated = true;

		if (isNumber(root->element.op)) {
			root->element.resultInt = stringToInt11(root->element.op);
		}
		else {
			 //this is not a number - so must be a variable name - must evaluate it accordingly
			//firstly, evaluate the variable name, first getting the necessary block name and then bound location
			std::string bnForEval = getB(root->element.op,path,se);
			int location = getLoc(root->element.op,bnForEval,sg);

			//get the current value of the variable in question
			int currVal;
			std::tr1::unordered_map<int,int >::const_iterator got2 = sg.sigma->find(location);
			if (got2 == sg.sigma->end()) { //location is not found in sigma
				std::cout << "Error: expected location not found in sigma";
			}
			else { //we have the location required
				currVal = got2->second; //set currVal to the current value of the given location
				root->element.resultInt = currVal;
			}
		}

		root->element.type = 0;
	}
	else {
		//not a leaf, not yet evaluated, children (at least one) not yet evaluated
		//so make a step of either the left or right child (depending on if one is already finished
		if (root->leftTree->element.evaluated == false) {
			evalStepOfTree(root->leftTree,path,se,sg);
		}
		else if (root->rightTree->element.evaluated == false) {
			evalStepOfTree(root->rightTree,path,se,sg);
		}
		else {
			cout << "Children evaluated returned false but both appear to have already been evaluated\n";
			return;
		}
	}


}

void resetTree(expTree *root) {
	//reset all evaluated booleans to false - in case this condition is to be evaluated again
	root->element.evaluated = false;

	if (!treeIsLeaf(root)) {
		resetTree(root->leftTree);
		resetTree(root->rightTree);
	}
}


void displayTreeWithEval(expTree *root) {
	if (treeIsLeaf(root)) {
		cout << " " << root->element.evaluated << ":" << root->element.op << " ";
	}
	else {
		cout << root->element.evaluated << ":(";
		displayTreeWithEval(root->leftTree);
		cout << " " << root->element.op << " ";
		displayTreeWithEval(root->rightTree);
		cout << ") ";
	}
}


/*
 * End of updated commands for non-atomic evaluation
 */

bool simConditionUpdated(expTree *conditionTree, string path, scopeEnvironment se, gammaSigma sg) {
	return evaluateBoolTreeUpdated(conditionTree,path,se,sg);
}

//simulate a condition - conditional or loop - of the form varName op newVal
bool simCondition(string varName, string blockName, string path, string op, int newVal, auxStore ar, scopeEnvironment se, gammaSigma sg) {

	//firstly, evaluate the variable name, first getting the necessary block name and then bound location
	std::string bnForEval = getB(varName,path,se);
	int location = getLoc(varName,bnForEval,sg);

	//get the current value of the variable in question
	int currVal;
	std::tr1::unordered_map<int,int >::const_iterator got2 = sg.sigma->find(location);
	if (got2 == sg.sigma->end()) { //location is not found in sigma
		std::cout << "Error: expected location not found in sigma";
	}
	else { //we have the location required
		currVal = got2->second; //set currVal to the current value of the given location
	}

	//now perform the evaluation - dependent on the type of operator
	if (op == ">") {
		return currVal > newVal;
	}
	else if (op == "<") {
		return currVal < newVal;
	}
	else if (op == ">=") {
		return currVal >= newVal;
	}
	else if (op == "<=") {
		return currVal <= newVal;
	}
	else if (op == "==") {
		//cout << "checking if " << currVal << " == " << newVal << ": Result = " << (currVal == newVal) << "\n";
		return currVal == newVal;
	}
	else {
		cout << "Invalid operator to simConEval - " << op << "\n";
		return NULL;
	}
}

/*
 * Given a statement that begins a program, this function creates a copy of each statement, linked into the provided linkedlist copy.
 * stopper is the statement we will reach that indicates the program is complete (since original program will be linked  eg. while loop body linked to the loop statement)
 */
void createVersionStatement(statement *temp, linkedList *copy, statement *stopper, Par *parParent, int left) {

	while (temp != stopper) {

		if (temp->type == 0) {
			DA *origSt = static_cast<DA*>(temp);
			//(*copy).addDAPar(0, origSt->full, origSt->varName, origSt->newValString, origSt->newVal, origSt->blockName, origSt->original, origSt->path, parParent, left);
			(*copy).addDAParUpdated(0, origSt->full, origSt->varName, origSt->blockName, origSt->newCondition, origSt->newCondTree, origSt->original, origSt->path, parParent, left, origSt->last);
		}
		else if (temp->type == 1) {
			CA *origSt = static_cast<CA*>(temp);
			//(*copy).addCAPar(1, origSt->full, origSt->varName, origSt->newValString, origSt->newVal, origSt->blockName, origSt->inc,origSt,origSt->path,parParent,left);
			(*copy).addCAParUpdated(1, origSt->full, origSt->varName, origSt->blockName, origSt->newCondition, origSt->newCondTree, origSt->inc,origSt,origSt->path,parParent,left, origSt->last);
		}
		else if (temp->type == 2) {
			IfS *origIf = static_cast<IfS*>(temp);
			IfS *copyIf = new IfS;

			linkedList *trueCopy = new linkedList;
			linkedList *falseCopy = new linkedList;

			createVersionStatement(origIf->trueBranch->nextStatement(),trueCopy,origIf,parParent,left);
			createVersionStatement(origIf->falseBranch->nextStatement(),falseCopy,origIf,parParent,left);

			linkEnd(trueCopy,copyIf);
			linkEnd(falseCopy,copyIf);

			copyIf->type = 2;
			copyIf->oldType = 2;
			copyIf->blockName = origIf->blockName;
			copyIf->choice = -1;
			copyIf->condID = origIf->condID;
			copyIf->full = origIf->full;
			copyIf->idendifiers = stack<int>();
			copyIf->left = left;
			copyIf->parentPar = parParent;
	//		copyIf->newVal = origIf->newVal;
			copyIf->next = NULL;
	//		copyIf->op = origIf->op;
			copyIf->original = origIf->original;
			copyIf->path = origIf->path;
			copyIf->seen = false;
	//		copyIf->varName = origIf->varName;
			copyIf->trueBranch = trueCopy;
			copyIf->falseBranch = falseCopy;

			copyIf->newConditionString = origIf->newConditionString;
			copyIf->newCondition = origIf->newCondition;
			copyIf->last = origIf->last;

			(*copy).addStatement(copyIf);
		}
		else if (temp->type == 3) {
			//make a copy of the while loop and add it to the list copy
			WlS *origLoop = static_cast<WlS*>(temp);

			WlS *loopCopy = new WlS;
			linkedList *loopBodyCopy = new linkedList;

			createVersionStatement(origLoop->loopBody->nextStatement(),loopBodyCopy,origLoop,parParent,left);

			linkEnd(loopBodyCopy,loopCopy);

			loopCopy->type = 3;
			loopCopy->oldType = 3;
			loopCopy->WID = origLoop->WID;
			loopCopy->alreadyStarted = false;
			loopCopy->blockName = origLoop->blockName;
			loopCopy->full = origLoop->full;
			loopCopy->idendifiers = stack<int>();
			loopCopy->left = left;
//			loopCopy->newVal = origLoop->newVal;
//			loopCopy->op = origLoop->op;
			loopCopy->parentPar = parParent;
			loopCopy->path = origLoop->path;
			loopCopy->saveNext = false;
//			loopCopy->varName = origLoop->varName;
			loopCopy->loopBody = loopBodyCopy;
			loopCopy->next = NULL;
			loopCopy->last = origLoop->last;

			loopCopy->newConditionString = origLoop->newConditionString;
			loopCopy->newCondition = origLoop->newCondition;

			(*copy).addStatement(loopCopy);
			//cout << "loop is done\n";
		}
		else if (temp->type == 4) {
			Par *origPar = static_cast<Par*>(temp);
			Par *parCopy = new Par;

			linkedList *leftCopy = new linkedList;
			linkedList *rightCopy = new linkedList;

			createVersionStatement(origPar->leftSide->nextStatement(),leftCopy,origPar,parCopy,1);
			createVersionStatement(origPar->rightSide->nextStatement(),rightCopy,origPar,parCopy,0);

			linkEnd(leftCopy,parCopy);
			linkEnd(rightCopy,parCopy);

			parCopy->type = 4;
			parCopy->oldType = 4;
			parCopy->left = left;
			parCopy->parentPar = parParent;
			parCopy->seenLeft = false;
			parCopy->seenRight = false;
			parCopy->leftSide = leftCopy;
			parCopy->currLeft = parCopy->leftSide->nextStatement();
			parCopy->rightSide = rightCopy;
			parCopy->currRight = parCopy->rightSide->nextStatement();
			parCopy->next = NULL;
			parCopy->optionChecked = false;
			parCopy->last = origPar->last;
			(*copy).addStatement(parCopy);
		}
		else if (temp->type == 5) {
			BlS *origBlock = static_cast<BlS*>(temp);
			BlS *blockCopy = new BlS;

			linkedList *bodyCopy = new linkedList;

			createVersionStatement(origBlock->blockBody->nextStatement(),bodyCopy,origBlock,parParent,left);
			linkEnd(bodyCopy,blockCopy);

			blockCopy->type = 5;
			blockCopy->oldType = 5;
			blockCopy->bid = origBlock->bid;
			blockCopy->full = origBlock->full;
			blockCopy->idendifiers = stack<int>();
			blockCopy->next = NULL;
			blockCopy->original = NULL;
			blockCopy->originalBlockName = origBlock->originalBlockName;
			blockCopy->seen = false;
			blockCopy->blockBody = bodyCopy;
			blockCopy->left = left;
			blockCopy->parentPar = parParent;

			blockCopy->last = origBlock->last;

			(*copy).addStatement(blockCopy);
		}
		else if (temp->type == 6) {
			VdS *origSt = static_cast<VdS*>(temp);
			(*copy).addLVDPar(6, origSt->full, origSt->varName, origSt->value, origSt->blockName, origSt->original, origSt->path, parParent,left, origSt->last);
		}
		else if (temp->type == 7) {
			PdS *origProc = static_cast<PdS*>(temp);
			PdS *copyProc = new PdS;

			linkedList *procBodyCopy = new linkedList;

			createVersionStatement(origProc->procBodyList->nextStatement(),procBodyCopy,NULL,parParent,left);

			copyProc->type = 7;
			copyProc->oldType = 7;
			copyProc->blockName = origProc->blockName;
			copyProc->full = origProc->full;
			copyProc->idendifiers = stack<int>();
			copyProc->left = left;
			copyProc->next = NULL;
			copyProc->original = NULL;
			copyProc->parentPar = parParent;
			copyProc->path = origProc->path;
			copyProc->procIden = origProc->procIden;
			copyProc->procName = origProc->procName;
			copyProc->seen = false;
			copyProc->procBodyList = procBodyCopy;
			copyProc->procBody = copyProc->procBodyList->nextStatement();

			copyProc->last = origProc->last;

			(*copy).addStatement(copyProc);
		}
		else if (temp->type == 8) {
			PcS *origSt = static_cast<PcS*>(temp);
			//(*copy).addPCPar(8, origSt->full, origSt->callID, origSt->procName, origSt->blockName, origSt->original, origSt->path, origSt->parentPar, origSt->left);
			(*copy).addPCPar(8, origSt->full, origSt->callID, origSt->procName, origSt->blockName, origSt->original, origSt->path, parParent, left, origSt->last);
		}
		else if (temp->type == 9) {
			PrS *origProc = static_cast<PrS*>(temp);
			PrS *copyProc = new PrS;

			linkedList *procBodyCopy = new linkedList;

			createVersionStatement(origProc->procBodyList->nextStatement(),procBodyCopy,NULL,parParent,left);

			copyProc->type = 9;
			copyProc->oldType = 9;
			copyProc->blockName = origProc->blockName;
			copyProc->full = origProc->full;
			copyProc->idendifiers = stack<int>();
			copyProc->left = left;
			copyProc->next = NULL;
			copyProc->original = NULL;
			copyProc->parentPar = parParent;
			copyProc->path = origProc->path;
			copyProc->procIden = origProc->procIden;
			copyProc->procName = origProc->procName;
			copyProc->seen = false;
			copyProc->procBodyList = procBodyCopy;
			copyProc->procBody = copyProc->procBodyList->nextStatement();

			copyProc->last = origProc->last;

			(*copy).addStatement(copyProc);
		}
		else if (temp->type == 10) {
			VrS *origSt = static_cast<VrS*>(temp);
			(*copy).addLVRPar(10, origSt->full, origSt->varName, origSt->value, origSt->blockName, origSt->original, origSt->path, parParent, left, origSt->last);
		}
		else if (temp->type == 11) {
			cout << "UPDATE CREATEVERSIONSTATEMENT FOR STATEMENTS OF TYPE 11\n";
		}
		else if (temp->type == 12) {
			PH *origSt = static_cast<PH*>(temp);
			PH *copySt = new PH;

			copySt->type = 12;
			copySt->oldType = 12;
			copySt->parentPar = parParent;
			copySt->left = left;
			copySt->next = NULL;

			(*copy).addStatement(copySt);

		}
		else if (temp->type == 99) {
			Abort *origSt = static_cast<Abort*>(temp);
			Abort *copySt = new Abort;

			copySt->type = 99;
			copySt->oldType = 99;
			copySt->parentPar = parParent;
			copySt->left = left;
			copySt->next = NULL;

			(*copy).addStatement(copySt);
		}

		//finally, move the pointer on to the next statement
		temp = temp->next;
	}
}

/*
 * Before we remove a copy of a while loop or a procedure body, we must save all of the identifiers from that program otherwise they will be lost.
 * Go through the program beginning with the given statement start (until we reach the stopper), pushing all identifiers for each statement into a
 * string that will be saved onto the auxiliary store. Each group of identifiers are separated using | (meaning we can successfully capture the
 * structure of the identifiers (as well as the values).
 */
string saveAnnotationInformationWhile(statement* start, statement* stopper) {
	string toReturn = "";
	statement *temp = new statement;
	temp = start;

	while (temp != stopper) {

		if (temp->type == 0) {
			DA *i = static_cast<DA*>(temp);
			string iden = stackOfIden(i->idendifiers);
			toReturn = iden + " | " + toReturn;
			temp=temp->next;
		}
		else if (temp->type == 1) {
			CA *i = static_cast<CA*>(temp);
			string iden = stackOfIden(i->idendifiers);

			toReturn = iden + " | " + toReturn;
			temp=temp->next;
		}
		else if (temp->type == 2) {
			IfS *i = static_cast<IfS*>(temp);
			string iden = stackOfIden(i->idendifiers);
			string idsOfTrue = saveAnnotationInformationWhile(i->trueBranch->nextStatement(),i);
			string idsOfFalse = saveAnnotationInformationWhile(i->falseBranch->nextStatement(),i);
			toReturn = iden + " | " + idsOfTrue + idsOfFalse + toReturn;
			temp=temp->next;
		}
		else if (temp->type == 3) {
			//while loop
			WlS *i = static_cast<WlS*>(temp);
			string iden = stackOfIden(i->idendifiers);

			string idOfBody = saveAnnotationInformationWhile(i->loopBody->nextStatement(),i);
			toReturn = idOfBody + " | " + toReturn;

			temp=temp->next;
		}
		else if (temp->type == 4) {
			Par *i = static_cast<Par*>(temp);

			string idOfLeft = saveAnnotationInformationWhile(i->leftSide->nextStatement(),i);
			string idOfRight = saveAnnotationInformationWhile(i->rightSide->nextStatement(),i);

			toReturn = idOfLeft + idOfRight + toReturn;

			temp = temp->next;
		}
		else if (temp->type == 5) {
			BlS *i = static_cast<BlS*>(temp);
			string idOfBody = saveAnnotationInformationWhile(i->blockBody->nextStatement(),i);
			toReturn = idOfBody +  toReturn;
			temp = temp->next;
		}
		else if (temp->type == 6) {
			VdS *i = static_cast<VdS*>(temp);
			string iden = stackOfIden(i->idendifiers);
			toReturn = iden + " | " + toReturn;
			temp=temp->next;
		}
		else if (temp->type == 7) {
			PdS *i = static_cast<PdS*>(temp);
			string iden = stackOfIden(i->idendifiers);
			toReturn = iden + " | " + toReturn;
			temp=temp->next;
		}
		else if (temp->type == 8) {
			PcS *i = static_cast<PcS*>(temp);
			string iden = stackOfIden(i->idendifiers);
			toReturn = iden + " | " + toReturn;
			temp=temp->next;
		}
		else if (temp->type == 9) {
			PrS *i = static_cast<PrS*>(temp);
			string iden = stackOfIden(i->idendifiers);
			toReturn = iden + " | " + toReturn;
			temp=temp->next;
		}
		else if (temp->type == 10) {
			VrS *i = static_cast<VrS*>(temp);
			string iden = stackOfIden(i->idendifiers);
			toReturn = iden + " | " + toReturn;
			temp=temp->next;
		}
		else if (temp->type == 11) {
			cout << "type 11 is saveAnnInfo function\n";
		}
		else if (temp->type == 12) {
			cout << "type 12 in saveAnnInfo function-1\n";
			temp = temp->next;

		}
	}
	return toReturn;
}

//evaluate a procedure to return the unique procedure identifier
std::string evalP(std::string procNameInCode, std::string path, scopeEnvironment se, procedureIdentifiers pi) {

	//get the block name in which this procedure was declared - using the path and the scope environment
	string bnForEval = getBUpdated(procNameInCode,path,se);

	//get the unique procedure identifier for this base mapping
	std::string procIdentifier;

	//now query the procedure environment using the given procedure name and
	std::tr1::unordered_map<myPair,std::string,pair_hash> ::iterator itFind = (*pi.pi).find(make_pair(procNameInCode,bnForEval));
	if (itFind == (*pi.pi).end()) {
		std::cout << "Crictial Error: procedure mapping expected here but not found";
	}
	else { //the expected mapping has been found
		procIdentifier = (itFind->second);
		return procIdentifier; //return the unique procedure identifier
	}

	//if we get here - an error has occurred
	return "ERROR: PROC IDENTIFIER NOT FOUND CORRECTLY\n";
}

/*
 * query the procedure environment to find the procedure body mapped to the given procedure name and dependent on
 * the given path. Used at the beginning of a call to retrieve the basis mapping from which a version is created
 */
pair<std::string,statement*> queryPE(std::string procName, procedureEnvironment mu, string path, scopeEnvironment se, procedureIdentifiers pi) {

	//must find the appropriate Pn for procName and path
	std::string procIdenifier = evalP(procName,path,se,pi);

	//cout << "evalP - procName = " << procName << " path = " << path << " returns procIdentifier = " << procIdenifier << "\n";

	//now return the pair that is mapped to this given procedure identifier from the procedure environment
	pair<std::string,statement*> toReturn;

	//query the procedure environment
	std::tr1::unordered_map<std::string,pair<std::string,statement*> >::const_iterator got = mu.pe->find(procIdenifier);
	if (got == mu.pe->end()) {
		cout << "Critical Error: procedure mapping expected but not found\n";
		return toReturn; //return the empty pair
	}
	else { //otherwise, the mapping has been found, so return it
		return got->second;
	}

	//this return statement shouldn't actually be reached
	return toReturn;
}

string nextPartOfInterleavingSequence(string *original) {
	string toReturn = "";

	int i = original->find_first_of("|");
	if (i == -1) {
		//last element
		string ret = *original;
		*original = "";
		return ret;
	}
	else {
		toReturn = original->substr(0,i);
		original->erase(0,i+1);
	}

	return toReturn;
}

stack<int> makeStackOfInterleavingPath(string path) {

	stack<int> toReturn;

	while (path != "") {
		int i = path.find_first_of("-");
		if (i == -1) {
			//last element
			string nextEl = path;
			if (nextEl == "L") {
				toReturn.push(1);
			}
			else {
				toReturn.push(0);
			}
			path = "";
		}
		else {
			string nextEl = path.substr(0,i);
			if (nextEl == "L") {
				toReturn.push(1);
			}
			else {
				toReturn.push(0);
			}
			path = path.erase(0,i+1);
		}

	}

	return toReturn;
}

/*
 * Get the set of available actions that could be the next step of the execution.
 * @param current The current position within the code
 * @return The set containing all of the next available steps of execution
 */
set<statement*> getOptions(statement *current) {
	set<statement*> choices;

	if (current == NULL) { //if the current statement is null, no options exist - return the empty set
		return choices;
	}

	if (current->type == 4) { //parallel statement - need to look deeper
		Par *ap = static_cast<Par*>(current);

		if (ap->currLeft == current) { //left is finished, so show no option
			choices.insert(current);
		}
		else if (ap->currLeft == NULL){ //don't insert this null option
		}
		else {
			choices = combine(choices,getOptions(ap->currLeft));
		}

		if (ap->currRight == current) { //right is finshed, so show no option
			choices.insert(current);
		}
		else if (ap->currRight == NULL) { //don't insert this null option
		}
		else {
			choices = combine(choices,getOptions(ap->currRight));
		}
	}
	else if ((current->type >= 0) && (current->type < 4)) { //if type = 0,1,2 or 3
		choices.insert(current);
	}
	else if ((current->type >= 5) && (current->type < 14)) { //if type = 5,6,7,8,9,10,11,12,13
		choices.insert(current);
	}
	else if (current->type == 99) {
		choices.insert(current);
	}
	else {
		cout << "ABORT ERROR - a possible next step of the simulation is of an invalid type " << current->type << "\n";
	}

	//finally, return the set of all possible options
	return choices;
}

/*
 * In order to display a possible choice to the user, we must be able to get a string representing a statement
 * Will not necessarily be the whole statement output - enough to distiguish it from other statements of the same type (e.g unique identifier of constructs)
 * Skip statement will display as skip - with original statement enclosed with $ - allowing the choice to be correct
 */
string stringOfSingleOption(statement *choice) {
	string toReturn = "";

	if (choice->type == 0) {
		DA *i = static_cast<DA*>(choice);
		toReturn = i->varName + " = " + i->newCondition;

		/*if (i->newVal != 0) { toReturn += intToString11(i->newVal); }
		else {
			if (intToString11(i->newVal) != i->newValString) { toReturn += intToString11(i->newVal); }
			else { toReturn += intToString11(i->newVal); }
		}*/
		toReturn = toReturn +  " (" + i->path + ")";
	}
	else if (choice->type == 1) {
		CA *i = static_cast<CA*>(choice);
		toReturn = i->varName;
		if (i->inc) { toReturn += " += "; }
		else { toReturn += " -= "; }

		/*if (i->newVal != 0) { toReturn += intToString11(i->newVal); }
		else {
			if (intToString11(i->newVal) != i->newValString) { toReturn += intToString11(i->newVal); }
			else { toReturn += intToString11(i->newVal); }
		}*/

		toReturn = toReturn + i->newCondition + " (" + i->path + ")";
	}
	else if (choice->type == 2) {
		IfS *temp = static_cast<IfS*>(choice);
		toReturn = "if " + temp->condID;
		if (temp->seen) { toReturn += " (close)"; }
	}
	else if (choice->type == 3) {
		WlS *temp = static_cast<WlS*>(choice);
		toReturn = "while " + temp->WID + " (" + temp->newConditionString + ")";
	}
	else if (choice->type == 4) {
		Par *temp = static_cast<Par*>(choice);
		toReturn = "par";
		if (temp->seenLeft || temp->seenRight) { toReturn += " (close)"; }
		else { toReturn += " (open)";}
	}
	else if (choice->type == 5) {
		BlS *temp = static_cast<BlS*>(choice);
		toReturn = "begin " + temp->bid;
		if (temp->seen) { toReturn += " (close)"; }
		else { toReturn += " (open)";}
	}
	else if (choice->type == 6) {
		VdS *temp = static_cast<VdS*>(choice);
		toReturn = "var " + temp->varName + " = " + intToString11(temp->value) + " (" + temp->path + ");";
	}
	else if (choice->type == 7) {
		PdS *temp = static_cast<PdS*>(choice);
		toReturn = "proc " + temp->procIden;
	}
	else if (choice->type == 8) {
		PcS *temp = static_cast<PcS*>(choice);
		toReturn = "call " + temp->callID;
	}
	else if (choice->type == 9) {
		PrS *temp = static_cast<PrS*>(choice);
		toReturn = "remove proc " + temp->procIden;
	}
	else if (choice->type == 10) {
		VrS *temp = static_cast<VrS*>(choice);
		toReturn = "remove var " + temp->varName + " = " + intToString11(temp->value) + " (" + temp->path + ");";
	}
	else if (choice->type == 11) {
		toReturn = "skip $";
		choice->type = choice->oldType;
		toReturn = toReturn + stringOfSingleOption(choice) + "$";
		choice->type = 11;
	}
	return toReturn;
}

/*
 * For the trace of both executions, we need to display all possible options at a particular point. Given a set of statements that could be the next statement
 * to be executed, we need to display them all (using the function above).
 */
string stringOfOptions(set<statement*> choices) {
	string toReturn = "[ "; //open square brackets
	int count = 0;

	for (std::set<statement*>::iterator it=choices.begin(); it!=choices.end(); ++it) { //iterate through a set of statement pointers (the options)
		if (count == (choices.size()-1)) { //last element, so no | required
			string state = stringOfSingleOption(*it);
			toReturn = toReturn + state + "";
		}
		else { //this is a middle element, so include |
			string state = stringOfSingleOption(*it);
			toReturn = toReturn + state + " | ";
		}
		count++;
	}

	toReturn = toReturn + " ]"; //close square brackets
	return toReturn;
}

/*
 * Take a set of possible next statements, assign a number number to each (in ascending order) - allowing the user to make a choice
 */
std::tr1::unordered_map<int,statement* > displayNumberedOptions(set<statement*> choices) {
	//no order in a set, so cannot use order to map this choice
	int count = 1;
	std::tr1::unordered_map<int,statement* > toReturn;

	for (std::set<statement*>::iterator it=choices.begin(); it!=choices.end(); ++it) {
		toReturn[count] = *it;
		count += 1;
	}

	return toReturn;
}

//function used in force sequence only
bool getLeft(statement *temp) {
	if (temp->type == 0) {
		DA *t = static_cast<DA*>(temp);
		if (t->left == 1) {
			return true;
		}
		else {
			return false;
		}
	}
	else if (temp->type == 1) {
		CA *t = static_cast<CA*>(temp);
		if (t->left == 1) {
			return true;
		}
		else {
			return false;
		}
	}
	else if (temp->type == 4) {
		Par *t = static_cast<Par*>(temp);
		if (t->left == 1) {
			return true;
		}
		else {
			return false;
		}
	}
	else {
		cout << "UPDATE GETLEFT FOR STATEMENTS OF TYPE " << temp->type << "\n";
		return false;
	}
}

statement* checkPar(statement *temp) {
	if (temp->type == 0) {
		DA *t = static_cast<DA*>(temp);
		return t->parentPar;
	}
	else if (temp->type == 1) {
		CA *t = static_cast<CA*>(temp);
		return t->parentPar;
	}
	else if (temp->type == 4) {
		Par *t = static_cast<Par*>(temp);
		return t->parentPar;
	}
	else {
		cout << "UPDATE checkPar FOR STATEMENTS OF TYPE " << temp->type << "\n";
		return NULL;
	}
}

/*
 * Display a single statement without positioning arrows
 */
string displayStatement(statement *print, bool forwards) {
	if (print->type == 0) {
		DA *i = static_cast<DA*>(print);
		string s = i->varName + " = " + i->newCondition;
		s = s + " (" + i->path + ")";
		if (forwards) {
			if (i->newCondTree->element.evaluated == false) {
				s += "(expression evaluation)";
			}
		}

		return s;
	}
	else if (print->type == 1) {
		CA *i = static_cast<CA*>(print);
		string s = i->varName;
		if (i->inc) { s = s + " += "; }
		else { s = s + " -= "; }

		s = s + i->newCondition + " (" + i->path + ")";

		if (forwards) {
			if (i->newCondTree->element.evaluated == false) {
				s += "(expression evaluation)";
			}
		}

		return s;
	}
	else if (print->type == 2) {
		IfS *temp = static_cast<IfS*>(print);

		string s = "if " + temp->condID;

		if (temp->seen) {
			s += " (close)";
		}
		else {
			if (forwards) {
				if (temp->newCondition->element.evaluated == false) {
					s += " (expression evaluation)";
				}
			}

			s += " (open)";
		}

		return s;
	}
	else if (print->type == 3) {
		WlS *temp = static_cast<WlS*>(print);

		string s = "";
		s += "while " + temp->WID;

		if (temp->newCondition->element.evaluated == false) {
			s += " (expression evaluation)";
		}

		return s;
	}
	else if (print->type == 4) {
		Par *temp = static_cast<Par*>(print);

		string s = "par";

		if (temp->seenLeft || temp->seenRight) {
			s += " (close)";
		}
		else {
			s += " (open)";
		}

		return "par";
	}
	else if (print->type == 5) {
		BlS *temp = static_cast<BlS*>(print);
		return "begin " + temp->bid;
	}
	else if (print->type == 6) {
		VdS *temp = static_cast<VdS*>(print);
		return "var " + temp->varName + " = " + intToString11(temp->value) + " (" + temp->path + ");";
	}
	else if (print->type == 7) {
		PdS *temp = static_cast<PdS*>(print);
		return "proc " + temp->procIden + " " + temp->procName;
	}
	else if (print->type == 8) {
		PcS *temp = static_cast<PcS*>(print);
		return "call " + temp->callID + " " + temp->procName;
	}
	else if (print->type == 9) {
		PrS *temp = static_cast<PrS*>(print);
		return "remove proc " + temp->procIden + " " + temp->procName;
	}
	else if (print->type == 10) {
		VrS *temp = static_cast<VrS*>(print);
		return "remove var " + temp->varName + " = " + intToString11(temp->value) + " (" + temp->path + ");";
	}
	else if (print->type == 11) {
		string toReturn = "Skip (";
		print->type = print->oldType;
		toReturn += displayStatement(print,forwards);
		print->type = 11;
		toReturn += ")";
		return toReturn;
	}
	else if (print->type == 12) {
		return "skip (placeholder)";
	}
	else {
		return "error - type = " + intToString11(print->type) + "\n";
	}
	return "error";
}

//function to check the path used for force sequence methods
bool checkPath(statement *temp, stack<int> interleavingOrder) {

	if (interleavingOrder.empty()) {
		if (checkPar(temp) == NULL) {
			return true;
		}
		else {
			return false;
		}
	}

	//stack is not empty - so must check the next element

	int nextChoice = interleavingOrder.top();

	interleavingOrder.pop();

	if (nextChoice == 1) {
		//statement must be left
		if (getLeft(temp)) {
			if (checkPar(temp) != NULL) {
				return checkPath(checkPar(temp),interleavingOrder);
			}
			else {
				return false;
			}
		}
		else {
			return false;
		}
	}
	else if (nextChoice == 0) {
		//statement must be right
		if (!getLeft(temp)) {
			if (checkPar(temp) != NULL) {
				return checkPath(checkPar(temp),interleavingOrder);
			}
			else {
				return false;
			}
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}
}


//Update for annotation information setting


string getNextString(string *AI) {
	//will remove it from the original string, as well as return it
	string toReturn = *AI;
	int i = toReturn.find_first_of("|");
	toReturn = toReturn.erase(i-1);
	AI->erase(0,toReturn.size()+3);

	//cout << "[nextString = " << toReturn << "]";
	return toReturn;
}

stack<int> makeStackReverse(string iden) {
//	cout << "in make stack reverse\n";
	stack<int> toReturn;

	while (iden != "") {
		int i = iden.find_last_of(",");
		if (i == -1) {
			int toPush1 = stringToInt11(iden);
			iden = "";
			toReturn.push(toPush1);
		}
		else {
			string next = iden.substr(i+1,iden.length()-1);
			//	cout << "next = " << next << "\n";
			iden = iden.erase(i,iden.length()-1);
			//	cout << "updated iden = " << iden << "\n";
			int toPush = stringToInt11(next);
			toReturn.push(toPush);
		}
	}
	return toReturn;
}

stack<string> makeStackOfStrings(string s) {
	stack<string> toReturn;

	while (s != "") {
		int i = s.find_first_of("|");
		string next = s.substr(0, i);
		cout << "next = " << next << "\n";
		s = s.erase(0,i);
		cout << "s updated to " << s << "\n";
		toReturn.push(next);
		cout << "PUSHED " << next << "\n";
	}

	return toReturn;
}

void getStacksFromProgram(statement *temp, statement *stopper, std::stack<std::stack<int> > *toReturn) {

	///http://courses.cs.vt.edu/~cs1206/Fall00/bugs_CAS.html
	while (temp != stopper) {

		if (temp->type == 0) {
			DA *i = static_cast<DA*>(temp);
			toReturn->push(i->idendifiers);

		}
		else if (temp->type == 1) {
			CA *i = static_cast<CA*>(temp);
			toReturn->push(i->idendifiers);
		}
		else if (temp->type == 2) {
			IfS *i = static_cast<IfS*>(temp);
			getStacksFromProgram(i->trueBranch->nextStatement(),i,toReturn);
			getStacksFromProgram(i->falseBranch->nextStatement(),i,toReturn);
			toReturn->push(i->idendifiers);
		}
		else if (temp->type == 3) {
			//this will only be reached for nested while loops - so ignore it
			WlS *i = static_cast<WlS*>(temp);
		}
		else if (temp->type == 4) {
			Par *i = static_cast<Par*>(temp);
			getStacksFromProgram(i->leftSide->nextStatement(),i,toReturn);
			getStacksFromProgram(i->rightSide->nextStatement(),i,toReturn);
		}
		else if (temp->type == 5) {
			BlS *i = static_cast<BlS*>(temp);
			getStacksFromProgram(i->blockBody->nextStatement(),i,toReturn);
		}
		else if (temp->type == 6) {
			VdS *i = static_cast<VdS*>(temp);
			toReturn->push(i->idendifiers);
		}
		else if (temp->type == 7) {
			//only get the identifiers of this statement - and not the actual procedure body
			PdS *i = static_cast<PdS*>(temp);
			toReturn->push(i->idendifiers);
		}
		else if (temp->type == 8) {
			PcS *i = static_cast<PcS*>(temp);
			toReturn->push(i->idendifiers);
		}
		else if (temp->type == 9) {
			//only get the identifiers of this statement - and not the actual procedure body
			PrS *i = static_cast<PrS*>(temp);
			toReturn->push(i->idendifiers);
		}
		else if (temp->type == 10) {
			VrS *i = static_cast<VrS*>(temp);
			toReturn->push(i->idendifiers);
		}
		else if (temp->type == 11) {
			//a statement has gone to skip but not been removed
		}
		else if (temp->type == 12) {
			//placeholder statement
		}

		temp = temp->next;
	}
}
//JIMBO HERE
void updatedRetrieveAI(statement *temp, statement *stopper, std::string constructName, AnnInfo whereToSave,
		bool isWhile, std::stack<int> *loopIden) {

	//cout << "in retrieve AI\n";


	//std::tr1::unordered_map<std::string, std::queue<std::stack<int> > >

	std::stack<std::stack<int> > *stacks = new std::stack<std::stack<int> >();



	std::queue<std::stack<int> > test;
	//cout << "calling getStacksFromProgram\n";
	getStacksFromProgram(temp,stopper,stacks);
	//cout << "finsihed calling getStacksFromProgram\n";
//cout << "helloooooo\n";
	//now push this to AnnInfo - linked to the call name

	//whereToSave.aiMap->insert({constructName,test});

	if (isWhile) {
		//add the loop identifiers to the stack
		stacks->push(*loopIden);
	}

	//cout << "adding to construct " << constructName;

	((*whereToSave.aiMap)[constructName]) = *stacks;
	//cout << "... finished - size = " << whereToSave.aiMap->size() << "\n";



//	(*whereToSave.aiMap)[constructName] = stacks;

}

void setStacksIntoPrograms(statement *temp, statement *stopper, std::stack<std::stack<int> > *idens) {
	while (temp != stopper) {
		//cout << "loop\n";
		//if (idens->empty()) {
		//	cout << "idens empty - not reached stopper\n";
		//}
		if (temp->type == 0) {
			DA *i = static_cast<DA*>(temp);
			i->idendifiers = idens->top();
			idens->pop();
		}
		else if (temp->type == 1) {
			CA *i = static_cast<CA*>(temp);
			i->idendifiers = idens->top();
			idens->pop();
		}
		else if (temp->type == 2) {
			IfS *i = static_cast<IfS*>(temp);
			i->idendifiers = idens->top();
			idens->pop();
			setStacksIntoPrograms(i->falseBranch->nextStatement(),i,idens);
			setStacksIntoPrograms(i->trueBranch->nextStatement(),i,idens);


		}
		else if (temp->type == 3) {
			//this will only be reached for nested while loops - so ignore it
			WlS *i = static_cast<WlS*>(temp);
		}
		else if (temp->type == 4) {
			Par *i = static_cast<Par*>(temp);
			setStacksIntoPrograms(i->leftSide->nextStatement(),i,idens);
			setStacksIntoPrograms(i->rightSide->nextStatement(),i,idens);
		}
		else if (temp->type == 5) {
			BlS *i = static_cast<BlS*>(temp);
			setStacksIntoPrograms(i->blockBody->nextStatement(),i,idens);
		}
		else if (temp->type == 6) {
			VdS *i = static_cast<VdS*>(temp);
			i->idendifiers = idens->top();
			idens->pop();
		}
		else if (temp->type == 7) {
			//only get the identifiers of this statement - and not the actual procedure body
			PdS *i = static_cast<PdS*>(temp);
			i->idendifiers = idens->top();
			idens->pop();
		}
		else if (temp->type == 8) {
			PcS *i = static_cast<PcS*>(temp);
			i->idendifiers = idens->top();
			idens->pop();
		}
		else if (temp->type == 9) {
			//only get the identifiers of this statement - and not the actual procedure body
			PrS *i = static_cast<PrS*>(temp);
			i->idendifiers = idens->top();
			idens->pop();
		}
		else if (temp->type == 10) {
			VrS *i = static_cast<VrS*>(temp);
			i->idendifiers = idens->top();
			idens->pop();
		}
		else if (temp->type == 11) {
			//a statement has gone to skip but not been removed
		}
		else if (temp->type == 12) {
			//placeholder statement
		}

		temp = temp->next;
	}
}

void updatedRestoreAI(statement *temp, statement *stopper, string constructName, AnnInfo whereToSave, bool isWhile, statement *loop) {

	//cout << "\n\n\n size of construct = " << whereToSave.aiMap->size() << " \n\n\n";

//	cout << "construct name = " << constructName << ".\n";
	//we must now set the annotation information back into the new version of the mapping
//	cout << "start\n";
	std::stack<std::stack<int> > stacks = ((*whereToSave.aiMap)[constructName]);
//	cout << "got\n";

//	if (stacks.empty()) {
		//cout << "stack has been got but are empty\n";
//	}
//	else {
//		cout << "got the stacks\n";
//	}



	if (isWhile) {
		WlS *l = static_cast<WlS*>(loop);
		l->idendifiers = stacks.top();
		stacks.pop();
	}
	//else {
	//	cout << "in else\n";
	//}

	//cout << "starting set stacks\n";
	setStacksIntoPrograms(temp,stopper,&stacks);
	//cout << "finished\n";

	//(*we.we).erase(currentMapping->WID);
	(*whereToSave.aiMap).erase(constructName);

//	if (stacks.empty()) {
//		cout << "Concluded correctly - queue is empty\n";
//	}
//	else {
//		cout << "Something unexpectedly left in queue\n";
//	}
}

/*void updatedRetrieveAI(statement *temp, statement *stopper, string constructName, AnnInfo whereToSave) {
	std::queue<std::stack<int> > toSave;

	while (temp != stopper) {

		if (temp->type == 0) {
			DA *i = static_cast<DA*>(temp);
			toSave.push(i->idendifiers);
		}
		else if (temp->type == 1) {
			CA *i = static_cast<CA*>(temp);
			toSave.push(i->idendifiers);
		}
		else if (temp->type == 2) {
			IfS *i = static_cast<IfS*>(temp);
			string iden = stackOfIden(i->idendifiers);

			toSave.push(i->idendifiers);
		}
		else if (temp->type == 3) {
			//this will only be reached for nested while loops - so ignore it
			WlS *i = static_cast<WlS*>(temp);
		}
		else if (temp->type == 4) {
			Par *i = static_cast<Par*>(temp);
			string idOfLeft = updateGetAi(i->leftSide->nextStatement(),i);
			string idOfRight = updateGetAi(i->rightSide->nextStatement(),i);
			toReturn = idOfLeft + idOfRight + toReturn;
		}
		else if (temp->type == 5) {
			BlS *i = static_cast<BlS*>(temp);
			string idOfBody = updateGetAi(i->blockBody->nextStatement(),i);
			toReturn = idOfBody +  toReturn;
		}
		else if (temp->type == 6) {
			VdS *i = static_cast<VdS*>(temp);
			string iden = stackOfIden(i->idendifiers);
			toReturn = iden + " | " + toReturn;
		}
		else if (temp->type == 7) {
			//only get the identifiers of this statement - and not the actual procedure body
			PdS *i = static_cast<PdS*>(temp);
			string iden = stackOfIden(i->idendifiers);
			toReturn = iden + " | " + toReturn;
		}
		else if (temp->type == 8) {
			PcS *i = static_cast<PcS*>(temp);
			string iden = stackOfIden(i->idendifiers);
			toReturn = iden + " | " + toReturn;
		}
		else if (temp->type == 9) {
			//only get the identifiers of this statement - and not the actual procedure body
			PrS *i = static_cast<PrS*>(temp);
			string iden = stackOfIden(i->idendifiers);
			toReturn = iden + " | " + toReturn;
		}
		else if (temp->type == 10) {
			VrS *i = static_cast<VrS*>(temp);
			string iden = stackOfIden(i->idendifiers);
			toReturn = iden + " | " + toReturn;
		}
		else if (temp->type == 11) {
			//a statement has gone to skip but not been removed
		}
		else if (temp->type == 12) {
			//placeholder statement
		}

		temp = temp->next;
	}

}*/

string updateGetAi(statement *temp, statement *stopper) {

	string toReturn = "";

	while (temp != stopper) {

		if (temp->type == 0) {
			DA *i = static_cast<DA*>(temp);
			string iden = stackOfIden(i->idendifiers);
			toReturn = iden + " | " + toReturn;
		}
		else if (temp->type == 1) {
			CA *i = static_cast<CA*>(temp);
			string iden = stackOfIden(i->idendifiers);
			toReturn = iden + " | " + toReturn;
		}
		else if (temp->type == 2) {
			IfS *i = static_cast<IfS*>(temp);
			string iden = stackOfIden(i->idendifiers);
			string idsOfTrue = updateGetAi(i->trueBranch->nextStatement(),i);
			string idsOfFalse = updateGetAi(i->falseBranch->nextStatement(),i);
			toReturn = iden + " | " + idsOfTrue + idsOfFalse + toReturn;
		}
		else if (temp->type == 3) {
			//this will only be reached for nested while loops - so ignore it
			WlS *i = static_cast<WlS*>(temp);
		}
		else if (temp->type == 4) {
			Par *i = static_cast<Par*>(temp);
			string idOfLeft = updateGetAi(i->leftSide->nextStatement(),i);
			string idOfRight = updateGetAi(i->rightSide->nextStatement(),i);
			toReturn = idOfLeft + idOfRight + toReturn;
		}
		else if (temp->type == 5) {
			BlS *i = static_cast<BlS*>(temp);
			string idOfBody = updateGetAi(i->blockBody->nextStatement(),i);
			toReturn = idOfBody +  toReturn;
		}
		else if (temp->type == 6) {
			VdS *i = static_cast<VdS*>(temp);
			string iden = stackOfIden(i->idendifiers);
			toReturn = iden + " | " + toReturn;
		}
		else if (temp->type == 7) {
			//only get the identifiers of this statement - and not the actual procedure body
			PdS *i = static_cast<PdS*>(temp);
			string iden = stackOfIden(i->idendifiers);
			toReturn = iden + " | " + toReturn;
		}
		else if (temp->type == 8) {
			PcS *i = static_cast<PcS*>(temp);
			string iden = stackOfIden(i->idendifiers);
			toReturn = iden + " | " + toReturn;
		}
		else if (temp->type == 9) {
			//only get the identifiers of this statement - and not the actual procedure body
			PrS *i = static_cast<PrS*>(temp);
			string iden = stackOfIden(i->idendifiers);
			toReturn = iden + " | " + toReturn;
		}
		else if (temp->type == 10) {
			VrS *i = static_cast<VrS*>(temp);
			string iden = stackOfIden(i->idendifiers);
			toReturn = iden + " | " + toReturn;
		}
		else if (temp->type == 11) {
			//a statement has gone to skip but not been removed
		}
		else if (temp->type == 12) {
			//placeholder statement
		}

		temp = temp->next;
	}
	return toReturn;

}

void updateSetAi(statement *temp, statement *stopper, string *AI) {

	while (temp != stopper) {

		if (temp->type == 0) {
			DA *i = static_cast<DA*>(temp);
			string singleID = getNextString(AI);
			stack<int> t = makeStackReverse(singleID);
			i->idendifiers = t;
		}
		else if (temp->type == 1) {
			CA *i = static_cast<CA*>(temp);
			string singleID = getNextString(AI);
			stack<int> t = makeStackReverse(singleID);
			i->idendifiers = t;
		}
		else if (temp->type == 2) {
			IfS *i = static_cast<IfS*>(temp);
			string singleID = getNextString(AI);
			stack<int> t = makeStackReverse(singleID);
			i->idendifiers = t;
			updateSetAi(i->trueBranch->nextStatement(),i,AI);
			updateSetAi(i->falseBranch->nextStatement(),i,AI);
		}
		else if (temp->type == 3) {
			//this will only be reached for nested while loops - so ignore
			WlS *i = static_cast<WlS*>(temp);
		}
		else if (temp->type == 4) {
			Par *i = static_cast<Par*>(temp);
			updateSetAi(i->leftSide->nextStatement(),i,AI);
			updateSetAi(i->rightSide->nextStatement(),i,AI);
		}
		else if (temp->type == 5) {
			BlS *i = static_cast<BlS*>(temp);
			updateSetAi(i->blockBody->nextStatement(),i,AI);
		}
		else if (temp->type == 6) {
			VdS *i = static_cast<VdS*>(temp);
			string singleID = getNextString(AI);
			stack<int> t = makeStackReverse(singleID);
			i->idendifiers = t;
		}
		else if (temp->type == 7) {
			PdS *i = static_cast<PdS*>(temp);
			string singleID = getNextString(AI);
			stack<int> t = makeStackReverse(singleID);
			i->idendifiers = t;
		}
		else if (temp->type == 8) {
			PcS *i = static_cast<PcS*>(temp);
			string singleID = getNextString(AI);
			stack<int> t = makeStackReverse(singleID);
			i->idendifiers = t;
		}
		else if (temp->type == 9) {
			PrS *i = static_cast<PrS*>(temp);
			string singleID = getNextString(AI);
			stack<int> t = makeStackReverse(singleID);
			i->idendifiers = t;
		}
		else if (temp->type == 10) {
			VrS *i = static_cast<VrS*>(temp);
			string singleID = getNextString(AI);
			stack<int> t = makeStackReverse(singleID);
			i->idendifiers = t;
		}
		else if (temp->type == 11) {
			//a statement has gone to skip but not been removed
		}
		else if (temp->type == 12) {
			//placeholder statement
		}

		temp = temp->next;
	}


}

/*
 * Function that goes through a list of available next steps, returns one that must be executed next as it appears within an atomic block that has
 * already started
 */
statement* checkAtomic(set<statement*> original) {

	for (std::set<statement*>::iterator it=original.begin(); it!=original.end(); ++it) {

		//cout << "statement of type " << (*it)->type << "\n";


		if ((*it)->atomic == NULL) {
			//not in atomic, so ignore
		}
		else if ((*it)->type == 13) {
			//one option is an atomic statement - check if its the active one

			Atom *inner = static_cast<Atom*>(*it);
			if (inner->started) {
//				cout << "\n\nFOUND AN ATOMIC STATEMENT THAT MUST CLOSE\n\n";
				return *it;
			}
			else {
				//ignore
			}
		}
		else {
			Atom *at = static_cast<Atom*>((*it)->atomic);
			if (at->started) {
//				cout << "checkAtomic found statement of type " << (*it)->type << "\n";
				//found a statement that must be the next to be executed
				return *it;
			}
			else {
				//not started, so we can ignore it also
			}
		}
	}
	//if we reach this point, we have checked all options and not found an atomic action - so return NULL
	return NULL;
}

/*
 * Get all possible atomic rules (for nested parallel statements
 */

set<statement*> getAllAtomic(set<statement*> original) {
	set<statement*> toReturn;

	for (std::set<statement*>::iterator it=original.begin(); it!=original.end(); ++it) {

		if ((*it)->atomic == NULL) {
			//not in atomic, so ignore
		}
		else if ((*it)->type == 13) {
			//one option is an atomic statement - check if its the active one
			Atom *inner = static_cast<Atom*>(*it);
			if (inner->started) {
				toReturn.insert(*it);
			}
			else {
				//ignore
			}
		}
		else {
			Atom *at = static_cast<Atom*>((*it)->atomic);
			if (at->started) {
				//found a statement that must be the next to be executed
				toReturn.insert(*it);
			}
			else {
				//not started, so we can ignore it also
			}
		}
	}
	//if we reach this point, we have checked all options and not found an atomic action - so return NULL
	return toReturn;

}

/*
 * Simulating a single step of the forwards execution - depending on which type of statement it is and how far the execution we already
 * are of that statement - returning an updated pointer - pointing to the next statement
 */
statement* simStep(parser *pp, statement *program, statement *&current, auxStore st, scopeEnvironment se, gammaSigma sg,  procedureEnvironment mu,
		whileEnvironment we, int* next, int *previous, nextNames nextNames, procedureIdentifiers pi, int *nextLoc, int *stateSaving,
		vector<string>* rules, bool recordRules, bool atomEx, AnnInfo AI, bool output) {

//	cout << "*****SIMSTEP : Type = " << program->type << " (current type = " << current->type << ")" << ".\n";

	if (program->type == 0) {
//		cout << "sim step called on destructive assignment\n";
		//simulating a destructive assignment [D1a]
		DA *temp = static_cast<DA*>(program);

		//get the block id necessary for evalaution, and work out the location
		std::string bnForEval = getB(temp->varName,temp->path,se);
		int location = getLoc(temp->varName,bnForEval,sg);

		//if the location returned is -1, an error has occurred as the variable cannot be
		if (location == -1) {
			cout << "ABORT: Location returned incorrectly in Destructive Assignment\n";
			current = NULL;
			return NULL;
		}

		//atomic expression evaluation
		//evaluate the expression (that must be an arithmetic tree) to get the new value of the variable
		//int newValue = evaluateArithTree(temp->newCondTree,temp->path,se,sg);

		//now non-atomically evaluate the expression

		if ((temp->newCondTree->element.evaluated == false) && (!atomEx)) {
			if (output) {
				cout << "Simulating Atomic Destructive Assignment Expression Evaluation\n";
			}

			//evalStepOfTree(temp->newCondTree,temp->path,se,sg);

			int newValue = evaluateArithTree(temp->newCondTree,temp->path,se,sg);
			temp->newCondTree->element.evaluated = true;
			temp->newCondTree->element.resultInt = newValue;

			if (recordRules) {
				rules->push_back("[D1a{eval}]");
			}

		}
		else {
			if (output) {
				cout << "Simulating Destructive Assignment (type " << program->type << ") : Rule [D1a] --- ";
				cout << "statement = " << temp->full << "\n";
			}

			//add the rule name to the sequence of rules for tracing if recordrules is enabled
			if (recordRules) {
				rules->push_back("[D1a]");
			}

			int newValue;

			if (!atomEx) { //clean the tree for next time and use the evaluated value
				resetTree(temp->newCondTree); //reset the booleans indicating if a subtree has been evaluated
				newValue = temp->newCondTree->element.resultInt;
			}
			else { //atomically evaluate the expression
				//evaluate the expression (that must be an arithmetic tree) to get the new value of the variable
				newValue = evaluateArithTree(temp->newCondTree,temp->path,se,sg);

			}

			//save the old value as a pair to the auxiliary store - depending on whether state-saving is enabled
			if (*stateSaving == 1) {
				pair<int,int> toSave;
				toSave.first = *next;
				toSave.second = (*sg.sigma)[location];
				(*st.aux)[temp->varName].push(toSave);

				//save the identifier, reflect change in original code (if required) and update next
				temp->idendifiers.push(*next);
				*next += 1;
				*previous += 1;
			}

			//now update the value held in sigma
			((*sg.sigma)[location]) = newValue;

			//change this statement to its skip equivalent - rule [S2a] must be applied to remove this
			temp->type = 11;
		}
	}

	else if (program->type == 1) {
//		cout << "sim step called on constructive assignment\n";
		CA *temp = static_cast<CA*>(program);

		//get the block id necessary for evaluating the variable in question, and use this to get the location
		std::string bnForEval = getB(temp->varName,temp->path,se);
		int location = getLoc(temp->varName,bnForEval,sg);

		//if the location returned is -1, an error has occurred as the variable cannot be
		if (location == -1) {
			cout << "ABORT: Location returned incorrectly in Destructive Assignment\n";
			current = NULL;
			return NULL;
		}


		if ((temp->newCondTree->element.evaluated == false) && (!atomEx)) {
			if (output) {
				cout << "Simulating Atomic Constructive Assignment Expression Evaluation\n";
			}

			int newValue = evaluateArithTree(temp->newCondTree,temp->path,se,sg);
			temp->newCondTree->element.evaluated = true;
			temp->newCondTree->element.resultInt = newValue;

			if (recordRules) {
				rules->push_back("[C1a{eval}]");
			}

		}
		else {
			if (output) {
				cout << "Simulating Constructive Assignment (type " << program->type << ") : Rule [C1a]\n";
			}

			//add the rule name to the sequence of rules for tracing if recordrules is enabled
			if (recordRules) {
				rules->push_back("[C1a]");
			}

			int newValue;
			if (!atomEx) { //clean the tree for next time and use the evaluated value
				resetTree(temp->newCondTree); //reset the booleans indicating if a subtree has been evaluated
				newValue = temp->newCondTree->element.resultInt;
			}
			else { //atomically evaluate the expression
				//evaluate the expression (that must be an arithmetic tree) to get the new value of the variable
				newValue = evaluateArithTree(temp->newCondTree,temp->path,se,sg);
			}

			//now look up the current value of the variable in question, necessary as this is an increment/decrement
			int currentVal = 0, update = 0;
			std::tr1::unordered_map<int,int >::const_iterator got2 = sg.sigma->find(location);

			if (got2 == sg.sigma->end()) {
				std::cout << "Abort: Unable to query the memory location " << location << "\n";
				current = NULL;
				return NULL;
			}
			else {
				 currentVal = got2->second; //current value of the variable

				//now actually perform the increment or decrement
				if (temp->inc) {
					update = currentVal + newValue;
				} else {
					update = currentVal - newValue;
				}

				//update the value held in sigma for the corresponding location
				((*sg.sigma)[location]) = update;

				//push the necessary identifier into the necessary stack, and update the values of next and previous
				if (*stateSaving == 1) {
					temp->idendifiers.push(*next);
					*next += 1;
					*previous += 1;
				}

				//change this statement to its skip equivalent - rule [S2a] must be applied to remove this
				temp->type = 11;
			}
		}
	}

	else if (program->type == 2) {
		//Simulate a conditional statement
		IfS *temp = static_cast<IfS*>(program);

		if (!temp->seen) { // Not seen before - first time through - evaluate condition and move to branch

			if ((temp->newCondition->element.evaluated == false) && (!atomEx)) { //not yet evaluated the condition
				if (output) {
					cout << "Simulating Step of Conditional Statement Condition Evaluation\n";
				}

				//evalStepOfTree(temp->newCondition,temp->path,se,sg);

				bool result = evaluateBoolTreeUpdated(temp->newCondition,temp->path,se,sg);
				temp->newCondition->element.evaluated = true;
				temp->newCondition->element.resultBool = result;

				if (recordRules) {
					if (result) {
						rules->push_back("[I1aT{eval}]");
					}
					else {
						rules->push_back("[I1aF{eval}]");
					}

				}

			}
			else {
				if (output) {
					cout << "Simulating Conditional Statement Opening (type " << program->type << ") : Rule [I1a";
				}

				bool condition;
				if (!atomEx) { //clean the tree for next time and use the evaluated value
					condition = temp->newCondition->element.resultBool;
					resetTree(temp->newCondition); //reset the booleans indicating if a subtree has been evaluated
				}
				else { //atomically evaluate the expression
					//evaluate the expression (that must be an arithmetic tree) to get the new value of the variable
					condition = evaluateBoolTreeUpdated(temp->newCondition,temp->path,se,sg);
				}

				temp->seen = true; //indicate that we have now seen the conditional - already started

				if (*stateSaving == 1) {
					//pair<int,int> toSave;
					//toSave.first = *next;
					//toSave.second = temp->choice;
					//(*st.aux)["B"].push(toSave);
					temp->idendifiers.push(*next);

					*next += 1;
					*previous += 1;
				}


				if (condition) {
					if (output) {
						cout << "T] (since the condition was evaluated to true)\n";
					}

					//condition is true, meaning we must start the execution of the true branch - rule [I1aT]
					if (recordRules) {
						rules->push_back("[I1aT]");
					}

					//for state-saving later, we remember that we follow the true branch
					temp->choice = 1;

					//update the pointer to the true branch (depending on whether its within a parallel statement)
					if (temp->parentPar == NULL) {
						//no parent, so not a member of a parallel statement
						current = temp->trueBranch->nextStatement();
					}
					else {
						//this is in a parallel statement, so update either currLeft or currRight
						if (temp->left == 1) {
							//on left side of parallel - so update currLeft
							temp->parentPar->currLeft = temp->trueBranch->nextStatement();
						}
						else {
							//on right side of parallel - so update currRight
							temp->parentPar->currRight = temp->trueBranch->nextStatement();
						}
					}
				}
				else {
					if (output) {
						//cout << "F] (since the condition was evaluated to false)\n";
						cout << "F]\n";
					}

					//condition is false, meaning we must start the execution of the false branch - rule [I1aF]
					if (recordRules) {
						rules->push_back("[I1aF]");
					}
					//for state-saving later, we remember that we follow the false branch
					temp->choice = 0;

					//update the pointer to the false branch (depending on whether its within a parallel statement)
					if (temp->parentPar == NULL) {
						//no parent, so not a member of a parallel statement
						current = temp->falseBranch->nextStatement();
					}
					else {
						//this is in a parallel statement, so update either currLeft or currRight
						if (temp->left == 1) {
							//on left side of parallel - so update currLeft
							temp->parentPar->currLeft = temp->falseBranch->nextStatement();
						}
						else {
							//on right side of parallel - so update currRight
							temp->parentPar->currRight = temp->falseBranch->nextStatement();
						}
					}
				}
		}
		}
		else { //We have already started the conditional - meaning we have returned after the execution of the appropriate branch
			if (output) {
				cout << "Simulating Conditional Statement Closing (type " << program->type << ") : Rule ";
			}

			temp->seen = false; //we reset the seen flag in case the conditional is repeated later

			//depending the value to be saved, we perform either [I4a] or [I5a]
			if (output) {
				if (temp->choice == 1) {
					cout << "[I4a] (since the condition was evaluated to true)\n";
				}
				else {
					cout << "[I5a] (since the condition was evaluated to false)\n";
				}
			}

			//reflect the same into the recording of the rules applied
			if (recordRules) {
				if (temp->choice == 1) {
					rules->push_back("[I4a]");
				}
				else {
					rules->push_back("[I5a]");
				}
			}

			//we now perform the state-saving if required, saving the identifier and the boolean indicating the choice
			if (*stateSaving == 1) {
				pair<int,int> toSave;
				toSave.first = *next;
				toSave.second = temp->choice;
				(*st.aux)["B"].push(toSave);
				temp->idendifiers.push(*next);

				*next += 1;
				*previous += 1;
			}

			//change this statement to its skip equivalent - rule [S2a] must be applied to remove this
			temp->type = 11;
		}
	}

	else if (program->type == 3) {
//		cout << "sim step called on a while loop\n";

		//Simulate a while loop
		WlS *temp = static_cast<WlS*>(program);

		//query the while environment, determining whether this loop has already been mapped (already started), or not (starting now)
		std::tr1::unordered_map<std::string,statement* > ::iterator it = (*we.we).begin();

		if (((*we.we).count(temp->WID)) == 0) { //if there is no mapping for the while loop name - not already started
			//we must start the execution of this loop - we only make a copy when there is at least one iteration (when the condition evaluates to T now)

			if ((temp->newCondition->element.evaluated == false)  && (!atomEx)) {
				if (output) {
					cout << "Simulating Atomic While Loop Condition Evaluation\n";
				}

				//evalStepOfTree(temp->newCondition,temp->path,se,sg);

				bool result = evaluateBoolTreeUpdated(temp->newCondition,temp->path,se,sg);
				temp->newCondition->element.evaluated = true;
				temp->newCondition->element.resultBool = result;

				if (recordRules) {
					if (result) {
						rules->push_back("[W3a{eval}]");
					}
					else {
						rules->push_back("[W1a{eval}]");
					}

				}

			}
			else {

				bool condition;
				if (!atomEx) { //clean the tree for next time and use the evaluated value
					condition = temp->newCondition->element.resultBool;
					resetTree(temp->newCondition); //reset the booleans indicating if a subtree has been evaluated
				}
				else { //atomically evaluate the expression
					//evaluate the expression (that must be an arithmetic tree) to get the new value of the variable
					condition = evaluateBoolTreeUpdated(temp->newCondition,temp->path,se,sg);
				}

				if (!condition) { //this is a loop with zero iterations
					//do not make a copy of the while loop - save the opening F - will be the only element of the sequence
					if (output) {
						cout << "Simulating While Loop Opening with False Condition (type " << program->type << ") : Rule [W1a]\n";
					}

					//loop with zero iterations - rule [W1a]
					if (recordRules) {
						rules->push_back("[W1a]");
					}

					//if state-saving is required, save the necessary identifier and the only element of the loop sequence; F
					if (*stateSaving == 1) {
						pair<int,int> toSave2;
						toSave2.first = *next;
						toSave2.second = 0;
						(*st.aux)["W"].push(toSave2);
						temp->idendifiers.push(*next);

						*next += 1;
						*previous += 1;
					}

					//now set the loop to its skip equivalent - meaning rule [S2a] can be used to continue the execution
					temp->type = 11;
				}
				else { //this is a loop with at least one iteration

					if (output) {
						cout << "Simulating While Loop Opening with True Condition (type " << program->type << ") : Rule [W3a]\n";
					}

					//condition is true, and mapping does not exist, meaning it must be created via rule [W3a]
					if (recordRules) {
						rules->push_back("[W3a]");
					}

					//create a mapping to a unique version of the loop body - created dynamically
					linkedList lc, lb;
					linkedList *localCopy = &lc;

					createVersionStatement(temp,localCopy,temp->next,temp->parentPar,temp->left); //this creates the unique version stored in localCopy

					WlS *aq = static_cast<WlS*>(localCopy->nextStatement()); //cast the new version of the loop to a while statement (allowing us to access its specifics
					localCopy->modifyWithPrem(NULL, temp->parentPar, temp->left); //modify the local copy with parallel information - so that each statement knows of its parents

					aq->wsixa = true;

					aq->next = temp->next; //link the end of the copy to the same place as the original

					//rename the local copy to make it unique
					//renameLoopBody(localCopy->nextStatement(),NULL,nextNames);
					renameLoopBodyUpdated(localCopy->nextStatement(),NULL,nextNames);

					//insert the mapping into the while environment
					((*we.we)[temp->WID]) = localCopy->nextStatement();

					//save the next element of the while loop sequence - this will be an F (0) as this is the first iteration (of n)
					if (*stateSaving == 1) {
						pair<int,int> toSave2;
						toSave2.first = *next;
						toSave2.second = 0;
						(*st.aux)["W"].push(toSave2);
						aq->idendifiers.push(*next);

						*next += 1;
						*previous += 1;
					}

					//now update the current pointer to point to the beginning of the loop body - allowing the necessary iteration to occur
					if (temp->parentPar == NULL) {
						//no parent, so move pointer on
						current = aq->loopBody->nextStatement();
					}
					else {
						//is within a parallel statement, so move currentLeft or currentRight
						if (temp->left == 1) {
							//on the left side of the parallel
							temp->parentPar->currLeft = aq->loopBody->nextStatement();
						}
						else {
							//on the right side of the parallel
							temp->parentPar->currRight = aq->loopBody->nextStatement();
						}
					}
				}
		}
		}
		else { //Mapping already exists, we have already started the loop execution
			//since we back at the while loop again, the loop body has finished, meaning we have just performed [W6a]




			//retrieve the current mapping for this while loop - this will be unique
			WlS *currentMapping;

			std::tr1::unordered_map<std::string,statement* > ::iterator itFind = (*we.we).find(temp->WID);
			if (itFind == (*we.we).end()) {
				std::cout << "while loop mapping not found even when expected.  not found-4005";
			}
			else {
				//the mapping has been found, and is cast to a while loop named currentMapping
				currentMapping = static_cast<WlS*>(itFind->second);
			}


			//simulate the step of [W6a] here

			if (currentMapping->wsixa) {
				//must perform a w6a step
				currentMapping->wsixa = false;

				if (recordRules) {
					rules->push_back("[W6a]");
				}

				cout << "Simulating While Loop Reset (type " << program->type << ") : Rule [W6a]\n";

			}
			else {



			//now simulate the condition to determine whether we perform another iteration or not
			if ((temp->newCondition->element.evaluated == false) && (!atomEx)) {
				if (output) {
					cout << "Simulating Atomic While Loop Condition Evaluation\n";
				}

				//evalStepOfTree(temp->newCondition,temp->path,se,sg);

				bool result = evaluateBoolTreeUpdated(temp->newCondition,temp->path,se,sg);
				temp->newCondition->element.evaluated = true;
				temp->newCondition->element.resultBool = result;

				if (recordRules) {
					if (result) {
						rules->push_back("[W4a{eval}]");
					}
					else {
						rules->push_back("[W2a{eval}]");
					}
				}

			}
			else { //the entire tree has now been (already been) evaluated fully - so use the value and move depending on it
				bool condition;
				if (!atomEx) { //clean the tree for next time and use the evaluated value
					condition = temp->newCondition->element.resultBool;
					resetTree(temp->newCondition); //reset the booleans indicating if a subtree has been evaluated
				}
				else { //atomically evaluate the expression
					//evaluate the expression (that must be an arithmetic tree) to get the new value of the variable
					condition = evaluateBoolTreeUpdated(temp->newCondition,temp->path,se,sg);
				}

				//due to how we produce the while loop sequence, this will always be a T, so perform this state-saving now
				if (*stateSaving == 1) {
					pair<int,int> toSave2;
					toSave2.first = *next;
					toSave2.second = 1;
					(*st.aux)["W"].push(toSave2);
					currentMapping->idendifiers.push(*next);
				}

				if (condition) { //we must cycle again and perform the loop body again
					currentMapping->wsixa = true;
					if (output) {
						cout << "Simulating While Loop Iteration with True Condition (type " << program->type << ") : Rule [W4a]\n";
					}

					//this is simulating the rule [W4a]
					if (recordRules) {
						rules->push_back("[W4a]");
					}

					//in order to keep nested constructs unique, all construct identifiers are incremented to their next version - renaming
					renameLoopBodyUpdated(currentMapping,NULL,nextNames);

					//now move the current pointer onto the beginning of the renamed loop body
					if (currentMapping->parentPar == NULL) {
						//no parent, so move pointer on
						current = currentMapping->loopBody->nextStatement();
					}
					else {
						//is within a parallel statement, so move currentLeft or currentRight
						if (currentMapping->left == 1) {
							//on the left side of the parallel
							currentMapping->parentPar->currLeft = currentMapping->loopBody->nextStatement();
						}
						else {
							//on the right side of the parallel
							currentMapping->parentPar->currRight = currentMapping->loopBody->nextStatement();
						}
					}
				}
				else { //the condition is false, meaning this is where the loop should finish
					if (output) {
						cout << "Simulating While Loop Closing due to False Condition (type " << program->type << ") : Rule [W2a]\n";
					}

					//this is simulating rule [W2a]
					if (recordRules) {
						rules->push_back("[W2a]");
					}

					//now perform state-saving, the necessary identifier, annotation information - while the loop sequence element has already been pushed above
					if (*stateSaving == 1) {

						updatedRetrieveAI(currentMapping->loopBody->nextStatement(),currentMapping,currentMapping->WID,AI,true,&(currentMapping->idendifiers));

/*						//get the identifiers from the current mapping
						string idenOfLoop = updateGetAi(currentMapping->loopBody->nextStatement(),currentMapping);


						//now  add the identifierss of this loop as the first part of these loop identifiers
						string iden = stackOfIden(currentMapping->idendifiers);
						idenOfLoop = iden + " | " + idenOfLoop;

						pair<int,std::string> toSave;
						toSave.first = *next;
						toSave.second = idenOfLoop;
						(*st.auxAI)["WI"].push(toSave);
*/
					}



					//now remove the mapping from the while environment
					(*we.we).erase(currentMapping->WID);

					//finally, set the statement to its skip equivalent, meaning [S2a] cam be used to continue the execution
					currentMapping->type = 11;
				}
				//now update the next and previous identifier pointers
				*next += 1;
				*previous += 1;
			}
		}
		}
	}
	else if (program->type == 4) {
		//Simulate a parallel statement
		Par *temp = static_cast<Par*>(program);

		//cout << "simulating step of parallel statement\n";

/*
		//if both sides are finished, but both have not yet been marked as free
		if ((temp->currLeft == program) && (temp->currRight == program)) {
			//pick the left side (could be random but doesn't add anything)
			if (output) {
				cout << "AUTO ------ Simulating Parallel Statement - left side has finished (type " << program->type << ") : Rule [P4a]\n";
			}

			//record the execution of the rule [P4a]
			if (recordRules) {
				rules->push_back("[P4a]");
			}

			//set the left finished flag to true and the corresponding pointer to NULL
			temp->seenLeft = true;
			temp->currLeft = NULL;
		}
		else if (temp->currLeft == program) { //if the left side of the parallel has completed, the program should become sequential - rule [P4a]
			if (output) {
				cout << "Simulating Parallel Statement - left side has finished (type " << program->type << ") : Rule [P4a]\n";
			}

			//record the execution of the rule [P4a]
			if (recordRules) {
				rules->push_back("[P4a]");
			}

			//set the left finished flag to true and the corresponding pointer to NULL
			temp->seenLeft = true;
			temp->currLeft = NULL;
		}
		else if (temp->currRight == program) { //if the right side of the parallel has completed, the program should become sequential - rule [P3a]
			if (output) {
				cout << "Simulating Parallel Statement - right side has finished (type " << program->type << ") : Rule [P3a]\n";
			}

			//record the execution of the rule [P4a]
			if (recordRules) {
				rules->push_back("[P3a]");
			}

			//set the right finished flag to true and the corresponding pointer to NULL
			temp->seenRight = true;
			temp->currRight = NULL;
		}
*/

		//now handle when a parallel statement finally completes - when both sides have completed
		if (temp->seenLeft && temp->seenRight) { //move onto the next statement

			cout << "Simulating Parallel Statement Closure : Rule [P3a]\n";

			if (recordRules) {
				rules->push_back("[P3a]");
			}

			//reset the various flags and pointers in case the parallel statement is repeated
			temp->seenLeft = false;
			temp->seenRight = false;
			temp->currLeft = temp->leftSide->nextStatement();
			temp->currRight = temp->rightSide->nextStatement();

			//no further iterations are required, so move on (depending on whether its within a parallel statement)
			//set the statement to its skip equivalent
			temp->type = 11;
		}
	}

	else if (program->type == 5) {
//		cout << "sim step called on block\n";

		//Simulate a block statement
		BlS *temp = static_cast<BlS*>(program);

		if (!temp->seen) { //we have not yet seen this block - meaning this is the first time around - rule [B1a]
			if (output) {
				cout << "Simulating Block Statement Opening (type " << program->type << ") : Rule [B1a]\n";
			}

			temp->seen = true; //record that we have seen this block now
			//record that we have/are performed the rule [B1a]
			if (recordRules) {
				rules->push_back("[B1a]");
			}

			//now set the pointer to the beginning of the block body
			if (temp->parentPar == NULL) {
				//no parent, so not a member of a parallel statement
				current = temp->blockBody->nextStatement();
			}
			else {
				//this is in a parallel statement, so update either currLeft or currRight
				if (temp->left == 1) {
					//on left side of parallel - so update currLeft
					temp->parentPar->currLeft = temp->blockBody->nextStatement();
				}
				else {
					//on right side of parallel - so update currRight
					temp->parentPar->currRight = temp->blockBody->nextStatement();
				}
			}
		}
		else { //we have seen this block before - so we must be closing the block - rule [G3a]
			//reset the seen flag in case the block is repeated (within a loop for example)
			temp->seen = false;

			if (output) {
				cout << "Simulating Block Statement Closing (type " << program->type << ") : Rule [B3a]\n";
			}

			//record that we have performed the rule [B3a]
			if (recordRules) {
				rules->push_back("[B3a]");
			}

			//now set the block to its skip equivalent, so that the rule [S2a] can be used to continue
			temp->type = 11;
		}
	}

	else if  (program->type == 6) {
		if (output) {
			cout << "Simulating Local Variable Declaration (type " << program->type << ") : Rule [L1a]\n";
		}

		//Simulate a local variable declaration statement
		VdS *temp = static_cast<VdS*>(program);

		//this is a single step action, namely rule [L1a]
		if (recordRules) {
			rules->push_back("[L1a]");
		}

		//extract the most direct block name (end of path) that will be used to index this local version
		string blockNameForEvaluation = getMostDirectBlockName(temp->path);

		//make the new entry into sigma and gamma, using nextLoc
		((*sg.gamma)[make_pair(temp->varName,blockNameForEvaluation)]) = *nextLoc;
		((*sg.sigma)[*nextLoc]) = temp->value;
		*nextLoc += 1;

		//if state-saving is required, save the next identifier and update next and previous identifiers
		if (*stateSaving == 1) {
			temp->idendifiers.push(*next);
			*next += 1;
			*previous += 1;
		}

		//update the scope information with this new version of a variable
		//add the variable name into the stack for this blockname
		string blockNameReduced = removeVersionOfBlockName(temp->blockName);

		//insert the variable name to the set for this reduced block name (without the version)
		((*se.se)[blockNameReduced]).insert(temp->varName);

		//now set the statement to its skip equivalent - so that the rule [S2a] can be used to continue the execution
		temp->type = 11;
	}

	else if  (program->type == 7) {
		if (output) {
			cout << "Simulating Procedure Declaration (type " << program->type << ") : Rule [L2a]\n";
		}

		//Simulate a procedure declaration statement
		PdS *temp = static_cast<PdS*>(program);

		//this is a single step action, namely rule [L2a]
		if (recordRules) {
			rules->push_back("[L2a]");
		}

		//this statement makes the basis entry onto the procedure environment - this is a pair containing a pointer to the body and the procedure name (used in code)
		pair<std::string,statement*> entry;
		entry.second = temp->procBody;
		entry.first = temp->procName;

		//map this pair to the unique procedure identifier in the procedure environment
		((*mu.pe)[temp->procIden]) = entry;

		//if state-saving is required, save the next identifier as well updating the next and previous identifiers
		if (*stateSaving == 1) {
			temp->idendifiers.push(*next);
			*next += 1;
			*previous += 1;
		}

		//we now need to make changes to ensure this procedure name can be evaluated
		//link the current block and the procedure name used in code to the procedure identifier
		std::string blo = reduceToMostDirectBlock(temp->path); //block to which this procedure is local
		std::string baseBlockName = reduceARenamedBlockName(temp->path); //remove the verison number

		//insert a mapping between this block and the procedure name to its unique identifier
		(*pi.pi)[make_pair(temp->procName,blo)] = temp->procIden;

		//then insert the procedure name used in code into the scope environment
		set<std::string> cur = (*se.se)[baseBlockName];
		cur.insert(temp->procName);
		(*se.se)[baseBlockName] = cur;

		//finally, set this statement to its skip equivalent - so that the rule [S2a] can be used to continue the execution
		temp->type = 11;
	}
	else if  (program->type == 8) {
		//Simulate a procedure call statement
		PcS *temp = static_cast<PcS*>(program);

		if (!temp->seen) { //we have not seen the call before, meaning we must create the unique copy of the procedure body
			if (output) {
				cout << "Simulating Procedure Call Opening (type " << program->type << ") : Rule [G1a]\n";
			}

			temp->seen = true; //set the seen flag to indicate we have started the call

			//as this is the beginning of a call, this must be the rule [G1a]
			if (recordRules) {
				rules->push_back("[G1a]");
			}

			//get the basis mapping for the given procedure identifier from the procedure environment
			pair<std::string,statement*> pairAnswer = queryPE(temp->procName,mu,temp->path,se,pi);;
			string formalParameterName = pairAnswer.first; //the procedure name used in code
			statement *answer = pairAnswer.second; //pointer to the basis procedure body

			//now create the local version of this procedure body - local to this specific call
			linkedList lc;
			linkedList *localCopy = &lc;

			createVersionStatement(answer,localCopy,NULL,temp->parentPar,temp->left); //this function produces the local version of the basis procedure body

			//now rename this local copy to make it unique - using the unique call id of this call
			renameProcBody(localCopy->nextStatement(),NULL,temp->callID);

			//link the end of this new copy of the procedure body back to the call
			linkProcBodyToCall(localCopy,temp);

			//now insert this as part of a mapping into the procedure environment
			pair<std::string,statement*> entry;
			entry.second = localCopy->nextStatement();
			entry.first = temp->procName;
			((*mu.pe)[temp->callID]) = entry;

			if (*stateSaving == 1) {
				temp->idendifiers.push(*next);
				*next += 1;
				*previous += 1;
			}

			//now begin the execution of the procedure body - move current pointer to the beginning of the local copy of the procedure body
			if (temp->parentPar == NULL) {
				//no parent, so not a member of a parallel statement
				current = localCopy->nextStatement();
			}
			else {
				//this is in a parallel statement, so update either currLeft or currRight
				if (temp->left == 1) {
					//on left side of parallel - so update currLeft
					temp->parentPar->currLeft = localCopy->nextStatement();
				}
				else {
					//on right side of parallel - so update currRight
					temp->parentPar->currRight = localCopy->nextStatement();
				}
			}
		}
		else { //this call has already been seen, meaning we now finish the call statement
			temp->seen = false; //reset the seen flag in case this call is repeated (not actually a problem due to the renaming/version)
			if (output) {
				cout << "Simulating Procedure Call Closing (type " << program->type << ") : Rule [G3a]\n";
			}

			//as this is the ending of a call, this must be the rule [G3a]
			if (recordRules) {
				rules->push_back("[G3a]");
			}

			//push the next identifier to the appropriate stack of identifiers
			temp->idendifiers.push(*next);

			//now find the mapping for this call statement in the procedure environment
			std::tr1::unordered_map<std::string,pair<std::string,statement*> >::iterator it = ((*mu.pe)).find(temp->callID);
			if (it != (*mu.pe).end()) { //mapping exists here
				//perform the necessary state-saving if required
				if (*stateSaving == 1) {
					//extract the necessary annotation information
					//string ai = saveAnnotationInformationWhile(it->second.second,temp);

					//new
				//	cout << "saving annotation inform from procedure call\n";
					updatedRetrieveAI(it->second.second,temp,temp->callID,AI,false,NULL);

				/*	string ai = updateGetAi(it->second.second,temp);

					//and push this to the stack pr
					pair<int,string> toSave;
					toSave.first = *next;
					toSave.second = ai;
					(*st.auxAI)["Pr"].push(toSave);
				*/

				}
				//finally, remove the mapping to finish the call
				((*mu.pe).erase(it));
			}
			else {
				cout << "ABORT: mapping not found meaning it cannot be removed\n";
			}

			//now update the next and previous pointers
			*next += 1;
			*previous += 1;

			//now set this statement to its skip equivalent, meaning [S2a] can be used to continue the execution
			temp->type = 11;
		}
	}

	else if (program->type == 9) {
		if (output) {
			cout << "Simulating Procedure Removal (type " << program->type << ") : Rule [H2a]\n";
		}

		//Simulate a procedure removal statement
		PrS *temp = static_cast<PrS*>(program);

		//as this is a single step action, it must be rule [H2a]
		if (recordRules) {
			rules->push_back("[H2a]");
		}

		//we must remove the basis mapping, but first we have to find it
		std::tr1::unordered_map<std::string,pair<std::string,statement*> >::iterator it = ((*mu.pe)).find(temp->procIden);
		if (it != (*mu.pe).end()) { //mapping has been found
			//now remove the mapping from the procedure environment
			((*mu.pe).erase(it));
		}
		else { //mapping cannot be found - ERROR
			cout << "ABORT: error - procedure environment call mapping not found and so cannot be removed\n";
		}

		//now perform any necessary state-saving
		if (*stateSaving == 1) {
			//save the identifier, and update both next and previous
			temp->idendifiers.push(*next);
			*next += 1;
			*previous += 1;
		}

		//now set this statement to its skip equivalent, so that the rule [S2a] can be used to continue the execution
		temp->type = 11;
	}

	else if (program->type == 10) {
		if (output) {
			cout << "Simulating Local Variable Removal (type " << program->type << ") : Rule [H1a]\n";
		}

		//Simulate a variable removal statement
		VrS *temp = static_cast<VrS*>(program);

		//as this is a single step action, it must be rule [H1a]
		if (recordRules) {
			rules->push_back("[H1a]");
		}

		//we must reset the memory location back to 0, meaning we need to evaluate it to find this location
		//this requires the block name to which this variable is local
		std::string bnForEval = getMostDirectBlockName(temp->path);
		int location = getLoc(temp->varName,bnForEval,sg);

		//perform the necessary state-saving - identifier, final value of this location and update both next and previous
		if (*stateSaving == 1) {
			pair<int,int> toSave;
			toSave.first = *next;
			toSave.second = (*sg.sigma)[location]; //save the old value of the local variable (final value)
			(*st.aux)[temp->varName].push(toSave);
			temp->idendifiers.push(*next);

			*next += 1;
			*previous += 1;
		}

		//now remove the information from sigma and gamma
		(*sg.sigma).erase(location);
		(*sg.gamma).erase(make_pair(temp->varName,bnForEval));

		//and remove the local variable name from the scope environment for this block name
		string blockNameReduced = removeVersionOfBlockName(temp->blockName);
		((*se.se)[blockNameReduced]).erase(temp->varName);

		//finally, set this statement to its skip equivalent, so that the rule [S2a] can be used to continue the execution
		temp->type = 11;
	}

	else if (program->type == 11) {
//		cout << "sim step of sequential composition\n";

		if (output) {
			cout << "Simulating Sequential Composition (type " << program->type << ") : Rule [S2a] - completing statement type " << program->oldType << "\n";
		}

		//Simulate a skip operation (sequential composition - [S2a])
		//record the application of an instance of this rule
		if (recordRules) {
			rules->push_back("[S2a]");
		}

		//now reset the statement back to its original
		if (program->oldType == 0) {
			//cout << "skip of a destructive assignment\n";
			program->type = program->oldType;
			DA *pro = static_cast<DA*>(program);

			//now update the program pointer to the next statement (depending on whether this is in a parallel statement)
			if (pro->parentPar == NULL) {
				//no parent, so not a member of a parallel statement
				current = current->next;
			}
			else {
				//this is in a parallel statement, so update either currLeft or currRight
				if (pro->left == 1) {
					//on left side of parallel - so update currLeft
					pro->parentPar->currLeft = program->next;
				}
				else {
					//on right side of parallel - so update currRight
					pro->parentPar->currRight = program->next;
				}
			}
		}
		else if (program->oldType == 1) {
			program->type = program->oldType;
			CA *pro = static_cast<CA*>(program);

			//now update the program pointer to the next statement (depending on whether this is in a parallel statement)
			if (pro->parentPar == NULL) {
				//no parent, so not a member of a parallel statement
				current = current->next;
			}
			else {
				//this is in a parallel statement, so update either currLeft or currRight
				if (pro->left == 1) {
					//on left side of parallel - so update currLeft
					pro->parentPar->currLeft = program->next;
				}
				else {
					//on right side of parallel - so update currRight
					pro->parentPar->currRight = program->next;
				}
			}

		}
		else if (program->oldType == 2) {
			program->type = program->oldType;
			IfS *pro = static_cast<IfS*>(program);

			//now update the program pointer to the next statement (depending on whether this is in a parallel statement)
			if (pro->parentPar == NULL) {
				//no parent, so not a member of a parallel statement
				current = current->next;
			}
			else {
				//this is in a parallel statement, so update either currLeft or currRight
				if (pro->left == 1) {
					//on left side of parallel - so update currLeft
					pro->parentPar->currLeft = program->next;
				}
				else {
					//on right side of parallel - so update currRight
					pro->parentPar->currRight = program->next;
				}
			}

		}
		else if (program->oldType == 3) {
			program->type = program->oldType;
			WlS *pro = static_cast<WlS*>(program);

			//now update the program pointer to the next statement (depending on whether this is in a parallel statement)
			if (pro->parentPar == NULL) {
				//no parent, so not a member of a parallel statement
				current = current->next;
			}
			else {
				//this is in a parallel statement, so update either currLeft or currRight
				if (pro->left == 1) {
					//on left side of parallel - so update currLeft
					pro->parentPar->currLeft = program->next;
				}
				else {
					//on right side of parallel - so update currRight
					pro->parentPar->currRight = program->next;
				}
			}

		}
		else if (program->oldType == 4) {
			program->type = program->oldType;
			Par *pro = static_cast<Par*>(program);

			if (pro->parentPar == NULL) {
				//no parent, so move pointer on
				current = current->next;
			}
			else {
				//is within a parallel statement, so move currentLeft or currentRight
				if (pro->left == 1) {
					//on the left side of the parallel
					pro->parentPar->currLeft = program->next;
				}
				else {
					//on the right side of the parallel
					pro->parentPar->currRight = program->next;
				}
			}
		}
		else if (program->oldType == 5) {
			program->type = program->oldType;
			BlS *pro = static_cast<BlS*>(program);

			//now update the program pointer to the next statement (depending on whether this is in a parallel statement)
			if (pro->parentPar == NULL) {
				//no parent, so not a member of a parallel statement
				current = current->next;
			}
			else {
				//this is in a parallel statement, so update either currLeft or currRight
				if (pro->left == 1) {
					//on left side of parallel - so update currLeft
					pro->parentPar->currLeft = program->next;
				}
				else {
					//on right side of parallel - so update currRight
					pro->parentPar->currRight = program->next;
				}
			}

		}
		else if (program->oldType == 6) {
			program->type = program->oldType;
			VdS *pro = static_cast<VdS*>(program);

			//now update the program pointer to the next statement (depending on whether this is in a parallel statement)
			if (pro->parentPar == NULL) {
				//no parent, so not a member of a parallel statement
				current = current->next;
			}
			else {
				//this is in a parallel statement, so update either currLeft or currRight
				if (pro->left == 1) {
					//on left side of parallel - so update currLeft
					pro->parentPar->currLeft = program->next;
				}
				else {
					//on right side of parallel - so update currRight
					pro->parentPar->currRight = program->next;
				}
			}

		}
		else if (program->oldType == 7) {
			program->type = program->oldType;
			PdS *pro = static_cast<PdS*>(program);

			//now update the program pointer to the next statement (depending on whether this is in a parallel statement)
			if (pro->parentPar == NULL) {
				//no parent, so not a member of a parallel statement
				current = current->next;
			}
			else {
				//this is in a parallel statement, so update either currLeft or currRight
				if (pro->left == 1) {
					//on left side of parallel - so update currLeft
					pro->parentPar->currLeft = program->next;
				}
				else {
					//on right side of parallel - so update currRight
					pro->parentPar->currRight = program->next;
				}
			}

		}
		else if (program->oldType == 8) {
			program->type = program->oldType;
			PcS *pro = static_cast<PcS*>(program);

			//now update the program pointer to the next statement (depending on whether this is in a parallel statement)
			if (pro->parentPar == NULL) {
				//no parent, so not a member of a parallel statement
				current = current->next;
			}
			else {
				//this is in a parallel statement, so update either currLeft or currRight
				if (pro->left == 1) {
					//on left side of parallel - so update currLeft
					pro->parentPar->currLeft = program->next;
				}
				else {
					//on right side of parallel - so update currRight
					pro->parentPar->currRight = program->next;
				}
			}

		}
		else if (program->oldType == 9) {
			program->type = program->oldType;
			PrS *pro = static_cast<PrS*>(program);

			//now update the program pointer to the next statement (depending on whether this is in a parallel statement)
			if (pro->parentPar == NULL) {
				//no parent, so not a member of a parallel statement
				current = current->next;
			}
			else {
				//this is in a parallel statement, so update either currLeft or currRight
				if (pro->left == 1) {
					//on left side of parallel - so update currLeft
					pro->parentPar->currLeft = program->next;
				}
				else {
					//on right side of parallel - so update currRight
					pro->parentPar->currRight = program->next;
				}
			}

		}
		else if (program->oldType == 10) {
			program->type = program->oldType;
			VrS *pro = static_cast<VrS*>(program);

			//now update the program pointer to the next statement (depending on whether this is in a parallel statement)
			if (pro->parentPar == NULL) {
				//no parent, so not a member of a parallel statement
				current = current->next;
			}
			else {
				//this is in a parallel statement, so update either currLeft or currRight
				if (pro->left == 1) {
					//on left side of parallel - so update currLeft
					pro->parentPar->currLeft = program->next;
				}
				else {
					//on right side of parallel - so update currRight
					pro->parentPar->currRight = program->next;
				}
			}
		}
		else if (program->oldType == 11) {
			cout << "skip of a skip PROBLEM \n";
		}
		else if (program->oldType == 13) {
			program->type = program->oldType;
//			cout << "skip of an atomic statement\n";
			Atom *pro = static_cast<Atom*>(program);
			//now update the program pointer to the next statement (depending on whether this is in a parallel statement)
			if (pro->parentPar == NULL) {
				//no parent, so not a member of a parallel statement
				current = current->next;
			}
			else {
				//this is in a parallel statement, so update either currLeft or currRight
				if (pro->left == 1) {
					//on left side of parallel - so update currLeft
					pro->parentPar->currLeft = program->next;
				}
				else {
					//on right side of parallel - so update currRight
					pro->parentPar->currRight = program->next;
				}
			}

		}
		else {
			cout << "[S2a] error : incorrect oldType of statement - using oldType = " << program->oldType << "\n";
		}
	}
	else if (program->type == 12) {
		if (output) {
			cout << "Simulating a skip statement placeholder for an empty branch\n";
		}

		PH *pro = static_cast<PH*>(program);
		//so simply move the pointer onto the next statement
		//now update the program pointer to the next statement (depending on whether this is in a parallel statement)
		if (pro->parentPar == NULL) {
			//no parent, so not a member of a parallel statement
			current = current->next;
		}
		else {
			//this is in a parallel statement, so update either currLeft or currRight
			if (pro->left == 1) {
				//on left side of parallel - so update currLeft
				pro->parentPar->currLeft = program->next;
			}
			else {
				//on right side of parallel - so update currRight
				pro->parentPar->currRight = program->next;
			}
		}
	}

	else if (program->type == 13) {

		Atom *pro = static_cast<Atom*>(program);

		if (pro->started) {
//			cout << "Simulating the closing of an atomic section\n";
			//this is the second time through - so finishing an atomic block
			pro->started = false;

			if (*stateSaving == 1) {
				pro->idendifiers.push(*next);
				*next += 1;
				*previous += 1;
			}

			//move the pointer on
			pro->type = 11;

		}
		else {
//			cout << "Simulating a step - entering an atomic block\n";
			//this is the first time through - so opening an atomic block
			pro->started = true;

			//simply move pointer onto first step of atomic body

			if (pro->parentPar == NULL) {
//				cout << "parent is null\n";
				//no parent, so not a member of a parallel statement
				current = pro->atomBody->nextStatement();
			}
			else {
				//this is in a parallel statement, so update either currLeft or currRight
				if (pro->left == 1) {
					//on left side of parallel - so update currLeft
					pro->parentPar->currLeft = pro->atomBody->nextStatement();
				}
				else {
					//on right side of parallel - so update currRight
					pro->parentPar->currRight = pro->atomBody->nextStatement();
				}
			}

		}

	}
	else if (program->type == 99) {
		if (output) {
			cout << "Encountered an abort statement - Execution Aborted.\n";
		}

		current = NULL;
	}
	else {
		cout << "Incorrect statement type in simStep of " << program->type << "\n";
	}
	return current;
}


/*
 * Clean the current program - if a skip is the last step of a complex action; should not be executed separatly, so we `clean' it
 */
statement* clean(statement *program) {

//	cout << "called clean on statement of type " << program->type << " - last statement = " << program->last << "\n";

	if (program == NULL) {
//		cout << "call to clean - statement type NULL\n";
		return program;
	}

	//cout << "call to clean - statement type " << program->type << "\n";

	if (program->type == 11 && program->last) {
		//cout << "clean has hit type = 11\n";
		//this is a skip statement and is the end of a complex statement
		//so move this statement on

		//must cast to type of statement to access parentPar
		if (program->oldType == 0) { //destructive assignment
	//		cout << "c1\n";
			program->type = program->oldType;
			DA *pro = static_cast<DA*>(program);
	//		cout << "c2\n";

			//now update the program pointer to the next statement (depending on whether this is in a parallel statement)
			if (pro->parentPar == NULL) {
				//no parent, so not a member of a parallel statement
	//			cout << "c3\n";
				program = program->next;
	//			cout << "c4\n";
			}
			else {
	//			cout << "c5\n";
				//this is in a parallel statement, so update either currLeft or currRight
				if (pro->left == 1) {
	//				cout << "c6\n";
					//on left side of parallel - so update currLeft
					pro->parentPar->currLeft = program->next;
	//				cout << "c7 - currLeft changed to statement of type " << pro->parentPar->currLeft->type << "\n";
				}
				else {
	//				cout << "c8\n";
					//on right side of parallel - so update currRight
					pro->parentPar->currRight = program->next;
	//				cout << "c9\n";
				}
			}
		}
		else if (program->oldType == 1) { //constructive assignment
			//cout << "cleaning a constructive assignment that is skip and the end of a complex statement - set to ";
			program->type = program->oldType;
			CA *pro = static_cast<CA*>(program);

			//now update the program pointer to the next statement (depending on whether this is in a parallel statement)
			if (pro->parentPar == NULL) {
				//no parent, so not a member of a parallel statement
				program = program->next;
			}
			else {
				//this is in a parallel statement, so update either currLeft or currRight
				if (pro->left == 1) {
					//on left side of parallel - so update currLeft
					pro->parentPar->currLeft = program->next;
				}
				else {
					//on right side of parallel - so update currRight
					pro->parentPar->currRight = program->next;
				}
			}
			//cout << program->type << "\n";
		}
		else if (program->oldType == 2) {
			program->type = program->oldType;
			IfS *pro = static_cast<IfS*>(program);

			//now update the program pointer to the next statement (depending on whether this is in a parallel statement)
			if (pro->parentPar == NULL) {
				//no parent, so not a member of a parallel statement
				program = program->next;
			}
			else {
				//this is in a parallel statement, so update either currLeft or currRight
				if (pro->left == 1) {
					//on left side of parallel - so update currLeft
					pro->parentPar->currLeft = program->next;
				}
				else {
					//on right side of parallel - so update currRight
					pro->parentPar->currRight = program->next;
				}
			}

		}
		else if (program->oldType == 3) {
			program->type = program->oldType;
			WlS *pro = static_cast<WlS*>(program);

			//now update the program pointer to the next statement (depending on whether this is in a parallel statement)
			if (pro->parentPar == NULL) {
				//no parent, so not a member of a parallel statement
				program = program->next;
			}
			else {
				//this is in a parallel statement, so update either currLeft or currRight
				if (pro->left == 1) {
					//on left side of parallel - so update currLeft
					pro->parentPar->currLeft = program->next;
				}
				else {
					//on right side of parallel - so update currRight
					pro->parentPar->currRight = program->next;
				}
			}

		}
		else if (program->oldType == 4) {
			program->type = program->oldType;
			Par *pro = static_cast<Par*>(program);

			if (pro->parentPar == NULL) {
				//no parent, so move pointer on
				program = program->next;
			}
			else {
				//is within a parallel statement, so move currentLeft or currentRight
				if (pro->left == 1) {
					//on the left side of the parallel
					pro->parentPar->currLeft = program->next;
				}
				else {
					//on the right side of the parallel
					pro->parentPar->currRight = program->next;
				}
			}
		}
		else if (program->oldType == 5) {
		//	cout << "cleaning a skip statement that is the end of a program (the block is the last statement)";

			program->type = program->oldType;
			BlS *pro = static_cast<BlS*>(program);

			//now update the program pointer to the next statement (depending on whether this is in a parallel statement)
			if (pro->parentPar == NULL) {
				//no parent, so not a member of a parallel statement
				program = program->next;
			}
			else {
				//this is in a parallel statement, so update either currLeft or currRight
				if (pro->left == 1) {
					//on left side of parallel - so update currLeft
					pro->parentPar->currLeft = program->next;
				}
				else {
					//on right side of parallel - so update currRight
					pro->parentPar->currRight = program->next;
				}
			}

		}
		else if (program->oldType == 6) {
			program->type = program->oldType;
			VdS *pro = static_cast<VdS*>(program);

			//now update the program pointer to the next statement (depending on whether this is in a parallel statement)
			if (pro->parentPar == NULL) {
				//no parent, so not a member of a parallel statement
				program = program->next;
			}
			else {
				//this is in a parallel statement, so update either currLeft or currRight
				if (pro->left == 1) {
					//on left side of parallel - so update currLeft
					pro->parentPar->currLeft = program->next;
				}
				else {
					//on right side of parallel - so update currRight
					pro->parentPar->currRight = program->next;
				}
			}

		}
		else if (program->oldType == 7) {
			program->type = program->oldType;
			PdS *pro = static_cast<PdS*>(program);

			//now update the program pointer to the next statement (depending on whether this is in a parallel statement)
			if (pro->parentPar == NULL) {
				//no parent, so not a member of a parallel statement
				program = program->next;
			}
			else {
				//this is in a parallel statement, so update either currLeft or currRight
				if (pro->left == 1) {
					//on left side of parallel - so update currLeft
					pro->parentPar->currLeft = program->next;
				}
				else {
					//on right side of parallel - so update currRight
					pro->parentPar->currRight = program->next;
				}
			}

		}
		else if (program->oldType == 8) {
			program->type = program->oldType;
			PcS *pro = static_cast<PcS*>(program);

			//now update the program pointer to the next statement (depending on whether this is in a parallel statement)
			if (pro->parentPar == NULL) {
				//no parent, so not a member of a parallel statement
				program = program->next;
			}
			else {
				//this is in a parallel statement, so update either currLeft or currRight
				if (pro->left == 1) {
					//on left side of parallel - so update currLeft
					pro->parentPar->currLeft = program->next;
				}
				else {
					//on right side of parallel - so update currRight
					pro->parentPar->currRight = program->next;
				}
			}

		}
		else if (program->oldType == 9) {
			program->type = program->oldType;
			PrS *pro = static_cast<PrS*>(program);

			//now update the program pointer to the next statement (depending on whether this is in a parallel statement)
			if (pro->parentPar == NULL) {
				//no parent, so not a member of a parallel statement
				program = program->next;
			}
			else {
				//this is in a parallel statement, so update either currLeft or currRight
				if (pro->left == 1) {
					//on left side of parallel - so update currLeft
					pro->parentPar->currLeft = program->next;
				}
				else {
					//on right side of parallel - so update currRight
					pro->parentPar->currRight = program->next;
				}
			}

		}
		else if (program->oldType == 10) {
			program->type = program->oldType;
			VrS *pro = static_cast<VrS*>(program);

			//now update the program pointer to the next statement (depending on whether this is in a parallel statement)
			if (pro->parentPar == NULL) {
				//no parent, so not a member of a parallel statement
				program = program->next;
			}
			else {
				//this is in a parallel statement, so update either currLeft or currRight
				if (pro->left == 1) {
					//on left side of parallel - so update currLeft
					pro->parentPar->currLeft = program->next;
				}
				else {
					//on right side of parallel - so update currRight
					pro->parentPar->currRight = program->next;
				}
			}
		}
		else {
			cout << "oldType of statement to be cleaned is not between 1 and 10\n";
		}


	}
	else if (program->type == 4) {
		//a parallel statement; must consider the current position within each side of the parallel
		Par *temp = static_cast<Par*>(program);

	//	if (temp->seenLeft && temp->seenRight) {
	//		cout << "clean called on a parallel that has finished\n";
	//	}
	//	else {
	//		cout << "both sides not finished\n";
	//	}

		if (!temp->seenLeft) {
			clean(temp->currLeft);
		}
		if (!temp->seenRight) {
			clean(temp->currRight);
		}


		if ((temp->currLeft == NULL) && (temp->currRight == temp)) {
			//ignore
			temp->seenRight = true;
		}
		else if ((temp->currLeft == temp) && (temp->currRight == NULL)) {
			//ignore
			temp->seenLeft = true;
		}
		else if (temp->currLeft == temp) {
		//	cout << "left is finished\n";
			//auto close the left side
			temp->seenLeft = true;
			temp->currLeft = NULL;
		}
		else if (temp->currRight == temp) {
		//	cout << "right is finished\n";
			//auto close the right side
			temp->seenRight = true;
			temp->currRight = NULL;
		}

	}

	return program;
}

bool simulator::executeNSteps(int n, bool forceRandom, bool forceUser, bool forceSequence, string* sequence, bool atom, bool output) {

	srand(time(0)); //set the random number generation for random interleaving decisions

	int original = n; //copy of n for comparison after execution

	int stepCount = 0;

	bool simulationFinished = false;
	bool all = false;

	bool userCancelled = false;

	statement *temp = new statement;
	temp = placeInExecution; //current position within the execution

	int userChoice = -1;
	string userChoiceString = "";

	//if the number of steps to execute is -1, the user wants the entire execution
	if (n == -1) {
		all = true;
	}

	if (output) {
		if (all) {
			if (!forceUser) {
				cout << "--Simulating entire execution ... \n";
			}
			else {
				cout << "--Simulating entire execution\n";
			}

		} //else the output will be provided by the interface
	}

	while ((n != 0) && (!simulationFinished) && (!userCancelled)) {
		stepCount += 1;

		//get the options now
		set<statement*> nextOptions;
		nextOptions = getOptions(placeInExecution);

		if (nextOptions.size() == 0) { //no available options - so the execution has finished
			simulationFinished = true;
		}
		else { //there is at least one statement to execute
			int randomChoice = -1;

			if (nextOptions.size() == 1) { //only one option, so we must follow it
				statement* onlyChoice = new statement;
				for (std::set<statement*>::iterator it=nextOptions.begin(); it!=nextOptions.end(); ++it) {
					onlyChoice = *it;
				}

				//add the information to the trace
				string toAddToTrace = "";
				if (recordTrace) {
					toAddToTrace = intToString11(stepCount) + ". " + stringOfOptions(nextOptions);
					int lengthOfFirst = toAddToTrace.length();
					int spaceToAdd = 100 - lengthOfFirst;
					for (int i = 0; i < spaceToAdd; i++) {
						toAddToTrace = toAddToTrace + " ";
					}
					toAddToTrace = toAddToTrace + "---> ";
					toAddToTrace = toAddToTrace + stringOfSingleOption(onlyChoice);
					//pointToTrace->push_back(toAddToTrace);
				}

				if (output) {
					cout << "----Step " << stepCount << ": ";
				}


		//		if (placeInExecution != NULL) {
		//			cout << "--before execution = " << placeInExecution->type << "\n";
		//		}

		//		if (onlyChoice != NULL) {
		//			cout << "--before execution choice = " << onlyChoice->type << "\n";
		//		}

	//			cout << "-------------Executing step - " << onlyChoice->type << " (temp type = " << temp->type << ") " << "\n";
				placeInExecution = simStep(pp,onlyChoice,temp,as,se,gs,pe,we,next,previous,nextName,pi,nextLoc,stateSaving,sequenceOfRules,recordRules,atom,AI,output);
				//cout << "--after simstep = " << placeInExecution->type << "\n";

	//			cout << "-------------Calling clean on - " << placeInExecution->type << " (temp type = " << temp->type << "." << "\n";
//				cout << "starting clean\n";
				placeInExecution = clean(placeInExecution);
//				cout << "return from clean\n";
				temp = placeInExecution;

		//		if (placeInExecution != NULL) {
		//			cout << "-------------after clean = " << placeInExecution->type << "\n";
		//		}

			//	if (placeInExecution == NULL) {
			//		cout << "placeInEx is NULL\n";
			//	}



				//add the information to the trace
				if (recordTrace) {

					toAddToTrace = toAddToTrace + " via rule " + sequenceOfRules->at(sequenceOfRules->size()-1);

					pointToTrace->push_back(toAddToTrace);
				}

			}
			else { //there is at least 2 options - we must make a choice (either random or by user)
				statement *choice = new statement;

				if (forceRandom) {
					set<statement*>::const_iterator it(nextOptions.begin());
					randomChoice = rand() % nextOptions.size();
					advance(it,randomChoice);
					choice = *it;
				}
				else if (forceUser) {
					cout << "The next step is the non-deterministic choice between the following multiple statements. \n";

					std::tr1::unordered_map<int,statement* > numberedOptions = displayNumberedOptions(nextOptions);

					for (int i = 1; i <= nextOptions.size(); i++) {
						cout << i << ": " << displayStatement(numberedOptions[i],true) << "\n";
					}

					bool validChoice = false;
					while (!validChoice) {
						cout << "Pick a statement to execute: ";
						getline(cin, userChoiceString);
						//cout << "user has chosen\n";
						if (userChoiceString == "cancel") {
							cout << "User wants to cancel execution\n";
							userCancelled = true;
							validChoice = true;
							simulationFinished = true;
						}
						else {
							userChoice = stringToInt11(userChoiceString);
							if ((0 < userChoice) && (userChoice <= nextOptions.size()) && (userChoice != 0)) {
								//this is valid
								validChoice = true;
							}
							else {
								cout << "Invalid input - please try again\n";
							}
						}
					}
					if (validChoice) {
						choice = numberedOptions[userChoice];
					}

				}
				else {
					cout << "NOT CLEAR HOW TO HANDLE INTERLEAVING\n";
				}

				if (!userCancelled) { //as long as the choice was not to cancel, we now execute it

					//add the information to the trace
					string toAddToTrace = "";
					if (recordTrace) {
						toAddToTrace = intToString11(stepCount) + ". " + stringOfOptions(nextOptions);
						int lengthOfFirst = toAddToTrace.length();
						int spaceToAdd = 100 - lengthOfFirst;
						for (int i = 0; i < spaceToAdd; i++) {
							toAddToTrace = toAddToTrace + " ";
						}
						toAddToTrace = toAddToTrace + "---> ";
						toAddToTrace = toAddToTrace + stringOfSingleOption(choice);
					}


					if (output) {
						cout << "----Step " << stepCount << ": ";
					}
					placeInExecution = simStep(pp,choice,temp,as,se,gs,pe,we,next,previous,nextName,pi,nextLoc,stateSaving,sequenceOfRules,recordRules,atom,AI,output);
//					cout << "starting clean - aa\n";
					placeInExecution = clean(placeInExecution);
//					cout << "ending clean - aa\n";
					temp = placeInExecution;

					//add the information to the trace
					if (recordTrace) {
						toAddToTrace = toAddToTrace + " via rule " + sequenceOfRules->at(sequenceOfRules->size()-1);
						pointToTrace->push_back(toAddToTrace);
					}


				}
			}

			nextOptions = getOptions(placeInExecution); //update the options for the next step of execution

	//		if (placeInExecution != NULL) {
	//			cout << "calling get options on statement of type " << placeInExecution->type << " (old type " << placeInExecution->oldType << ") : size = " << nextOptions.size() << "\n";
	//		}
			//cout << "number of available options: " << nextOptions.size() << "\n";

			//now decide whether we have completed the execution or not
			if (output) {
				if (nextOptions.size() == 0) {
					if (all) {
						if (!forceUser) {
							cout << "--done (completed in " << stepCount << " steps)\n";
						}
						else {
							cout << "--Execution complete (completed after " << stepCount << " steps)\n";
						}
					}
					else {
						if (!forceUser) {
							if ((n-1) == 0) {
								//all steps were executed
								cout << "--done (completed after " << (original-(n)) << " steps)\n";
							}
							else {
								cout << "--done (completed after " << (original-(n)) << " steps)\n";
							}
						}
						else {
							if ((n-1) == 0) {
								cout << "--Execution complete\n";
							}
							else {
								cout << "--Execution complete (completed after " << (original-(n-1)) << " steps)\n";
							}
						}
					}

					//save the forwards trace to file now as the execution is finished
					fileIO sfio = fileIO();
					if (recordTrace == true) {
						cout << "--Saving the trace of forwards execution to file: execution-traces/trace.txt\n";
						sfio.writeTrace("execution-traces/trace.txt", simulator::getTrace());
					}
					//return true;
				}
				else {
				}
			}
			if (nextOptions.size() == 0) {
				return true;
			}

		}
		n -= 1;

		if (userCancelled) {
			break;
		}
	}

	if (output) {
		//finally, the while loop has finished all n steps (or as many as were available)
		if (!forceUser) {
			if ((n) == 0) {
				//all steps were executed
				cout << "--done (completed after " << (original-(n)) << " steps)\n";
			}
			else {
				cout << "--done (completed after " << (original-(n)) << " steps)\n";
			}
		}
		else {
			if ((n-1) == 0) {
				cout << "--Execution complete\n";
			}
			else {
				cout << "--Execution complete (completed after " << (original-(n)) << " steps)\n";
			}

		}
	}
	return false;
}

/*
 * Alternative method that can be called to execute an entire execution - not used in the updated interface
 */
void simAll(parser *pp, linkedList *program, auxStore st, scopeEnvironment se, gammaSigma sg, procedureEnvironment mu, whileEnvironment we, int *next, int *previous,
		nextNames nextNames, procedureIdentifiers pi, int *nextLoc, int *stateSaving, bool recordTrace, vector<string> *trace, bool recordRules,
		vector<string> *rules, bool atomEval, AnnInfo AI, bool output) {

	statement *temp = new statement;
	temp = program->nextStatement();

	set<statement*> nextOptions;
	nextOptions = getOptions(temp);

	srand(time(NULL));
	int randomChoice = -1;
	int count = 0;

	while (nextOptions.size() != 0) {
		count += 1;
		if (nextOptions.size() == 1) {
			//only one option, so we must follow it
			statement* onlyChoice = new statement;
			for (std::set<statement*>::iterator it=nextOptions.begin(); it!=nextOptions.end(); ++it) {
				onlyChoice = *it;
			}

			//add the information to the trace
			if ((recordTrace) && (*stateSaving == 1)) {
				string toAddToTrace = stringOfOptions(nextOptions);
				toAddToTrace = toAddToTrace + "\t  ---> ";
				toAddToTrace = toAddToTrace + stringOfSingleOption(onlyChoice);
				trace->push_back(toAddToTrace);
			}

			simStep(pp,onlyChoice,temp,st,se,sg,mu,we,next,previous,nextNames,pi,nextLoc,stateSaving,rules,recordRules,atomEval,AI,output);
		}
		else {
			//more than one option, so we must make a random choice
			set<statement*>::const_iterator it(nextOptions.begin());
			randomChoice = rand() % nextOptions.size();
			advance(it,randomChoice);
			statement* choice = new statement;
			choice = *it;

			//add the information to the trace
			if ((recordTrace) && (*stateSaving == 1)) {
				string toAddToTrace = stringOfOptions(nextOptions);
				toAddToTrace = toAddToTrace + "\t  ---> ";
				toAddToTrace = toAddToTrace + stringOfSingleOption(choice);
				trace->push_back(toAddToTrace);
			}

			//random choice now made, so now execute a step of this
			simStep(pp,choice,temp,st,se,sg,mu,we,next,previous,nextNames,pi,nextLoc,stateSaving,rules,recordRules,atomEval,AI,output);
		}
		//update the options for the next step of execution
		nextOptions = getOptions(temp);

	}
	cout << "count = " << count << "\n";
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////// Reverse Execution /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//change the full string representing an increment or a decrement - replacing += with -= and vice versa
string changeFullCon(string original, bool currOp) {
	string before = original;
	string after = original;
	string toReturn = "";
	if (currOp) {
		//looking for -
		int i = original.find_first_of("-");
		before = before.erase(i-1);
		after = after.erase(0,i+1);
		toReturn = before + " +" + after;
	}
	else {
		//looking for +
		int i = original.find_first_of("+");
		before = before.erase(i-1);
		after = after.erase(0,i+1);
		toReturn = before + " -" + after;
	}

	return toReturn;
}

/*
 * Invert a program - called on a final annotated version of a program
 * Inverts the entire program statement order
 * Increments and decrements are inverted
 * Declarations become removals, and removals become declarations
 */
linkedList* invert(linkedList* orig, statement *stopper) {
	linkedList* toReturn = new linkedList();

	statement *currState = orig->nextStatement(), *prevState = NULL, *nextState = NULL;

	while (currState != stopper) {
		//update the pointers to now reverse the order of the forwards links
		nextState = currState->next;
		currState->next = prevState;
		currState->last = false;

		if (currState->type == 0) { //no change is required
			//DA *tem = static_cast<DA*>(currState);
		}
		else if (currState->type == 1) {
			CA *tem = static_cast<CA*>(currState);
			tem->inc = !tem->inc; //change increment to decrement, and decrement to increment
			tem->full = changeFullCon(tem->full,tem->inc); //update full string representation of the constructive update
		}
		else if (currState->type == 2) {
			//conditional statement
			((IfS*) currState)->trueBranch = invert(((IfS*) currState)->trueBranch,currState);
			((IfS*) currState)->falseBranch = invert(((IfS*) currState)->falseBranch,currState);
			((IfS*) currState)->seen = false;
		}
		else if (currState->type == 3) {
			//while loop
			((WlS*) currState)->loopBody = invert(((WlS*) currState)->loopBody,currState);
		}
		else if (currState->type == 4) {
			//parallel statement
			Par *curr = static_cast<Par*>(currState);

			curr->leftSide = invert(curr->leftSide,curr);
			curr->rightSide = invert(curr->rightSide,curr);

			curr->currLeft = curr->leftSide->nextStatement();
			curr->currRight = curr->rightSide->nextStatement();
			curr->seenLeft = false;
			curr->seenRight = false;
		}
		else if (currState->type == 5) {
			//block statement
			BlS *curr = static_cast<BlS*>(currState);
			curr->blockBody = invert(curr->blockBody,currState);
			curr->seen = false;
		}
		else if (currState->type == 6) {
			//local variable declaration
			currState->type = 10;
		}
		else if (currState->type == 7) {
			PdS *curr = static_cast<PdS*>(currState);
			curr->procBodyList = invert(curr->procBodyList,NULL); //must take into account the procBody (that is a statement)
			curr->type = 9;
		}
		else if (currState->type == 9) {
			PrS *curr = static_cast<PrS*>(currState);
			curr->procBodyList = invert(curr->procBodyList,NULL); //must take into account the procBody (that is a statement)
			curr->type = 7;
		}
		else if (currState->type == 10) {
			//local variable removal
			currState->type = 6;
		}
		else if (currState->type == 11) {
			//this will be an error - a statement in skip form has not been completed - this is not the final annotated version
			cout << "Error in invert function - incomplete statement of type 11\n";
		}
		prevState = currState;
		currState = nextState;
	}
	toReturn->setStatement(prevState);
	return toReturn;
}

/*
 * Inverse Simulation of condition evaluation - no actual evaluation - the result is taken from a stack
 * Evaluating either conditional statement (stackName = B) or loop (stackName = W)
 */
bool invSimCondition(string stackName, auxStore st, int* previous) {


	//retrieve the top element of the given stackname
	int tEf = (*st.aux)[stackName].top().first; //identifier from the pair
	int tEs = (*st.aux)[stackName].top().second; //either true or false value stored


	if (tEf == *previous) { //this is the correct peice of reversal information
		(*st.aux)[stackName].pop(); //remove the pair as we are going to use it now

		if (tEs == 1) {
			return true;
		}
		else {
			return false;
		}
	}
	else {
		cout << "ERROR: Comparing " << tEf << " with " << *previous << ".\n";
		cout << "ERROR: sim inv condition with wrong identifier in reversal information\n";
	}

	//error has occurred in the evaluation - should make this clear
	return false;
}

/*
 * Opposite of function that got the next version number of a given construct identifier - this returns the previous one
 * Does update the value held within the next name function to maintain correct use
 */
string getPreviousVersion(string baseConstructName,nextNames nnf) {

	int nextVal = 0;

	std::tr1::unordered_map<std::string,int >::const_iterator got = nnf.nn->find(baseConstructName);

	if (got == nnf.nn->end()) {
		std::cout << "reduced name not found-3";
	}
	else {
		nextVal = got->second;
	}

	//finally update this value for future use
	((*nnf.nn)[baseConstructName]) = nextVal-1;

	string nextValString = intToString11(nextVal);

	return baseConstructName + "." + nextValString;
}


void invRenameLoopBodyHelper(statement *temp, statement *stopper, nextNames nnf) {
	while (temp != stopper) {

		if (temp->type == 2) {
			IfS *origSt = static_cast<IfS*>(temp);
			string oldWhileLoopName = origSt->condID;
			string baseConstructName = removeVersionOfBlockName(origSt->condID);
			string newBlockName = getPreviousVersion(baseConstructName,nnf);
			origSt->condID = newBlockName;

			invRenameLoopBodyHelper(origSt->trueBranch->nextStatement(),origSt,nnf);
			invRenameLoopBodyHelper(origSt->falseBranch->nextStatement(),origSt,nnf);
		}
		else if (temp->type == 3) {
			//nested while loop
			WlS *origSt = static_cast<WlS*>(temp);
			string oldWhileLoopName = origSt->WID;
			string baseConstructName = removeVersionOfBlockName(origSt->WID);
			string newBlockName = getPreviousVersion(baseConstructName,nnf);
			origSt->WID = newBlockName;

			//now call on the body

		}
		else if (temp->type == 5) {
			//block statement
			BlS *origSt = static_cast<BlS*>(temp);

			string oldBlockNameHere = origSt->bid;

			string baseConstructName = removeVersionOfBlockName(origSt->bid);

			string newBlockName = getPreviousVersion(baseConstructName,nnf);

			//cout << "\n\n\n new blockname = " << newBlockName << "\n\n\n";

			origSt->bid = newBlockName;

			//now reflect the change of this block name into all paths of its inner statements
			updatePathsWithBlockName(origSt->blockBody,origSt,oldBlockNameHere,newBlockName);

			//now update any constructs in the block body
			//renameLoopBodyHelper(origSt->blockBody,origSt,nnf);
		}
		else if (temp->type == 8) {
			PcS *origSt = static_cast<PcS*>(temp);
			string oldWhileLoopName = origSt->callID;
			string baseConstructName = removeVersionOfBlockName(origSt->callID);
			string newBlockName = getPreviousVersion(baseConstructName,nnf);
			origSt->callID = newBlockName;
		}

		temp = temp->next;

	}
}

void invRenameLoopBody(statement *body, statement *stopper, nextNames nnf) {

	WlS *originalLoop = static_cast<WlS*>(body);

	statement *temp = originalLoop->loopBody->nextStatement();

	while (temp != originalLoop) {

		if (temp->type == 2) {
			IfS *origSt = static_cast<IfS*>(temp);
			string oldWhileLoopName = origSt->condID;
			string baseConstructName = removeVersionOfBlockName(origSt->condID);
			string newBlockName = getPreviousVersion(baseConstructName,nnf);
			origSt->condID = newBlockName;

			invRenameLoopBodyHelper(origSt->trueBranch->nextStatement(),origSt,nnf);
			invRenameLoopBodyHelper(origSt->falseBranch->nextStatement(),origSt,nnf);
		}
		else if (temp->type == 3) {
			//nested while loop
			WlS *origSt = static_cast<WlS*>(temp);
			string oldWhileLoopName = origSt->WID;
			string baseConstructName = removeVersionOfBlockName(origSt->WID);
			string newBlockName = getPreviousVersion(baseConstructName,nnf);
			origSt->WID = newBlockName;

			//now call on the body

		}
		else if (temp->type == 5) {
			//block statement
			BlS *origSt = static_cast<BlS*>(temp);

			string oldBlockNameHere = origSt->bid;

			string baseConstructName = removeVersionOfBlockName(origSt->bid);

			string newBlockName = getPreviousVersion(baseConstructName,nnf);

			//cout << "\n\n\n new blockname = " << newBlockName << "\n\n\n";

			origSt->bid = newBlockName;

			//now reflect the change of this block name into all paths of its inner statements
			updatePathsWithBlockName(origSt->blockBody,origSt,oldBlockNameHere,newBlockName);

			//now update any constructs in the block body
			//renameLoopBodyHelper(origSt->blockBody,origSt,nnf);
		}
		else if (temp->type == 8) {
			PcS *origSt = static_cast<PcS*>(temp);
			string oldWhileLoopName = origSt->callID;
			string baseConstructName = removeVersionOfBlockName(origSt->callID);
			string newBlockName = getPreviousVersion(baseConstructName,nnf);
			origSt->callID = newBlockName;
		}

		temp = temp->next;

	}

}


void invRenameProcBodyLoop(statement* body, statement* stopper, string callID, nextNames nnf) {

	//cout << "IN RENAME PROC BODY----------------------------------------type = " << body->type << "\n";
	while (body != stopper) {
		//cout << "type = " << body->type << "\n";
		if (body->type == 2) {
			//cout << "body type is 2 starting here\n";
			IfS *temp = static_cast<IfS*>(body);
			string oldIfName = temp->condID;
			string newBlockNameHere = callID + ":" + oldIfName;
			temp->condID = newBlockNameHere;

			invRenameProcBodyLoop(temp->trueBranch->nextStatement(),temp,callID,nnf);
			invRenameProcBodyLoop(temp->falseBranch->nextStatement(),temp,callID,nnf);
		}

		else if (body->type == 3) {
			WlS *temp = static_cast<WlS*>(body);
			string oldIfName = temp->WID;
			string newBlockNameHere = callID + ":" + oldIfName;
			temp->WID = newBlockNameHere;

			invRenameProcBodyLoop(temp->loopBody->nextStatement(),temp,callID,nnf);
		}
		else if (body->type == 4) {
			Par *temp = static_cast<Par*>(body);
			invRenameProcBodyLoop(temp->leftSide->nextStatement(),temp,callID,nnf);
			invRenameProcBodyLoop(temp->rightSide->nextStatement(),temp,callID,nnf);
		}
		else if (body->type == 5) {
			BlS *temp = static_cast<BlS*>(body);
			string oldBlockName = temp->bid;
			string newBlockNameHere = callID + ":" + temp->bid;
			temp->bid = newBlockNameHere;

			updatePathsWithBlockName(temp->blockBody,temp,oldBlockName,newBlockNameHere);
			invRenameProcBodyLoop(temp->blockBody->nextStatement(),temp,callID,nnf);
		}
		else if (body->type == 7) {
			//cout << "im here\n";
			//proc dec
			PdS *temp = static_cast<PdS*>(body);
			temp->procIden = callID + ":" + temp->procIden;
			//renameProcBody(temp->procBodyList->nextStatement(),NULL,callID);
		}
		else if (body->type == 8) {
			PcS *temp = static_cast<PcS*>(body);

			cout << "-----------------------RENAME HERE\n";

			string oldWhileLoopName = temp->callID;
			string baseConstructName = removeVersionOfBlockName(temp->callID);
			string newBlockName = getPreviousVersion(baseConstructName,nnf);
			temp->callID = newBlockName;

			//spot a recursive call
			if (temp->callID == callID) {
				//this is a recursive call
			}
			else {
				temp->callID = callID + ":" + temp->callID;
			}
		}
		else if (body->type == 9) {
			//cout << "im here\n";
			//proc dec
			PrS *temp = static_cast<PrS*>(body);
			temp->procIden = callID + ":" + temp->procIden;
			//renameProcBody(temp->procBodyList->nextStatement(),NULL,callID);
		}

		body = body->next;
	}

}

void invRenameProcBody(statement* body, statement* stopper, string callID, nextNames nnf) {

//	cout << "IN RENAME PROC BODY----------------------------------------type = " << body->type << "\n";
	while (body != stopper) {
		//cout << "type = " << body->type << "\n";
		if (body->type == 2) {
			//cout << "body type is 2 starting here\n";
			IfS *temp = static_cast<IfS*>(body);
			string oldIfName = temp->condID;
			string newBlockNameHere = callID + ":" + oldIfName;
			temp->condID = newBlockNameHere;

			invRenameProcBody(temp->trueBranch->nextStatement(),temp,callID,nnf);
			invRenameProcBody(temp->falseBranch->nextStatement(),temp,callID,nnf);
		}

		else if (body->type == 3) {
			WlS *temp = static_cast<WlS*>(body);
			string oldIfName = temp->WID;
			string newBlockNameHere = callID + ":" + oldIfName;
			temp->WID = newBlockNameHere;

			invRenameProcBodyLoop(temp->loopBody->nextStatement(),temp,callID,nnf);
		}
		else if (body->type == 4) {
			Par *temp = static_cast<Par*>(body);
			invRenameProcBody(temp->leftSide->nextStatement(),temp,callID,nnf);
			invRenameProcBody(temp->rightSide->nextStatement(),temp,callID,nnf);
		}
		else if (body->type == 5) {
			BlS *temp = static_cast<BlS*>(body);
			string oldBlockName = temp->bid;
			string newBlockNameHere = callID + ":" + temp->bid;
			temp->bid = newBlockNameHere;

			updatePathsWithBlockName(temp->blockBody,temp,oldBlockName,newBlockNameHere);
			invRenameProcBody(temp->blockBody->nextStatement(),temp,callID,nnf);
		}
		else if (body->type == 7) {
			//cout << "im here\n";
			//proc dec
			PdS *temp = static_cast<PdS*>(body);
			temp->procIden = callID + ":" + temp->procIden;
			//renameProcBody(temp->procBodyList->nextStatement(),NULL,callID);
		}
		else if (body->type == 8) {
			PcS *temp = static_cast<PcS*>(body);

			//spot a recursive call
			if (temp->callID == callID) {
				//this is a recursive call
			}
			else {
				temp->callID = callID + ":" + temp->callID;
			}
		}
		else if (body->type == 9) {
			//cout << "im here\n";
			//proc dec
			PrS *temp = static_cast<PrS*>(body);
			temp->procIden = callID + ":" + temp->procIden;
			//renameProcBody(temp->procBodyList->nextStatement(),NULL,callID);
		}

		body = body->next;
	}

}

void setWhileLoopAIHelper(statement *start, statement *stopper, string *AI) {
	//start IS NOT REQUIRED to be a while loop here
	statement *temp = start;

	while (temp != stopper) {
		if (temp->type == 0) {
			DA *i = static_cast<DA*>(temp);
			string singleID = getNextString(AI);
			stack<int> t = makeStackReverse(singleID);
			i->idendifiers = t;
		}
		else if (temp->type == 1) {
			CA *i = static_cast<CA*>(temp);
			string singleID = getNextString(AI);
			stack<int> t = makeStackReverse(singleID);
			i->idendifiers = t;
		}
		else if (temp->type == 2) {
			IfS *i = static_cast<IfS*>(temp);
			string singleID = getNextString(AI);
			stack<int> t = makeStackReverse(singleID);
			i->idendifiers = t;
			setWhileLoopAIHelper(i->trueBranch->nextStatement(),i,AI);
			setWhileLoopAIHelper(i->falseBranch->nextStatement(),i,AI);
		}
		else if (temp->type == 3) {
			WlS *nested = static_cast<WlS*>(temp);
			cout << "IN HERE-----------------------------------------------------------\n";
			setWhileLoopAIHelper(nested->loopBody->nextStatement(),nested,AI);
		}
		else if (temp->type == 4) {
			Par *i = static_cast<Par*>(temp);
			setWhileLoopAIHelper(i->leftSide->nextStatement(),i,AI);
			setWhileLoopAIHelper(i->rightSide->nextStatement(),i,AI);
		}
		else if (temp->type == 5) {
			BlS *i = static_cast<BlS*>(temp);
			//setAnnotationInformationWhile(i->blockBody->nextStatement(),i,AI);
		}
		else if (temp->type == 6) {
			VdS *i = static_cast<VdS*>(temp);
			string singleID = getNextString(AI);
			stack<int> t = makeStackReverse(singleID);
			i->idendifiers = t;
		}
		else if (temp->type == 7) {
			PdS *i = static_cast<PdS*>(temp);
			string singleID = getNextString(AI);
			stack<int> t = makeStackReverse(singleID);
			i->idendifiers = t;
		}
		else if (temp->type == 8) {
			PcS *i = static_cast<PcS*>(temp);
			string singleID = getNextString(AI);
			stack<int> t = makeStackReverse(singleID);
			i->idendifiers = t;
		}
		else if (temp->type == 9) {
			PrS *i = static_cast<PrS*>(temp);
			string singleID = getNextString(AI);
			stack<int> t = makeStackReverse(singleID);
			i->idendifiers = t;
		}
		else if (temp->type == 10) {
			VrS *i = static_cast<VrS*>(temp);
			string singleID = getNextString(AI);
			stack<int> t = makeStackReverse(singleID);
			i->idendifiers = t;
		}
		else if (temp->type == 12) {
			string singleID = getNextString(AI);

		}

		temp = temp->next;
	}
}


void setWhileLoopAI(statement *start, string *AI) {
	//start must be the while loop in which the identifiers must be inserted
	//	cout << "start - type of start = " << start->type << "\n";
	WlS *whileLoop = static_cast<WlS*>(start);
	//	cout << "cast\n";
	string singleID = getNextString(AI);
	//	cout << "singleID = " << singleID << "\n";

	stack<int> t = makeStackReverse(singleID);
	//	cout << "stack size = " << t.size() << "\n";

	//	cout << "making stack\n";
	whileLoop->idendifiers = t;

	//	cout << "outer identifiers done\n";

	//now update the body of the loop
	statement *temp = whileLoop->loopBody->nextStatement();

	while (temp != whileLoop) {
		//	cout << "type of temp = " << temp->type << " and AI = " << *AI << "\n";
		if (temp->type == 0) {
			DA *i = static_cast<DA*>(temp);
			string singleID = getNextString(AI);
			stack<int> t = makeStackReverse(singleID);
			i->idendifiers = t;
		}
		else if (temp->type == 1) {
			CA *i = static_cast<CA*>(temp);
			string singleID = getNextString(AI);
			stack<int> t = makeStackReverse(singleID);
			i->idendifiers = t;
		}
		else if (temp->type == 2) {
			//cout << "PLEASE UPDATE setWhileLoopAI FOR STATEMENTS OF TYPE 2\n";

			IfS *i = static_cast<IfS*>(temp);
			string singleID = getNextString(AI);
			stack<int> t = makeStackReverse(singleID);
			i->idendifiers = t;
			setWhileLoopAIHelper(i->trueBranch->nextStatement(),i,AI);
			setWhileLoopAIHelper(i->falseBranch->nextStatement(),i,AI);

		}
		else if (temp->type == 3) {
			WlS *nested = static_cast<WlS*>(temp);
			setWhileLoopAI(nested,AI);
		}
		else if (temp->type == 4) {
			Par *i = static_cast<Par*>(temp);
			setWhileLoopAIHelper(i->leftSide->nextStatement(),i,AI);
			setWhileLoopAIHelper(i->rightSide->nextStatement(),i,AI);
		}
		else if (temp->type == 5) {
			BlS *i = static_cast<BlS*>(temp);
			//setAnnotationInformationWhile(i->blockBody->nextStatement(),i,AI);
			setWhileLoopAIHelper(i->blockBody->nextStatement(),i,AI);
		}
		else if (temp->type == 6) {
			VdS *i = static_cast<VdS*>(temp);
			string singleID = getNextString(AI);
			stack<int> t = makeStackReverse(singleID);
			i->idendifiers = t;
		}
		else if (temp->type == 7) {
			PdS *i = static_cast<PdS*>(temp);
			string singleID = getNextString(AI);
			stack<int> t = makeStackReverse(singleID);
			i->idendifiers = t;
		}
		else if (temp->type == 8) {
			PcS *i = static_cast<PcS*>(temp);
			string singleID = getNextString(AI);
			stack<int> t = makeStackReverse(singleID);
			i->idendifiers = t;
		}
		else if (temp->type == 9) {
			PrS *i = static_cast<PrS*>(temp);
			string singleID = getNextString(AI);
			stack<int> t = makeStackReverse(singleID);
			i->idendifiers = t;
		}
		else if (temp->type == 10) {
			VrS *i = static_cast<VrS*>(temp);
			string singleID = getNextString(AI);
			stack<int> t = makeStackReverse(singleID);
			i->idendifiers = t;
		}
		else if (temp->type == 12) {

		}
		temp = temp->next;
	}

}



void setAnnotationInformationWhile(statement *start, statement *stopper, string *AI) {

	//cout << "Calling set annotation information while with string = " << *AI << "\n";

	statement *temp = new statement;
	temp = start;

	while (temp != stopper) {
		//	cout << "type of temp = " << temp->type << " and AI = " << *AI << "\n";
		if (temp->type == 0) {
			DA *i = static_cast<DA*>(temp);
			string singleID = getNextString(AI);
			stack<int> t = makeStackReverse(singleID);
			i->idendifiers = t;
		}
		else if (temp->type == 1) {
			CA *i = static_cast<CA*>(temp);
			//	cout << "statement CA = " << i->full << "\n";
			string singleID = getNextString(AI);
			//	cout << "Setting AI info to a constructive assignment (" << i->full << ") - info to push = " << singleID << ". original AI = " << *AI << ".\n";
			stack<int> t = makeStackReverse(singleID);
			i->idendifiers = t;
		}
		else if (temp->type == 2) {
			IfS *i = static_cast<IfS*>(temp);
			string singleID = getNextString(AI);
			stack<int> t = makeStackReverse(singleID);
			i->idendifiers = t;
			setAnnotationInformationWhile(i->trueBranch->nextStatement(),i,AI);
			setAnnotationInformationWhile(i->falseBranch->nextStatement(),i,AI);
		}
		else if (temp->type == 3) {
			WlS *i = static_cast<WlS*>(temp);
			string singleID = getNextString(AI);
			//	cout << "\nidentifiers for loop itself = " << singleID << "\n";
			//	cout << "updated AI string = " << *AI << "\n\n";
			stack<int> t = makeStackReverse(singleID);
			i->idendifiers = t;
			//	cout << "nested while ---- ";
			//	cout << "************************************Calling setAI for the nested while loop\n";
			setAnnotationInformationWhile(i->loopBody->nextStatement(),i,AI);
			//	cout << "************************************END OF Calling setAI for the nested while loop\n";
		}
		else if (temp->type == 4) {
			Par *i = static_cast<Par*>(temp);
			setAnnotationInformationWhile(i->leftSide->nextStatement(),i,AI);
			setAnnotationInformationWhile(i->rightSide->nextStatement(),i,AI);
		}
		else if (temp->type == 5) {
			BlS *i = static_cast<BlS*>(temp);
			setAnnotationInformationWhile(i->blockBody->nextStatement(),i,AI);
		}
		else if (temp->type == 6) {
			VdS *i = static_cast<VdS*>(temp);
			string singleID = getNextString(AI);
			stack<int> t = makeStackReverse(singleID);
			i->idendifiers = t;
		}
		else if (temp->type == 7) {
			PdS *i = static_cast<PdS*>(temp);
			string singleID = getNextString(AI);
			stack<int> t = makeStackReverse(singleID);
			i->idendifiers = t;
		}
		else if (temp->type == 8) {
			PcS *i = static_cast<PcS*>(temp);
			string singleID = getNextString(AI);
			stack<int> t = makeStackReverse(singleID);
			i->idendifiers = t;
		}
		else if (temp->type == 9) {
			PrS *i = static_cast<PrS*>(temp);
			string singleID = getNextString(AI);
			stack<int> t = makeStackReverse(singleID);
			i->idendifiers = t;
		}
		else if (temp->type == 10) {
			VrS *i = static_cast<VrS*>(temp);
			string singleID = getNextString(AI);
			stack<int> t = makeStackReverse(singleID);
			i->idendifiers = t;
		}

		temp = temp->next;
	}

	//cout << "Execution finished\n";

}


/*
 * Simulate a step of the inverse execution. Depending on the type of statement and how far through its execution it currently is.
 * Returns the pointer to the statement next to be executed in the same 'thread' of execution
 */
statement* invSimStep(parser *pp, statement *program, statement *&current, auxStore st, scopeEnvironment se, gammaSigma sg,  procedureEnvironment mu,
		int *previous, whileEnvironment we, int* next, nextNames nextNames, procedureIdentifiers pi, int *nextLoc, int *stateSaving,
		vector<string> *rulesInv, bool recordRulesInv, AnnInfo AI, bool output) {

	if (program->type == 0) {

		if (output) {
			cout << "Simulating Inverse Destructive Assignment (type " << program->type << ") : Rule [D1r]\n";
		}


		//Simulate the inversion of a destructive assignment
		DA *temp = static_cast<DA*>(program);

		//ensure the identifiers match the current value of previous - meaning this is the statement to reverse
		if (temp->idendifiers.top() == *previous) {
			//pop the identifier from the statements stack of identifiers
			temp->idendifiers.pop();

			//add the rule name to the sequence of rules for tracing if recordrules is enabled
			if (recordRulesInv) {
				rulesInv->push_back("[D1r]");
			}

			//get the old value from the stack on the auxiliary store
			pair<int,int> topElement = (*st.aux)[temp->varName].top();
			if (topElement.first == *previous) { //ensure the old value of the variable at the top of the stack for this variable has the appropriate identifier

				//get the location of the variable
				std::string bnForEval = getB(temp->varName,temp->path,se);
				int location = getLoc(temp->varName,bnForEval,sg);


				//update this location to this old value - second element of the pair
				((*sg.sigma)[location]) = topElement.second;

				//finally remove this element from the auxiliary store
				(*st.aux)[temp->varName].pop();
			}
			else {
				cout << "ERROR: In InvSimStep - destructive assignment\n";
			}

			//finally update previous and next
			*previous -= 1;
			*next -= 1;
		}
		else {
			cout << "ERROR inverting destructive (" << temp->full << ") \n";
		}

		//finally, set this statement to its skip equivalent meaning the rule [S2r] can be used to continue the execution
		temp->type = 11;
//		cout << "here2\n";
	}

	else if (program->type == 1) {
		//Simulate the inverse of a constructive assignment
		CA *temp = static_cast<CA*>(program);

		//ensure the identifiers match the current value of previous - meaning this is the statement to reverse
	//	cout << "comparing " << temp->idendifiers.top() << " with " << *previous << "\n";
		if (temp->idendifiers.top() == *previous) {

			//now evaluate the variable in question as this is either an increment or a decrement
			std::string bnForEval = getB(temp->varName,temp->path,se);
			int location = getLoc(temp->varName,bnForEval,sg);

			//if the location returned in -1, this is a critical error and execution must abort
			if (location == -1) {
				cout << "ABORT: ERROR OCCURRED IN GETLOC\n";
				current = NULL;
				return NULL;
			}

			//evaluate this variable using the location returned above
			int currentVal = 0, update = 0;
			std::tr1::unordered_map<int,int >::const_iterator got2 = sg.sigma->find(location);
			if (got2 == sg.sigma->end()) {
				std::cout << "not found-32 - location " << location << "\n";
			}
			else { //location has been queried
				currentVal = got2->second; //this is the current value of the original variable
			}

			if (temp->newCondTree->element.evaluated == false) {
				if (output) {
					cout << "Simulating Step of Inverse Constructive Assignment Expression Evaluation\n";
				}

				evalStepOfTree(temp->newCondTree,temp->path,se,sg);
			}
			else {
				if (output) {
					cout << "Simulating Inverse Constructive Assignment (type " << program->type << ") : Rule [C1r]\n";
				}

				temp->idendifiers.pop(); //pop the identifier

				//add the rule name to the sequence of rules for tracing if recordrules is enabled
				if (recordRulesInv) {
					rulesInv->push_back("[C1r]");
				}

				resetTree(temp->newCondTree);

				int newValue = temp->newCondTree->element.resultInt;

				//evaluate the expression (that must be an arithmetic expression)
				//int newValue = evaluateArithTree(temp->newCondTree,temp->path,se,sg);

				//finally, increment or decrement the variable
				if (temp->inc) {
					update = currentVal + newValue;
				} else {
					update = currentVal - newValue;
				}

				//update the location to the new value - incrementing or decrementing it
				((*sg.sigma)[location]) = update;

				//update the values of next and previous to maintain the use of identifiers
				*previous -= 1;
				*next -= 1;

				//finally, set this statement to its skip equivalent, so that the rule [S2r] can be used to continue the exectution
				temp->type = 11;
//				cout << "---------------------------------cONSTRUCTIVE\n";
			}
		}
		else {
			cout << "ERROR inverting constructive \n";
		}


	}

	else if (program->type == 2) {
		//Simulate the inverse of a conditional statement
		IfS *temp = static_cast<IfS*>(program);

		if (!temp->seen) { //this is the first time through - must evaluate the condition and move the pointer to the appropriate branch
			temp->seen = true; //record we have seen the conditional

			if (output) {
				cout << "Simulating Inverse Conditional Statement Opening (type " << program->type << ") : Rule [I1r";
			}
//
			//check that the identifiers match and that this is the next statement to invert
			if (temp->idendifiers.top() == *previous) {

				//remove the identifier from the conditional statements stack
				temp->idendifiers.pop();
				//now retrieve the condition evaluation from the stack
				bool condition;
				int m = (*st.aux)["B"].top().first; //identifier on the stack
				int con = (*st.aux)["B"].top().second; //which branch should be inverted
				(*st.aux)["B"].pop(); //remove the pair from the stack

				if (con == 1) {
					condition = true;
				}
				else {
					condition = false;
				}

				//ensure this pair is the correct pair for inversion
				if (m != *previous) {
					cout << "ERROR: inverting conditional has incorrect identifiers! m = " << m << " and previous = " << *previous <<" \n";
				}
				else {
					//now decrement previous to indicate this statement is performed
					*previous -= 1;
					*next -= 1;

					//now move the pointer to the beginning of the appropriate branch
					if (condition) {
						if (output) {
							cout << "T] (due to true value retrieved from stack B)\n";
						}
						//this must be the rule [I1rT]
						if (recordRulesInv) {
							rulesInv->push_back("[I1rT]");
						}

						//update the pointer to the true branch (depending on whether its within a parallel statement)
						if (temp->parentPar == NULL) {
							//no parent, so not a member of a parallel statement
							current = temp->trueBranch->nextStatement();
						}
						else {
							//this is in a parallel statement, so update either currLeft or currRight
							if (temp->left == 1) {
								//on left side of parallel - so update currLeft
								temp->parentPar->currLeft = temp->trueBranch->nextStatement();
							}
							else {
								//on right side of parallel - so update currRight
								temp->parentPar->currRight = temp->trueBranch->nextStatement();
							}
						}
					}
					else {
						//this must be the rule [I1rF]

						if (output) {
							cout << "F] (due to false value retrieved from stack B)\n";
						}

						if (recordRulesInv) {
							rulesInv->push_back("[I1rF]");
						}

						//update the pointer to the true branch (depending on whether its within a parallel statement)
						if (temp->parentPar == NULL) {
							//no parent, so not a member of a parallel statement
							current = temp->falseBranch->nextStatement();
						}
						else {
							//this is in a parallel statement, so update either currLeft or currRight
							if (temp->left == 1) {
								//on left side of parallel - so update currLeft
								temp->parentPar->currLeft = temp->falseBranch->nextStatement();
							}
							else {
								//on right side of parallel - so update currRight
								temp->parentPar->currRight = temp->falseBranch->nextStatement();
							}
						}

					}
				}
			}
			else {
				cout << "Incorrect identifiers for executing step of inverse conditional-1\n";
			}
		}
		else { //this is the second time through - branch has been completed - so move on
			temp->seen = false; //reset the seen flag - in case this is repeated

			if (output) {
				cout << "Simulating Inverse Conditional Statement Closing (type " << program->type << ") : Rule ";
			}

			//determine whether this is the next available step
			if (temp->idendifiers.top() == *previous) {

				temp->idendifiers.pop();
				*previous -= 1;
				*next -= 1;

				//now record the rule that we are executing
				if (temp->choice == 1) { //this must be the rule [I4r]
					if (output) {
						cout << "[I4a] (end of true branch)\n";
					}

					if (recordRulesInv) {
						rulesInv->push_back("[I4r]");
					}
				}
				else { //this must be the rule [I5r]
					if (output) {
						cout << "[I5a] (end of false branch)\n";
					}

					if (recordRulesInv) {
						rulesInv->push_back("[I5r]");
					}
				}

				//finally, set this statement to its skip equivalent - rule [S2r] can be used to continue the execution
				temp->type = 11;

			}
			else {
				cout << "213ERROR: inverting conditional has incorrect identifiers! m = " << temp->idendifiers.top() << " and previous = " << *previous <<" \n";
			}

		}
	}

	else if (program->type == 3) {
		//Simulate the inversion of a while loop
		WlS *temp = static_cast<WlS*>(program);

		//check if this is the first iteration - depending on whether the mapping for this loop exists
		std::tr1::unordered_map<std::string,statement* > ::iterator it = (*we.we).begin();

		if (((*we.we).count(temp->WID)) == 0) { //mapping does not exist - meaning this is the first iteration
			//determine whether this is a loop of zero iterations, or n iterations - can be done using identifiers
			//if the previous identifier is in the original loop stack = zero iterations

			if (((!temp->idendifiers.empty())) && (temp->idendifiers.top() == *previous)) {
				//this is a zero iteration loop - so don't create a mapping
				//remove the necessary information and move the pointer onto skip

				if (!invSimCondition("W",st,previous)) { //condition is false - meaning this really is a zero iteration
					if (output) {
						cout << "Simulating Inverse While Loop (type " << program->type << ") : Rule [W1r] due to false first condition\n";
					}
					*previous -= 1;
					*next -= 1;
					temp->idendifiers.pop();

					//finally, set this statement to its skip equivalent
					temp->type = 11;
				}
				else { //condition stored is true - so there has been an error
					cout << "ABORT: ERROR in invSimStep - condition is expected to be false for a zero iteration while loop\n";
				}
			}
			else {
				//previous is not in the source code - meaning this loop must have n iterations, where n >= 1
				//so we must make a copy of this loop
				bool condition = invSimCondition("W",st,previous);
				if (!condition) {
					cout << "ERROR: False received in loop with at least 1 iteration - should have been true\n";
				}
				else { //this is the statement to invert - now create a unique version
					if (output) {
						cout << "Simulating Inverse While Loop Opening (type " << program->type << ") : Rule [W3r] due to true first condition\n";
					}

					linkedList lc, lb;
					linkedList *localCopy = &lc;
					createVersionStatement(temp,localCopy,temp->next,temp->parentPar,temp->left); //this creates the version

					//now modify the copy with necessary parallel information (parent and left)
					WlS *aq = static_cast<WlS*>(localCopy->nextStatement()); //cast to a while loop
					localCopy->modifyWithPrem(NULL, temp->parentPar, temp->left);
					aq->next = temp->next; //link to the next statement so that execution can continue

					//now rename the inverse version of the loop body
					invRenameLoopBody(localCopy->nextStatement(),NULL,nextNames);

	//				//retrieve the annotation information from the stack
	//				int idenOfAI = (*st.auxAI)["WI"].top().first;
	//				string annInfo = (*st.auxAI)["WI"].top().second;
	//				string *pointAnnInfo = &annInfo;
	//				(*st.auxAI)["WI"].pop();

	//				//ensure this annotation information is actually for this statement
	//				if (idenOfAI != *previous) {
	//					cout << "ERROR: annotation information has the wrong identifier\n";
	//				}
	//				else { //this is the correct statement - execution can continue
	//					//firstly set the loop identifiers for the loop itself (the first part of the string retrieved from the stack)
	//					string singleID = getNextString(pointAnnInfo);
	//					stack<int> t = makeStackReverse(singleID);
	//					aq->idendifiers = t;

						//now set the identifiers of all statements within the loop body
	//					updateSetAi(aq->loopBody->nextStatement(),aq,pointAnnInfo);

					updatedRestoreAI(aq->loopBody->nextStatement(),aq,aq->WID,AI,true,aq);

						WlS *loop = static_cast<WlS*>(localCopy->nextStatement()); //cast the new version to a while statement

						//now insert the mapping into the while environment - making it accessible later
						((*we.we)[temp->WID]) = localCopy->nextStatement();

						//now remove the identifier and decrement both next and previous
						loop->idendifiers.pop();
						*previous -= 1;
						*next -= 1;

						//cout << "here\n";

						//now move the pointer to the first step of the loop body
						if (temp->parentPar == NULL) {
							//no parent, so move pointer on
							current = aq->loopBody->nextStatement();
						}
						else {
							//is within a parallel statement, so move currentLeft or currentRight
							if (temp->left == 1) {
								//on the left side of the parallel
								temp->parentPar->currLeft = aq->loopBody->nextStatement();
							}
							else {
								//on the right side of the parallel
								temp->parentPar->currRight = aq->loopBody->nextStatement();
							}
						}
						//cout << "current set to " << current->type << "\n";
					//} //comment this on out
				}
			}
		}
		else { //mapping exists - this is NOT the first iteration
			//find this mapping and cast it into a while statement
			WlS *currentMapping;
			std::tr1::unordered_map<std::string,statement* > ::iterator itFind = (*we.we).find(temp->WID);
			if (itFind == (*we.we).end()) {
				std::cout << "CRITICAL ERROR: loop mapping should exist but doesn't\n";
			}
			else { //mapping has been found
				currentMapping = static_cast<WlS*>(itFind->second);
				invRenameLoopBody(currentMapping,NULL,nextNames); //rename this mapping to make this iteration unique
			}

			//mapping now exists - meaning we can begin the execution
			if (currentMapping->idendifiers.top() != *previous) {
				cout << "ERROR: incorrect identifier in inverting a while loop\n";
			}
			else { //this is the statement to invert - so carry on
				//now simulate the condition - NO EVALUATION - retrieve it from the stack W on DELTA
				bool condition = invSimCondition("W",st,previous);
				currentMapping->idendifiers.pop(); //remove the identifier from the stack

				if (condition) { //must perform another iteration - move pointer to the beginning of unqiue loop body version
					if (output) {
						cout << "Simulating Inverse While Loop (type " << program->type << ") : Rule [W4r] due to true condition\n";
					}

					if (temp->parentPar == NULL) {
						//no parent, so move pointer on
						current = currentMapping->loopBody->nextStatement();
					}
					else {
						//is within a parallel statement, so move currentLeft or currentRight
						if (temp->left == 1) {
							//on the left side of the parallel
							temp->parentPar->currLeft = currentMapping->loopBody->nextStatement();
						}
						else {
							//on the right side of the parallel
							temp->parentPar->currRight = currentMapping->loopBody->nextStatement();
						}
					}
				}
				else { //condition is false, meaning the loop execution will finish here
					if (output) {
						cout << "Simulating Inverse While Loop Closing (type " << program->type << ") : Rule [W2r] due to false condition\n";
					}

					//remove the mapping from the while environment
					(*we.we).erase(currentMapping->WID);

					//now set this statement to its skip equivalent
					currentMapping->type = 11;
				}

				//now decrement the next and previous identifier values
				*previous -= 1;
				*next -= 1;
			}
		}
	}

	else if (program->type == 4) {
		//Simulate the inverse of a parallel statement
		Par *temp = static_cast<Par*>(program);

/*		//if both sides are finished, but both have not yet been marked as free
		if ((temp->currLeft == program) && (temp->currRight == program)) {
			//pick the left side (could be random but doesn't add anything)
			if (output) {
				cout << "AUTO ------ Simulating Parallel Statement - left side has finished (type " << program->type << ") : Rule [P4a]\n";
			}
			//record the execution of the rule [P4a]
			if (recordRulesInv) {
				rulesInv->push_back("[P4a]");
			}

			//set the left finished flag to true and the corresponding pointer to NULL
			temp->seenLeft = true;
			temp->currLeft = NULL;
		}
		else if (temp->currLeft == program) { //if the left side of the parallel has completed, the program should become sequential - rule [P4a]
			if (output) {
				cout << "Simulating Parallel Statement - left side has finished (type " << program->type << ") : Rule [P4a]\n";
			}
			//record the execution of the rule [P4a]
			if (recordRulesInv) {
				rulesInv->push_back("[P4a]");
			}

			//set the left finished flag to true and the corresponding pointer to NULL
			temp->seenLeft = true;
			temp->currLeft = NULL;
		}
		else if (temp->currRight == program) { //if the right side of the parallel has completed, the program should become sequential - rule [P3a]
			if (output) {
				cout << "Simulating Parallel Statement - right side has finished (type " << program->type << ") : Rule [P3a]\n";
			}

			//record the execution of the rule [P4a]
			if (recordRulesInv) {
				rulesInv->push_back("[P3a]");
			}

			//set the right finished flag to true and the corresponding pointer to NULL
			temp->seenRight = true;
			temp->currRight = NULL;
		}

		//now handle when a parallel statement finally completes - when both sides have completed
		if (temp->seenLeft && temp->seenRight) { //move onto the next statement
			//reset the various flags and pointers in case the parallel statement is repeated
			temp->seenLeft = false;
			temp->seenRight = false;
			temp->currLeft = temp->leftSide->nextStatement();
			temp->currRight = temp->rightSide->nextStatement();

			//no further iterations are required, so move on (depending on whether its within a parallel statement)
			//set the statement to its skip equivalent
			temp->type = 11;
		}*/

		//now handle when a parallel statement finally completes - when both sides have completed
		if (temp->seenLeft && temp->seenRight) { //move onto the next statement

			cout << "Simulating Inverse Parallel Statement Closure : Rule [P3r]\n";

			if (recordRulesInv) {
				rulesInv->push_back("[P3a]");
			}

			//reset the various flags and pointers in case the parallel statement is repeated
			temp->seenLeft = false;
			temp->seenRight = false;
			temp->currLeft = temp->leftSide->nextStatement();
			temp->currRight = temp->rightSide->nextStatement();

			//no further iterations are required, so move on (depending on whether its within a parallel statement)
			//set the statement to its skip equivalent
			temp->type = 11;
		}



	}

	else if (program->type == 5) {
		//Simulate the inverse of a block statement
		BlS *temp = static_cast<BlS*>(program);

		if (!temp->seen) { //we have not yet started this block, so we start it now
			temp->seen = true; //record that we now start the block

			if (output) {
				cout << "Simulating Inverse Block Statement Opening (type " << program->type << ") : Rule [B1r]\n";
			}

			//this rule must be [G1r], so record it
			if (recordRulesInv) {
				rulesInv->push_back("[B1r]");
			}

			//now update the program pointer to the next statement (depending on whether this is in a parallel statement)
			if (temp->parentPar == NULL) {
				//no parent, so not a member of a parallel statement
				current = temp->blockBody->nextStatement();
			}
			else {
				//this is in a parallel statement, so update either currLeft or currRight
				if (temp->left == 1) {
					//on left side of parallel - so update currLeft
					temp->parentPar->currLeft = temp->blockBody->nextStatement();
				}
				else {
					//on right side of parallel - so update currRight
					temp->parentPar->currRight = temp->blockBody->nextStatement();
				}
			}
		}
		else { //block has already been started, meaning we must be finishing it now
			temp->seen = false; //reset the seen flag in case we repeat this block


			if (output) {
				cout << "Simulating Inverse Block Statement Closing (type " << program->type << ") : Rule [G3r]\n";
			}

			//this rule must be [G3r], so record it
			if (recordRulesInv) {
				rulesInv->push_back("[B2r]");
			}

			//now set this statement to its skip equivalent
			temp->type = 11;
		}
	}

	else if (program->type == 6) {
		//Simulate the inversion of a local variable removal (which will be a declaration in the inverse execution)
		VdS *temp = static_cast<VdS*>(program);

//		cout << "-------------------simulating local var removal - using " << *next << ".\n";

		if (output) {
			cout << "Simulating Inverse Local Variable Declaration (type " << program->type << ") : Rule [L1r]\n";
		}

		//this rule must be [L1r], so record it
		if (recordRulesInv) {
			rulesInv->push_back("[L1r]");
		}

		//get the block to which the local variable should be local to
		string blockNameForEvaluation = getMostDirectBlockName(temp->path);

		//ensure this is the statement to invert next
		int top = temp->idendifiers.top();

		if (top != *previous) {
			cout << "ERROR: Incorrect identifiers for inverting a declaration statement\n";
		}
		else { //this is the correct statement to invert
			temp->idendifiers.pop(); //remove the identifier from the stack

			//get the final value of this variable before it was removed, from the stack
			pair<int,int> topElement = (*st.aux)[temp->varName].top();
			if (topElement.first != *previous) {
				cout << "ERROR: 2Incorrect identifiers for inverting a declaration statement\n";
			}
			else { //correct value found
				//pop the element from the store and decrement previous
				(*st.aux)[temp->varName].pop();
				*previous -= 1;
				*next -= 1;

				//create the entry on both sigma and gamma using the next memory location
				((*sg.gamma)[make_pair(temp->varName,blockNameForEvaluation)]) = *nextLoc;
				((*sg.sigma)[*nextLoc]) = topElement.second;
				*nextLoc += 1;

				//update the scope information with this new version of a variable
				string blockNameReduced = removeVersionOfBlockName(temp->blockName);
				((*se.se)[blockNameReduced]).insert(temp->varName);

				//finally, set this to its skip equivalent
				temp->type = 11;
			}
		}
	}

	else if (program->type == 7) {
		//Simulate the inverse of a procedure removal (a declaration in the inverse execution)
		PdS *temp = static_cast<PdS*>(program);

		if (output) {
			cout << "Simulating Inverse Procedure Declaration (type " << program->type << ") : Rule [L2r]\n";
		}

	//	cout << "--------------------------------------------\n";
	//	temp->procBodyList->displayWithPosition(NULL, 0, NULL);
	//	cout << "--------------------------------------------\n";

		//this rule must be [L2r], so record it
		if (recordRulesInv) {
			rulesInv->push_back("[L2r]");
		}

		//ensure this is the correct statement to invert
		if (temp->idendifiers.top() != *previous) {
			cout << "ERROR: incorrect identifiers trying to invert procedure declaration\n";
		}
		else { //this is the correct statement
			//create the basis mapping in the procedure environment
			pair<std::string,statement*> entry;
			entry.second = temp->procBodyList->nextStatement();
			entry.first = temp->procName;
			((*mu.pe)[temp->procIden]) = entry;

			//now update the scope environment to allow this procedure to be evaluated
			std::string blo = reduceToMostDirectBlock(temp->path);
			std::string baseBlockName = reduceARenamedBlockName(temp->path);
			(*pi.pi)[make_pair(temp->procName,blo)] = temp->procIden;

			//INSERT INTO SCOPE ENVIRONMENT NECESSARY FOR EVALUATION
			set<std::string> cur = (*se.se)[baseBlockName];
			cur.insert(temp->procName);
			(*se.se)[baseBlockName] = cur;

			//update the next and previous functions
			*previous -= 1;
			*next -= 1;
			temp->idendifiers.pop();

			//finally, update this statement to its skip equivalent
			temp->type = 11;
		}
	}

	else if (program->type == 8) {
		//Simulate the inversion of a procedure call
		PcS *temp = static_cast<PcS*>(program);

		if (!temp->seen) { //first time - so we must begin the procedure call now
			if (output) {
				cout << "Simulating Inverse Procedure Call Opening (type " << program->type << ") : Rule [G1r]\n";
			}

			//this rule must be [G1r], so record it
			if (recordRulesInv) {
				rulesInv->push_back("[G1r]");
			}

			//ensure this is the statement to invert
//			cout << "1\n";
			if (temp->idendifiers.top() == *previous) {
				temp->seen = true; //reset the seen flag for when we return


//				cout << "2\n";

				//no mapping for the call name will exist at this point, so we must create it
				//get the annotated information
//				int idenOfAI = (*st.auxAI)["Pr"].top().first;
//				string annInfo = (*st.auxAI)["Pr"].top().second;
//				string *pointAnnInfo = &annInfo;
//				(*st.auxAI)["Pr"].pop(); //remove the annotation information from the auxiliary store

				//ensure this annotation information is the correct one for this statement
//				if (idenOfAI != *previous) {
//					cout << "ERROR: inversing a procedure call where AI info has incorrect idenitifer\n";
//				}
//				else { //this is the correct statement
					//query the procedure environment and find the basis mapping for this procedure
					pair<std::string,statement*> pairAnswer = queryPE(temp->procName,mu,temp->path,se,pi);;
					string formalParameterName = pairAnswer.first; //procedure name used in code
					statement *answer = pairAnswer.second; //the procedure body

//					cout << "3\n";
					//now create the local copy of this procedure body
					linkedList lc;
					linkedList *localCopy = &lc;
					createVersionStatement(answer,localCopy,NULL,temp->parentPar,temp->left); //this creates the new version
//					cout << "4\n";
					//now rename the procedure body to make it unique
					invRenameProcBody(localCopy->nextStatement(),NULL,temp->callID,nextNames);


//					cout << "5\n";
					//link the end of the procedure body back to the original call
					linkProcBodyToCall(localCopy,temp);
//					cout << "6\n";
					//now insert the annotated information in the necessary places
				//	setAnnotationInformationWhile(localCopy->nextStatement(),temp,pointAnnInfo);
					//updateSetAi(localCopy->nextStatement(),temp,pointAnnInfo);

					//new
//					cout << "starting restore call\n";
					updatedRestoreAI(localCopy->nextStatement(),temp,temp->callID,AI,false,NULL);
//					cout << "returned from restore call\n";
//					cout << "7\n";
		//			cout << "----------Call body that I am saving------------------\n";
		//			localCopy->displayWithPosition(temp, 0, current);
		//			cout << "----------Call body that I am saving------------------\n";

					//now insert the mapping into the procedure environment, indexed with the unique call name
					pair<std::string,statement*> entry;
					entry.second = localCopy->nextStatement();
					entry.first = temp->procName;
					((*mu.pe)[temp->callID]) = entry;

					//now update the values of next and previous, and remove the identifier
					*previous -= 1;
					*next -= 1;
					temp->idendifiers.pop();

					//now move the pointer to the beginning of the new version of the procedure body
					if (temp->parentPar == NULL) {
						//no parent, so not a member of a parallel statement
						current = localCopy->nextStatement();
					}
					else {
						//this is in a parallel statement, so update either currLeft or currRight
						if (temp->left == 1) {
							//on left side of parallel - so update currLeft
							temp->parentPar->currLeft = localCopy->nextStatement();
						}
						else {
							//on right side of parallel - so update currRight
							temp->parentPar->currRight = localCopy->nextStatement();
						}
					}
				//} //comment out this line

			}
			else {
				cout << "ERROR: incorrect identifiers in inverting a procedure call\n";
			}
		}
		else { //second time through - so must finish the call statement here
			if (output) {
				cout << "Simulating Inverse Procedure Call Closing (type " << program->type << ") : Rule [G3r]\n";
			}

			temp->seen = false; //reset the seen flag in case this call is repeated

			if (temp->idendifiers.top() == *previous) {

				//now update the values of next and previous, and remove the identifier
				*previous -= 1;
				*next -= 1;
				temp->idendifiers.pop();

				//this rule must be [G3r], so record it
				if (recordRulesInv) {
					rulesInv->push_back("[G3r]");
				}

				//find the mapping for this call name in the procedure environment
				std::tr1::unordered_map<std::string,pair<std::string,statement*> >::iterator it = ((*mu.pe)).find(temp->callID);
				if (it != (*mu.pe).end()) {
					((*mu.pe).erase(it)); //remove the mapping
				}
				else {
					cout << "ERROR: critical mistake as call mapping not found\n";
				}

				//finally, set this statement to its skip equivalent
				temp->type = 11;



			}
			else {
				cout << "incorrect identifiers for closing inverse procedure call statement\n";
			}


		}
	}

	else if (program->type == 9) {
		//Simulate the inverse of a procedure declaration (removal in the inverse execution)
		PrS *temp = static_cast<PrS*>(program);

		if (output) {
			cout << "Simulating Inverse Procedure Removal (type " << program->type << ") : Rule [H2r]\n";
		}

		//this rule must be [H2r], so record it
		if (recordRulesInv) {
			rulesInv->push_back("[H2r]");
		}

		//ensure this is the statement to execute next
		if (temp->idendifiers.top() != *previous) {
			cout << "ERROR: incorrect identifiers trying to invert a procedure removal\n";
		}
		else { //this is the correct statement
			//find the basis mapping for this procedure
			std::tr1::unordered_map<std::string,pair<std::string,statement*> >::iterator it = ((*mu.pe)).find(temp->procIden);
			if (it != (*mu.pe).end()) {
				((*mu.pe).erase(it)); //remove it if it exists
			}
			else { //mapping doesn't exist - something critical has gone wrong
				cout << "not found\n";
			}

			//decrement next and previous, and remove the identifier
			*previous -= 1;
			*next -= 1;
			temp->idendifiers.pop();

			//finally, set this statement to its skip equivalent
			temp->type = 11;
		}
	}

	else if (program->type == 10) {
		//Simulate the inversion of local variable declaration (a removal in the inverse execution)
		VrS *temp = static_cast<VrS*>(program);

		if (output) {
			cout << "Simulating Inverse Local Variable Removal (type " << program->type << ") : Rule [H1r]\n";
		}

		//this rule must be [H1r], so record it
		if (recordRulesInv) {
			rulesInv->push_back("[H1r]");
		}

		//get the block to which this variable in question is local to
		string blockNameForEvaluation = getMostDirectBlockName(temp->path);

		//ensure this is the statement to invert
		if (temp->idendifiers.top() != *previous) {
			cout << "ERROR - comparing " << temp->idendifiers.top() << " with " << *previous <<": incorrect identifiers while inverting a variable removal!\n";
		}
		else { //this is the statement to invert
			//evaluate the variable in question
			string blockNameReduced = removeVersionOfBlockName(temp->blockName);
			int location = getLoc(temp->varName,blockNameForEvaluation,sg);

			//now remove the information from sigma and gamma
			(*sg.sigma).erase(location);
			(*sg.gamma).erase(make_pair(temp->varName,blockNameForEvaluation));

			//remove the variable name from the scope information
	//		((*se.se)[blockNameReduced]).erase(temp->varName);

			//now remove the identifiers and decrement the value of previous
			temp->idendifiers.pop();
			*previous -= 1;
			*next -= 1;

			//finally, set the statement to its skip equivalent
			temp->type = 11;
		}
	}

	else if (program->type == 11) {
		if (output) {
			cout << "Simulating Inverse Sequential Composition (type " << program->type << ") : Rule [S2r] - completing statement type " << program->oldType << "\n";
		}
		//Simulate a skip operation (sequential composition - [S2a])
		//record the application of an instance of this rule
		if (recordRulesInv) {
			rulesInv->push_back("[S2r]");
		}

		//now reset the statement back to its original
		if (program->oldType == 0) {
			program->type = program->oldType;
			DA *pro = static_cast<DA*>(program);

			//now update the program pointer to the next statement (depending on whether this is in a parallel statement)
			if (pro->parentPar == NULL) {
				//no parent, so not a member of a parallel statement
				current = current->next;
			}
			else {
				//this is in a parallel statement, so update either currLeft or currRight
				if (pro->left == 1) {
					//on left side of parallel - so update currLeft
					pro->parentPar->currLeft = program->next;
				}
				else {
					//on right side of parallel - so update currRight
					pro->parentPar->currRight = program->next;
				}
			}
		}
		else if (program->oldType == 1) {
			program->type = program->oldType;
			CA *pro = static_cast<CA*>(program);

			//now update the program pointer to the next statement (depending on whether this is in a parallel statement)
			if (pro->parentPar == NULL) {
				//no parent, so not a member of a parallel statement
				current = current->next;
			}
			else {
				//this is in a parallel statement, so update either currLeft or currRight
				if (pro->left == 1) {
					//on left side of parallel - so update currLeft
					pro->parentPar->currLeft = program->next;
				}
				else {
					//on right side of parallel - so update currRight
					pro->parentPar->currRight = program->next;
				}
			}
		}
		else if (program->oldType == 2) {
			program->type = program->oldType;
			IfS *pro = static_cast<IfS*>(program);

			//now update the program pointer to the next statement (depending on whether this is in a parallel statement)
			if (pro->parentPar == NULL) {
				//no parent, so not a member of a parallel statement
				current = current->next;
			}
			else {
				//this is in a parallel statement, so update either currLeft or currRight
				if (pro->left == 1) {
					//on left side of parallel - so update currLeft
					pro->parentPar->currLeft = program->next;
				}
				else {
					//on right side of parallel - so update currRight
					pro->parentPar->currRight = program->next;
				}
			}
		}
		else if (program->oldType == 3) {
			program->type = program->oldType;
			WlS *pro = static_cast<WlS*>(program);

			//now update the program pointer to the next statement (depending on whether this is in a parallel statement)
			if (pro->parentPar == NULL) {
				//no parent, so not a member of a parallel statement
				current = current->next;
			}
			else {
				//this is in a parallel statement, so update either currLeft or currRight
				if (pro->left == 1) {
					//on left side of parallel - so update currLeft
					pro->parentPar->currLeft = program->next;
				}
				else {
					//on right side of parallel - so update currRight
					pro->parentPar->currRight = program->next;
				}
			}
		}
		else if (program->oldType == 4) {
			program->type = program->oldType;
			Par *pro = static_cast<Par*>(program);

			if (pro->parentPar == NULL) {
				//no parent, so move pointer on
				current = current->next;
			}
			else {
				//is within a parallel statement, so move currentLeft or currentRight
				if (pro->left == 1) {
					//on the left side of the parallel
					pro->parentPar->currLeft = program->next;
				}
				else {
					//on the right side of the parallel
					pro->parentPar->currRight = program->next;
				}
			}
		}
		else if (program->oldType == 5) {
			program->type = program->oldType;
			BlS *pro = static_cast<BlS*>(program);

			//now update the program pointer to the next statement (depending on whether this is in a parallel statement)
			if (pro->parentPar == NULL) {
				//no parent, so not a member of a parallel statement
				current = current->next;
			}
			else {
				//this is in a parallel statement, so update either currLeft or currRight
				if (pro->left == 1) {
					//on left side of parallel - so update currLeft
					pro->parentPar->currLeft = program->next;
				}
				else {
					//on right side of parallel - so update currRight
					pro->parentPar->currRight = program->next;
				}
			}
		}
		else if (program->oldType == 6) {
			program->type = program->oldType;
			VdS *pro = static_cast<VdS*>(program);

			//now update the program pointer to the next statement (depending on whether this is in a parallel statement)
			if (pro->parentPar == NULL) {
				//no parent, so not a member of a parallel statement
				current = current->next;
			}
			else {
				//this is in a parallel statement, so update either currLeft or currRight
				if (pro->left == 1) {
					//on left side of parallel - so update currLeft
					pro->parentPar->currLeft = program->next;
				}
				else {
					//on right side of parallel - so update currRight
					pro->parentPar->currRight = program->next;
				}
			}
		}
		else if (program->oldType == 7) {
			program->type = program->oldType;
			PdS *pro = static_cast<PdS*>(program);

			//now update the program pointer to the next statement (depending on whether this is in a parallel statement)
			if (pro->parentPar == NULL) {
				//no parent, so not a member of a parallel statement
				current = current->next;
			}
			else {
				//this is in a parallel statement, so update either currLeft or currRight
				if (pro->left == 1) {
					//on left side of parallel - so update currLeft
					pro->parentPar->currLeft = program->next;
				}
				else {
					//on right side of parallel - so update currRight
					pro->parentPar->currRight = program->next;
				}
			}
		}
		else if (program->oldType == 8) {
			program->type = program->oldType;
			PcS *pro = static_cast<PcS*>(program);

			//now update the program pointer to the next statement (depending on whether this is in a parallel statement)
			if (pro->parentPar == NULL) {
				//no parent, so not a member of a parallel statement
				current = current->next;
			}
			else {
				//this is in a parallel statement, so update either currLeft or currRight
				if (pro->left == 1) {
					//on left side of parallel - so update currLeft
					pro->parentPar->currLeft = program->next;
				}
				else {
					//on right side of parallel - so update currRight
					pro->parentPar->currRight = program->next;
				}
			}
		}
		else if (program->oldType == 9) {
			program->type = program->oldType;
			PrS *pro = static_cast<PrS*>(program);

			//now update the program pointer to the next statement (depending on whether this is in a parallel statement)
			if (pro->parentPar == NULL) {
				//no parent, so not a member of a parallel statement
				current = current->next;
			}
			else {
				//this is in a parallel statement, so update either currLeft or currRight
				if (pro->left == 1) {
					//on left side of parallel - so update currLeft
					pro->parentPar->currLeft = program->next;
				}
				else {
					//on right side of parallel - so update currRight
					pro->parentPar->currRight = program->next;
				}
			}
		}
		else if (program->oldType == 10) {
			program->type = program->oldType;
			VrS *pro = static_cast<VrS*>(program);

			//now update the program pointer to the next statement (depending on whether this is in a parallel statement)
			if (pro->parentPar == NULL) {
				//no parent, so not a member of a parallel statement
				current = current->next;
			}
			else {
				//this is in a parallel statement, so update either currLeft or currRight
				if (pro->left == 1) {
					//on left side of parallel - so update currLeft
					pro->parentPar->currLeft = program->next;
				}
				else {
					//on right side of parallel - so update currRight
					pro->parentPar->currRight = program->next;
				}
			}
		}
		else {
			cout << "ERROR: incorrect statement type of " << program->oldType << "in simstep 11\n";
		}
	}
	else if (program->type == 12) {
//		cout << "Simulating Inverse Placeholder for an empty branch\n";
		PH *pro = static_cast<PH*>(program);
		//now update the program pointer to the next statement (depending on whether this is in a parallel statement)
		if (pro->parentPar == NULL) {
			//no parent, so not a member of a parallel statement
			current = current->next;
		}
		else {
			//this is in a parallel statement, so update either currLeft or currRight
			if (pro->left == 1) {
				//on left side of parallel - so update currLeft
				pro->parentPar->currLeft = program->next;
			}
			else {
				//on right side of parallel - so update currRight
				pro->parentPar->currRight = program->next;
			}
		}

	}

	else if (program->type == 99) {
//		cout << "Trying to invert an abort statement\n - ERROR ERROR ERROR ERROR ERROR ERROR ERROR ERROR ERROR\n";
	}

	return current;
}

/*
 * Takes a statement and the current value of previous, and determines whether this statement is available for execution at this point.
 * StartLoop is required to ensure a while loop that has not yet started is correctly chosen - using identifiers from the stack
 */
bool isAvailable(statement *st, int *previous, bool startLoop, AnnInfo AI) {
	//returns the next identifier in the stack of a given statement ST
	//-1 - not a possible statement

	if (st->type == 0) {
		DA *temp = static_cast<DA*>(st);
		if (temp->idendifiers.empty()) {
			cout << "STACK INCORRECTLY EMPTY IN isAvailable()-1 : statement = " << temp->full << "\n\n\n\n";
			return false;
		}
		else {
			if (temp->idendifiers.top() == *previous) {
				return true;
			}
			else {
				return false;
			}
		}
		return false;
	}
	else if (st->type == 1) {
		CA *temp = static_cast<CA*>(st);
		if (temp->idendifiers.empty()) {
			cout << "STACK INCORRECTLY EMPTY IN isAvailable()-2 : statement = " << temp->full << "\n\n\n\n";
			return false;
		}
		else {
			if (temp->idendifiers.top() == *previous) {
				return true;
			}
			else {
				return false;
			}
		}
		return false;
	}
	else if (st->type == 2) {
		IfS *temp = static_cast<IfS*>(st);
		if (temp->idendifiers.empty()) {
			cout << "STACK INCORRECTLY EMPTY IN isAvailable()-3";
			return false;
		}
		else {
			if (temp->idendifiers.top() == *previous) {
				return true;
			}
			else {
				return false;
			}
		}
		return false;

/*
	//	cout << "checking conditional\n";
		IfS *temp = static_cast<IfS*>(st);
		if (temp->seen == true) {
			return true;
		}
		else {
			//not already started, so identifiers must match
			if (temp->idendifiers.empty()) {
				cout << "STACK INCORRECTLY EMPTY IN isAvailable()-3";
				return false;
			}
			else {
				if (temp->idendifiers.top() == *previous) {
					return true;
				}
				else {
					return false;
				}
			}
			return false;
		}
		return false;
*/
	}
	else if (st->type == 3) {
	//	cout << "checking while loop\n";
		WlS *temp = static_cast<WlS*>(st);

		if (temp->idendifiers.empty()) {

			std::tr1::unordered_map<std::string, std::stack<std::stack<int> > > ::iterator it = (*AI.aiMap).begin();

			if (((*AI.aiMap).count(temp->WID)) == 0) {
				return false;
			}
			else {
				//there is some annotation information that must be used

				//CHANGE HERE - match on the identifier saved alongside annotation information

				std::stack<std::stack<int> > stacks = ((*AI.aiMap)[temp->WID]);
				std::stack<int> loopStack = stacks.top();
				int topIden = loopStack.top();
				if (topIden == *previous) {
					return true;
				}
				else {
					return false;
				}


			}/**/

		}
		else {
			if (temp->idendifiers.top() == *previous) {
				//cout << "identifiers match\n";
				return true;
			}
			else {
				//cout << "identifiers don't match\n";
				return false;
			}
		}
		return true;
	/*	//if the stack is empty - it could be the loop has not yet been started
		if (temp->idendifiers.empty()) {
			cout << "stack of identifiers is empty\n";
			//if it is, then check the stack WI to have a top entry containing the current value of previous
			if (startLoop) {
				cout << "start loop is true\n";
				return true;
			}
			else {
				cout << "start loop is false\n";
				return false;
			}
		}
		else {
			cout << "stack of identifiers is not empty\n";
			if (temp->idendifiers.top() == *previous) {
				cout << "identifiers match\n";
				return true;
			}
			else {
				cout << "identifiers don't match\n";
				return false;
			}
		}
	*/
		return false;
	}
	else if (st->type == 4) {
		//parallel statement can always potentially be executed
		return true;
	}
	else if (st->type == 5) {
		//block statement does not require identifiers to match, so always returns true
		return true;
	}
	else if (st->type == 6) {
		VdS *temp = static_cast<VdS*>(st);
		if (temp->idendifiers.empty()) {
			cout << "STACK INCORRECTLY EMPTY IN isAvailable()-4";
			return false;
		}
		else {
			if (temp->idendifiers.top() == *previous) {
				return true;
			}
			else {
				return false;
			}
		}
		return false;
	}
	else if (st->type == 7) {
		PdS *temp = static_cast<PdS*>(st);
		if (temp->idendifiers.empty()) {
			cout << "STACK INCORRECTLY EMPTY IN isAvailable()-5";
			return false;
		}
		else {
			if (temp->idendifiers.top() == *previous) {
				return true;
			}
			else {
				return false;
			}
		}
		return false;
	}
	else if (st->type == 8) {
		PcS *temp = static_cast<PcS*>(st);
		if (temp->idendifiers.empty()) {
			cout << "STACK INCORRECTLY EMPTY IN isAvailable()-6";
			return false;
		}
		else {
			if (temp->idendifiers.top() == *previous) {
				return true;
			}
			else {
				return false;
			}
		}
		return false;

/*
		PcS *temp = static_cast<PcS*>(st);
		if (temp->seen == true) {
			return true;
		}
		else {
			//not already started, so identifiers must match
			if (temp->idendifiers.empty()) {
				cout << "STACK INCORRECTLY EMPTY IN isAvailable()-6";
				return false;
			}
			else {
				if (temp->idendifiers.top() == *previous) {
					return true;
				}
				else {
					return false;
				}
			}
			return false;
		}
		return false;
*/
	}
	else if (st->type == 9) {
		PrS *temp = static_cast<PrS*>(st);
		if (temp->idendifiers.empty()) {
			cout << "STACK INCORRECTLY EMPTY IN isAvailable()-7";
			return false;
		}
		else {
			if (temp->idendifiers.top() == *previous) {
				return true;
			}
			else {
				return false;
			}
		}
		return false;
	}
	else if (st->type == 10) {
		VrS *temp = static_cast<VrS*>(st);
		if (temp->idendifiers.empty()) {
			cout << "STACK INCORRECTLY EMPTY IN isAvailable()-8";
			return false;
		}
		else {
			if (temp->idendifiers.top() == *previous) {
				return true;
			}
			else {
				return false;
			}
		}
		return false;
	}
	else if (st->type == 11) {
		//skip operations are always possible actions
		return true;
	}

	//any block or par statement will return -1; as will any other statement that has zero identifiers
	return -1;
}

/*
 * Takes the set of all possible next steps of execution (all of the leaf statements returned by getOptions), and returns only those actions
 * that are possible for execution
 */
set<statement*> getOnlyAvailableActionsUpdated(set<statement*> allOptions, int *previous, bool startLoop, AnnInfo AI) {
//	cout << "number of possible options passed to be checked for availability = " << allOptions.size() << "\n";

	set<statement*> toReturn;

	for (std::set<statement*>::iterator it=allOptions.begin(); it!=allOptions.end(); ++it) {
		if (isAvailable(*it,previous,startLoop,AI)) {
			//statement is available
			toReturn.insert(*it);
		}
		else {
			//statement is not available, so don't include it
		}
	}
	return toReturn;
}

/*
 * In order to check if a while loop statement is available as the next step of execution, we must get the identifiers from the stack WI (as they
 * will not be in while loop environment yet). This function returns true if the next entry on the loop stack is for this value of previous
 */
bool simulator::checkTopOfWI(std::string constructName) {

	//NEW APPROACH
/*
	std::tr1::unordered_map<std::string,statement* > ::iterator it = (*AI.aiMap).begin();

	if (((*AI.aiMap).count(constructName)) == 0) {
		return false;
	}
	else {
		return true;
	}
*/

	//OLD APPROACH - comment/uncomment above and below to change approach
	/*
	if ((*as.auxAI)["WI"].empty()) {
		return false;
	}
	else {
		stack<pair<int,std::string> > whileAI = (*as.auxAI)["WI"];
		int topm = whileAI.top().first;
		if (topm == *previous) {
			return true;
		}
		else {
			return false;
		}
	}
	*/
}

/*
 * The function used in the interface to execute n steps of the inverse execution (or however many are possible)
 */
bool simulator::executeNStepsInv(int n, bool forceRandom, bool forceUser, bool output) {
	//simulate n steps of the inverse execution

	int original = n;

	bool simulationFinished = false;
	bool userCancelled = false;
	statement *temp = new statement;

	temp = placeInRevEx;

	int count = 0;


	if (n == -1) {
		if (output) {
			cout << "--Simulating entire execution ...\n";
		}
	}
	else {
		if (output) {
			cout << "--Simulating " << intToString11(n) << " steps (if available)\n";
		}
	}

	while ((n != 0) && (!simulationFinished)) {
		count += 1;
		set<statement*> nextOptions;
		nextOptions = getOptions(temp); //get all leaf statements (possible next steps)

		if (nextOptions.size() == 1) { //if there is only one possible step - we must follow it - will be an available action (otherwise deadlock)
//			cout << "???????????????? ONLY ONE OPTION\n";
			statement* onlyChoice = new statement;
			for (std::set<statement*>::iterator it=nextOptions.begin(); it!=nextOptions.end(); ++it) {
				onlyChoice = *it;
			}

			//add the information to the trace
			if (recordTrace) {
				string toAddToTrace = stringOfOptions(nextOptions);
				int lengthOfFirst = toAddToTrace.length();
				int spaceToAdd = 100 - lengthOfFirst;
				for (int i = 0; i < spaceToAdd; i++) {
					toAddToTrace = toAddToTrace + " ";
				}
				toAddToTrace = toAddToTrace + "---> ";
				toAddToTrace = toAddToTrace + stringOfSingleOption(onlyChoice);
				pointToInvTrace->push_back(toAddToTrace);
			}

			if (output) {
				cout << "----Step " << count << ": ";
			}
			placeInRevEx = invSimStep(pp,onlyChoice,temp,as,se,gs,pe,previous,we,next,nextName,pi,nextLoc,stateSaving,sequenceOfRulesInv,recordRulesInv,AI,output);
			//cout << "before clean - statement type: " << placeInRevEx->type << "\n";
			placeInRevEx = clean(placeInRevEx);

			//if (placeInRevEx != NULL) {
			//	cout << "after clean - statement type: " << placeInRevEx->type << "\n";
			//}

			temp = placeInRevEx;

		}
		else {
//			cout << "possible next options size = " << nextOptions.size() << "\n";

//			cout << "All possible options--\n";
			std::tr1::unordered_map<int,statement* > numberedOptions = displayNumberedOptions(nextOptions);

//			for (int i = 1; i <= nextOptions.size(); i++) {
//				cout << i << ": " << displayStatement(numberedOptions[i],false) << "\n";
//			}
//			cout << "END OF ALL POSSIBLE OPTIONS--\n";

			//more than one option, we must chose the one with the correct id
			//must take into account the causal-consistent reversibility of statements without identifiers
			//extract all options that could happen now - all those with no identifiers and the one with identifiers

			bool startLoop = this->checkTopOfWI("");

			set<statement*> availableOptions = getOnlyAvailableActionsUpdated(nextOptions,previous,startLoop,AI); //reduce the possible steps to only the available ones

			statement* onlyChoice = new statement;

		//	cout << "number of these options that are available = " << availableOptions.size() << "\n";

			//then make a random choice from these available actions
			if (availableOptions.size() == 0) { //error here if no actions are available
				cout << "NO AVAILABLE OPTIONS\n";
			}
			else { //at least one available action
				if (availableOptions.size() == 1) { //only one available action - so must do it
					for (std::set<statement*>::iterator it=availableOptions.begin(); it!=availableOptions.end(); ++it) {
						//this will only run once as the set has exactly one element
						onlyChoice = *it;
						break;
					}
				}
				else { //at least 2 options - so either randomly decide or let the user chose
					if (forceUser) {
						string userChoiceString = "";
						int userChoice;
						cout << "The next step is the non-deterministic choice between the following multiple statements. \n";

						std::tr1::unordered_map<int,statement* > numberedOptions = displayNumberedOptions(nextOptions);

						for (int i = 1; i <= nextOptions.size(); i++) {
							cout << i << ": " << displayStatement(numberedOptions[i],false) << "\n";
						}

						bool validChoice = false;
						while (!validChoice) {
							cout << "Pick a statement number to execute: ";
							getline(cin, userChoiceString);
							if (userChoiceString == "cancel") {
								cout << "User wants to cancel execution\n";
								userCancelled = true;
								validChoice = true;
								simulationFinished = true;
							}
							else {
								userChoice = stringToInt11(userChoiceString);

								if ((0 < userChoice) && (userChoice <= nextOptions.size()) && (userChoice != 0)) {
									//this is valid
									validChoice = true;
								}
								else {
									cout << "Invalid input - please try again\n";
								}
							}
						}
						if (validChoice) {
							onlyChoice = numberedOptions[userChoice];
						}
					}
					else {
						//make a random choice of all available actions and execute it
						set<statement*>::const_iterator it2(availableOptions.begin());
						int randomChoice = rand() % availableOptions.size();
						advance(it2,randomChoice);
						onlyChoice = *it2;
					}
				}
				if (!userCancelled) { //as long as the choice was not to cancel, we now execute it
					if (recordTrace) {
						//this will show all leaf statements - including those that cannot be executed at this point
						//change this to stringOfOptions(availableOptions) to include only the possible statements
						string toAddToTrace = stringOfOptions(nextOptions);
						int lengthOfFirst = toAddToTrace.length();
						int spaceToAdd = 100 - lengthOfFirst;
						for (int i = 0; i < spaceToAdd; i++) {
							toAddToTrace = toAddToTrace + " ";
						}
						toAddToTrace = toAddToTrace + "---> ";
						toAddToTrace = toAddToTrace + stringOfSingleOption(onlyChoice);
						pointToInvTrace->push_back(toAddToTrace);
					}

					if (output) {
						cout << "----Step " << count << ": ";
					}
					placeInRevEx = invSimStep(pp,onlyChoice,temp,as,se,gs,pe,previous,we,next,nextName,pi,nextLoc,stateSaving,sequenceOfRulesInv,recordRulesInv,AI,output);
			//		cout << "before clean - statement type: " << placeInRevEx->type << "\n";
					placeInRevEx = clean(placeInRevEx);
			//		cout << "after clean - statement type: " << placeInRevEx->type << "\n";
					temp = placeInRevEx;
				}
			}
		}

		nextOptions = getOptions(temp);
		if (nextOptions.size() == 0) {
			if (output) {
				cout << "--done (completed in " << count << " steps)\n";
			}

			//save the forwards trace to file now as the execution is finished
			fileIO sfio = fileIO();
			if (recordTrace == true) {
				if (output) {
					cout << "--Saving the trace of reverse execution to file: execution-traces/invTrace.txt\n";
				}

				sfio.writeTrace("execution-traces/invTrace.txt", simulator::getInvTrace());
			}

			return true;
		}
		else if ((n-1) == 0) {
			//reached the end of the n desired steps
			if (output) {
				cout << "--done (completed after " << count << " steps)\n";
			}
		}
		n -= 1;
	}
	return false;
}

/*
 * Alternate function for simulating the entire inverse execution - not used in the new interface
 */
void invSimAll(parser *pp, linkedList *program, auxStore st, scopeEnvironment se, gammaSigma sg, procedureEnvironment mu, whileEnvironment we, int *next,
		int *previous, nextNames nextNames, procedureIdentifiers pi, int *nextLoc, int *stateSaving, bool recordTrace, vector<string> *invTrace,
		vector<string> *rulesInv, bool recordRulesInv, AnnInfo AI, bool output) {

	statement *temp = new statement;
	temp = program->nextStatement();

	set<statement*> nextOptions;
	nextOptions = getOptions(temp);

	while (nextOptions.size() != 0) {

		if (nextOptions.size() == 1) {
			//only one option, so must follow this
			statement* onlyChoice = new statement;
			for (std::set<statement*>::iterator it=nextOptions.begin(); it!=nextOptions.end(); ++it) {
				onlyChoice = *it;
			}

			//add the information to the trace
			if (recordTrace) {
				string toAddToTrace = stringOfOptions(nextOptions);
				toAddToTrace = toAddToTrace + "\t  ---> ";
				toAddToTrace = toAddToTrace + stringOfSingleOption(onlyChoice);
				invTrace->push_back(toAddToTrace);
			}

			invSimStep(pp,onlyChoice,temp,st,se,sg,mu,previous,we,next,nextNames,pi,nextLoc,stateSaving,rulesInv,recordRulesInv,AI,output);
		}
		else {
			//more than one option, we must chose the one with the correct id

			//must take into account the causal-consistent reversibility of statements without identifiers
			//extract all options that could happen now - all those with no identifiers and the one with identifiers
			set<statement*> availableOptions = getOnlyAvailableActionsUpdated(nextOptions,previous,true,AI);

			statement* onlyChoice = new statement;

			//then make a random choice from these available actions
			if (availableOptions.size() == 1) {
				//only one available action - so must do it
				for (std::set<statement*>::iterator it=availableOptions.begin(); it!=availableOptions.end(); ++it) {
					//this will only run once as the set has exactly one element
					onlyChoice = *it;
					break;
				}
			}
			else {
				//make a random choice of all available actions and execute it
				set<statement*>::const_iterator it2(availableOptions.begin());
				int randomChoice = rand() % availableOptions.size();
				advance(it2,randomChoice);
				//statement* choice = new statement;
				onlyChoice = *it2;

			}

			if (recordTrace) {
				string toAddToTrace = stringOfOptions(nextOptions);
				toAddToTrace = toAddToTrace + "\t  ---> ";
				toAddToTrace = toAddToTrace + stringOfSingleOption(onlyChoice);
				invTrace->push_back(toAddToTrace);
			}

			invSimStep(pp,onlyChoice,temp,st,se,sg,mu,previous,we,next,nextNames,pi,nextLoc,stateSaving,rulesInv,recordRulesInv,AI,output);

		}
		//now update the options to reflect this step of execution
		//nextOptions.clear();
		nextOptions = getOptions(temp);
		//nextOptions = new set<statement*>;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////// Interface /////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//check if a file exists - if it is possible to open a file in the default directory
bool fexists(const char *filename) {
	ifstream ifile(filename);
	return ifile.good();
}

//go through the next name structure and remove one from all entries - to correct the beginning state of the inverse execution
void decreaseNextNames(nextNames n) {

	std::tr1::unordered_map<std::string,int >::iterator it = (*n.nn).begin();

	while (it != (*n.nn).end()) {
		it->second = it->second - 1;
		it++;
	}
}

//function that performs all actions of inverting a program
void simulator::invertProgram() {
	program = invert(program,NULL);
	cout << "--Program has been inverted\n";
	pp->useLinkFunctions(program);
	cout << "--Inverse program has been linked\n";
	placeInRevEx = program->nextStatement();
	//minus one from all of nextNames
	decreaseNextNames(nextName);
}

//return a string containing all identifiers in a given stack of integers
string getIdens(stack<int> s) {
	string toReturn = "";
	while (!s.empty()) {
		if (s.size() == 1) {
			//last element
			toReturn = toReturn + intToString11(s.top());
			s.pop();
		}
		else {
			toReturn = toReturn + intToString11(s.top()) + ", ";
			s.pop();
		}
	}
	return toReturn;
}

//helper function that combines two vectors of strings
vector<string> mergeVecString(vector<string> original, vector<string> toAdd) {
	for (int i = 0; i < toAdd.size(); i++) {
		original.push_back(toAdd.at(i));
	}
	return original;
}

/*
 * Convert a program into a vector of strings. Used to write a program to a file. MUST BE COMPLETED
 */
vector<string> programToVecString(statement *temp, statement *stopper) {

	vector<string> toReturn;

	while (temp != stopper) {

		if (temp->type == 0) {
			DA *t = static_cast<DA*>(temp);
			string s = t->varName + " = " + t->newCondition;
			/*if (t->newVal != 0) {
				s = s + intToString11(t->newVal);
			}
			else {
				if (intToString11(t->newVal) != t->newValString) {
					s = s + t->newValString;
				}
				else {
					s = s + intToString11(t->newVal);
				}
			}*/

			s = s + " (" + t->path + ", [" + getIdens(t->idendifiers) + "]);";
			toReturn.push_back(s);
		}
		else if (temp->type == 1) {
			CA *t = static_cast<CA*>(temp);
			if (t->inc) {
				string s = t->varName + " += " + t->newCondition + " (" + t->path + ", [" + getIdens(t->idendifiers) + "]);";
				toReturn.push_back(s);
			}
			else {
				string s = t->varName + " -= " + t->newCondition + " (" + t->path + ", [" + getIdens(t->idendifiers) + "]);";
				toReturn.push_back(s);
			}
		}
		else if (temp->type == 2) {

		}
		else if (temp->type == 3) {

		}
		else if (temp->type == 4) {
			Par *t = static_cast<Par*>(temp);
			toReturn.push_back("par {");
			vector<string> left = programToVecString(t->leftSide->nextStatement(),t);
			toReturn = mergeVecString(toReturn,left);
			toReturn.push_back("}");
			toReturn.push_back("{");
			vector<string> right = programToVecString(t->rightSide->nextStatement(),t);
			toReturn = mergeVecString(toReturn,right);
			toReturn.push_back("}");
		}
		else if (temp->type == 5) {

		}
		else if (temp->type == 6) {

		}
		else if (temp->type == 7) {

		}
		else if (temp->type == 8) {

		}
		else if (temp->type == 9) {

		}
		else if (temp->type == 10) {

		}

		temp = temp->next;
	}

	return toReturn;

}
//dispay a stack of pairs of ints
void displayStack(stack<pair<int, int> > toPrint) {
	while (!toPrint.empty()) {
		cout << "(k:" << toPrint.top().first << ", v:" << toPrint.top().second << "), ";
		toPrint.pop();
	}
}

//display a stack of pairs of ints and strings
void displayStackString(stack<pair<int, string> > toPrint) {
	while (!toPrint.empty()) {
		cout << "(k:" << toPrint.top().first << ", v:'"  << toPrint.top().second << "'), ";
		toPrint.pop();
	}
}

void outputLineNumber1(int *ln) {
	if (*ln < 10) {
		cout << "  " << *ln << ": ";
	}
	else if (*ln < 99) {
		cout << " " << *ln << ": ";
	}
	else { //bigger than 100
		cout << *ln << ": ";
	}
	*ln += 1;
}

void updatedDisplayUsingEnviron(statement *program, statement *stopper, statement *current, int ind, whileEnvironment whileenviron, int *lineNumber) {

	bool foundCurrent = false;

	while (program != stopper) {

	//	cout << "Looking for Current type = " << current->type << "\n";

		if (program == current) {
			foundCurrent = true;
		}

		//output the line number
		outputLineNumber1(lineNumber);

		//now display the correct identation
		indent1(ind);

		if (program->type == 0) {
			DA *temp = static_cast<DA*>(program);
			cout << temp->varName << " = " << temp->newCondition;

			cout << " (" << temp->path << ") [";
			showIden1(temp->idendifiers);
			cout << "]";

			if (foundCurrent) {
				cout << "   <------- ";
				foundCurrent = false;
			}

			cout << "\n";
			program=program->next;
		}
		else if (program->type == 1) {
			CA *i = static_cast<CA*>(program);

			cout << i->varName;
			if (i->inc) { cout << " += "; }
			else { cout << " -= "; }

			cout << i->newCondition;
			/*if (i->newVal != 0) { cout << i->newVal; }
			else {
				if (intToStringInList(i->newVal) != i->newValString) { cout << i->newValString; }
				else { cout << i->newVal; }
			}*/

			cout << " (" << i->path << ") [";
			showIden1(i->idendifiers);
			cout << "]";

			if (foundCurrent) {
				cout << "   <------- ";
				foundCurrent = false;
			}
			cout << "\n";
			program=program->next;

		}
		else if (program->type == 2) {
			//If statement
			IfS *i = static_cast<IfS*>(program);

			//cout << "if " << i->condID << " (" << i->varName << " " << i->op << " " << i->newVal << ") then ";
			cout << "if " << i->condID << " (" << i->newConditionString << ") then ";

			if ((current == program) && (!i->seen)) {
				cout << "  <------- ";
			}
			cout << "\n";

			//i->trueBranch->displayWithPositionAndNum(temp,ind+1,current,lineNumber);
			updatedDisplayUsingEnviron(i->trueBranch->nextStatement(), program, current, ind+1, whileenviron, lineNumber);

			//output the line number
			outputLineNumber1(lineNumber);

			indent1(ind);
			cout  << "else\n";

			//i->falseBranch->displayWithPositionAndNum(temp,ind+1,current,lineNumber);
			updatedDisplayUsingEnviron(i->falseBranch->nextStatement(), program, current, ind+1, whileenviron, lineNumber);

			//output the line number
			outputLineNumber1(lineNumber);

			indent1(ind);
			cout  << "fi (" << i->path << ") [";
			showIden1(i->idendifiers);
			cout << "]";
			if ((current == program) && (i->seen)) {
				cout << "  <------- ";
			}
			cout << "\n";
			program=program->next;

		}
		else if (program->type == 3) {
			WlS *i = static_cast<WlS*>(program);

			//check if this while loop has a mapping in beta (meaning it is active)
			WlS *currentMapping;

			std::tr1::unordered_map<std::string,statement* > ::iterator itFind = (whileenviron.we)->find(i->WID);
			if (itFind == (whileenviron.we)->end()) {
				//no mapping found - so not active - meaning we display the normal while loop

				cout << "while " << i->WID << " (" << i->newConditionString << ") do";
				if (foundCurrent) {
					cout << "  <----- ";
					foundCurrent = false;
				}
				cout << "\n";

				updatedDisplayUsingEnviron(i->loopBody->nextStatement(), program, current, ind+1, whileenviron, lineNumber);



				//output the line number
				outputLineNumber1(lineNumber);

				indent1(ind);
				cout << "elihw (" << i->path << ") [";
				showIden1(i->idendifiers);
				cout << "]\n";

			}
			else {
				//the mapping has been found, and is cast to a while loop named currentMapping
				currentMapping = static_cast<WlS*>(itFind->second);

				cout << "FROMBETA-while " << i->WID << " (" << i->newConditionString << ") do";
				if (currentMapping == current) {
					cout << "  <----- ";
					foundCurrent = false;
				}
				cout << "\n";

	//			currentMapping->loopBody->displayWithPositionAndNum(currentMapping, ind+1, currentMapping, lineNumber);

				updatedDisplayUsingEnviron(currentMapping->loopBody->nextStatement(), itFind->second, current, ind+1, whileenviron, lineNumber);

				//output the line number
				//outputLineNumber1(lineNumber);

				indent1(ind);
				cout << "elihw (" << i->path << ") [";
				showIden1(currentMapping->idendifiers);
				cout << "]\n";


			}


			//cout << "while " << i->WID << " (" << i->varName << " " << i->op << " " << i->newVal << ")" << " do";
			program=program->next;
		}

		else if (program->type == 4) {
			//par statement
			Par *i = static_cast<Par*>(program);

			program = program->next;
		}
		else if (program->type == 5) {
			BlS *i = static_cast<BlS*>(program);

			cout << "begin " << i->bid << " ";
			if (foundCurrent && (!i->seen)) {
				cout << "  <----- ";
				foundCurrent = false;
			}
			cout << "\n";
			//i->blockBody->displayWithPositionAndNum(temp,ind+1,current,lineNumber);
			updatedDisplayUsingEnviron(i->blockBody->nextStatement(), program, current, ind+1, whileenviron, lineNumber);

			//output the line number
			outputLineNumber1(lineNumber);

			indent1(ind);
			cout << "end [";
			showIden1(i->idendifiers);
			cout << "]";
			if (foundCurrent && (i->seen)) {
				cout << "  <----- ";
				foundCurrent = false;
			}

			//if (i->last) {
			//	cout << "--- LAST";
			//}

			cout << "\n";
			program=program->next;
		}
		else if (program->type == 6) {
			VdS *i = static_cast<VdS*>(program);

			cout << "var " << i->varName << " = " << i->value << " (" << i->path << ") [";
			showIden1(i->idendifiers);
			cout << "]";
			if (foundCurrent) {
				cout << "  <----- ";
				foundCurrent = false;
			}
			cout << "\n";
			program=program->next;
		}
		else if (program->type == 7) {
			PdS *i = static_cast<PdS*>(program);

			cout << "proc " << i->procIden << " " << i->procName << " is\n";
			i->procBodyList->displayWithPositionAndNum(NULL,ind+1,current,lineNumber);

			//output the line number
			outputLineNumber1(lineNumber);

			indent1(ind);
			cout << "corp (" << i->path << ") [";
			showIden1(i->idendifiers);
			cout << "]";
			if (foundCurrent) {
				cout << "  <----- ";
				foundCurrent = false;
			}
			cout << "\n";
			program=program->next;
		}
		else if (program->type == 8) {
			PcS *i = static_cast<PcS*>(program);

			cout << "call " << i->callID << " " << i->procName << " (" << i->path << ") [";
			showIden1(i->idendifiers);
			cout << "]";
			if (foundCurrent) {
				cout << "  <----- ";
				foundCurrent = false;
			}
			cout << "\n";
			program=program->next;
		}
		else if (program->type == 9) {
			PrS *i = static_cast<PrS*>(program);

			cout << "remove proc " << i->procIden << " " << i->procName << " is\n";

			i->procBodyList->displayWithPositionAndNum(NULL,ind+1,current,lineNumber);

			//output the line number
			outputLineNumber1(lineNumber);

			indent1(ind);
			cout << "corp (" << i->path << ") [";
			showIden1(i->idendifiers);
			cout << "]";
			if (foundCurrent) {
				cout << "  <----- ";
				foundCurrent = false;
			}
			cout << "\n";
			program=program->next;
		}
		else if (program->type == 10) {
			VrS *i = static_cast<VrS*>(program);

			cout << "remove var " << i->varName << " = " << i->value << " (" << i->path << ") [";
			showIden1(i->idendifiers);
			cout << "]";
			if (foundCurrent) {
				cout << "  <----- ";
				foundCurrent = false;
			}
			cout << "\n";
			program=program->next;
		}
		else if (program->type == 11) {
			cout << "skip";
			//displayStatementIdentifiers(temp);
			if (foundCurrent) {
				cout << "  <----- ";
				foundCurrent = false;
			}
			cout << "\n";
			program=program->next;
		}


		else {
			cout << "statement type = " << program->type << "\n";
			program = program->next;
		}



	}

}


/*
 * Reset the state of the simulator - allowing execution of a new program within manually restarting. MUST BE COMPLETE
 */
void simulator::reset() {
	//reset the state of the simulator - remove the program and all environments
	program = NULL;
	placeInExecution = NULL;
	placeInRevEx = NULL;

}

/*
 * Help function that can be called from the interface
 */
void help(string indicate) {
	if (indicate == "") {
		//no specifics - so maybe a brief overview of important commands
		cout << "*********************************************************************\n";
		cout << "****Help Centre\n";
		cout << "*********************************************************************\n";
		cout << "****Important Commands\n";
		cout << " 1: 'read f' - opens the file named 'f' if available, parses the program and performs initial environment setup\n";
		cout << " 2: 'start' - starts the next execution - either forwards or reverse depending on current state of the simulator\n";
		cout << " 3: 'go n' - performs n steps of the current execution (or as many as are available) - or complete execution when n = all\n";
		cout << " 4: 'display program' - prints the current program\n";
		cout << " 5: 'display procs/mu' - prints the current procedure environment\n";
		cout << " 6: 'display loops/beta' - prints the current while environment\n";
		cout << " 7: 'display aux/delta' - prints the current auxiliary store (all stacks)\n";
		cout << " 8: 'display aux/delta S' - prints the current auxiliary store (only the stack S)\n";
		cout << " 9: 'display ftrace' - prints the current trace of the forwards execution\n";
		cout << "10: 'display rtrace' - prints the current trace of the reverse execution\n";
		cout << "11: 'display status' - prints the current status of the simulator (modes enabled etc)\n";
		cout << "12: 'statesaving on/off' - switches state-saving mode on or off\n";
		cout << "13: 'tracing on/off' - switches tracing mode on or off\n";
		cout << "14: 'autorem on/off' - switches auto insertion of removal statements on or off\n";
		cout << "15: 'autoconstructs on/off' - switches auto insertion of construct identifiers on or off\n";
		cout << "15: 'directory d' - sets the directory that all files will be loaded from to d (default working directory if d is empty)\n";
		cout << "16: 'force user' - forces all interleaving non-determinism issues to be settled by the user at runtime\n";
		cout << "17: 'force random' - forces all interleaving non-determinism issues to be settled randomly at runtime\n";
		cout << "*********************************************************************\n";
		cout << "*********************************************************************\n";
	}

	if (indicate == "statements") {
		cout << "-- Type - Description of Statement\n";
		cout << "----------------------------------\n";
		cout << "--  0   - Destructive Assignment\n";
		cout << "--  1   - Constructive Assignment\n";
		cout << "--  2   - Conditional Statement\n";
		cout << "--  3   - While Loop\n";
		cout << "--  4   - Parallel Composition\n";
		cout << "--  5   - Block Statement\n";
		cout << "--  6   - Local Variable Declaration\n";
		cout << "--  7   - Procedure Declaration\n";
		cout << "--  8   - Procedure Call\n";
		cout << "--  9   - Procedure Removal\n";
		cout << "-- 10   - Local Variable Removal\n";
		cout << "-- 11   - Skip Statement (Original Statement Type will typcially follow in brackets)\n";
	}
}

/*
 * Output the current state of the simulator - allowing for quick understanding of execution
 */
void simulator::status() {
	//current status of the execution
	cout << "State-Saving Mode: ";
	if (*stateSaving == 1) { cout << "On\n"; }
	else { cout << "Off\n"; }

	cout << "Execution Tracing Mode: ";
	if (recordTrace) { cout << "On\n"; }
	else { cout << "Off\n"; }

	cout << "Current State of Simulator: ";

	cout << "Current Examples Directory: ";


	cout << "\n";
}

/*
 * Updated interface for the simulator
 */
void simulator::interface() {
	bool quit = false; //is the interface active

	//booleans to record current state of the simulator - how far through a simulation we are
	bool setUp = true; //before starting an execution
	bool fileRead = false;
	bool inFwdEx = false;
	bool postFwdEx = false;
	bool inRevEx = false;
	bool postRevEx = false;

	//booleans to indicate how non-determinism should be broken
	bool forceUser = false;
	bool forceRandom = true;

	//boolean for auto removal
	bool autoRemoval = true;

	//boolean for auto generation of paths
	bool autoPaths = true;

	//boolean for atomic or non-atomic evaluation of expressions
	bool atomicExpressions = false;

	//boolean for auto insertion of construct identifiers
	bool autoConstructIdens = true;

	//boolean to switch output on or off
	bool output = true;

	string input = ""; //line input from the user
	char *filename = ""; //file containing the program that is being simulated

	string fileNameOpened = "";

	string sequence = "";
	string *seq = &sequence;

	string directory = "examples/";
	cout << "\n\n";
	cout << "---------******----*----******----******----*---------******---------\n";
	cout << "---------*----*----*----*----*----*----*----*---------*--------------\n";
	cout << "---------*----*----*----*----*----*----*----*---------*--------------\n";
	cout << "---------*----*----*----*----*----*----*----*---------*--------------\n";
	cout << "---------******----*----******----******----*---------******---------\n";
	cout << "---------*----*----*----*---------*---------*---------*--------------\n";
	cout << "---------*----*----*----*---------*---------*---------*--------------\n";
	cout << "---------*----*----*----*---------*---------******----******---------\n";

	cout << "\n";
	cout << "*********************************************************************\n";
	cout << "*********************************************************************\n";
	cout << "****Ripple Simulator Started: Interface Version 4\n";
	cout << "****Reversing an Imperative Parallel Programming Language\n";
	cout << "****James Hoey. Copyright 2019\n";
	cout << "****\n";
	cout << "****For help: use command `help'\n";
	cout << "****To start: read file named f via command `read f'\n";
	cout << "*********************************************************************\n";
	cout << "*********************************************************************\n";



	//cout << "To start - read a file via `read filename' or get help via `help'\n\n";
	//cout << "StateSaving = " << *stateSaving << "\n";

	while (!quit) { //continually wait for input until the user closes the simulator

		//output to the console a brief description of current location
		cout << "\nSimulator: ";
		if (setUp) { cout << "Setup> "; }
		if (inFwdEx) { cout << "Fwd-Ex> "; }
		if (postFwdEx) { cout << "Post-Fwd-Ex> "; }
		if (inRevEx) { cout << "Rev-Ex> "; }
		if (postRevEx) { cout << "Post-Rev-Ex> "; }

		//now wait for the input from the user
		getline(cin, input);

		//now decide on the next action based on what is contained within the input from the user
		if ((input == "exit") || (input == "close") || (input == "quit")) { //close the simulator at the request of the user
			cout << "\n*********************************************************************\n";
			cout << "*********************************************************************\n";
			cout << "****Ripple Simulator Stopped\n";
			cout << "****Reversing an Imperative Parallel Programming Language\n";
			cout << "****James Hoey. Copyright 2019\n";
			cout << "*********************************************************************\n";
			cout << "*********************************************************************\n\n";

			quit = true;
		}
		else if (input == "reset") { //reset the state of the simulator in order to begin inversion of a different file (or the same)
			this->reset();
		}
		else if (input.find("help") == 0) { //help commands - will take parameters indicating which info to display
			if (input == "help") { //no parameters
				help("");
			}
			else {
				input.erase(0,5); //remove the help and the following space
				help(input); //call help function with the given parameter/s
			}
		}
		else if (input.find("read") == 0) { //load a file into the simulator - must be .txt extension so can be omitted
			//cout << "statesaving at beginning of read = " << *stateSaving << "\n";
			if (input == "read") {
				cout << "Incorrect parameters for command `read'\n";
			}
			else {
				//cout << "statesaving at beginning of read 1 = " << *stateSaving << "\n";
				input.erase(0,5); //remove the read and following space - no input is the filename
				input = directory + input;
				if (!(input.find(".txt") != string::npos)) { //if the user has not supplied the file extension
					input += ".txt"; //then add it automatically
				}
				fileNameOpened = input;
				//cout << "setting fileNameOpened = " << fileNameOpened << "\n";
				filename = &input.at(0);
				//now try to open the file and perform the setup
				if (fexists(filename)) {
					fileIO fi = fileIO(); //instance of file manager
					vector<string> prog = fi.read(filename,autoPaths,autoConstructIdens);
					fileRead = true;
					cout << "--Program read from file '" << fileNameOpened << "'\n";

					if (autoPaths) {
						cout << "--Automatically Generating Paths\n";
					}

					if (autoRemoval) {
						cout << "--Automatically Adding Removal Statements\n";
					}



					//now parse the file into a linked list of our program
					parser p = parser(prog); //instance of parser
					parser *pp = &p;
					linkedList *orig = p.parseProgram(prog,autoRemoval); //parse the program read above
					p.useLinkFunctions(orig); //link the program in the required way for execution
					cout << "--Program has been parsed (view via the command `display program')\n";

					program = orig; //set the pointers to the newly parsed program
					placeInExecution = program->nextStatement();

					//now perform the initial set up of all environments - variables etc
					simulator::preExecutionSetUp();
					cout << "--Initial environments and global variables initialised\n";

					cout << "--Forwards execution can be started via the command `start'\n";

				}
				else {
					cout << "Failed to open a file with given name `" << filename << "' - please try again!\n";
				}
			}
		}
		else if (input.find("display") == 0) { //command to display information - requires a parameter
			if (input == "display") {
				cout << "Incorrect parameters for command `read'\n";
			}
			else {
				input.erase(0,8); //remove the display and the following space
				if ((input == "program")) {
					int lineNumbers = 1;
					int *ln = &lineNumbers;

					if (!fileRead) {
						cout << "--Read a file first to have a program to display\n";
					}
					else {

						cout << "--Displaying the current program";

						if ((inFwdEx) || (postFwdEx)) {
							cout << " (Forwards Execution)";
						}
						else if ((inRevEx) || (postRevEx)) {
							cout << " (Reverse Execution)";
						}

						cout << "\n";

						cout << "--Current position indicated using the arrow `<-------'\n";
						cout << "--No arrows indicate the current execution has finished\n";

						cout << "----------------------------------------------------------------------\n";

						if (!inRevEx) {
							//program->displayWithPosition(NULL,0,placeInExecution);
							program->displayWithPositionAndNum(NULL,0,placeInExecution,ln);

							//updatedDisplayUsingEnviron(program->nextStatement(), NULL, placeInExecution, 0, we, ln);



						}
						else {
							//program->displayWithPosition(NULL,0,placeInRevEx);
							program->displayWithPositionAndNum(NULL,0,placeInRevEx,ln);



						}
						cout << "----------------------------------------------------------------------\n";

					}


				}
				else if (input == "vars") { //display the current state of variables
					cout << "--Displaying the current value of all variables\n";
					cout << "----------------------------------------------------------------------\n";
					this->displaySigGam();
					cout << "----------------------------------------------------------------------\n";
				}
				else if (input == "scope") {
					this->displayScopeEnvironment();
				}
				else if ((input == "loops") || (input == "beta")) { //display current while environment
					cout << "--Displaying the current while loop environment\n";
					cout << "----------------------------------------------------------------------\n";
					if (inRevEx) {
						if (placeInRevEx->type == 4) {
							cout << "currently executing a parallel statement\n";

						}
						simulator::displayBetaInterface(placeInRevEx);
					}
					else {

						simulator::displayBetaInterface(placeInExecution);
					}
					cout << "----------------------------------------------------------------------\n";
				}
				else if ((input == "procs") || (input == "mu")) { //display current procedure environment
					cout << "--Displaying the current procedure environment\n";
					cout << "----------------------------------------------------------------------\n";
					if (inRevEx) {
						//cout << "----------------------------------------------------------------------------------------------\n";
						simulator::displayMuInterface(placeInRevEx);
						//cout << "----------------------------------------------------------------------------------------------\n";

					}
					else {
						//cout << "----------------------------------------------------------------------------------------------\n";
						simulator::displayMuInterface(placeInExecution);
						//cout << "----------------------------------------------------------------------------------------------\n";
					}
					cout << "----------------------------------------------------------------------\n";

				}
				else if ((input.find("aux") == 0) || (input.find("delta") == 0)) { //display current auxiliary environment
					if (input.find("aux") == 0) { input.erase(0,4); }
					else if (input.find("delta") == 0) { input.erase(0,6); }

					cout << "--Displaying the current auxiliary environment";


					if (input == "") { //no extra parameter given - so display all stacks
						cout << " (all stacks)\n";
						cout << "----------------------------------------------------------------------\n";
						simulator::displayAuxInterface(true, "");
						cout << "----------------------------------------------------------------------\n";
					}
					else { //display a single stack based on the parameter given
						cout << " (stack " << input << ") only\n";
						cout << "----------------------------------------------------------------------\n";
						simulator::displayAuxInterface(false, input);
						cout << "----------------------------------------------------------------------\n";
					}
				}
				else if (input.find("ftrace") == 0) { //trace of the forwards execution
					cout << "Displaying the forwards trace file";
					cout << "----------------------------------------------------------------------\n";
					simulator::displayTrace();
					cout << "----------------------------------------------------------------------\n";
				}
				else if (input.find("rtrace") == 0) { //trace of the reverse execution
					cout << "Displaying the reverse trace file";
					cout << "----------------------------------------------------------------------\n";
					simulator::displayTraceInv();
					cout << "----------------------------------------------------------------------\n";
				}
				else if (input.find("status") == 0) { //output the current status of the execution
					cout << "--Current State:             ";
					if (setUp) { cout << "SetUp stage: Prior to All Execution\n"; }
					else if (inFwdEx) { cout << "In Forwards Execution\n"; }
					else if (postFwdEx) { cout << "Post Forwards Execution - Prior to Reverse Execution\n"; }
					else if (inRevEx) { cout << "In Inverse Execution\n"; }
					else if (postRevEx) { cout << "Post All Execution\n"; }

					cout << "--File Read:                 ";
					if (fileRead == 1) { cout << "Yes (" << fileNameOpened << ")\n"; }
					else { cout << "No (" << "Read a file via `read filename'" << ")\n"; }
					cout << "--State-Saving Mode:         ";
					if (*stateSaving == 1) { cout << "On (" << "switch off via `statesaving off'" << ")\n"; }
					else { cout << "Off (" << "switch on via `statesaving on'" << ")\n"; }
					cout << "--Execution Tracing Mode:    ";
					if (recordTrace) { cout << "On (" << "switch off via `tracing off'" << ")\n"; }
					else { cout << "Off (" << "switch on via `tracing on'" << ")\n"; }

					cout << "--Auto Removal Statements:   ";
					if (autoRemoval) { cout << "On (" << "switch off via `autorem off'" << ")\n"; }
					else { cout << "Off (" << "switch on via `autorem on'" << ")\n"; }

					cout << "--Output Mode:               ";
					if (output) { cout << "Yes (" << "switch off via `output off'" << ")\n"; }
					else { cout << "No (" << "switch on via `output on'" << ")\n"; }

					cout << "--Interleaving Mode:         ";
					if (forceUser) { cout << "User determined\n"; }
					else { cout << "Randomly determined\n"; }

					cout << "--Current Working Directory: " << directory << "\n";
					cout << "--Next Identifier:           " << *next << "\n";
					cout << "--Previous Identifier:       " << *previous << "\n";
					cout << "--Next Memory Location:      " << *nextLoc << "\n";
				}
				else {
					cout << "Unrecognised or currently unavailable argument or for command `display'";
				}
			}
		}
		else if (input.find("tracing") == 0) { //switch tracing mode on or off
			input.erase(0,8);
			if (input.find("on") == 0) {
				cout << "--Tracing mode now on\n";
				recordTrace = true;
			}
			else if (input.find("off") == 0) {
				cout << "--Tracing mode now off\n";
				recordTrace = false;
			}
			else {
				cout << "Incorrect Parameter passed to command 'tracing'\n";
			}
		}
		else if (input.find("autoconstructs") == 0) {
			input.erase(0,15);
			if (input.find("on") == 0) {
				cout << "--Construct identifiers will be automatically inserted\n";
				autoConstructIdens = true;
			}
			else if (input.find("off") == 0) {
				cout << "--Construct identifiers will NOT be automatically inserted\n";
				autoConstructIdens = false;
			}
			else {
				cout << "Incorrect Parameter passed to command 'autoconstructs'\n";
			}

		}
		else if (input.find("perm") == 0) {
			input.erase(0,5);
			if (input.find("ftrace") == 0) {
				cout << "Saving the trace of reverse execution to file: invTrace.txt\n";
				fileIO perm = fileIO();
				string fname = "execution-traces/perm-saved/fwd-trace[";

				//remove all path except the actual file name
				int i = fileNameOpened.find_last_of('/');
				fileNameOpened.erase(0,i+1);
				int j = fileNameOpened.find_first_of('.');
				fileNameOpened.erase(j);
				cout << "filename saved = " << fileNameOpened << "\n";


				fname += fileNameOpened;
				fname += "--";
				//add the time here

				time_t rawtime;
				struct tm * timeinfo;
				char buffer [80];

				time (&rawtime);
				timeinfo = localtime (&rawtime);

				strftime (buffer,80,"%H-%M%p",timeinfo);

				string timeString = buffer;

				fname += timeString;

				//end of adding time

				fname += "].txt";





				char *cstr = new char[fname.length() + 1];
				strcpy(cstr, fname.c_str());

				cout << "fname saved converted = " << cstr << "\n";

				perm.writeTrace(cstr, simulator::getTrace());
			}
			else if (input.find("rtrace") == 0) {

			}
			else {
				cout << "Incorrect Parameter passed to command 'tracing'\n";
			}
		}
		else if (input.find("statesaving") == 0) { //switch state-saving on or off
			input.erase(0,12);
			if (input.find("on") == 0) {
				cout << "--State-saving mode now on\n";
				*stateSaving = 1;
			}
			else if (input.find("off") == 0) {
				cout << "--State-saving mode now off\n";
				*stateSaving = 0;
			}
			else {
				cout << "Incorrect Parameter passed to command 'tracing'\n";
			}
		}
		else if (input.find("autorem") == 0) { //switch tracing mode on or off
			input.erase(0,8);
			if (input.find("on") == 0) {
				cout << "--Removal Statements will be inserted automatically during parsing\n";
				autoRemoval = true;
			}
			else if (input.find("off") == 0) {
				cout << "--Removal Statements must be explicitly provided by the programmer\n";
				autoRemoval = false;
			}
			else {
				cout << "Incorrect Parameter passed to command 'autorem'\n";
			}
		}
		else if (input.find("atomicexp") == 0) {
			input.erase(0,10);
			if (input.find("on") == 0) {
				cout << "--Expressions and Conditions will be atomically evaluated\n";
				atomicExpressions = true;
			}
			else if (input.find("off") == 0) {
				cout << "--Expressions and Conditions will be non-atomically evaluated\n";
				atomicExpressions = false;
			}
			else {
				cout << "Incorrect Parameter passed to command 'autorem'\n";
			}
		}
		else if (input.find("autopath") == 0) { //switch tracing mode on or off
			input.erase(0,9);
			if (input.find("on") == 0) {
				cout << "--Paths will be generated automatically during parsing\n";
				autoPaths = true;
			}
			else if (input.find("off") == 0) {
				cout << "--Paths must be explicitly provided by the programmer\n";
				autoPaths = false;
			}
			else {
				cout << "Incorrect Parameter passed to command 'autopath'\n";
			}
		}
		else if (input.find("output") == 0) { //switch output mode on or off
			input.erase(0,7);
			if (input.find("on") == 0) {
				cout << "--Output mode on\n";
				output = true;
			}
			else if (input.find("off") == 0) {
				cout << "--Output mode off\n";
				output = false;
			}
			else {
				cout << "Incorrect Parameter passed to command 'autopath'\n";
			}
		}
		else if (input.find("directory") == 0) { //change the directory that all examples will be read from
			input.erase(0,10);
			directory = input;
			cout << "Examples directory set to: ";
			if (directory == "") {
				cout << "Default\n";
			}
			else {
				cout << directory << "\n";
			}
		}
		else if (input.find("force") == 0) {
			input.erase(0,6);
			if (input == "user") { //force the user to settle any non-deterministic step choice
				forceUser = true;
				forceRandom = false;
				cout << "--All interleaving decisions will be made by the user at runtime.\n";
			}
			else if (input == "random") { //allow all non-determinism to be settled randomly at runtime
				forceUser = false;
				forceRandom = true;
				cout << "--All interleaving decisions will be made randomly at runtime.\n";
			}
			else {
				cout << "Incorrect Parameter for command `force'\n";
			}
		}
		else if (input.find("ss") == 0) {
			cout << "State-saving = " << *this->stateSaving << "\n";
			cout << "Directory = " << directory << "\n";
			cout << "next = " << *next << "\n";
			cout << "previous = " << *previous << "\n";
		}
		else if ((input.find("start") == 0) && (setUp)) { //start an execution from setup phase - must be forwards execution
			setUp = false;
			inFwdEx = true;
			cout << "--Starting Forwards Execution\n";
			cout << "--Execute fully via `go all' or perform n steps via `go n' (n >= 1)\n";
		}
		else if ((input.find("go") == 0) && (inFwdEx)) {
			bool finished = false;
			input.erase(0,3);
			if (input == "all") {
				//fix this problem - what if it has more than 100 steps
				finished = simulator::executeNSteps(-1, forceRandom, forceUser, false, seq, atomicExpressions,output);
			}
			else {
				string number = input;
				int nSteps = stringToInt11(number);
				cout << "--Executing " << nSteps << " steps if possible\n";
				finished = simulator::executeNSteps(nSteps, forceRandom, forceUser, false, seq, atomicExpressions,output);
			}

			if (finished) {
				inFwdEx = false;
				postFwdEx = true;
				cout << "--Inverse execution can be began via `start'\n";
			}

		}
		else if ((input.find("start") == 0) && (postFwdEx)) {
			cout << "--Inverse Execution Beginning\n";
			//invert the program
			program = invert(program,NULL);
			cout << "--Program has been inverted\n";
			pp->useLinkFunctions(program);
			cout << "--Inverse program has been linked\n";
			placeInRevEx = program->nextStatement(); //set pointer to beginning of inverse execution
			cout << "--Execute fully via `go all' or perform n steps via `go n' (n >= 1)\n";

			//minus one from all of nextNames
			decreaseNextNames(nextName);

			inRevEx = true;
			postFwdEx = false;
		}
		else if ((input.find("go") == 0) && (inRevEx)) {
			//inverse execution can now be simulated
			bool finished = false;
			input.erase(0,3);
			if (input == "all") {
				//cout << "Executing 500 steps\n";
				finished = simulator::executeNStepsInv(-1,forceRandom,forceUser,output);
			}
			else {
				string number = input;
				int nSteps = stringToInt11(number);
				cout << "Executing " << nSteps << " steps if possible\n";
				finished = simulator::executeNStepsInv(nSteps,forceRandom,forceUser,output);
			}

			if (finished) {
				inRevEx = false;
				postRevEx = true;
			}

		}
		else {
			cout << "Unrecognized command: please try again (command `help' for valid commands)\n";
		}
	}
}

/*
 * Switch state-saving on or off
 */
void simulator::switchStateSaving() {
	if (*stateSaving == 1) {
		*stateSaving = 0;
	}
	else {
		*stateSaving = 1;
	}
}

/*
 * Decide if statesaving is on
 */
int simulator::currentStateSaving() {
	return *stateSaving;
}

/*
 * Function to display entire program state
 */
void simulator::displayState() {

}

/*
 * Display the auxiliary data store -
 */
void simulator::displayAuxInterface(bool all, string stackName) {
	if (all) {
		std::tr1::unordered_map<std::string, stack<pair<int,int> > >::iterator it = (aux).begin();
		while (it != aux.end()) {

			stack<pair<int,int> > toPrintString = it->second;
			cout << it->first << "\t| ";
			displayStack(toPrintString);
			cout << "\n";
			cout << "----------------------------------------------------------------------\n";
			it++;
		}

		std::tr1::unordered_map<std::string, stack<pair<int,string> > >::iterator it2 = (auxAI).begin();
		while (it2 != auxAI.end()) {
			stack<pair<int,string> > toPrintString = it2->second;
			cout << it2->first << "\t| ";
			displayStackString(toPrintString);
			cout << "\n";
			cout << "----------------------------------------------------------------------\n";
			it2++;
		}

		if ((*AI.aiMap).empty()) {
			cout << "while loop info is empty\n";
		}
		else {
			cout << "while loop info is NOT EMPTY\n";
		}
	}
	else {
		if ((stackName == "WI") || (stackName == "Pr")) {
			//will be AUXAI store
			std::tr1::unordered_map<std::string, stack<pair<int,string> > >::const_iterator got3 = auxAI.find(stackName);
			if (got3 == auxAI.end()) {
				cout << "Stack " << stackName << " not found\n";
			}
			else {
				stack<pair<int,string> > toPrintString = got3->second;
				cout << got3->first << "\t| ";
				displayStackString(toPrintString);
				cout << "\n";
			}
		}
		else {
			//will in the other store
			std::tr1::unordered_map<std::string, stack<pair<int,int> > >::const_iterator got4 = aux.find(stackName);
			if (got4 == aux.end()) {
				cout << "Stack " << stackName << " not found\n";
			}
			else {
				stack<pair<int,int> > toPrintString = got4->second;
				cout << got4->first << "\t| ";
				displayStack(toPrintString);
				cout << "\n";
			}
		}
	}
}

/*
 * Display the procedure environment - always display entire environment
 */
void simulator::displayMuInterface(statement *current) {
	std::tr1::unordered_map<std::string,pair<std::string,statement*> >::iterator it = mu.begin();

	cout << "Procedure/Call Name" << setw(15) << "| Program" << "\n";

	while (it != mu.end()) {
		// Accessing KEY from element pointed by it.
		std::string procIden = it->first;

		cout << "--------------------------------------------------------------------\n";
		cout <<  procIden;

		// Accessing VALUE from element pointed by it.
		pair<std::string,statement*> toDisplay = it->second;
		string procName = toDisplay.first;

		programDisplayIndent(toDisplay.second,NULL,27,0,current,true,procIden,true,procIden.size());

		cout << "--------------------------------------------------------------------\n";

		it++;
	}
}

void simulator::displayBetaInterface(statement *current) {
	//cout << "--------------------------------------------------------------------\n";
	std::tr1::unordered_map<std::string,statement* > ::iterator it = beta.begin();

	statement *temp = current;

	cout << "Loop Name" << setw(15) << "| Program" << "\n";

	while (it != beta.end()) {
		current = temp;
		// Accessing KEY from element pointed by it.
		std::string loopName = it->first;

		cout << "----------------------------------------------------------------------\n";
		cout <<  loopName;

		WlS *i = static_cast<WlS*>(it->second);

		//cout << setw(19) << "| while " << i->WID << " (" << i->varName << " " << i->op << " " << i->newVal << ") do";
		cout << setw(19) << "| while " << i->WID << " (" << i->newConditionString << ") do";

		if (i == current) {
			cout << " <----------";
		}
		cout << "\n";


		//Update this section to handle nested parallel statements
		if (current->type == 4) {
			Par *cuPar = static_cast<Par*>(current);
			if (i->left == 1) {
				current = cuPar->currLeft;
			}
			else {
		//		cout << "current set to currRight\n";
				current = cuPar->currRight;
			}
		}
		//END of section to update for nested parallel statements

		programDisplayIndent(i->loopBody->nextStatement(),i,17,1,current,false,"",false,0);

		cout << setw(24) << "| elihw (" << i->path << ") [";
		showIden1(i->idendifiers);
		cout << "]\n";

		cout << "----------------------------------------------------------------------\n";

		// Increment the Iterator to point to next entry
		it++;
	}

	//cout << "--------------------------------------------------------------------\n";
}

void displayScopeSet(set<string> setOfStr) {
	//for(string it = toDisplay.begin(); it != toDisplay.end(); it++)
	//    {
	//        cout << it << endl;
	//    }

	std::set<std::string>::iterator it = setOfStr.begin();

	// Iterate till the end of set
	while (it != setOfStr.end())
	{
		// Print the element
		std::cout << (*it) << " , ";
		//Increment the iterator
		it++;
	}
	cout << "\n";
}

void simulator::displayScopeEnvironment() {
	//display the scope environment - std::tr1::unordered_map<std::string,set<std::string> >

	std::tr1::unordered_map<std::string,set<std::string> >::iterator it = (se.se)->begin();
		while(it != (se.se)->end()) {

			cout << it->first << " -- ";
			displayScopeSet(it->second);
			it++;
		}

}

void simulator::displaySigGam() {

	std::tr1::unordered_map<myPair,int,pair_hash>::iterator it = (gamma).begin();
	while(it != (gamma).end()) {
		myPair p = it->first;
		int loc = it->second;

		std::tr1::unordered_map<int,int >::const_iterator got3 = sigma.find(loc);
		if (got3 == sigma.end()) {
			std::cout << "location " << loc << " not found-3";
		}
		else {
			cout << "location " << loc << " -- (" << p.first << "," <<p.second << ") = " << got3->second << "\n";
		}
		it++;
	}
}

vector<string>* simulator::getTrace() {
	return pointToTrace;
}

vector<string>* simulator::getInvTrace() {
	return pointToInvTrace;
}

void simulator::TraceOnOff() {
	recordTrace = !recordTrace;
}

/*
 * Output the trace of the forwards execution to the console
 */
void simulator::displayTrace() {
	for (int i = 0; i < pointToTrace->size(); i++) {
		cout << pointToTrace->at(i) << "\n";
	}
}

/*
 * Output the trace of the inverse execution to the console
 */
void simulator::displayTraceInv() {
	for (int i = 0; i < pointToInvTrace->size(); i++) {
		cout << pointToInvTrace->at(i) << "\n";
	}
}

string removeVersionSim(string ori) {
	int i = ori.find_first_of(".");
	return ori.substr(0,i);
}

/*
 * Extract initial information from the program original source code - all variable names used - block names used, procedure names used
 * UPDATE for variable names used conditions, expressions, local declarations
 */
void getInitialInformation(linkedList *program, statement *stopper, set<string> *intermed, set<string> *blocknames, set<string> *constructNames) {

	statement *temp = program->nextStatement();

	while (temp != stopper) {

		if (temp->type == 0) {
			DA *t = static_cast<DA*>(temp);
			intermed->insert(t->varName);
		}
		else if (temp->type == 1) {
			CA *t = static_cast<CA*>(temp);
			intermed->insert(t->varName);
		}
		else if (temp->type == 2) {
			IfS *t = static_cast<IfS*>(temp);
			constructNames->insert(removeVersionSim(t->condID));
			//t->condID = t->condID + ".0";
			getInitialInformation(t->trueBranch,t,intermed,blocknames,constructNames);
			getInitialInformation(t->falseBranch,t,intermed,blocknames,constructNames);
		}
		else if (temp->type == 3) {
			WlS *t = static_cast<WlS*>(temp);
			constructNames->insert(removeVersionSim(t->WID));
			//t->WID = t->WID + ".0";
			getInitialInformation(t->loopBody,t,intermed,blocknames,constructNames);
		}
		else if (temp->type == 4) {
			Par *t = static_cast<Par*>(temp);
			getInitialInformation(t->leftSide,t,intermed,blocknames,constructNames);
			getInitialInformation(t->rightSide,t,intermed,blocknames,constructNames);
		}
		else if (temp->type == 5) {

			BlS *t = static_cast<BlS*>(temp);
			//cout << "inserting blockname = " << removeVersionSim(t->bid) << "\n";
			blocknames->insert(removeVersionSim(t->bid));
			constructNames->insert(removeVersionSim(t->bid));
			//t->bid = t->bid + ".0";
			getInitialInformation(t->blockBody,t,intermed,blocknames,constructNames);
		}
		else if (temp->type == 6) {
			//VdS *t = static_cast<VdS*>(temp);
		}
		else if (temp->type == 7) {
			PdS *t = static_cast<PdS*>(temp);
			constructNames->insert(removeVersionSim(t->procIden));
			//t->procIden = t->procIden + ".0";
			//is the proc body going to be linked here
			getInitialInformation(t->procBodyList,NULL,intermed,blocknames,constructNames);
		}
		else if (temp->type == 8) {
			PcS *t = static_cast<PcS*>(temp);
			constructNames->insert(removeVersionSim(t->callID));
			//t->callID = t->callID + ".0";
		}
		else if (temp->type == 9) {
			//potentially not necessary as everything in here will already have been handled in the declaration
			//PrS *t = static_cast<PrS*>(temp);
			//constructNames->insert(t->procIden);
			//t->procIden = t->procIden + ".0";
		}
		else if (temp->type == 10) {
			//similar to above - maybe not necessary to do this
			//VrS *t = static_cast<VrS*>(temp);
		}
		else if (temp->type == 13) {
			//atomic statement
			Atom *at = static_cast<Atom*>(temp);
			getInitialInformation(at->atomBody,at,intermed,blocknames,constructNames);
		}

		temp = temp->next;
	}
}

/*
 * Function to set up all necessary environments to begin an execution - all global versions of variables
 * All necessary stacks and environments
 */
void simulator::preExecutionSetUp() {
	//this will create all things that are necessary
	//start with variable names
	set<string> *vn = new set<string>();
	set<string> *bn = new set<string>();
	set<string> *cn = new set<string>();

	getInitialInformation(program,NULL,vn,bn,cn);
	vector<string> vars;

	//now use the set of var names to generate the vector<string>
	set<string>::iterator it;
	for (it = vn->begin(); it != vn->end(); ++it) {
		//push this variable name into the vector of variable names
		varNames.push_back(*it);

		//Initialise this global version of this variable to the next available location
		gamma[make_pair(*it,"lam")] = *nextLoc;

		//Initialise this location to the initial value of 0
		sigma[*nextLoc] = 0;
		*nextLoc += 1;

		//add a stack for this variable to the auxiliary store
		stack<pair<int,int> > s1;
		aux[*it] = s1;
	}

	//now produce the stack names necessary for evaluation
	stackNames = varNames;
	stackNames.push_back("B");
	stackNames.push_back("W");
	stackNames.push_back("WI");
	stackNames.push_back("Pr");

	//now finish the definition of the auxiliary store for stacks B/W/Wi/Pr
	stack<pair<int,int> > sb, sw;
	aux["B"] = sb;
	aux["W"] = sw;
	stack<pair<int,std::string> > sw1, spr;
	auxAI["WI"] = sw1;
	auxAI["Pr"] = spr;

	//populate the scope environment with necessary stacks for each block name
	set<string>::iterator it1;
	for (it1 = bn->begin(); it1 != bn->end(); ++it1) {
		set<std::string> sbn;
		scopeEnviron[*it1] = sbn;
	}

	//populate nextnames function - each construct initialised to 0
	set<string>::iterator it2;
	for (it2 = cn->begin(); it2 != cn->end(); ++it2) {
		nextNameFunction[*it2] = 1;
	}
}
}
