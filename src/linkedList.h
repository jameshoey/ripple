//============================================================================
// Name        : linkedList.h
// Author      : James Hoey
// Version     :
// Copyright   : Copyright 2018
// Description : Header file for the implementation of the linkedlist
//============================================================================

#ifndef LIST_H_
#define LIST_H_

#include <iostream>
#include <vector>
#include <stack>
#include <stdlib.h>
#include <iterator>
#include <utility>
#include <tr1/unordered_map>
#include "linkedList.h"

namespace std {

class linkedList;

//STRUCTURE FOR READING CONDITIONS AND EXPRESSIONS - TREE

struct node {
	std::string op;
	bool evaluated;
	int type; //0 = arith, 1 = bool, 2 = not yet decided

	//variables to handle the result of intermediate evaluation
	int resultInt;
	bool resultBool;
};

struct expTree {
	node element;
	expTree *leftTree;
	expTree *rightTree;
};

//ORIGINAL PROGRAM STRUCTURES

struct statement { //overall structure used to represent a program - has a type, backup of this type (for when its set to a skip statement) and a pointer to the statement of the sequence
	int type;
	int oldType;
	//type = 0 -> destructive assignment
	//type = 1 -> constructive assignment
	//type = 2 -> conditional assignment
	//type = 3 -> while loop
	//type = 4 -> parallel statement
	//type = 5 -> block statement
	//type = 6 -> local variable declaration
	//type = 7 -> procedure declaration
	//type = 8 -> procedure call
	//type = 9 -> procedure removal
	//type = 10 -> variable removal
	//type = 11 -> skip operation (as a result of execution)
	//type = 12 -> skip operation (placeholder for empty branch)
	//type = 13 -> atomic statement
	//type = 99 -> abort statement

	bool last = false; //used to indicate the last statement - default is false

	statement *next;

	statement* atomic;
};

struct Par : statement {
	statement *currLeft; //pointer to the next statement of the left side of the par
	statement *currRight; //pointer to the next statement of the right side of the par

	linkedList *leftSide; //pointer to the entire left side of the parallel
	linkedList *rightSide; //pointer to the entire right side of the parallel

	bool optionChecked; //purely for use in options2 - whether to enter this parallel statement or check each side (last statement link or beginning)
	bool seenLeft; //whether the left side is now complete
	bool seenRight; //whether the right side is now complete
	//only when both are true, can the execution move onto the next statement

	Par *parentPar; //links to the par statement that this statement occurs in, NULL otherwise
	int left; //1 - left side of parent, 0 - right side of parent, -1 - n/a
};

struct DA : statement {
	std::string full; //full statement as string for display purposes
	std::string varName; //the variable name that will be re-assigned
//	int newVal; //the value that will be assigned
//	std::string newValString;

	std::string newCondition;
	expTree *newCondTree;

	Par *parentPar; //links to the par statement that this statement occurs in, NULL otherwise
	int left; //1 - left side of parent, 0 - right side of parent, -1 - n/a
	std::string blockName;
	stack<int> idendifiers; //stack for identifiers
	statement *original;
	string path;
};

struct CA : statement {
	std::string full; //full statement as string for display purposes
	std::string varName; //the variable name that will be re-assigned
	bool inc; //whether its an increment (true) or decrement (false)
//	int newVal; //the value that will be added/subtracted
//	std::string newValString;

	std::string newCondition;
	expTree *newCondTree;

	Par *parentPar; //links to the par statement that this statement occurs in, NULL otherwise
	int left; //1 - left side of parent, 0 - right side of parent, -1 - n/a
	std::string blockName;
	stack<int> idendifiers; //stack for identifiers
	statement *original;
	string path;
};

struct IfS : statement {
	std::string full; //full statement as string for display purposes
//	std::string varName; //the variable used in the condition
//	std::string op; //the operator used to produce the boolean value
//	int newVal; //the value used in the condition
	std::string blockName;

	expTree *newCondition;
	std::string newConditionString;

	std::string condID;

	stack<int> idendifiers; //stack for identifiers

	statement *original;

	Par* parentPar;
	int left; //1 - for left branch -- 0 - for right branch

