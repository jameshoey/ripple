//============================================================================
// Name        : simulator.h
// Author      : James Hoey
// Version     :
// Copyright   : Copyright 2018
// Description : Header file for the implementation of the simulator - all features including the interface
//============================================================================

#ifndef SIMULATOR_H_
#define SIMULATOR_H_

#include <iostream>
#include <vector>
#include <stack>
#include <queue>
#include <set>
#include <stdlib.h>
#include <iterator>
#include <utility>
#include <tr1/unordered_map>
#include "linkedList.h"
#include "parser.h"

namespace std {

//-------now define some structures necessary for passing unordered maps as parameters - time constraints mean these must be passed as pointers

//Definition of a pair and a hash function for it, necessary for using a pair as a key of an unordered map
typedef std::pair<std::string,std::string> myPair;

struct pair_hash {
    template <class T1, class T2>
    std::size_t operator() (const std::pair<T1, T2> &pair) const {
        return std::tr1::hash<T1>()(pair.first) ^ std::tr1::hash<T2>()(pair.second);
    }
};

//sigma and gamma passed together as they are often used together
typedef struct { std::tr1::unordered_map<myPair,int,pair_hash> *gamma; std::tr1::unordered_map<int, int> *sigma; } gammaSigma;

//scope environment
typedef struct { std::tr1::unordered_map<std::string,std::set<std::string> > *se; } scopeEnvironment;

//procedure environment
typedef struct { std::tr1::unordered_map<std::string,pair<std::string,statement*> > *pe; } procedureEnvironment;

//structure for mapping procedure identifiers to name used in code and block name in which it is defined
typedef struct { std::tr1::unordered_map<myPair,std::string,pair_hash> *pi; } procedureIdentifiers;

//while environment
typedef struct { std::tr1::unordered_map<std::string,statement* > *we; } whileEnvironment;

//auxiliary store - both parts that make our one auxiliary store
typedef struct { std::tr1::unordered_map<std::string, stack<pair<int,int> > > *aux; std::tr1::unordered_map<std::string, stack<pair<int,string> > > *auxAI;  } auxStore;

//structure to represent the updated annotation information - a map of a call/loop name to a queue of stacks of ints
typedef struct { std::tr1::unordered_map<std::string, std::stack<std::stack<int> > > *aiMap; } AnnInfo;

//next name function for version numbers of all constructs
typedef struct { std::tr1::unordered_map<std::string,int > *nn; } nextNames;


//-------end of structure definitions

//-------now define the class simulator
class simulator {

public:
	//------define all variables that are necessary for a simulator

	vector<string> varNames; //list of all variable names - initially only the global names
	vector<string> stackNames; //list of all variable names, plus B, W, WI and Pr

	//gamma
	std::tr1::unordered_map<myPair,int,pair_hash> gamma;

	//sigma
	std::tr1::unordered_map<int, int> sigma;

	//gamma and sigma together as pointers necessary for parameter passing
	gammaSigma gs;

	//integer holding the value of the next available memory location
	int nextLocInt = 0;
	int *nextLoc = &nextLocInt;

	//while environment and its pointer version for passing
	std::tr1::unordered_map<std::string,statement* > beta; //is initially empty
	whileEnvironment we;

	//procedure environment and its pointer version for passing
	std::tr1::unordered_map<std::string,pair<std::string,statement*> > mu; //is initially empty
	procedureEnvironment pe;

	//procedure identifier information necessary for evaluation
	std::tr1::unordered_map<myPair,std::string,pair_hash> procIdentifiers;
	procedureIdentifiers pi;

	//scope information and its pointer version for passing
	std::tr1::unordered_map<std::string,std::set<std::string> > scopeEnviron;
	scopeEnvironment se;

	//auxiliary store (both parts) and its pointer version for passing
	std::tr1::unordered_map<std::string, stack<pair<int,int> > > aux;
	std::tr1::unordered_map<std::string, stack<pair<int,string> > > auxAI;
	auxStore as;

	//Update

	std::tr1::unordered_map<std::string, std::stack<std::stack<int> > > AIinfo;
	AnnInfo AI;

	//End of Update

	//next name function and its pointer version for passing
	std::tr1::unordered_map<std::string,int > nextNameFunction;
	nextNames nextName;

