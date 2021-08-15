//============================================================================
// Name        : parser.h
// Author      : James Hoey
// Version     :
// Copyright   : Copyright 2018
// Description : Header file for the implementation of the parser
//============================================================================

#ifndef PARSER_H_
#define PARSER_H_

#include "linkedList.h"
#include <iostream>
#include <vector>
#include <stack>
#include <stdlib.h>
#include <iterator>
#include <utility>
#include <tr1/unordered_map>
#include <stdlib.h>

namespace std {

class parser;

class parser {

public:
	//copy of the original program - simply a vector of strings, program line by line
	vector<string> originalProgram;

	//the linkedList that is produced as a result of parsing the original vector of the program
	linkedList *programList;

	//-------constructor
	parser(vector<string> orig);

	//parse a program and produce the linkedList version - bool to indicate whether to automatically add removal statements
	linkedList* parseProgram(vector<string> program, bool autoRemoval);

	void useLinkFunctions(linkedList *program); //make the link functions available to other classes - used in createVersion and renaming (!!!)

	void test(string test);

};

}

#endif /* PARSER_H_ */
