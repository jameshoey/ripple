//============================================================================
// Name        : parser.cpp
// Author      : James Hoey
// Version     :
// Copyright   : Copyright 2018
// Description : Implementation of the parser
//============================================================================

#include "parser.h"
#include "linkedList.h"
#include <iostream>
#include <vector>
#include <stack>
#include <stdlib.h>
#include <iterator>
#include <utility>
#include <tr1/unordered_map>
#include <stdlib.h>
#include <sstream>

namespace std {

//function to take a condition and build an expression tree structure from it

void displayTree1(expTree *root) {

	if ((root->leftTree == NULL) && (root->rightTree == NULL)) {
		cout << root->element.op << " ";
	}
	else {
		displayTree1(root->leftTree);
		cout << root->element.op << " ";
		displayTree1(root->rightTree);
	}
}



string getFirstBracket(string condition) {
	//extract the first bracket
	string toReturn = "";
	int openBrackets= 0;

	if (condition.find_first_of("(") == std::string::npos) {
//		cout << "no brackets contained\n";

		int i = condition.find_first_of(" ");
		return condition.erase(i);

	}
	else {

		for (int i = 0; i < condition.size(); i++) {
			if (condition.at(i) == '(') {
				openBrackets += 1;
				toReturn += condition.at(i);
			}
			else if (condition.at(i) == ')') {
				openBrackets -= 1;
				toReturn += condition.at(i);
				if (openBrackets == 0) {
					return toReturn;
				}
			}
			else {
				toReturn += condition.at(i);
			}
		}


	}

	return toReturn;
}


string extractFirst(string condition) {
	string toReturn = "";

	bool finished = false;

	int openBrackets = 0;

	//must begin with an open bracket
	if (condition.at(0) != '(') {
		cout << "ERROR: extracting first without brackets at the beginning\n";
	}

	toReturn += condition.at(0);
	int count = 1;
	while ((!finished) && (count < condition.size())) {
		if (condition.at(count) == '(') {
			openBrackets += 1;
			toReturn += condition.at(count);
		}
		else if ((condition.at(count) == ')') && (openBrackets == 0)) {
			//found the end - so return
			toReturn += condition.at(count);
			finished = true;
		}
		else {
			toReturn += condition.at(count);
		}
		count += 1;
	}


	return toReturn;
}

void splitCondition(string condition, string *f, string *s, string *o) { //condition of the form (X + 3) == (Y + 3)

	string fi = getFirstBracket(condition);
	int sizeOfFirst = fi.size();
	*f = fi;

	string op = condition.erase(0,sizeOfFirst+1);
	//cout << "first version of op = " << op << "\n";
	int endOfOp = op.find_first_of(" ");
	*o = op.erase(endOfOp);

	int startOfSecond = (op.size()+1);

	//cout << "condition before removal for second = " << condition << "\n";
	*s = condition.erase(0,startOfSecond);



}

bool isLeaf(string condition) {
	if (condition.find(" ") == -1) {
		return true;
	}
	return false;
}

int stringToInt(string s) {
	return atoi(s.c_str());
}

void parser::test(string test) {


	string first = "", second = "", op = "";
	string *f = &first, *s = &second, *o = &op;

	splitCondition(test,f,s,o);

	cout << "f = " << *f << ".\n";
	cout << "o = " << *o << ".\n";
	cout << "s = " << *s << ".\n";

}

bool typeOfOp1(string op) {
	//return true if this is a boolean operator
	//false if arithmetic

	if ((op == "==") || (op == "!=") || (op == ">") || (op == ">=") || (op == "<") || (op == "<=") || (op == "&&") || (op == "||")) {
		return true;
	}
	else {
		return false;
	}
}

/*
 * Function to take a condition (as a pointer) and creates a tree structure representing it.
 *
 */
void buildTree(expTree *result, string *condition) {

	string first = "", second = "", op = "";
	string *f = &first, *s = &second, *o = &op;

	if (isLeaf(*condition)) {

		result->element.op = *condition;
		result->element.evaluated = false;

		if (typeOfOp1(result->element.op)) {
			result->element.type = 1; //bool
		}
		else {
			result->element.type = 0; //arith
		}

		result->leftTree = NULL;
		result->rightTree = NULL;
		return;
	}

	splitCondition(*condition,f,s,o);

	result->element.op = *o;
	result->element.evaluated = false;
	if (typeOfOp1(result->element.op)) {
		result->element.type = 1; //bool
	}
	else {
		result->element.type = 0; //arith
	}

	expTree *left = new expTree;
	expTree *right = new expTree;

	//before recursively calling - remove the outer brackets
	if (first.find_first_of("(") != std::string::npos) {

		first.erase(0,1);
		first.erase(first.size()-1);
		second.erase(0,1);
		second.erase(second.size()-1);

	}

	buildTree(left,f);
	buildTree(right,s);

	result->leftTree = left;
	result->rightTree = right;

}






//end of update for expression trees

/*
 * Functions to link a program - all block bodies linked back the block - all loop bodies linked back to the loop
 * - all sides of parallel back to the parallel statement that contains them - all conditional branches linked back to the conditional
 */


void singBlock(statement *bl, statement *toAdd, statement *stopper) {
	BlS *block = static_cast<BlS*>(bl);

	//now link the end of the blockBody
	statement *temp = new statement;
	temp = block->blockBody->nextStatement();
	bool finished = false;

	while (!finished) {
		if (temp->next == stopper) {
			temp->next = toAdd;
			temp->last = true;
			finished = true;
		}
		else {
			temp = temp->next;
		}
	}
}

/*
 * Traverse the program, linking each block body end back to the block statement that contains it
 */
void allBlocks(linkedList *program, statement *stopper) {
	statement *temp = new statement;
	temp = program->nextStatement();
	bool finished = false;

	while (!finished) {
		if (temp == stopper) {
			finished = true;
		}
		else {
			if (temp->type == 2) {
				//found a conditional - look inside for parallels
				IfS *is = static_cast<IfS*>(temp);
				allBlocks(is->trueBranch,temp);
				allBlocks(is->falseBranch,temp);
			}
			else if (temp->type == 3) {
				WlS *wl = static_cast<WlS*>(temp);
				allBlocks(wl->loopBody,temp);
			}
			else if (temp->type == 4) {
				//found a parallel statement, link the last statement to the parallel statement again
				Par *ap = static_cast<Par*>(temp);
				allBlocks(ap->leftSide,ap);
				allBlocks(ap->rightSide,ap);
			}
			else if (temp->type == 5) {
				BlS *bls = static_cast<BlS*>(temp);
				singBlock(bls,temp,NULL);
				allBlocks(bls->blockBody,temp);
			}
			else if (temp->type == 7) {
				PdS *pds = static_cast<PdS*>(temp);
				allBlocks(pds->procBodyList,NULL);
			}
			else if (temp->type == 9) {
				PrS *prs = static_cast<PrS*>(temp);
				allBlocks(prs->procBodyList,NULL);
			}
			else if (temp->type == 13) {
				Atom *at = static_cast<Atom*>(temp);
				allBlocks(at->atomBody,NULL);
			}
			temp = temp->next;
		}
	}
}

//parallel statements

void singP(linkedList *branch, statement *toAdd, statement *stopper) {
	statement *temp = new statement;
	temp = branch->nextStatement();
	bool finished = false;

	while (!finished) {
		if (temp->next == stopper) {
			temp->next = toAdd;
			temp->last = true;
			finished = true;
		}
		else {
			temp = temp->next;
		}
	}
}

/*
 * Traverse the program, linking the end of each parallel side (thread) back to the original par statement that contains it
 */
void allP(linkedList *program, statement *stopper) {
	statement *temp = new statement;
	temp = program->nextStatement();
	bool finished = false;

	while (!finished) {
		if (temp == stopper) {
			finished = true;
		}
		else {
			if (temp->type == 2) {
				//found a conditional - look inside for parallels
				IfS *is = static_cast<IfS*>(temp);
				allP(is->trueBranch,temp);
				allP(is->falseBranch,temp);
			}
			else if (temp->type == 3) {
				WlS *wl = static_cast<WlS*>(temp);
				allP(wl->loopBody,temp);
			}
			else if (temp->type == 4) {
				//found a parallel statement, link the last statement to the parallel statement again
				Par *ap = static_cast<Par*>(temp);
				singP(ap->leftSide,temp,NULL);
				singP(ap->rightSide,temp,NULL);
				allP(ap->leftSide,temp);
				allP(ap->rightSide,temp);
			}
			else if (temp->type == 5) {
				BlS *bls = static_cast<BlS*>(temp);
				//allP(bls->localProcDec,NULL);
				allP(bls->blockBody,NULL);
			}
			else if (temp->type == 7) {
				PdS *i = static_cast<PdS*>(temp);
				allP(i->procBodyList,NULL);
			}
			else if (temp->type == 9) {
				PrS *i = static_cast<PrS*>(temp);
				allP(i->procBodyList,NULL);
			}
			else if (temp->type == 13) {
				Atom *at = static_cast<Atom*>(temp);
				allP(at->atomBody,NULL);
			}
			temp = temp->next;
		}
	}
}

//now for conditional statements

void singC(linkedList *branch, statement *toAdd, statement *stopper) {
	statement *temp = new statement;
	temp = branch->nextStatement();
	bool finished = false;


	while (!finished) {

		if (temp->next == stopper) {
			//temp is the last statement
			temp->next = toAdd;
			temp->last = true;
			finished = true;
		}
		else {
			temp = temp->next;
		}
	}
}

/*
 * Traverse the program, linking all conditional branches back to the conditional statement that contains them
 */
void allC(linkedList *program, statement *stopper) {
	statement *temp = new statement;
	temp = program->nextStatement();

	bool finished = false;

	while (!finished) {

		if (temp == stopper) {
			finished = true;
		}
		else {
			if (temp->type == 2) {
				//found a conditional statement
				IfS *is = static_cast<IfS*>(temp);
				singC(is->trueBranch,temp,NULL);
				singC(is->falseBranch,temp,NULL);
				allC(is->trueBranch,temp);
				allC(is->falseBranch,temp);
			}
			else if (temp->type == 3) {
				//while loop
				WlS *ws = static_cast<WlS*>(temp);
				allC(ws->loopBody,temp);
			}
			else if (temp->type == 4) {
				//parallel statement
				Par *ps = static_cast<Par*>(temp);
				allC(ps->leftSide,NULL);
				allC(ps->rightSide,NULL);
			}
			else if (temp->type == 5) {
				BlS *ps = static_cast<BlS*>(temp);
				//allC(ps->localProcDec,NULL);
				allC(ps->blockBody,NULL);
			}
			else if (temp->type == 7) {
				PdS *i = static_cast<PdS*>(temp);
				allC(i->procBodyList,NULL);
			}
			else if (temp->type == 9) {
				PrS *i = static_cast<PrS*>(temp);
				allC(i->procBodyList,NULL);
			}
			else if (temp->type == 13) {
				Atom *at = static_cast<Atom*>(temp);
				allC(at->atomBody,NULL);
			}
			temp = temp->next;
		}
	}
}

//now for while loops

void singW(linkedList *branch, statement *toAdd) {
	bool finished = false;

	statement *temp = new statement;
	temp = branch->nextStatement();

	while (!finished) {
		if (temp->next == NULL) {
			//then temp is the last statement
			temp->next = toAdd;
		//	cout << "last of a while loop found\n";
			temp->last = true;
			finished = true;
		}
		else {
			temp = temp->next;
		}
	}
}

/*
 * Traverse the program, linking all loop bodies back to the original loop statement that contains it
 */
void allW(linkedList *program) {
	statement *temp = new statement;
	temp = program->nextStatement();

	while (temp != NULL) {
		if (temp->type == 3) {
			//found a while loop
			WlS *aw = static_cast<WlS*>(temp);
			allW(aw->loopBody);
			singW(aw->loopBody,temp);

		}
		else if (temp->type == 2) {
			//conditional statement
			IfS *is = static_cast<IfS*>(temp);
			allW(is->trueBranch);
			allW(is->falseBranch);
		}
		else if (temp->type == 4) {
			//parallel statement
			Par *i = static_cast<Par*>(temp);
			allW(i->leftSide);
			allW(i->rightSide);
		}
		else if (temp->type == 5) {
			BlS *i = static_cast<BlS*>(temp);
			//allW(i->localProcDec);
			allW(i->blockBody);
		}
		else if (temp->type == 7) {
			PdS *i = static_cast<PdS*>(temp);
			allW(i->procBodyList);
		}
		else if (temp->type == 9) {
			PrS *i = static_cast<PrS*>(temp);
			allW(i->procBodyList);
		}
		else if (temp->type == 13) {
			Atom *at = static_cast<Atom*>(temp);
			allW(at->atomBody);
		}
		temp = temp->next;
	}
}

void singAtomic(linkedList *body, statement *toLink) {
	bool finished = false;

	statement *temp = new statement;
	temp = body->nextStatement();

	while (!finished) {
		if (temp->next == NULL) {
			//then temp is the last statement
			temp->next = toLink;
			temp->last = true;
			finished = true;
		}
		else {
			temp = temp->next;
		}
	}
}

void allAtomic(linkedList *program, statement *stopper) {
	statement *temp = new statement;
	temp = program->nextStatement();
	bool finished = false;

	while (!finished) {
		if (temp == stopper) {
			finished = true;
		}
		else {
			if (temp->type == 2) {
				//found a conditional - look inside for parallels
				IfS *is = static_cast<IfS*>(temp);
				allAtomic(is->trueBranch,temp);
				allAtomic(is->falseBranch,temp);
			}
			else if (temp->type == 3) {
				WlS *wl = static_cast<WlS*>(temp);
				allAtomic(wl->loopBody,temp);
			}
			else if (temp->type == 4) {
				//found a parallel statement, link the last statement to the parallel statement again
				Par *ap = static_cast<Par*>(temp);
				allAtomic(ap->leftSide,temp);
				allAtomic(ap->rightSide,temp);
			}
			else if (temp->type == 5) {
				BlS *bls = static_cast<BlS*>(temp);
				allAtomic(bls->blockBody,NULL);
			}
			else if (temp->type == 7) {
				PdS *i = static_cast<PdS*>(temp);
				allAtomic(i->procBodyList,NULL);
			}
			else if (temp->type == 9) {
				PrS *i = static_cast<PrS*>(temp);
				allAtomic(i->procBodyList,NULL);
			}
			else if (temp->type == 13) {
				Atom *at = static_cast<Atom*>(temp);
				singAtomic(at->atomBody,at);
				allAtomic(at->atomBody,at);
			}
			temp = temp->next;
		}
	}

}

/*
 * Function that performs all necessary linking of a program - calls the functions defined above
 */
void linkAll(linkedList *program) {
	program->modifiyList();
	allW(program);
	allC(program,NULL);
	allP(program,NULL);
	allBlocks(program,NULL);

	//finally, link all atomic statement bodies back to the original atomic statement
//	allAtomic(program,NULL);
}

//-----End of link all statements

//+++++++++++++++++++++++++++HELPER PARSER FUNCTIONS+++++++++++++++++++++++++++++++++++++++++++++++++

/*
 * Returns only the first statement from the given vector of strings - will return a single line for some statements, but a shorter vector for others
 */
vector<string> splitVectorFirstStatement(vector<string> program) {
	vector<string> toReturn;
	int count = 0;
	string head = program.at(0);

	if (head.find("if") == 0) {
		count += 1;
		toReturn.push_back(head);

		for (int i = 1; i < program.size(); i++) { //begin at 1 - remove the head
			if (program.at(i).find("if") == 0) {
				count+=1;
				toReturn.push_back(program.at(i));
			}
			else if ((program.at(i).find("fi") == 0) && (count == 1)) {
				//found the end - so return
				toReturn.push_back(program.at(i));
				return toReturn;
			}
			else if ((program.at(i).find("fi") == 0) && (count != 1)) {
				//not the end yet
				count-=1;
				toReturn.push_back(program.at(i));
			}
			else {
				toReturn.push_back(program.at(i));
			}
		}
	}
	else if (head.find("while") == 0) {
		count += 1;
		toReturn.push_back(head);

		for (int i = 1; i < program.size(); i++) { //begin at 1 - remove the head
			if (program.at(i).find("while") == 0) {
				count+=1;
				toReturn.push_back(program.at(i));
			}
			else if ((program.at(i).find("elihw") == 0) && (count == 1)) {
				//found the end - so return
				toReturn.push_back(program.at(i));
				return toReturn;
			}
			else if ((program.at(i).find("elihw") == 0) && (count != 1)) {
				//not the end yet
				count-=1;
				toReturn.push_back(program.at(i));
			}
			else {
				toReturn.push_back(program.at(i));
			}
		}
	}
	else if (head.find("proc") == 0) {
		count += 1;
		toReturn.push_back(head);

		for (int i = 1; i < program.size(); i++) { //begin at 1 - remove the head
			if (program.at(i).find("proc") == 0) {
				count+=1;
				toReturn.push_back(program.at(i));
			}
			else if (program.at(i).find("remove proc") == 0) {
				count+=1;
				toReturn.push_back(program.at(i));
			}
			else if ((program.at(i).find("corp") == 0) && (count == 1)) {
				//found the end - so return
				toReturn.push_back(program.at(i));
				return toReturn;
			}
			else if ((program.at(i).find("corp") == 0) && (count != 1)) {
				//not the end yet
				count-=1;
				toReturn.push_back(program.at(i));
			}
			else {
				toReturn.push_back(program.at(i));
			}
		}
	}
	else if (head.find("remove proc") == 0) {
		count += 1;
		toReturn.push_back(head);

		for (int i = 1; i < program.size(); i++) { //begin at 1 - remove the head
			if (program.at(i).find("proc") == 0) {
				count+=1;
				toReturn.push_back(program.at(i));
			}
			else if (program.at(i).find("remove proc") == 0) {
				count+=1;
				toReturn.push_back(program.at(i));
			}
			else if ((program.at(i).find("corp") == 0) && (count == 1)) {
				//found the end - so return
				toReturn.push_back(program.at(i));
				return toReturn;
			}
			else if ((program.at(i).find("corp") == 0) && (count != 1)) {
				//not the end yet
				count-=1;
				toReturn.push_back(program.at(i));
			}
			else {
				toReturn.push_back(program.at(i));
			}
		}
	}
	else if (head.find("begin") == 0) {
		count += 1;
		toReturn.push_back(head);

		for (int i = 1; i < program.size(); i++) { //begin at 1 - remove the head
			if (program.at(i).find("begin") == 0) {
				count+=1;
				toReturn.push_back(program.at(i));
			}
			else if ((program.at(i).find("end") == 0) && (count == 1)) {
				//found the end - so return
				toReturn.push_back(program.at(i));
				return toReturn;
			}
			else if ((program.at(i).find("end") == 0) && (count != 1)) {
				//not the end yet
				count-=1;
				toReturn.push_back(program.at(i));
			}
			else {
				toReturn.push_back(program.at(i));
			}

		}
	}
	else if (head.find("par") == 0) {
		//cout << "\n parse a par\n";
		count += 1;
		toReturn.push_back(head);

		for (int i = 1; i < program.size(); i++) { //begin at 1 - remove the head
			if (program.at(i).find("par") == 0) {
				count+=2;
				toReturn.push_back(program.at(i));
			}
			else if ((program.at(i).find("}") == 0) && (count == 0)) {
				//found the end - so return
				toReturn.push_back(program.at(i));
				return toReturn;
			}
			else if ((program.at(i).find("}") == 0) && (count != 0)) {
				//not the end yet
				count-=1;
				toReturn.push_back(program.at(i));
			}
			else {
				toReturn.push_back(program.at(i));
			}
		}
		//cout << "par done\n";
	}
	else if (head.find("atomic") == 0) {
		//parsing an atomic block

		count += 1;
		toReturn.push_back(head);

		for (int i = 1; i < program.size(); i++) { //begin at 1 - remove the head
			if (program.at(i).find("atomic") == 0) {
				count+=1;
				toReturn.push_back(program.at(i));
			}
			else if ((program.at(i).find("cimota") == 0) && (count == 1)) {
				//found the end - so return
				toReturn.push_back(program.at(i));
				return toReturn;
			}
			else if ((program.at(i).find("cimota") == 0) && (count != 1)) {
				//not the end yet
				count-=1;
				toReturn.push_back(program.at(i));
			}
			else {
				toReturn.push_back(program.at(i));
			}
		}



	}
	else {
		//all single line statements will be handled here - assignments/calls/local variable declaration
		toReturn.push_back(head);
		return toReturn;
	}
	return toReturn;
}

/*
 * Takes a vector string that is only the parallel statement as a whole, extracts only the left side of that statement (shorter vector of strings)
 */
vector<string> getParLeftSide(vector<string> program) {
	vector<string> toReturn;
	int count = 0;

	if (program.at(0).find("par") != 0) {
		cout << "error in getParLeftSide - program doesn't begin with a par statement!\n";
		return toReturn;
	}

	for (int i = 1; i < program.size(); i++) { //begin at 1 - remove the head
		if (program.at(i).find("par") == 0) {
			count += 2;
			toReturn.push_back(program.at(i));
		}
		else if ((program.at(i).find("}") == 0) && (count == 0)) {
			//dont add this statement to the result
			return toReturn;
		}
		else if ((program.at(i).find("}") == 0) && (count != 0)) {
			count-=1;
			toReturn.push_back(program.at(i));
		}
		else {
			toReturn.push_back(program.at(i));
		}
	}
	return toReturn;
}

/*
 * Takes a vector string that is only the parallel statement as a whole, extracts only the right side of that statement (shorter vector of strings)
 */
vector<string> getParRightSide(vector<string> program) {
	vector<string> toReturn;

	vector<string> leftSide = getParLeftSide(program);
	int sizeOfLeft = leftSide.size();

	for (int i = sizeOfLeft+3; i < program.size() - 1; i++) {
		toReturn.push_back(program.at(i));
	}

	return toReturn;
}

/*
 * Takes a vector string that is only the conditional statement as a whole, extracts only the true branch of that statement (shorter vector of strings)
 */
vector<string> getCondTrueBranch(vector<string> program) {
	//RETURN THE TRUE BRANCH OF A GIVEN CONDITIONAL - PROGRAM IS RESULT OF SPLIT - CONTAIN ONLY A COND STATEMENT
	vector<string> toReturn;
	int count = 0;

	if (program.at(0).find("if") != 0) {
		cout << "error in getCondTrueBranch - program doesn't begin with an if statement!\n";
		return toReturn;
	}

	for (int i = 1; i < program.size(); i++) {
		if (program.at(i).find("if") == 0) {
			count+=1;
			toReturn.push_back(program.at(i));
		}
		else if ((program.at(i).find("else") == 0) && (count == 0)) {
			//found the end of the original true branch
			//dont add this statement to the result
			return toReturn;
		}
		else if ((program.at(i).find("else") == 0) && (count != 0)) {
			count-=1;
			toReturn.push_back(program.at(i));
		}
		else {
			toReturn.push_back(program.at(i));
		}
	}

	return toReturn;
}

/*
 * Takes a vector string that is only the conditional statement as a whole, extracts only the false branch of that statement (shorter vector of strings)
 * Makes use of getCondTrueBranch above
 */
vector<string> getCondFalseBranch(vector<string> program) {
	vector<string> toReturn;

	vector<string> trueBranch = getCondTrueBranch(program);
	int sizeOfTrue = trueBranch.size();

	for (int i = sizeOfTrue+2; i < program.size() - 1; i++) {
		toReturn.push_back(program.at(i));
	}

	return toReturn;
}

/*
 * Takes a vector string that is only the while loop as a whole, this extracts only the loop body (shorter vector of strings)
 */
vector<string> getLoopBody(vector<string> program) {
	vector<string> toReturn;
	int count = 0;

	if (program.at(0).find("while") != 0) {
		cout << "error in getLoopBody - program doesn't begin with a while statement!\n";
		return toReturn;
	}

	for (int i = 1; i < program.size(); i++) {
		if (program.at(i).find("while") == 0) {
			count+=1;
			toReturn.push_back(program.at(i));
		}
		else if ((program.at(i).find("elihw") == 0) && (count == 0)) {
			//found the end of the original true branch
			//dont add this statement to the result
			return toReturn;
		}
		else if ((program.at(i).find("elihw") == 0) && (count != 0)) {
			count-=1;
			toReturn.push_back(program.at(i));
		}
		else {
			toReturn.push_back(program.at(i));
		}
	}
	return toReturn;
}

/*
 * Get the procedure body from a declaration statement
 */
vector<string> getProcDecBody(vector<string> program) {
	vector<string> toReturn;
	int count = 0;

	if ((program.at(0).find("proc") != 0) && (program.at(0).find("remove proc") != 0)) {
		cout << "error in getProcDecBody - program doesn't begin with a procedure declaration statement!\n";
		return toReturn;
	}

	for (int i = 1; i < program.size(); i++) {
		if (program.at(i).find("proc") == 0) {
			count+=1;
			toReturn.push_back(program.at(i));
		}
		else if (program.at(i).find("remove proc") == 0) {
			count+=1;
			toReturn.push_back(program.at(i));
		}
		else if ((program.at(i).find("corp") == 0) && (count == 0)) {
			//found the end of the original true branch
			//dont add this statement to the result
			return toReturn;
		}
		else if ((program.at(i).find("corp") == 0) && (count != 0)) {
			count-=1;
			toReturn.push_back(program.at(i));
		}
		else {
			toReturn.push_back(program.at(i));
		}
	}

	return toReturn;
}

/*
 * Return all of the local variable declaration statements from a given block body - used to automatically generate removal statements
 */
vector<string> getBlockDV(vector<string> program) {
	//RETURN ALL LOCAL DECLARATIONS LOCAL TO THE MOST DIRECT BLOCK
	vector<string> toReturn;

	if (program.at(0).find("begin") != 0) {
		cout << "error in getBlockDV - program doesn't begin with a block statement!\n";
		return toReturn;
	}

	for (int i = 1; i < program.size(); i++) {
		if (program.at(i).find("var") == 0) {
			toReturn.push_back(program.at(i));
		}
		else {
			//found the end of local declarations
			return toReturn;
		}
	}

	return toReturn;
}

/*
 * Return all of the procedure declaration statements from a given block body - used to automatically generate removal statements
 */
vector<string> getBlockDP(vector<string> program) {
	//RETURN ALL PROCEDURE DECLARATIONS LOCAL TO THE MOST DIRECT BLOCK
	vector<string> toReturn;

	if (program.at(0).find("begin") != 0) {
		cout << "error in getBlockDP - program doesn't begin with a block statement!\n";
		return toReturn;
	}

	vector<string> dv = getBlockDV(program);
	int sizeOfDV = dv.size() + 1;

	vector<string> lc = program, temp;
	lc.erase(lc.begin(),lc.begin()+sizeOfDV);

	bool finished = false;

	while (!finished) {
		temp = splitVectorFirstStatement(lc);
		if (temp.at(0).find("proc") == 0) {
			//another procedure - so add to result
			for (int i = 0; i < temp.size(); i++) {
				toReturn.push_back(temp.at(i));
			}
			lc.erase(lc.begin(),lc.begin()+temp.size());
		}
		else {
			//found the end
			finished = true;
		}
	}

	return toReturn;
}

/*
vector<string> getBlockBody(vector<string> program) {
	//RETURN THE BODY OF THE LOOP
	vector<string> toReturn;

	if (program.at(0).find("begin") != 0) {
		cout << "error in getBlockBody - program doesn't begin with a block statement!\n";
		return toReturn;
	}

	vector<string> dv = getBlockDV(program);
	vector<string> dp = getBlockDP(program);
	int start = dv.size() + dp.size() + 1;

	for (int i = start; i < program.size()-1; i++) {
		toReturn.push_back(program.at(i));
	}

	return toReturn;

}
*/


/*
 * Take a vector of strings of a block statement, return the entire block body as a whole
 * Remove the first string (begin) and the last (end) - return everything else
 */
vector<string> getEntireBlockBody(vector<string> program) {
	vector<string> toReturn;

	if (program.at(0).find("begin") != 0) {
		cout << "error in getBlockBody - program doesn't begin with a block statement!\n";
		return toReturn;
	}

	//remove the first string (begin) and the final string (end) - return everything else
	for (int i = 1; i < program.size()-1; i++) {
		toReturn.push_back(program.at(i));
	}

	return toReturn;
}


/*
 * Get the body contained within atomic tags
 */

vector<string> getAtomicBody(vector<string> program) {
	vector<string> toReturn;

	if (program.at(0).find("atomic") != 0) {
		cout << "error in getBlockBody - program doesn't begin with a block statement!\n";
		return toReturn;
	}

	//remove the first string (begin) and the final string (end) - return everything else
	for (int i = 1; i < program.size()-1; i++) {
		toReturn.push_back(program.at(i));
	}


	return toReturn;
}

//+++++++++++++++++++++++++++CONVERSION++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//int stringToInt(string s) {
//	return atoi(s.c_str());
//}

string intToString(int i) {
	stringstream ss;
	ss << i;
	string str = ss.str();

	return str;
}

//---------------------------END OF CONVERSION-------------------------------------------------------

//+++++++++++++++++++++++++++EXTRACT PARTS OF STRINGS++++++++++++++++++++++++++++++++++++++++++++++++


string getLoopCondUpdated(string s) {
	s.erase(s.begin(),s.begin()+6);

	int i = s.find_first_of("(");
	s.erase(s.begin(),s.begin()+(i+1));
	int j = s.find_last_of(")");
	s.erase(j);

//	cout << "NEW CONDITION = " << s << "\n";

	return s;
}

string getLoopCond(string s) { //get the conditional from a loop statement
	s.erase(s.begin(),s.begin()+7);
	int i = s.find_first_of(" ");
	s.erase(0,i+1);
	s.erase(s.size()-3,s.size());
	return s;
}

string getIfID(string full) { //get the unique identifier of a conditional statement
	full = full.erase(0,3);
	int i = full.find_first_of("(");
	return full.erase(i-1);
}

string getWhileID(string full) { //get the unique identifier of a while loop statement
	full = full.erase(0,6);
	int i = full.find_first_of("(");
	return full.erase(i-1);
}

string extractVarFromCondition(string s) { //extract variable used in condition
	//remove bracket
	s.erase(0,1);
	int end = s.find_first_of(" ");
	return s.erase(end);
}

string extractVarFromAssign(string s) { //extract variable used in an assignment
	int end = s.find_first_of(" ");
	return s.erase(end);
}

string extractOpFromCondition(string s) { //get the operator from a given condition (e.g ==, < etc)
	int endOfVar = s.find_first_of(" ");
	s.erase(0,endOfVar+1);
	int endOfOp = s.find_first_of(" ");
	return s.erase(endOfOp);
}

string extractNewValFromCondition(string s) { //get other value used in condition - essential for evaluation
	int endOfVar = s.find_first_of(" ");
	s.erase(0,endOfVar+1);
	int endOfOp = s.find_first_of(" ");
	s.erase(0,endOfOp+1);
	return s.erase(s.size()-1,s.size());
}

string extractBlockNameFromLoop(vector<string> program) {
	string final = program.at(program.size()-1);
	final.erase(0,6); //remove the elihw
	final.erase(0,1); //remove first bracket
	final.erase(final.size()-1); //remove the end bracket
	return final;
}

string extractBlockNameFromProcDec(vector<string> program) {
	string final = program.at(program.size()-1);
	final.erase(0,5); //remove the corp
	final.erase(0,1); //remove first bracket
	final.erase(final.size()-1); //remove the end bracket
	return final;
}

string extractBlockNameFromProcRem(vector<string> program) {
	string final = program.at(program.size()-1);
	final.erase(0,5); //remove the corp
	final.erase(0,1); //remove first bracket
	final.erase(final.size()-1); //remove the end bracket
	return final;
}

string extractBlockNameFromCall(string call) {
	//form will be call c1 func(40) (b1)
	int i = call.find_last_of("(");
	call.erase(0,i+1);
	call.erase(call.size()-1,call.size());
	return call;
}

string extractBlockNameFromAssign(string update) {
	//form will be X = 10 (b1) / X += 10 (b1)
	int i = update.find_last_of("(");
	update.erase(0,i+1);
	update.erase(update.size()-1,update.size());
	return update;
}

string extractBlockNameFromLocDec(string update) {
	//form will be remove X = v (b1)
	int i = update.find_last_of("(");
	update.erase(0,i+1);
	update.erase(update.size()-2,update.size());
	return update;
}


string extractBlockNameFromLocRem(string update) {
	//form will be remove X = v (b1)
	int i = update.find_last_of("(");
	update.erase(0,i+1);
	update.erase(update.size()-2,update.size());
	return update;
}

string extractNewValFromAssign(string s) {
	int endOfVar = s.find_first_of(" ");
	s.erase(0,endOfVar+1);
	//cout << "s = " << s << "\n";
	int endOfOp = s.find_first_of(" ");
	s.erase(0,endOfOp+1);
	int endOfVal = s.find_first_of(" ");
	return s.erase(endOfVal);
}

string extractLocalDecNewVal(string s) {
	//form will be var X = 10 (b1)
	string toReturn = s.erase(0,4);
	int i = toReturn.find_first_of(" ");
	toReturn.erase(0,i+1);
	int j = toReturn.find_first_of(" ");
	toReturn.erase(0,j+1);
	int k = toReturn.find_first_of(" ");
	return toReturn.erase(k);
}

string extractLocalRemNewVal(string s) {
	//form will be remove var X = 10 (b1)
	string toReturn = s.erase(0,11);
	int i = toReturn.find_first_of(" ");
	toReturn.erase(0,i+1);
	int j = toReturn.find_first_of(" ");
	toReturn.erase(0,j+1);
	int k = toReturn.find_first_of(" ");
	return toReturn.erase(k);
}

string extractProcNameDec(string s) {
	//will now be of the form proc p1.0 func is
	string toReturn = s.erase(0,5);
	int i = toReturn.find_first_of(" ");
	toReturn = toReturn.erase(0,i+1);
	int j = toReturn.find_first_of(" ");
	toReturn = toReturn.erase(j);
	return toReturn;
}

string extractProcNameRem(string s) {
	//will now be of the form remove proc p1.0 func is
	string toReturn = s.erase(0,12);
	int i = toReturn.find_first_of(" ");
	toReturn = toReturn.erase(0,i+1);
	int j = toReturn.find_first_of(" ");
	toReturn = toReturn.erase(j);
	return toReturn;
}

string extractProcIden(string s) {
	string toReturn = s.erase(0,5);
	int i = toReturn.find_first_of(" ");
	toReturn.erase(i);
	return toReturn;
}

string extractProcIdenRem(string s) {
	string toReturn = s.erase(0,12);
	int i = toReturn.find_first_of(" ");
	toReturn.erase(i);
	return toReturn;
}

string extractFormalArgProcDec(string s) {
	//the form proc func(var X) is
	string toReturn = s.erase(0,5);
	int i = toReturn.find_first_of("(");
	toReturn = toReturn.erase(0,i+5);
	int j = toReturn.find_first_of(")");
	toReturn.erase(j);
	return toReturn;
}

string extractCallProcName(string s) {
	string toReturn = s.erase(0,5);
	int j = toReturn.find_first_of(" ");
	return toReturn.erase(j);
}

string extractProcNameCall(string s) {
	//of the form - call c1 func(100) (b1) - should return func
	std::string toReturn = "";

	toReturn = s.erase(0,5);
	int i = toReturn.find_first_of(" ");
	toReturn.erase(0,i+1);
	int j = toReturn.find_first_of("(");

	return toReturn.erase(j-1);
}

string extractActualArgProcCall(string s) {
	//of the form - call c1 func(100) (b1) - should return 100
	string toReturn = s.erase(0,5);
	int i = toReturn.find_first_of("(");
	toReturn = toReturn.erase(0,i+1);
	//cout << "toReturn = " << toReturn << "\n";
	int j = toReturn.find_first_of(")");
	toReturn = toReturn.erase(j);
	return toReturn;
}

string getPath(string s) {
	string toReturn = s;
	int i = toReturn.find_last_of(" ");
	//cout << "i = " << i << "\n";
	toReturn = toReturn.erase(0,i+1);
	//cout << "path at this point = " << toReturn << "\n";
	//now remove the brackets - first step
	toReturn = toReturn.erase(0,1);
	//cout << "path at this point 1 = " << toReturn << "\n";

	int k = toReturn.find_last_of(")");
	toReturn = toReturn.erase(k,toReturn.size());
	return toReturn;
}

string getOwnNameOfBlock(string s) {
	//the form begin bid
	//cout << "ORIGINAL block name here = |" << s << "|\n";
	s.erase(0,6);
	//cout << "block name here = |" << s << "|\n";
	return s;
}

string extractLocalDecVarName(string full) {
	//form will be var X = 10 (b1)
	string toReturn = full.erase(0,4);
	int i = toReturn.find_first_of(" ");
	return toReturn.erase(i);
}

string extractLocalRemVarName(string full) {
	//form will be remove var X = 10 (b1)
	string toReturn = full.erase(0,11);
	int i = toReturn.find_first_of(" ");
	return toReturn.erase(i);
}

bool determineIncOrDec(string s) {
	if (s.find("+=") != -1) {
		return true;
	}
	return false;
}

bool determineConst(string s) {
	if ((s.find("+=") != -1) || (s.find("-=") != -1)) {
		return true;
	}
	return false;
}

string extractNewValFromIfCondition(string s) {
	int endOfVar = s.find_first_of(" ");
	s.erase(0,endOfVar+1);
	//cout << "s = " << s << "\n";
	int endOfOp = s.find_first_of(" ");
	s.erase(0,endOfOp+1);
	return s;
}

string extractVarFromIfCondition(string s) {
	//cout << "-----------starting condition = " << s << "\n";
	int end = s.find_first_of(" ");
	return s.erase(end);
}

string getIfID1(string full) {
	full = full.erase(0,3);
	int i = full.find_first_of("(");
	return full.erase(i-1);
}

string extractBlockNameFromIf(vector<string> program) {
	string final = program.at(program.size()-1);
	final.erase(0,3); //remove the fi
	final.erase(0,1); //remove first bracket
	final.erase(final.size()-2); //remove the end bracket
	return final;
}

string getAssignmentExpression(string s) {
	int i = s.find_first_of("(");
	s.erase(s.begin(),s.begin()+(i+1));
	int j = s.find_last_of(")");
	s.erase(j);
	int k = s.find_last_of(")");
	s.erase(k);

//	cout << "NEW EXPRESSION = " << s << "\n";

	return s;
}

string getIfCondUpdated(string s) {
	s.erase(s.begin(),s.begin()+3);

	int i = s.find_first_of("(");
	s.erase(s.begin(),s.begin()+(i+1));
	int j = s.find_last_of(")");
	s.erase(j);

//	cout << "NEW CONDITION = " << s << "\n";

	return s;
}

string getIfCond(string s) {
	//EXTRACT CONDITION OF AN IF STATEMENT
	s.erase(s.begin(),s.begin()+3);

	int i = s.find_first_of("(");
	s.erase(s.begin(),s.begin()+(i+1));
	int j = s.find_first_of(")");
	s.erase(j);

	//s.erase(s.size()-6,s.size());
	return s;
}

string extractOpFromIfCondition(string s) {
	int endOfVar = s.find_first_of(" ");
	s.erase(0,endOfVar+1);
	//cout << "s = " << s << "\n";
	int endOfOp = s.find_first_of(" ");
	return s.erase(endOfOp);
}

//+++++++++++++++++++++++++++END OF EXTRACT PARTS OF STRINGS+++++++++++++++++++++++++++++++++++++++++

//+++++++++++++++++++++++++++RECURSION HELPER FUNCTIONS++++++++++++++++++++++++++++++++++++++++++++++

string increaseCallID(string currentID) {
	//of the form c1.0

	string toReturn;

	toReturn = currentID;
	int i = toReturn.find_first_of(".");
	toReturn.erase(i+1);

	string val = currentID.erase(0,i+1);
	int valI = stringToInt(val);
	//valI += 1;
	//string newVal = intToString(valI);

	toReturn = toReturn + intToString(valI+1);

	return toReturn;
}

vector<string> addRemoval(vector<string> original, vector<string> dv, vector<string> dp) {

	int openBlocks = 0;

	//add the removal of procedures
	for (int i = 0; i < dp.size(); i++) {

		if ((dp.at(i).find("proc") == 0) && (openBlocks == 0)) {
			original.push_back("remove " + dp.at(i));
		}
		else if (dp.at(i).find("begin") == 0) {
			openBlocks += 1;
			original.push_back(dp.at(i));
		}
		else if (dp.at(i).find("end") == 0) {
			openBlocks -= 1;
			original.push_back(dp.at(i));
		}
		else {
			original.push_back(dp.at(i));
		}

	}

	//now add the removal of variables
	for (int i = 0; i < dv.size(); i++) {
		original.push_back("remove " + dv.at(i));
	}

	return original;
}

//+++++++++++++++++++++++++++END OF RECURSION++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*
 * Constructor - sets a necessary link to the original program as a vector of strings
 */
parser::parser(vector<string> orig) {
	originalProgram = orig;
	programList = new linkedList();
}

/*
 * The function that will be called on programs read from file (as a series of strings) in order to parse it
 */
linkedList* parser::parseProgram(vector<string> program, bool autoRemoval) {
	linkedList *obj = new linkedList;

	while (!program.empty()) { //until we reach the end of the vector of strings

		//get the next statement from the program
		vector<string> partial = splitVectorFirstStatement(program);
		int j = partial.size();

		//remove this statement from the overall program as it will be handled here
		program.erase(program.begin(),program.begin()+j);

		//if size of next statement = 1, this must be a single line program statement (DA,CA,LVD,LVR,PD,PR,PC
		if (partial.size() == 1) {

			if (determineConst(partial.at(0))) {
				//constructive assignment
				string full = partial.at(0);

				//update for more complex expressions
				string condition = getAssignmentExpression(full);
				expTree *conTree = new expTree;
				buildTree(conTree,&condition);

				string varName = extractVarFromAssign(full);
				string blockName = extractBlockNameFromAssign(full);
				bool inc = determineIncOrDec(full);
				string path = getPath(full);

				obj->addCAUpdated(1,full,varName,blockName,condition,conTree,inc,NULL,path);
			}
			else if (partial.at(0).find("var") == 0) {
				//local variable declaration
				string full = partial.at(0);
				string varName = extractLocalDecVarName(full);
				string value = extractLocalDecNewVal(full);
				int newVal = stringToInt(value);
				string blockName = extractBlockNameFromLocDec(full);
				string path = getPath(full);
				obj->addLVD(6,full,varName,newVal,blockName,NULL,path);
			}
			else if (partial.at(0).find("call") != -1) {
				//call statement
				string full = partial.at(0);
				string cid = extractCallProcName(full);
				string procName = extractProcNameCall(full);
				string actualV = extractActualArgProcCall(full);
				int actualValue = stringToInt(actualV);
				string blockName = extractBlockNameFromCall(full);
				string path = getPath(full);
				obj->addPC(8,full,cid,procName,blockName,NULL,path);
			}
			else if (partial.at(0).find("remove") != -1) {
				//local variable removal
				string full = partial.at(0);
				string varName = extractLocalRemVarName(full);
				string value = extractLocalRemNewVal(full);
				int newVal = stringToInt(value);
				string blockName = extractBlockNameFromLocRem(full);
				string path = getPath(full);
				obj->addLVR(10,full,varName,newVal,blockName,NULL,path);
			}
			else if (partial.at(0) == "skip") {
				//indicates a placeholder used to represent an empty branch (either conditional, loop, block, procedure body

				PH *placeHolder = new PH;
				placeHolder->type = 12;
				placeHolder->oldType = 12;
				placeHolder->next = NULL;
				placeHolder->parentPar = NULL;
				placeHolder->left = -1;
				obj->addStatement(placeHolder);
			}
			else if (partial.at(0).find("abort") == 0) {
				//indicates that the execution should be stopped at this point - no further execution required (maybe breakpoint)
				Abort *ab = new Abort;
				ab->type = 99;
				ab->oldType = 99;
				ab->parentPar = NULL;
				ab->left = -1;
				ab->next = NULL;
				obj->addStatement(ab);
			}
			else {
				//destructive assignment
				string full = partial.at(0);
				string varName = extractVarFromAssign(full);

				//update for more complex expressions
				string condition = getAssignmentExpression(full);
				expTree *conTree = new expTree;
				buildTree(conTree,&condition);

				string blockName = extractBlockNameFromAssign(full);
				string path = getPath(full);

				obj->addDAUpdated(0,full,varName,blockName,condition,conTree,NULL,path);
			}
		}
		else { //if size is more than one - a more complex statement (IF,LOOP,Par,Proc,Remove Proc, Block)
			if (partial.at(0).find("if") == 0) {
				//conditional statement

				string full = partial.at(0);
				string blockName = extractBlockNameFromIf(partial);

				//update to use the new condition which is a tree
				string condition = getIfCondUpdated(full);
				expTree *conTree = new expTree;
				buildTree(conTree,&condition);
				string varName = "";
				string op = condition;
				int newVal = -1;
				//end of update for new condition

				string conID = getIfID(full);
				vector<string> trueBranch = getCondTrueBranch(partial);
				vector<string> falseBranch = getCondFalseBranch(partial);
				linkedList* tb = new linkedList();
				tb = parseProgram(trueBranch,autoRemoval);

				linkedList* fb = new linkedList();
				fb = parseProgram(falseBranch,autoRemoval);
				string path = getPath(partial.at(partial.size()-1));

				obj->addIFUpdated(2,full,blockName,condition,conTree,tb,fb,NULL,conID,path);
			}
			else if (partial.at(0).find("while") == 0) {
				//while loop
				string full = partial.at(0);
				string blockName = extractBlockNameFromLoop(partial);
				string condition = getLoopCondUpdated(full);

				expTree *conTree = new expTree;
				buildTree(conTree,&condition);

				string whID = getWhileID(full);

				vector<string> loopBody = getLoopBody(partial);
				linkedList* lb = new linkedList();
				lb = parseProgram(loopBody,autoRemoval);

				string path = getPath(partial.at(partial.size()-1));

				obj->addWHUpdated(3,full,blockName,condition,conTree,lb,NULL,whID,path);
			}
			else if (partial.at(0).find("par") == 0) {
				//parallel statement
				vector<string> leftSide = getParLeftSide(partial);
				vector<string> rightSide = getParRightSide(partial);
				linkedList* left = new linkedList();
				left = parseProgram(leftSide,autoRemoval);
				linkedList* right = new linkedList();
				right = parseProgram(rightSide,autoRemoval);
				obj->addPAR(4,left,right,left->nextStatement(),right->nextStatement());
			}
			else if (partial.at(0).find("proc") == 0) {
				//procedure declaration
				string full = partial.at(0);
				string blockName = extractBlockNameFromProcDec(partial);
				string procIden = extractProcIden(full);
				string procName = extractProcNameDec(full);
				vector<string> body = getProcDecBody(partial);

				linkedList* b = new linkedList();
				b = parseProgram(body,autoRemoval);
				string path = getPath(partial.at(partial.size()-1));

				obj->addPD(7,full,blockName,procIden,procName,b,NULL,path);
			}
			else if (partial.at(0).find("remove proc") == 0) {
				//procedure removal
				string full = partial.at(0);
				string blockName = extractBlockNameFromProcRem(partial);
				string procIden = extractProcIdenRem(full);
				string procName = extractProcNameRem(full);
				vector<string> body = getProcDecBody(partial);

				linkedList* b = new linkedList();
				b = parseProgram(body,autoRemoval);
				string path = getPath(partial.at(partial.size()-1));

				obj->addPR(9,full,blockName,procIden,procName,b,NULL,path);
			}
			else if (partial.at(0).find("begin") == 0) {
				//block statement
				string full = partial.at(0);
				string bid = getOwnNameOfBlock(full);

				vector<string> allBody = getEntireBlockBody(partial);

				if (autoRemoval) {
					//now automatically add the removal statements to the end of the body
					vector<string> dv = getBlockDV(partial);
					vector<string> dp = getBlockDP(partial);
					allBody = addRemoval(allBody,dv,dp);
				}

				linkedList *body = new linkedList();
				body = parseProgram(allBody,autoRemoval);

				obj->addBL(5,full,bid,body,NULL);
			}
			else if (partial.at(0).find("atomic") == 0) {
				string uniqueName = "";
				vector<string> atomBody = getAtomicBody(partial);

				linkedList *body = new linkedList();
				body = parseProgram(atomBody,autoRemoval);

				obj->addAtomStatement(13, uniqueName, body, NULL, -1);
			}
			else {
				cout << "error in parseProgram - no matching statement type\n";
				cout << "error in the statement: " << partial.at(0) << "\n";
			}
		}
	}
	return obj;
}

void parser::useLinkFunctions(linkedList *program) {
	linkAll(program);
	statement *temp = program->nextStatement();
	while (temp->next != NULL) {
		temp = temp->next;
	}
	temp->last = true;
}

}