	//next and previous integers used to maintain the use of identifiers
	int nextInt = 0;
	int *next = &nextInt;
	int previousInt = -1;
	int *previous = &previousInt;

	//the actual program and a pointer to the first statement (modified as the program executes)
	linkedList *program;
	statement *fwdPointer;
	statement *placeInExecution;
	statement *placeInRevEx;

	//variables indicating whether state-saving is enabled and direction of execution
	int stateSavingInt = 1;
	int *stateSaving = &stateSavingInt; // 1 = state-saving is enabled, 0 = state-saving is not enabled
	int *direction;

	//pointer to the original parser - making the link functions accessible later on during the simulation
	parser *pp;

	//boolean of whether tracing is on or off
	bool recordTrace;

	//boolean of whether to record the sequence of rules as they are applied
	bool recordRules, recordRulesInv;

	//vector of strings to maintain the sequence of rules as we execute
	vector<string> *sequenceOfRules, *sequenceOfRulesInv;

	//vector of strings to maintain a trace
	vector<string> trace;

	vector<string> *pointToTrace;

	vector<string> invTrace;

	vector<string> *pointToInvTrace;

	//------end of variables


	//------define constructor
	simulator();
	simulator(vector<string> varNames, vector<string> stackNames, std::tr1::unordered_map<myPair,int,pair_hash> gamma, std::tr1::unordered_map<int, int> sigma, int *nextLoc, std::tr1::unordered_map<std::string,statement* > beta, std::tr1::unordered_map<std::string,pair<std::string,statement*> > mu, std::tr1::unordered_map<myPair,std::string,pair_hash> procIdentifiers, std::tr1::unordered_map<std::string,std::set<std::string> > scopeEnviron, std::tr1::unordered_map<std::string, stack<pair<int,int> > > aux, std::tr1::unordered_map<std::string, stack<pair<int,string> > > auxAI, std::tr1::unordered_map<std::string,int > nextName, int *next, int *previous, int *stateSaving);
	simulator(parser *pp, linkedList *program, vector<string> varNames, vector<string> stackNames, std::tr1::unordered_map<myPair,int,pair_hash> gamma, std::tr1::unordered_map<int, int> sigma, int *nextLoc, std::tr1::unordered_map<std::string,statement* > beta, std::tr1::unordered_map<std::string,pair<std::string,statement*> > mu, std::tr1::unordered_map<myPair,std::string,pair_hash> procIdentifiers, std::tr1::unordered_map<std::string,std::set<std::string> > scopeEnviron, std::tr1::unordered_map<std::string, stack<pair<int,int> > > aux, std::tr1::unordered_map<std::string, stack<pair<int,string> > > auxAI, std::tr1::unordered_map<std::string,int > nextName, int *next, int *previous, int *stateSaving);

	//------end of constructor

	//------methods of a simulator

	void switchStateSaving(); //either turn state-saving on or off, depending on its current value
	int currentStateSaving(); //return whether state-saving is on or off

	void interface();
	bool executeNSteps(int n, bool forceRandom, bool forceUser, bool forceSequence, string *sequence, bool atomEval, bool output);
	bool executeNStepsInv(int n, bool forceRandom, bool forceUser, bool output);

	void preExecutionSetUp();
	void saveState(char *filename);
	void restoreState(char *filename);

	void invertProgram();

	void displayState(); //display the entire state of all environments with necessary display breaks
	void displaySigGam(); //display contents of any location (with varname and blockname) that is not 0

	//display methods for the interface
	void displaySigmaInterface(bool all, int location);
	void displySigGamInterface(bool all, string varName, string blockName, bool nonZero);
	void displayGamInterface(bool all, string varname, string blockname);
	void displayAuxInterface(bool all, string stackName);
	void displayMuInterface(statement *current);
	void displayBetaInterface(statement *current);
	void displayScopeEnvironment();

	//saving state to file
	void auxToVec(vector<string> *store);
	void saveSigmaGamma(vector<string> *store);

	void TraceOnOff();
	void displayTrace();
	void displayTraceInv();
	vector<string>* getTrace();
	vector<string>* getInvTrace();

	void status();

	bool compareTraces();

	void reset();

	bool checkTopOfWI(std::string constructName);

};

} /* namespace std */

#endif /* SIMULATOR_H_ */
