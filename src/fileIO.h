//============================================================================
// Name        : fileIO.h
// Author      : James Hoey
// Version     :
// Copyright   : Copyright 2018
// Description : Header file for the implementation of file input / output
//============================================================================

#ifndef FILEIO_H_
#define FILEIO_H_

#include <iostream>
#include <vector>
#include <stack>
#include <stdlib.h>
#include <iterator>
#include <utility>
#include <tr1/unordered_map>
#include "linkedList.h"

namespace std {

class fileIO {
public:
	//-------variables

	vector<string> program;

	//-------end of variables

	//-------constructor
	fileIO();
	fileIO(int cur);

	//-------end of constructor

	//-------other functions necessary through a fileIO object

	vector<string> read(char *filename, bool autoPaths, bool autoCI);
	void write(char *filename, linkedList *program);

	void writeTrace(char *filename, vector<string> *trace);

	//-------end of other functions

};

}

#endif /* SIMULATOR_H_ */
