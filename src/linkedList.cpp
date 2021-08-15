//============================================================================
// Name        : linkedList.cpp
// Author      : James Hoey
// Version     :
// Copyright   : Copyright 2018
// Description : Implementation of the linkedlist
//============================================================================
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

#include <fstream>
#include <iomanip>

namespace std {

//---------------------------------------------Helper functions---------------------------------------------

string removeVersion(string s) { //remove a version number from a given construct name (e.g. b1.1 becomes b1)
	return s.erase(s.find_first_of("."));
}

void showIden(stack<int> s) { //output the elements of a given stack (used to display stack of identifiers for each statement in display)
	while (!s.empty()) {
		if (s.size() == 1) {
			//last element
			cout << s.top();
			s.pop();
		}
		else {
			cout << s.top() << ", ";
			s.pop();
		}
	}
}

void updateAtomicBody(linkedList *program, statement *stopper) {

	cout << "Updating atomic body function called\n";

	statement *temp = new statement;
	temp = program->nextStatement();

	while (temp != NULL) {
		temp->atomic = stopper;


		if (temp->type == 4) {
			cout << "here\n\n\n";
			Par *pro = static_cast<Par*>(temp);
			updateAtomicBody(pro->leftSide,stopper);
			updateAtomicBody(pro->rightSide,stopper);
		}


		temp = temp->next;
	}
}


/*
 * Traverse a given program, updating each individual statement with the given parent and left values. Used to ensure the program is correctly linked
 */
void updateBranch(linkedList* branch, Par* parent, int left) {
	statement *temp = new statement;
	temp = branch->nextStatement();

	while (temp != NULL) {
		if (temp->type == 0) {
			DA *da = static_cast<DA*>(temp);
			da->parentPar = parent;
			da->left = left;
		}
		else if (temp->type == 1) {
			CA *da = static_cast<CA*>(temp);
			da->parentPar = parent;
			da->left = left;
		}
		else if (temp->type == 2) {
			IfS *is = static_cast<IfS*>(temp);
			is->parentPar = parent;
			is->left = left;
			updateBranch(is->trueBranch,parent,left);
			updateBranch(is->falseBranch,parent,left);
		}
		else if (temp->type == 3) {
			WlS *ws = static_cast<WlS*>(temp);
			ws->parentPar = parent;
			ws->left = left;
			updateBranch(ws->loopBody,parent,left);
		}
		else if (temp->type == 4) { //parallel statement
			Par *da = static_cast<Par*>(temp);
			da->parentPar = parent;
			da->left = left;
			updateBranch(da->leftSide,da,1);
			updateBranch(da->rightSide,da,0);
		}
		else if (temp->type == 5) {
			BlS *bls = static_cast<BlS*>(temp);
			bls->parentPar = parent;
			bls->left = left;
			updateBranch(bls->blockBody,parent,left);
		}
		else if (temp->type == 6) {
			VdS *vds = static_cast<VdS*>(temp);
			vds->parentPar = parent;
			vds->left = left;
		}
		else if (temp->type == 7) {
			PdS *pds = static_cast<PdS*>(temp);
			pds->parentPar = parent;
			pds->left = left;
			cout << "start\n";
			updateBranch(pds->procBodyList,parent,left);
			cout << "finish\n";
		}
		else if (temp->type == 8) {
			PcS *pcs = static_cast<PcS*>(temp);
			pcs->parentPar = parent;
			pcs->left = left;
		}
		else if (temp->type == 9) {
			PrS *prs = static_cast<PrS*>(temp);
			prs->parentPar = parent;
			prs->left = left;
			updateBranch(prs->procBodyList,parent,left);
		}
		else if (temp->type == 10) {
			VrS *vrs = static_cast<VrS*>(temp);
			vrs->parentPar = parent;
			vrs->left = left;
		}
		else if (temp->type == 11) {
			//error here - type 11 means a skip operation that has not been removed - so execution not complete
			cout << "Error in updateBranch - type of statement = 11\n";
		}
		else if (temp->type == 12) {
			PH *ph = static_cast<PH*>(temp);
			ph->parentPar = parent;
			ph->left = left;
		}
		else if (temp->type == 13) {
			Atom *at = static_cast<Atom*>(temp);
			at->parentPar = parent;
			at->left = left;
			updateAtomicBody(at->atomBody,at);
			updateBranch(at->atomBody,parent,left);
		}
		else if (temp->type == 99) {
			Abort *ph = static_cast<Abort*>(temp);
			ph->parentPar = parent;
			ph->left = left;
		}

		temp = temp->next;
	}
}

//---------------------------------------------End of helper functions--------------------------------------

//CONSTRUCTOR
linkedList :: linkedList() {
	head = NULL;
	tail = NULL;
}

//GETTERS AND SETTERS
statement* linkedList :: nextStatement() {
	return head;
}

void linkedList :: setStatement(statement* t) {
	head = t;
}

statement* linkedList :: getTail() {
	return tail;
}

statement* linkedList :: getHead() {
	return head;
}

void linkedList :: setTail(statement* t) {
	tail = t;
}

void linkedList :: setHead(statement* t) {
	head = t;
}

/*
 * Functions below are for adding statements into the given linked list - parallel and left variables are set to default values (NULL and -1 respectively)
 */

void linkedList::addDA(int t, string full, string varName, string newValStr, int newVal, string blockName, statement *original, string path) {
	DA *temp = new DA;
	temp->type = t;
	temp->oldType = t;
	temp->full = full;
	temp->varName = varName;
//	temp->newVal = newVal;
//	temp->newValString = newValStr;
	temp->blockName = blockName;
	temp->path = path;

	stack<int> sta;
	temp->idendifiers = sta;

	temp->original = original;

	//set parent info to NULL at this point, will be updated by the modifiy list function
	temp->parentPar = NULL;
	temp->left = -1;

	temp->next = NULL;

	temp->atomic = NULL;

	if(head==NULL) {
		head=temp;
		tail=temp;
		temp=NULL;
	}
	else {
		tail->next=temp;
		tail=temp;
	}
}

void linkedList::addDAUpdated(int t, string full, string varName, string blockName, string condition, expTree *conTree, statement *original, string path) {
	DA *temp = new DA;
	temp->type = t;
	temp->oldType = t;
	temp->full = full;

	//updates
	temp->newCondition = condition;
	temp->newCondTree = conTree;
	temp->varName = varName;
//	temp->newVal = 0;
//	temp->newValString = "";


//	temp->varName = varName;
//	temp->newVal = newVal;
//	temp->newValString = newValStr;
	temp->blockName = blockName;
	temp->path = path;

	stack<int> sta;
	temp->idendifiers = sta;

	temp->original = original;

	//set parent info to NULL at this point, will be updated by the modifiy list function
	temp->parentPar = NULL;
	temp->left = -1;

	temp->next = NULL;

	temp->atomic = NULL;

	if(head==NULL) {
		head=temp;
		tail=temp;
		temp=NULL;
	}
	else {
		tail->next=temp;
		tail=temp;
	}
}

void linkedList::addCA(int t, string full, string varName, string newValStr, int newVal, string blockName, bool inc, statement *original, string path) {
	CA *temp = new CA;
	temp->type = t;
	temp->oldType = t;
	temp->full = full;
	temp->varName = varName;
//	temp->newVal = newVal;
//	temp->newValString = newValStr;
	temp->blockName = blockName;
	temp->inc = inc;
	temp->original = original;
	temp->path = path;

	//set parent info to NULL at this point, will be updated by the modifiy list function
	temp->parentPar = NULL;
	temp->left = -1;

	stack<int> sta;
	temp->idendifiers = sta;

	temp->next = NULL;

	temp->atomic = NULL;

	if(head==NULL) {
		head=temp;
		tail=temp;
		temp=NULL;
	}
	else {
		tail->next=temp;
		tail=temp;
	}
}

void linkedList::addCAUpdated(int t, string full, string varName, string blockName, string newCondition, expTree *newCondTree, bool inc, statement *original, string path) {
	CA *temp = new CA;
	temp->type = t;
	temp->oldType = t;
	temp->full = full;

	//updates
	temp->newCondition = newCondition;
	temp->newCondTree = newCondTree;
	temp->varName = varName;
//	temp->newVal = 0;
//	temp->newValString = "";


//	temp->varName = varName;
//	temp->newVal = newVal;
//	temp->newValString = newValStr;
	temp->blockName = blockName;
	temp->inc = inc;
	temp->original = original;
	temp->path = path;

	//set parent info to NULL at this point, will be updated by the modifiy list function
	temp->parentPar = NULL;
	temp->left = -1;

	stack<int> sta;
	temp->idendifiers = sta;

	temp->next = NULL;

	temp->atomic = NULL;

	if(head==NULL) {
		head=temp;
		tail=temp;
		temp=NULL;
	}
	else {
		tail->next=temp;
		tail=temp;
	}
}


void linkedList::addIF(int t, string full, string blockName, string varName, string op, int newVal, linkedList *tb, linkedList *fb, statement *original, string conID, string path) {
	IfS *temp = new IfS;
	temp->type = t;
	temp->oldType = t;
	temp->full = full;
//	temp->varName = varName;
	temp->blockName = blockName;
//	temp->op = op;
//	temp->newVal = newVal;
	temp->trueBranch = tb;
	temp->falseBranch = fb;
	temp->path = path;

	stack<int> sta;
	temp->idendifiers = sta;

	temp->original = original;
	temp->condID = conID;



	//set parent info to NULL at this point, will be updated by the modifiy list function
	temp->parentPar = NULL;
	temp->left = -1;

	//initialise the seen variable
	temp->seen = false; //first time viewing this will be false
	temp->choice = -1;

	temp->next = NULL;

	temp->atomic = NULL;

	if(head==NULL) {
		head=temp;
		tail=temp;
		temp=NULL;
	}
	else
	{
		tail->next=temp;
		tail=temp;
	}
}

//=======================================UPDATED ADDIF FOR NEW CONDITION

void linkedList::addIFUpdated(int t, string full, string blockName, string condition, expTree *condTree, linkedList *tb, linkedList *fb, statement *original, string conID, string path) {
	IfS *temp = new IfS;
	temp->type = t;
	temp->oldType = t;
	temp->full = full;

	temp->newConditionString = condition;
	temp->newCondition = condTree;

//	temp->varName = varName;
	temp->blockName = blockName;
//	temp->op = op;
//	temp->newVal = newVal;
	temp->trueBranch = tb;
	temp->falseBranch = fb;
	temp->path = path;

	stack<int> sta;
	temp->idendifiers = sta;

	temp->original = original;
	temp->condID = conID;



	//set parent info to NULL at this point, will be updated by the modifiy list function
	temp->parentPar = NULL;
	temp->left = -1;

	//initialise the seen variable
	temp->seen = false; //first time viewing this will be false
	temp->choice = -1;

	temp->next = NULL;

	temp->atomic = NULL;

	if(head==NULL) {
		head=temp;
		tail=temp;
		temp=NULL;
	}
	else
	{
		tail->next=temp;
		tail=temp;
	}
}

//=======================================END

void linkedList::addWH(int t, string full, string varName, string op, int newVal, string blockName, linkedList *body, statement *original, string whID, string path) {
	WlS *temp = new WlS();
	temp->type = t;
	temp->oldType = t;
	temp->full = full;
//	temp->varName = varName;
//	temp->op = op;
//	temp->newVal = newVal;
	temp->blockName = blockName;
	temp->loopBody = body;
	temp->path = path;

	stack<int> sta;
	temp->idendifiers = sta;

	temp->original = original;
	temp->WID = whID;

	//set parent info to NULL at this point, will be updated by the modifiy list function
	temp->parentPar = NULL;
	temp->left = -1;

	//initialise the save next
	temp->saveNext = false;

	temp->next = NULL;

	temp->atomic = NULL;

	if(head==NULL) {
		head=temp;
		tail=temp;
		temp=NULL;
	}
	else {
		tail->next=temp;
		tail=temp;
	}

}

void linkedList::addWHUpdated(int t, string full, string blockName, string condition, expTree *condTree, linkedList *body, statement *original, string whID, string path) {
	WlS *temp = new WlS();
	temp->type = t;
	temp->oldType = t;
	temp->full = full;
//	temp->varName = "";
//	temp->op = "";
//	temp->newVal = -1;
	temp->blockName = blockName;

	temp->newConditionString = condition;
	temp->newCondition = condTree;

	temp->loopBody = body;
	temp->path = path;

	stack<int> sta;
	temp->idendifiers = sta;

	temp->original = original;
	temp->WID = whID;

	//set parent info to NULL at this point, will be updated by the modifiy list function
	temp->parentPar = NULL;
	temp->left = -1;

	//initialise the save next
	temp->saveNext = false;

	temp->next = NULL;

	temp->atomic = NULL;

	if(head==NULL) {
		head=temp;
		tail=temp;
		temp=NULL;
	}
	else {
		tail->next=temp;
		tail=temp;
	}

}

/*

void linkedList::addLinkedWH(int t, WlS *temp, string full, string varName, string op, int newVal, string blockName, linkedList *body, statement *original, string whID, string path) {
	//WlS *temp = new WlS();
	temp->type = t;
	temp->oldType = t;
	temp->full = full;
	temp->varName = varName;
	temp->op = op;
	temp->newVal = newVal;
	temp->blockName = blockName;

	temp->loopBody = body;

	temp->path = path;

	stack<int> sta;
	temp->idendifiers = sta;

	temp->original = original;
	temp->WID = whID;

	//set parent info to NULL at this point, will be updated by the modifiy list function
	temp->parentPar = NULL;
	temp->left = -1;

	//initialise the save next
	temp->saveNext = false;

	temp->next = NULL;

	if(head==NULL) {
		head=temp;
		tail=temp;
		temp=NULL;
	}
	else {
		tail->next=temp;
		tail=temp;
	}

}
*/

void linkedList::addBL(int t, string full, string bid, linkedList *bp, statement *original) {
	BlS *temp = new BlS;
	temp->type = t;
	temp->oldType = t;
	temp->bid = bid;
	temp->full = full;
	temp->blockBody = bp;
	temp->seen = false;

	stack<int> sta;
	temp->idendifiers = sta;

	temp->original = original;
	temp->originalBlockName = removeVersion(bid);

	//set parent info to NULL at this point, will be updated by the modifiy list function
	temp->parentPar = NULL;
	temp->left = -1;

	temp->next = NULL;

	temp->atomic = NULL;

	if(head==NULL) {
		head=temp;
		tail=temp;
		temp=NULL;
	}
	else {
		tail->next=temp;
		tail=temp;
	}
}

void linkedList::addPAR(int t, linkedList *ls, linkedList *rs, statement *nL, statement *nR) {
	Par *temp = new Par;
	temp->type = t;
	temp->oldType = t;

	//set parent info to NULL at this point, will be updated by the modifiy list function
	temp->parentPar = NULL;
	temp->left = -1;

	temp->leftSide = ls;
	temp->rightSide = rs;

	//link the current left and current right
	temp->currLeft = ls->nextStatement();
	temp->currRight = rs->nextStatement();

	//initialise the seen variables
	temp->optionChecked = false;
	temp->seenLeft = false;
	temp->seenRight = false;

	temp->next = NULL;

	temp->atomic = NULL;

	if(head==NULL) {
		head=temp;
		tail=temp;
		temp=NULL;
	}
	else {
		tail->next=temp;
		tail=temp;
	}
}

void linkedList::addPD(int t, string full, string blockName, string procIden, string procName, linkedList *body, statement *original, string path) {
	PdS *temp = new PdS;
	temp->type = t;
	temp->oldType = t;
	temp->full = full;
	temp->blockName = blockName;
	temp->procIden = procIden;
	temp->procName = procName;
	temp->procBodyList = body;
	temp->procBody = body->nextStatement();
	temp->seen = false;
	temp->path = path;

	stack<int> sta;
	temp->idendifiers = sta;

	temp->original = original;

	//set parent info to NULL at this point, will be updated by the modify list function
	temp->parentPar = NULL;
	temp->left = -1;

	temp->next = NULL;

	temp->atomic = NULL;

	if(head==NULL) {
		head=temp;
		tail=temp;
		temp=NULL;
	}
	else {
		tail->next=temp;
		tail=temp;
	}
}

void linkedList::addLVD(int t, string full, string varName, int newVal, string blockName, statement *original, string path) {
	VdS *temp = new VdS;
	temp->type = t;
	temp->oldType = t;
	temp->full = full;
	temp->varName = varName;
	temp->value = newVal;
	temp->blockName = blockName;
	temp->path = path;

	stack<int> sta;
	temp->idendifiers = sta;
	temp->original = original;

	//set parent info to NULL at this point, will be updated by the modifiy list function
	temp->parentPar = NULL;
	temp->left = -1;

	temp->next = NULL;

	temp->atomic = NULL;

	if(head==NULL) {
		head=temp;
		tail=temp;
		temp=NULL;
	}
	else {
		tail->next=temp;
		tail=temp;
	}
}

void linkedList::addPC(int t, string full, string cid, string procName, string blockName, statement *original, string path) {
	PcS *temp = new PcS;
	temp->type = t;
	temp->oldType = t;
	temp->full = full;
	temp->callID = cid;
	temp->procName = procName;
	temp->blockName = blockName;
	temp->seen = false;
	temp->path = path;

	stack<int> sta;
	temp->idendifiers = sta;

	temp->original = original;

	//set parent info to NULL at this point, will be updated by the modifiy list function
	temp->parentPar = NULL;
	temp->left = -1;

	temp->next = NULL;

	temp->atomic = NULL;

	if(head==NULL) {
		head=temp;
		tail=temp;
		temp=NULL;
	}
	else {
		tail->next=temp;
		tail=temp;
	}
}

void linkedList::addLVR(int t, string full, string varName, int newVal, string blockName, statement *original, string path) {
	VrS *temp = new VrS;
	temp->type = t;
	temp->oldType = t;
	temp->full = full;
	temp->varName = varName;
	temp->value = newVal;
	temp->blockName = blockName;
	temp->path = path;

	stack<int> sta;
	temp->idendifiers = sta;
	temp->original = original;

	//set parent info to NULL at this point, will be updated by the modifiy list function
	temp->parentPar = NULL;
	temp->left = -1;

	temp->next = NULL;

	temp->atomic = NULL;

	if(head==NULL) {
		head=temp;
		tail=temp;
		temp=NULL;
	}
	else {
		tail->next=temp;
		tail=temp;
	}
}

void linkedList::addPR(int t, string full, string blockName, string procIden, string procName, linkedList *body, statement *original, string path) {
	PrS *temp = new PrS;
	temp->type = t;
	temp->oldType = t;
	temp->full = full;
	temp->blockName = blockName;
	temp->procIden = procIden;
	temp->procName = procName;
	temp->procBodyList = body;
	temp->procBody = body->nextStatement();
	temp->seen = false;
	temp->path = path;

	stack<int> sta;
	temp->idendifiers = sta;
	temp->original = original;

	//set parent info to NULL at this point, will be updated by the modify list function
	temp->parentPar = NULL;
	temp->left = -1;

	temp->next = NULL;

	temp->atomic = NULL;

	if(head==NULL) {
		head=temp;
		tail=temp;
		temp=NULL;
	}
	else {
		tail->next=temp;
		tail=temp;
	}
}

//END OF SECTION

/*
 * Functions below are for adding statements to the given linked list - but sets correct parallel and left values (keeping the linking as required)
 */

/*void linkedList::addDAPar(int t, string full, string varName, string newValStr, int newVal, string blockName, statement *original, string path, Par *parParent, int left) {
	DA *temp = new DA;
	temp->type = t;
	temp->oldType = t;
	temp->full = full;
	temp->varName = varName;
//	temp->newVal = newVal;
//	temp->newValString = newValStr;
	temp->blockName = blockName;
	temp->path = path;

	stack<int> sta;
	temp->idendifiers = sta;

	temp->original = original;

	//set parent info to NULL at this point, will be updated by the modifiy list function
	temp->parentPar = parParent;
	temp->left = left;

	temp->next = NULL;

	if(head==NULL) {
		head=temp;
		tail=temp;
		temp=NULL;
	}
	else {
		tail->next=temp;
		tail=temp;
	}
}*/

void linkedList::addDAParUpdated(int t, string full, string varName, string blockName, string condition, expTree *conTree, statement *original, string path, Par *parParent, int left, bool last) {
	DA *temp = new DA;
	temp->type = t;
	temp->oldType = t;
	temp->full = full;
	temp->varName = varName;

	//updates for new conditions
	temp->newCondition = condition;
	temp->newCondTree = conTree;
//	temp->newVal = 0;
//	temp->newValString = "";

//	temp->newVal = newVal;
//	temp->newValString = newValStr;
	temp->blockName = blockName;
	temp->path = path;

	stack<int> sta;
	temp->idendifiers = sta;

	temp->original = original;

	//set parent info to NULL at this point, will be updated by the modifiy list function
	temp->parentPar = parParent;
	temp->left = left;

	temp->next = NULL;

	temp->atomic = NULL;

	temp->last = last;

	if(head==NULL) {
		head=temp;
		tail=temp;
		temp=NULL;
	}
	else {
		tail->next=temp;
		tail=temp;
	}
}

/*void linkedList::addCAPar(int t, string full, string varName, string newValStr, int newVal, string blockName, bool inc, statement *original, string path, Par *parParent, int left) {
	CA *temp = new CA;
	temp->type = t;
	temp->oldType = t;
	temp->full = full;
	temp->varName = varName;
//	temp->newVal = newVal;
//	temp->newValString = newValStr;
	temp->blockName = blockName;
	temp->inc = inc;
	temp->original = original;
	temp->path = path;

	//set parent info to NULL at this point, will be updated by the modifiy list function
	temp->parentPar = parParent;
	temp->left = left;

	stack<int> sta;
	temp->idendifiers = sta;

	temp->next = NULL;

	if(head==NULL) {
		head=temp;
		tail=temp;
		temp=NULL;
	}
	else {
		tail->next=temp;
		tail=temp;
	}
}*/

void linkedList::addCAParUpdated(int t, string full, string varName, string blockName, string condition, expTree *conTree, bool inc, statement *original, string path, Par *parParent, int left, bool last) {
	CA *temp = new CA;
	temp->type = t;
	temp->oldType = t;
	temp->full = full;
	temp->varName = varName;

	//updates for new conditions
	temp->newCondition = condition;
	temp->newCondTree = conTree;
//	temp->newVal = 0;
//	temp->newValString = "";

	//temp->newVal = newVal;
	//temp->newValString = newValStr;
	temp->blockName = blockName;
	temp->inc = inc;
	temp->original = original;
	temp->path = path;

	//set parent info to NULL at this point, will be updated by the modifiy list function
	temp->parentPar = parParent;
	temp->left = left;

	stack<int> sta;
	temp->idendifiers = sta;

	temp->next = NULL;

	temp->atomic = NULL;

	temp->last = last;

	if(head==NULL) {
		head=temp;
		tail=temp;
		temp=NULL;
	}
	else {
		tail->next=temp;
		tail=temp;
	}
}

/*
void linkedList::addIFPar(int t, string full, string blockName, string varName, string op, int newVal, linkedList *tb, linkedList *fb, statement *original, string conID, string path, Par *parParent, int left) {
	IfS *temp = new IfS;
	temp->type = t;
	temp->oldType = t;
	temp->full = full;
//	temp->varName = varName;
	temp->blockName = blockName;
//	temp->op = op;
//	temp->newVal = newVal;
	temp->trueBranch = tb;
	temp->falseBranch = fb;
	temp->path = path;

	stack<int> sta;
	temp->idendifiers = sta;

	temp->original = original;
	temp->condID = conID;

	//set parent info to NULL at this point, will be updated by the modifiy list function
	temp->parentPar = parParent;
	temp->left = left;

	//initialise the seen variable
	temp->seen = false; //first time viewing this will be false
	temp->choice = -1;

	temp->next = NULL;

	if(head==NULL) {
		head=temp;
		tail=temp;
		temp=NULL;
	}
	else
	{
		tail->next=temp;
		tail=temp;
	}
}

void linkedList::addWHPar(int t, string full, string varName, string op, int newVal, string blockName, linkedList *body, statement *original, string whID, string path, Par *parParent, int left) {
	WlS *temp = new WlS();
	temp->type = t;
	temp->oldType = t;
	temp->full = full;
//	temp->varName = varName;
//	temp->op = op;
//	temp->newVal = newVal;
	temp->blockName = blockName;

	temp->loopBody = body;

	temp->path = path;

	stack<int> sta;
	temp->idendifiers = sta;

	temp->original = original;
	temp->WID = whID;

	//set parent info to NULL at this point, will be updated by the modifiy list function
	temp->parentPar = parParent;
	temp->left = left;

	//initialise the save next
	temp->saveNext = false;

	temp->next = NULL;

	if(head==NULL) {
		head=temp;
		tail=temp;
		temp=NULL;
	}
	else {
		tail->next=temp;
		tail=temp;
	}
}
*/
void linkedList::addLVDPar(int t, string full, string varName, int newVal, string blockName, statement *original, string path, Par *parent, int left, bool last) {
	VdS *temp = new VdS;
	temp->type = t;
	temp->oldType = t;
	temp->full = full;
	temp->varName = varName;
	temp->value = newVal;
	temp->blockName = blockName;
	temp->path = path;

	stack<int> sta;
	temp->idendifiers = sta;

	temp->original = original;

	//set parent info to NULL at this point, will be updated by the modifiy list function
	temp->parentPar = parent;
	temp->left = left;

	temp->next = NULL;

	temp->atomic = NULL;

	temp->last = last;

	if(head==NULL) {
		head=temp;
		tail=temp;
		temp=NULL;
	}
	else {
		tail->next=temp;
		tail=temp;
	}
}

void linkedList::addLVRPar(int t, string full, string varName, int newVal, string blockName, statement *original, string path, Par *parent, int left, bool last) {
	VrS *temp = new VrS;
	temp->type = t;
	temp->oldType = t;
	temp->full = full;
	temp->varName = varName;
	temp->value = newVal;
	temp->blockName = blockName;
	temp->path = path;

	stack<int> sta;
	temp->idendifiers = sta;

	temp->original = original;

	//set parent info to NULL at this point, will be updated by the modifiy list function
	temp->parentPar = parent;
	temp->left = left;

	temp->next = NULL;

	temp->atomic = NULL;

	temp->last = last;

	if(head==NULL) {
		head=temp;
		tail=temp;
		temp=NULL;
	}
	else {
		tail->next=temp;
		tail=temp;
	}
}
/*
void linkedList::addBLPar(int t, string full, string bid, linkedList *bp, statement *original, Par *parParent, int left) {
	BlS *temp = new BlS;
	temp->type = t;
	temp->oldType = t;
	temp->bid = bid;
	temp->full = full;
	temp->blockBody = bp;
	temp->seen = false;

	stack<int> sta;
	temp->idendifiers = sta;
	temp->original = original;
	temp->originalBlockName = removeVersion(bid);

	//set parent info to NULL at this point, will be updated by the modifiy list function
	temp->parentPar = parParent;
	temp->left = left;

	temp->next = NULL;

	if(head==NULL) {
		head=temp;
		tail=temp;
		temp=NULL;
	}
	else {
		tail->next=temp;
		tail=temp;
	}
}

void linkedList::addPDPar(int t, string full, string blockName, string procIden, string procName, linkedList *body, statement *original, string path, Par *parParent, int left) {
	PdS *temp = new PdS;
	temp->type = t;
	temp->oldType = t;
	temp->full = full;
	temp->blockName = blockName;
	temp->procIden = procIden;
	temp->procName = procName;
	temp->procBodyList = body;
	temp->procBody = body->nextStatement();
	temp->seen = false;
	temp->path = path;

	stack<int> sta;
	temp->idendifiers = sta;
	temp->original = original;

	//set parent info to NULL at this point, will be updated by the modify list function
	temp->parentPar = parParent;
	temp->left = left;

	temp->next = NULL;

	if(head==NULL) {
		head=temp;
		tail=temp;
		temp=NULL;
	}
	else {
		tail->next=temp;
		tail=temp;
	}
}

void linkedList::addPRPar(int t, string full, string blockName, string procIden, string procName, linkedList *body, statement *original, string path, Par *parParent, int left) {
	PrS *temp = new PrS;
	temp->type = t;
	temp->oldType = t;
	temp->full = full;
	temp->blockName = blockName;
	temp->procIden = procIden;
	temp->procName = procName;
	temp->procBodyList = body;
	temp->procBody = body->nextStatement();
	temp->seen = false;
	temp->path = path;

	stack<int> sta;
	temp->idendifiers = sta;

	temp->original = original;

	//set parent info to NULL at this point, will be updated by the modify list function
	temp->parentPar = parParent;
	temp->left = left;

	temp->next = NULL;

	if(head==NULL) {
		head=temp;
		tail=temp;
		temp=NULL;
	}
	else {
		tail->next=temp;
		tail=temp;
	}
}
*/
void linkedList::addPCPar(int t, string full, string cid, string procName, string blockName, statement *original, string path, Par *parParent, int left, bool last) {
	PcS *temp = new PcS;
	temp->type = t;
	temp->oldType = t;
	temp->full = full;
	temp->callID = cid;
	temp->procName = procName;
	temp->blockName = blockName;
	temp->seen = false;
	temp->path = path;

	stack<int> sta;
	temp->idendifiers = sta;
	temp->original = original;

	//set parent info to NULL at this point, will be updated by the modifiy list function
	temp->parentPar = parParent;
	temp->left = left;

	temp->next = NULL;

	temp->atomic = NULL;

	temp->last = last;

	if(head==NULL) {
		head=temp;
		tail=temp;
		temp=NULL;
	}
	else {
		tail->next=temp;
		tail=temp;
	}
}

void linkedList::addAtomStatement(int t, string atomID, linkedList *ab, Par *parParent, int left) {
	Atom *at = new Atom;
	at->type = 13;
	at->oldType = 13;

	at->atomID = atomID;
	at->atomBody = ab;

	at->parentPar = parParent;
	at->left = left;

	at->started = false;

	stack<int> sta;
	at->idendifiers = sta;

	at->next = NULL;

	at->atomic = NULL;

	if(head==NULL) {
		head=at;
		tail=at;
		at=NULL;
	}
	else {
		tail->next=at;
		tail=at;
	}

}

void linkedList::addStatement(statement* temp) {
	if(head==NULL) {
		head=temp;
		tail=temp;
		temp=NULL;
	}
	else {
		tail->next=temp;
		tail=temp;
	}
}

//DISPLAY FUNCTIONS

void indent(int number) {
	for (int i = 0; i < number; i++) {
		cout << "  ";
	}
}

string intToStringInList(int i) {
	stringstream ss;
	ss << i;
	string str = ss.str();
	return str;
}

/*
 * Given statement is a skip action (type 11). Using the old type (backup of original type), we can determine what type of statement it really is.
 * Therefore output only the list of identifiers in this statement's stack (making it easier to choose a skip operation and to see it has been executed
 */
void displayStatementIdentifiers(statement *temp) {
	if (temp->oldType == 0) {
		DA *i = static_cast<DA*>(temp);
		cout << " [";
		showIden(i->idendifiers);
		cout << "]";
	}
	else if (temp->oldType == 1) {
		CA *i = static_cast<CA*>(temp);
		cout << " [";
		showIden(i->idendifiers);
		cout << "]";
	}
	else if (temp->oldType == 2) {
		//If statement
		IfS *i = static_cast<IfS*>(temp);
		cout << " [";
		showIden(i->idendifiers);
		cout << "]";
	}
	else if (temp->oldType == 3) {
		WlS *i = static_cast<WlS*>(temp);
		cout << " [";
		showIden(i->idendifiers);
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
		showIden(i->idendifiers);
		cout << "]";
	}
	else if (temp->oldType == 7) {
		PdS *i = static_cast<PdS*>(temp);
		cout << " [";
		showIden(i->idendifiers);
		cout << "]";
	}
	else if (temp->oldType == 8) {
		PcS *i = static_cast<PcS*>(temp);
		cout << " [";
		showIden(i->idendifiers);
		cout << "]";
	}
	else if (temp->oldType == 9) {
		PrS *i = static_cast<PrS*>(temp);
		cout << " [";
		showIden(i->idendifiers);
		cout << "]";
	}
	else if (temp->oldType == 10) {
		VrS *i = static_cast<VrS*>(temp);
		cout << " [";
		showIden(i->idendifiers);
		cout << "]";
	}
	else if (temp->oldType == 11) {
		//skip statement cannot be the original type of statement
		//this will only be reached in the case of a place holder
	}
}

/*
 * Function used to display a program with position highlighted via the position indication arrow. Program correctly indented when displayed.
 * Other display functions defined in simulator.cpp - used when we only have a pointer to the first statement and not the whole linkedList
 */
void linkedList::displayWithPosition(statement *stopper, int ind, statement *current) {
	statement *temp = new statement;
	temp = head;

	bool foundCurrent = false;

	while (temp != stopper) {
		//determine if this statement is the current
		if (temp == current) {
			foundCurrent = true;
		}

		//now display the correct identation
		indent(ind);

		if (temp->type == 0) {
			DA *i = static_cast<DA*>(temp);

			cout << i->varName << " = " << i->newCondition;
			/*if (i->newVal != 0) { cout << i->newVal; }
			else {
				if (intToStringInList(i->newVal) != i->newValString) { cout << i->newValString; }
				else { cout << i->newVal; }
			}*/
			cout << " (" << i->path << ") [";
			showIden(i->idendifiers);
			cout << "]";
			if (foundCurrent) {
				cout << "   <------- ";
				foundCurrent = false;
			}
			cout << "\n";
			temp=temp->next;
		}
		else if (temp->type == 1) {
			CA *i = static_cast<CA*>(temp);
			cout << i->varName;
			if (i->inc) { cout << " += "; }
			else { cout << " -= "; }

			/*if (i->newVal != 0) { cout << i->newVal; }
			else {
				if (intToStringInList(i->newVal) != i->newValString) { cout << i->newValString; }
				else { cout << i->newVal; }
			}*/

			cout << i->newCondition << " (" << i->path << ") [";
			showIden(i->idendifiers);
			cout << "]";
			if (foundCurrent) {
				cout << "   <------- ";
				foundCurrent = false;
			}
			cout << "\n";
			temp=temp->next;
		}
		else if (temp->type == 2) {
			//If statement
			IfS *i = static_cast<IfS*>(temp);
			//cout << "if " << i->condID << " (" << i->varName << " " << i->op << " " << i->newVal << ") then ";
			cout << "if " << i->condID << i->newConditionString << " then ";

			if ((current == temp) && (!i->seen)) {
				cout << "  <------- ";
			}
			cout << "\n";
			i->trueBranch->displayWithPosition(temp,ind+1,current);
			indent(ind);
			cout  << "else\n";
			i->falseBranch->displayWithPosition(temp,ind+1,current);
			indent(ind);
			cout  << "fi (" << i->path << ") [";
			showIden(i->idendifiers);
			cout << "]";
			if ((current == temp) && (i->seen)) {
				cout << "  <------- ";
			}
			cout << "\n";
			temp=temp->next;
		}
		else if (temp->type == 3) {
			//while loop
			WlS *i = static_cast<WlS*>(temp);
		//	cout << "while " << i->WID << " (" << i->varName << " " << i->op << " " << i->newVal << ")" << " do";
			cout << "while " << i->WID << i->newConditionString << " do";
			if (foundCurrent) {
				cout << "  <----- ";
				foundCurrent = false;
			}
			cout << "\n";

			i->loopBody->displayWithPosition(temp,ind+1,current);
			indent(ind);
			cout << "elihw (" << i->path << ") [";
			showIden(i->idendifiers);
			cout << "]\n";
			temp=temp->next;
		}
		else if (temp->type == 4) {
			//par statement
			Par *i = static_cast<Par*>(temp);
			cout << "par {\n";
			if (foundCurrent) {
				i->leftSide->displayWithPosition(temp,ind+1,i->currLeft);
			}
			else {
				i->leftSide->displayWithPosition(temp,ind+1,current);
			}
			indent(ind);
			cout << "}\n";
			indent(ind);
			cout << "{\n";
			if (foundCurrent) {
				i->rightSide->displayWithPosition(temp,ind+1,i->currRight);
			}
			else {
				i->rightSide->displayWithPosition(temp,ind+1,current);
			}
			indent(ind);
			cout << "}";
			if ((foundCurrent) && ((i->currLeft == i) || (i->currRight == i))) {
				cout << "  <----- ";
				foundCurrent = false;
			}
			else if (foundCurrent) {
				foundCurrent = false;
			}
			cout << "\n";

			temp = temp->next;
		}
		else if (temp->type == 5) {
			BlS *i = static_cast<BlS*>(temp);
			cout << "begin " << i->bid << " ";
			if (foundCurrent && (!i->seen)) {
				cout << "  <----- ";
				foundCurrent = false;
			}
			cout << "\n";
			i->blockBody->displayWithPosition(temp,ind+1,current);
			indent(ind);
			cout << "end [";
			showIden(i->idendifiers);
			cout << "]";
			if (foundCurrent && (i->seen)) {
				cout << "  <----- ";
				foundCurrent = false;
			}
			cout << "\n";
			temp=temp->next;
		}
		else if (temp->type == 6) {
			VdS *i = static_cast<VdS*>(temp);
			cout << "var " << i->varName << " = " << i->value << " (" << i->path << ") [";
			showIden(i->idendifiers);
			cout << "]";
			if (foundCurrent) {
				cout << "  <----- ";
				foundCurrent = false;
			}
			cout << "\n";
			temp=temp->next;
		}
		else if (temp->type == 7) {
			PdS *i = static_cast<PdS*>(temp);
			cout << "proc " << i->procIden << " " << i->procName << " is\n";
			i->procBodyList->displayWithPosition(NULL,ind+1,current);
			indent(ind);
			cout << "corp (" << i->path << ") [";
			showIden(i->idendifiers);
			cout << "]";
			if (foundCurrent) {
				cout << "  <----- ";
				foundCurrent = false;
			}
			cout << "\n";
			temp=temp->next;
		}
		else if (temp->type == 8) {
			PcS *i = static_cast<PcS*>(temp);
			cout << "call " << i->callID << " " << i->procName << "(" << i->path << ") [";
			showIden(i->idendifiers);
			cout << "]";
			if (foundCurrent) {
				cout << "  <----- ";
				foundCurrent = false;
			}
			cout << "\n";
			temp=temp->next;
		}
		else if (temp->type == 9) {
			PrS *i = static_cast<PrS*>(temp);
			cout << "remove proc " << i->procIden << " " << i->procName << " is\n";

			i->procBodyList->displayWithPosition(NULL,ind+1,current);

			indent(ind);
			cout << "corp (" << i->path << ") [";
			showIden(i->idendifiers);
			cout << "]";
			if (foundCurrent) {
				cout << "  <----- ";
				foundCurrent = false;
			}
			cout << "\n";
			temp=temp->next;
		}
		else if (temp->type == 10) {
			VrS *i = static_cast<VrS*>(temp);
			cout << "remove var " << i->varName << " = " << i->value << " (" << i->path << ") [";
			showIden(i->idendifiers);
			cout << "]";
			if (foundCurrent) {
				cout << "  <----- ";
				foundCurrent = false;
			}
			cout << "\n";
			temp=temp->next;
		}
		else if (temp->type == 11) {
			cout << "skip";
			displayStatementIdentifiers(temp);
			if (foundCurrent) {
				cout << "  <----- ";
				foundCurrent = false;
			}
			cout << "\n";
			temp=temp->next;
		}
		else if (temp->type == 99) {
			 cout << "Abort";
			if (foundCurrent) {
				cout << "  <----- ";
				foundCurrent = false;
			}
			cout << "\n";
			temp = temp->next;
		}
	}
}

void outputLineNumber(int *ln) {
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

void linkedList::displayWithPositionAndNum(statement *stopper, int ind, statement *current, int *lineNumber) {

	statement *temp = new statement;
	temp = head;

	bool foundCurrent = false;

	while (temp != stopper) {
		//determine if this statement is the current
		if (temp == current) {
			foundCurrent = true;
		}

		//output the line number
		outputLineNumber(lineNumber);
		//cout << *lineNumber << ": ";
		//*lineNumber += 1;

		//now display the correct identation
		indent(ind);

		if (temp->type == 0) {
			DA *i = static_cast<DA*>(temp);

			cout << i->varName << " = " << i->newCondition;
			/*if (i->newVal != 0) { cout << i->newVal; }
			else {
				if (intToStringInList(i->newVal) != i->newValString) { cout << i->newValString; }
				else { cout << i->newVal; }
			}*/
			cout << " (" << i->path << ") [";
			showIden(i->idendifiers);
			cout << "]";
			if (foundCurrent) {
				cout << "   <------- ";
				foundCurrent = false;
			}
			cout << "\n";
			temp=temp->next;
		}
		else if (temp->type == 1) {
			CA *i = static_cast<CA*>(temp);

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
			showIden(i->idendifiers);
			cout << "]";

			if (i->atomic == NULL) {
				cout << " null";
			}
			else {
				cout << " not null";
			}

			if (foundCurrent) {
				cout << "   <------- ";
				foundCurrent = false;
			}
			cout << "\n";
			temp=temp->next;
		}
		else if (temp->type == 2) {
			//If statement
			IfS *i = static_cast<IfS*>(temp);

			//cout << "if " << i->condID << " (" << i->varName << " " << i->op << " " << i->newVal << ") then ";
			cout << "if " << i->condID << " (" << i->newConditionString << ") then ";

			if ((current == temp) && (!i->seen)) {
				cout << "  <------- ";
			}
			cout << "\n";
			i->trueBranch->displayWithPositionAndNum(temp,ind+1,current,lineNumber);

			//output the line number
			outputLineNumber(lineNumber);

			indent(ind);
			cout  << "else\n";
			i->falseBranch->displayWithPositionAndNum(temp,ind+1,current,lineNumber);

			//output the line number
			outputLineNumber(lineNumber);

			indent(ind);
			cout  << "fi (" << i->path << ") [";
			showIden(i->idendifiers);
			cout << "]";
			if ((current == temp) && (i->seen)) {
				cout << "  <------- ";
			}
			cout << "\n";
			temp=temp->next;
		}
		else if (temp->type == 3) {
			//while loop
			WlS *i = static_cast<WlS*>(temp);

			//cout << "while " << i->WID << " (" << i->varName << " " << i->op << " " << i->newVal << ")" << " do";
			cout << "while " << i->WID << " (" << i->newConditionString << ") do";
			if (foundCurrent) {
				cout << "  <----- ";
				foundCurrent = false;
			}
			cout << "\n";

			i->loopBody->displayWithPositionAndNum(temp,ind+1,current,lineNumber);

			//output the line number
			outputLineNumber(lineNumber);

			indent(ind);
			cout << "elihw (" << i->path << ") [";
			showIden(i->idendifiers);
			cout << "]\n";
			temp=temp->next;
		}
		else if (temp->type == 4) {
			//par statement
			Par *i = static_cast<Par*>(temp);

			cout << "par {\n";
			if (foundCurrent) {
				i->leftSide->displayWithPositionAndNum(temp,ind+1,i->currLeft,lineNumber);
			}
			else {
				i->leftSide->displayWithPositionAndNum(temp,ind+1,current,lineNumber);
			}

			//output the line number
			outputLineNumber(lineNumber);

			indent(ind);
			cout << "}\n";

			//output the line number
			outputLineNumber(lineNumber);

			indent(ind);
			cout << "{\n";
			if (foundCurrent) {
				i->rightSide->displayWithPositionAndNum(temp,ind+1,i->currRight,lineNumber);
			}
			else {
				i->rightSide->displayWithPositionAndNum(temp,ind+1,current,lineNumber);
			}

			//output the line number
			outputLineNumber(lineNumber);

			indent(ind);
			cout << "}";
			if ((foundCurrent) && ((i->currLeft == i) || (i->currRight == i))) {
				cout << "  <----- ";
				foundCurrent = false;
			}
			else if (foundCurrent) {
				foundCurrent = false;
			}
			cout << "\n";

			temp = temp->next;
		}
		else if (temp->type == 5) {
			BlS *i = static_cast<BlS*>(temp);

			cout << "begin " << i->bid << " ";
			if (foundCurrent && (!i->seen)) {
				cout << "  <----- ";
				foundCurrent = false;
			}
			cout << "\n";
			i->blockBody->displayWithPositionAndNum(temp,ind+1,current,lineNumber);

			//output the line number
			outputLineNumber(lineNumber);

			indent(ind);
			cout << "end [";
			showIden(i->idendifiers);
			cout << "]";
			if (foundCurrent && (i->seen)) {
				cout << "  <----- ";
				foundCurrent = false;
			}
			cout << "\n";
			temp=temp->next;
		}
		else if (temp->type == 6) {
			VdS *i = static_cast<VdS*>(temp);

			cout << "var " << i->varName << " = " << i->value << " (" << i->path << ") [";
			showIden(i->idendifiers);
			cout << "]";
			if (foundCurrent) {
				cout << "  <----- ";
				foundCurrent = false;
			}
			cout << "\n";
			temp=temp->next;
		}
		else if (temp->type == 7) {
			PdS *i = static_cast<PdS*>(temp);

			cout << "proc " << i->procIden << " " << i->procName << " is\n";
			i->procBodyList->displayWithPositionAndNum(NULL,ind+1,current,lineNumber);

			//output the line number
			outputLineNumber(lineNumber);

			indent(ind);
			cout << "corp (" << i->path << ") [";
			showIden(i->idendifiers);
			cout << "]";
			if (foundCurrent) {
				cout << "  <----- ";
				foundCurrent = false;
			}
			cout << "\n";
			temp=temp->next;
		}
		else if (temp->type == 8) {
			PcS *i = static_cast<PcS*>(temp);

			cout << "call " << i->callID << " " << i->procName << " (" << i->path << ") [";
			showIden(i->idendifiers);
			cout << "]";
			if (foundCurrent) {
				cout << "  <----- ";
				foundCurrent = false;
			}
			cout << "\n";
			temp=temp->next;
		}
		else if (temp->type == 9) {
			PrS *i = static_cast<PrS*>(temp);

			cout << "remove proc " << i->procIden << " " << i->procName << " is\n";

			i->procBodyList->displayWithPositionAndNum(NULL,ind+1,current,lineNumber);

			//output the line number
			outputLineNumber(lineNumber);

			indent(ind);
			cout << "corp (" << i->path << ") [";
			showIden(i->idendifiers);
			cout << "]";
			if (foundCurrent) {
				cout << "  <----- ";
				foundCurrent = false;
			}
			cout << "\n";
			temp=temp->next;
		}
		else if (temp->type == 10) {
			VrS *i = static_cast<VrS*>(temp);

			cout << "remove var " << i->varName << " = " << i->value << " (" << i->path << ") [";
			showIden(i->idendifiers);
			cout << "]";
			if (foundCurrent) {
				cout << "  <----- ";
				foundCurrent = false;
			}
			cout << "\n";
			temp=temp->next;
		}
		else if (temp->type == 11) {
			cout << "skip";
			displayStatementIdentifiers(temp);
			if (foundCurrent) {
				cout << "  <----- ";
				foundCurrent = false;
			}
			cout << "\n";
			temp=temp->next;
		}
		else if (temp->type == 12) {
			cout << "skip (placeholder)";
			if (foundCurrent) {
				cout << "  <----- ";
				foundCurrent = false;
			}
			cout << "\n";
			temp = temp->next;
		}
		else if (temp->type == 13) {
			Atom *at = static_cast<Atom*>(temp);
			cout << "atomic";

			if (at->atomic == NULL) {
				cout << " null";
			}
			else {
				cout << " not null";
			}

			if ((foundCurrent) && (!at->started)) {
				cout << "  <----- ";
				foundCurrent = false;
			}
			cout << "\n";
			at->atomBody->displayWithPositionAndNum(at,ind+1,current,lineNumber);

			//output the line number
			outputLineNumber(lineNumber);

			indent(ind);
			cout << "cimota [";
			showIden(at->idendifiers);
			cout << "]";
			if (foundCurrent && (at->started)) {
				cout << "  <----- ";
				foundCurrent = false;
			}
			cout << "\n";
			temp=temp->next;

		}
		else if (temp->type == 99) {
			 cout << "Abort";
			if (foundCurrent) {
				cout << "  <----- ";
				foundCurrent = false;
			}
			cout << "\n";
			temp = temp->next;
		}
	}








}

/*
 * Function that modifys a given program, updating the sides of a parallel with correct parent and left values
 */
void linkedList :: modifiyList() {
	//cout << "modify list\n";

	statement *temp = new statement;
	temp = head;

	while (temp != NULL) {
		if (temp->type == 0) {
			//ignore
		}
		else if (temp->type == 1) {
			//ignore
		}
		else if (temp->type == 2) {
			IfS *is = static_cast<IfS*>(temp);
			is->trueBranch->modifiyList();
			is->falseBranch->modifiyList();
		}
		else if (temp->type == 3) {
			WlS *ws = static_cast<WlS*>(temp);
			ws->loopBody->modifiyList();
		}
		else if (temp->type == 4) {
			Par *ap = static_cast<Par*>(temp);
			updateBranch(ap->leftSide,ap,1);
			updateBranch(ap->rightSide,ap,0);
		}
		else if (temp->type == 5) {
			BlS *bls = static_cast<BlS*>(temp);
			bls->blockBody->modifiyList();
		}
		else if (temp->type == 6) {
			//local var dec - ignore
		}
		else if (temp->type == 7) {
			//procedure declaration
			PdS *pds = static_cast<PdS*>(temp);
			pds->procBodyList->modifiyList();
		}
		else if (temp->type == 8) {
			//procedure call - ignore
		}
		else if (temp->type == 9) {
			PrS *prs = static_cast<PrS*>(temp);
			prs->procBodyList->modifiyList();
		}
		else if (temp->type == 10) {
			//local variable removal - ignore
		}
		else if (temp->type == 11) {
			//cout << "Error in modifyList - statement type = 11\n";

		}
		else if (temp->type == 13) {
			cout << "in atomic\n";
			Atom *at = static_cast<Atom*>(temp);
			at->atomBody->modifiyList();
			//also add the pointer to this atomic statement within all statements within the body
			updateAtomicBody(at->atomBody,at);
		}
		else if (temp->type == 99) {


		}
		temp = temp->next;
	}
}

void linkedList:: modifyWithPrem(statement *stopper, Par *parent, int left) {
	statement *temp = new statement;
	temp = head;

	while (temp != stopper) {
		if (temp->type == 0) {
			DA* da = static_cast<DA*>(temp);
			da->parentPar = parent;
			da->left = left;
		}
		else if (temp->type == 1) {
			CA* ca = static_cast<CA*>(temp);
			ca->parentPar = parent;
			ca->left = left;
		}
		else if (temp->type == 2) {
			IfS* i = static_cast<IfS*>(temp);
			i->parentPar = parent;
			i->left = left;
			i->trueBranch->modifyWithPrem(i, parent, left);
			i->falseBranch->modifyWithPrem(i, parent, left);
		}
		else if (temp->type == 3) {
			WlS* wl = static_cast<WlS*>(temp);
			wl->parentPar = parent;
			wl->left = left;
	//	cout << "in loop\n";
			wl->loopBody->modifyWithPrem(wl, parent, left);
	//	cout << "loop done\n";
		}
		else if (temp->type == 4) {
			Par* i = static_cast<Par*>(temp);
			i->parentPar = parent;
			i->left = left;
		cout << "in par\n";
			i->leftSide->modifyWithPrem(i,i,1);
		cout << "left side of par done\n";
			i->rightSide->modifyWithPrem(i,i,0);
		cout << "right side of par done\n";
		}
		else if (temp->type == 5) {
			BlS* i = static_cast<BlS*>(temp);
			i->parentPar = parent;
			i->left = left;
		cout << "in block\n";
			i->blockBody->modifyWithPrem(i, parent, left);
		cout << "block done\n";
		}
		else if (temp->type == 6) {
			VdS* i = static_cast<VdS*>(temp);
			i->parentPar = parent;
			i->left = left;
		}
		else if (temp->type == 7) {
			PdS* i = static_cast<PdS*>(temp);
			i->parentPar = parent;
			i->left = left;
		cout << "in proc dec11111\n";
			i->procBodyList->modifyWithPrem(NULL, parent, left);
		cout << "11111proc dec done\n";
		}
		else if (temp->type == 8) {
			PcS* i = static_cast<PcS*>(temp);
			i->parentPar = parent;
			i->left = left;
		}
		else if (temp->type == 9) {
			PrS* i = static_cast<PrS*>(temp);
			i->parentPar = parent;
			i->left = left;
		cout << "in proc rem11111\n";
			i->procBodyList->modifyWithPrem(NULL, parent, left);
		cout << "proc rem done\n";
		}
		else if (temp->type == 10) {
			VrS* i = static_cast<VrS*>(temp);
			i->parentPar = parent;
			i->left = left;
		}
		else if (temp->type == 11) {
			cout << "trying to modify a skip operation - type = 11\n";
		}
		else if (temp->type == 12) {
			PH* i = static_cast<PH*>(temp);
			i->parentPar = parent;
			i->left = left;
		}
		else if (temp->type == 13) {
			Atom* i = static_cast<Atom*>(temp);
			i->parentPar = parent;
			i->left = left;
			i->atomBody->modifyWithPrem(i, parent, left);
		}
		else if (temp->type == 99) {
			Abort *i = static_cast<Abort*>(temp);
			i->parentPar = parent;
			i->left = left;
		}
		else {
			cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Type = " << temp->type << "\n";
		}
		temp = temp->next;
	}
}
} /* namespace std */