	bool seen; //whether this is the first time at the conditional (false), or the second (true)
	int choice; //indicate which branch was followed, and so which value to save (true or false)
	linkedList *trueBranch; //link to the true branch
	linkedList *falseBranch; //link to the false branch

	string path;
};

struct WlS : statement {
	std::string full; //full statement as string for display purposes
//	std::string varName; //the variable used in the condition
//	std::string op; //the operator used to produce the boolean value
//	int newVal; //the value used in the condition
	std::string blockName;

	//updates for new condition
	expTree *newCondition;
	std::string newConditionString;
	//end of update

	std::string WID;

	stack<int> idendifiers; //stack for identifiers

	statement *original;

	Par* parentPar;
	int left; //1 - for left branch -- 0 - for right branch

	linkedList *loopBody; //link to the loop body
	bool saveNext; //boolean value to be saved next into the auxiliary store (boolean sequence)

	string path;

	bool wsixa = false;

	bool alreadyStarted = false;
};

struct BlS : statement {
	std::string full;
	std::string bid;

	linkedList *localVarDec;
	linkedList *localProcDec;
	linkedList *blockBody;

	bool seen;

	stack<int> idendifiers; //stack for identifiers

	statement *original;

	string originalBlockName;

	Par* parentPar;
	int left; //1 - for left branch -- 0 - for right branch
};

struct VdS : statement {
	//typical format -> var X = v (bid)
	std::string full;
	std::string varName;
	int value;
	std::string blockName;

	stack<int> idendifiers; //stack for identifiers

	statement *original;

	Par* parentPar;
	int left; //1 - for left branch -- 0 - for right branch

	string path;
};

struct PdS : statement {
	//typical format -> proc n is P (bid)
	std::string full;
	std::string procName;
	std::string blockName;

	statement *procBody;
	linkedList *procBodyList;
	std::string procIden;
	//std::string formalArg;

	stack<int> idendifiers; //stack for identifiers

	statement *original;

	bool seen;

	Par* parentPar;
	int left; //1 - for left branch -- 0 - for right branch

	string path;
};

struct PcS : statement {
	//typical format -> call n (bid)
	std::string full;
	std::string procName;
	std::string blockName;

	std::string callID;

	stack<int> idendifiers; //stack for identifiers

	statement *original;

	statement* nextLink;
	bool seen;

	Par* parentPar;
	int left; //1 - for left branch -- 0 - for right branch

	string path;
};

struct VrS : statement {
	std::string full;
	std::string varName;
	int value;
	std::string blockName;

	stack<int> idendifiers; //stack for identifiers

	statement *original;

	Par* parentPar;
	int left; //1 - for left branch -- 0 - for right branch

	string path;
};

struct PrS : statement {
	//typical format -> proc n is P (bid)
	std::string full;
	std::string procName;
	std::string blockName;

	statement *procBody;
	linkedList *procBodyList;
	std::string procIden;

	stack<int> idendifiers; //stack for identifiers

	statement *original;

	bool seen;

	Par* parentPar;
	int left; //1 - for left branch -- 0 - for right branch

	string path;
};

struct PH : statement {
	Par* parentPar;
	int left; //1 - for left branch -- 0 - for right branch

};

struct Atom : statement {
	string atomID;

	linkedList *atomBody;

	bool started;

	stack<int> idendifiers; //stack for identifiers

	Par *parentPar;
	int left;
};

struct Abort : statement {
	string failMessage;
	Par* parentPar;
	int left;
};

//END OF ORIGINAL PROGRAM STRUCTURES

//NOW THE LIST CLASS

class linkedList {
	private:
		statement* head;
		statement* tail;

	public:
		//CONSTRUCTOR
		linkedList();

		//GETTERS AND SETTERS
		statement* nextStatement();
		void setStatement(statement* t);
		statement* getTail();
		statement* getHead();

		void setHead(statement* t);
		void setTail(statement* t);

		//modify a program with parent and left value information
		void modifiyList();
		void modifyWithPrem(statement *stopper, Par *parent, int left);

		//functions to add statements to a linkedlist (with default parent and left values)
		void addDA(int t, string full, string varName, string newValStr, int newVal, string blockName, statement *original, string path);
		void addCA(int t, string full, string varName, string newValStr, int newVal, string blockName, bool inc, statement *original, string path);
		void addIF(int t, string full, string blockName, string varName, string op, int newVal, linkedList *tb, linkedList *fb, statement *original, string conID, string path);
		void addWH(int t, string full, string varName, string op, int newVal, string blockName, linkedList *body, statement *original, string whID, string path);
		void addPAR(int t, linkedList *ls, linkedList *rs, statement *nL, statement *nR);
		void addBL(int t, string full, string bid, linkedList *body, statement *original);
		void addPD(int t, string full, string blockName, string procIden, string procName, linkedList *body, statement *original, string path);
		void addPC(int t, string full, string cid, string procName, string blockName,  statement *original, string path);
		void addLVD(int t, string full, string varName, int newVal, string blockName, statement *original, string path);
		void addLVR(int t, string full, string varName, int newVal, string blockName, statement *original, string path);
		void addPR(int t, string full, string blockName, string procIden, string procName, linkedList *body, statement *original, string path);

		//updated for new conditions
		void addDAUpdated(int t, string full, string varName, string blockName, string condition, expTree *condTree, statement *original, string path);
		void addCAUpdated(int t, string full, string varName, string blockName, string condition, expTree *condTree, bool inc, statement *original, string path);
		void addIFUpdated(int t, string full, string blockName, string condition, expTree *condTree, linkedList *tb, linkedList *fb, statement *original, string conID, string path);
		void addWHUpdated(int t, string full, string blockName, string condition, expTree *condTree, linkedList *body, statement *original, string whID, string path);

		void addDAParUpdated(int t, string full, string varName, string blockName, string condition, expTree *condTree, statement *original, string path, Par *parParent, int left, bool last);
		void addCAParUpdated(int t, string full, string varName, string blockName, string condition, expTree *condTree, bool inc, statement *original, string path, Par *parParent, int left, bool last);

		void addAtomStatement(int t, string atomID, linkedList *ab, Par *parParent, int left);

		//functions to add statements to a linkedlist (with provided parent and left values)
//		void addDAPar(int t, string full, string varName, string newValStr, int newVal, string blockName, statement *original, string path, Par *parParent, int left);
//		void addCAPar(int t, string full, string varName, string newValStr, int newVal, string blockName, bool inc, statement *original, string path, Par *parParent, int left);
//		void addIFPar(int t, string full, string blockName, string varName, string op, int newVal, linkedList *tb, linkedList *fb, statement *original, string conID, string path, Par *parParent, int left);
//		void addWHPar(int t, string full, string varName, string op, int newVal, string blockName, linkedList *body, statement *original, string whID, string path, Par *parParent, int left);
		void addLVDPar(int t, string full, string varName, int newVal, string blockName, statement *original, string path, Par *parParent, int left, bool last);
		void addLVRPar(int t, string full, string varName, int newVal, string blockName, statement *original, string path, Par *parParent, int left, bool last);
//		void addBLPar(int t, string full, string bid, linkedList *body, statement *original, Par *parParent, int left);
//		void addPDPar(int t, string full, string blockName, string procIden, string procName, linkedList *body, statement *original, string path, Par *parParent, int left);
//		void addPRPar(int t, string full, string blockName, string procIden, string procName, linkedList *body, statement *original, string path, Par *parParent, int left);
		void addPCPar(int t, string full, string cid, string procName, string blockName,  statement *original, string path, Par *parParent, int left, bool last);

		//add a general statement to the current linkedlist representing a program
		void addStatement(statement *toAdd);

		//display a program with position indication
		void displayWithPosition(statement *stopper, int ind, statement *current);

		//display a program with position indication and line numbers
		void displayWithPositionAndNum(statement *stopper, int ind, statement *current, int *lineNumbers);

		//HELPER
		int size();

		//invert a linkedlist representation of a program
		void invert(linkedList *program);
	};

} /* namespace std */

#endif /* LIST_H_ */
