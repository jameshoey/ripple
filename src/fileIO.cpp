//============================================================================
// Name        : fileIO.cpp
// Author      : James Hoey
// Version     :
// Copyright   : Copyright 2018
// Description : Implementation of file input / output
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

#include "fileIO.h"

namespace std {

//==============HELPER=================================================

int stringToInt1(string s) {
	return atoi(s.c_str());
}

string intToString1(int i) {
	stringstream ss;
	ss << i;
	string str = ss.str();

	return str;
}

string removeEntry(string line) {
	if (line.find_first_of(" ") == 0) {
		return removeEntry(line.erase(0,1));
	}
	else if (line.find_first_of("\t") == 0) {
		return removeEntry(line.erase(0,1));
	}
	else {
		return line;
	}
}

string removeExit(string line, int size) {

	if (line.find_last_of(" ") == size-1) {
		return removeExit(line.erase(size-1),size-1);
	}
	else if (line.find_last_of("\t") == size-1) {
			return removeExit(line.erase(size-1),size-1);
		}
	else {
		return line;
	}
}

string removeWindowsInfo(string line) {
	if (line.length() == 0) {
		return line;
	}
	else {
		if (line.at(line.length()-1) == '\r') {
			line.erase(line.length()-1);
		}
		return line;
	}

}

string removeWhiteSpace(string line) {
	line = removeEntry(line);
	if (line.length() == 0) {
		return line;
	}
	line = removeExit(line,line.size());
	return line;
}

/*
 * Returns true if the given line is a comment and should be ignored, or false otherwise
 */
bool isComment(string line) {
	if (line.find("//") == 0) {
		return true;
	}
	return false;
}

string removeEndComment(string line) {
	if (line.find("//") != std::string::npos) {
	    line.erase(line.find("//"));
	}
	return line;
}

/*
 * Take a filename - read the program saved in that file as a list of strings
 * Read each line of that file into a vector of strings and return this
 */
vector<string> readFile(char* filename) {
	vector <string> contents;

	bool fileFinished = false;

	string line; //this will contain the data read from the file
	ifstream myfile (filename); //opening the file.
	if (myfile.is_open()) {//if the file is open

		while ( getline (myfile,line) ) {
			line = removeEndComment(line);
			line = removeWindowsInfo(line);
			line = removeWhiteSpace(line);

			while (!fileFinished && ((line.length() == 0) || (isComment(line)))) {
				if (getline (myfile,line)) {
					line = removeEndComment(line);
					line = removeWindowsInfo(line);
					line = removeWhiteSpace(line);

				}
				else {
					//file has now finished
					fileFinished = true;
				}
			}

			if (!fileFinished) {
				//if the text file has made on a windows machine - remove the final \r
				if (line.at(line.length()-1) == '\r') {
					line.erase(line.length()-1);
				}
				contents.push_back(line);
			}
			else {
				break;
			}
		}
		myfile.close(); //closing the file
	}
	else cout << "Unable to open file"; //if the file is not open output

	//remove last line if its empty
	if (contents.at(contents.size()-1) == "") {
		contents.pop_back();
	}

	return contents;
}

/*
 * Incomplete - push a program to a file - this function will take a program and convert it into a vector of strings
 */
vector<string> listToVector(linkedList *program, statement *stopper) {
	vector<string> toReturn;

	statement *temp = program->nextStatement();

	while (temp != stopper) {
		if (temp->type == 0) {
			DA *ori = static_cast<DA*>(temp);
			string line = ori->varName + " = " + ori->newCondition + " (" + ori->path + ");";
			toReturn.push_back(line);
		}
		else if (temp->type == 1) {
			CA *ori = static_cast<CA*>(temp);
			string line = "";
			if (ori->inc) {
				line = ori->varName + " + " + ori->newCondition + " (" + ori->path + ");";
			}
			else {
				line = ori->varName + " - " + ori->newCondition + " (" + ori->path + ");";
			}
			toReturn.push_back(line);
		}
		else if (temp->type == 2) {

		}
		else if (temp->type == 3) {

		}
		else if (temp->type == 4) {

		}
		else if (temp->type == 5) {
			BlS *ori = static_cast<BlS*>(temp);
			string line1 = "begin " + ori->bid;
			toReturn.push_back(line1);

			vector<string> body = listToVector(ori->blockBody,ori);

			for (int i = 0; i < body.size(); i++) {
				toReturn.push_back(body.at(i));
			}

			string line2 = "end";
			toReturn.push_back(line2);
		}
		else if (temp->type == 6) {
			VdS *ori = static_cast<VdS*>(temp);
			string line = "var " + ori->varName + " = " + intToString1(ori->value) + " (" + ori->path + ");";
			toReturn.push_back(line);
		}
		else if (temp->type == 7) {
			PdS *ori = static_cast<PdS*>(temp);
			string line1 = "proc " + ori->procIden + " " + ori->procName + " is";
			toReturn.push_back(line1);

			vector<string> body = listToVector(ori->procBodyList,NULL);

			for (int i = 0; i < body.size(); i++) {
				toReturn.push_back(body.at(i));
			}

			string line2 = "corp (" + ori->path + ");";
			toReturn.push_back(line2);
		}
		else if (temp->type == 8) {
			PcS *ori = static_cast<PcS*>(temp);
			string line = "call " + ori->callID + " " + ori->procName + " (" + ori->path + ");";
			toReturn.push_back(line);
		}
		else if (temp->type == 9) {
			PrS *ori = static_cast<PrS*>(temp);
			string line1 = "remove proc " + ori->procIden + " " + ori->procName + " is";
			toReturn.push_back(line1);

			vector<string> body = listToVector(ori->procBodyList,NULL);

			for (int i = 0; i < body.size(); i++) {
				toReturn.push_back(body.at(i));
			}

			string line2 = "corp (" + ori->path + ");";
			toReturn.push_back(line2);
		}
		else if (temp->type == 10) {
			VrS *ori = static_cast<VrS*>(temp);
			string line = "remove var " + ori->varName + " = " + intToString1(ori->value) + " (" + ori->path + ");";
			toReturn.push_back(line);
		}
		temp = temp->next;
	}

	return toReturn;
}

/*
 * Insert a series of strings into a fil					cout << "-----------------------------\n";
 * e - used to save a program/trace to a file
 */
void writeFile(vector<string> program, char* filename) {
	std::ofstream output_file(filename);
	std::ostream_iterator<std::string> output_iterator(output_file, "\n");
	std::copy(program.begin(), program.end(), output_iterator);
}



bool endsWithSemiColon(string ori) {
	int size = ori.size();
	if (ori.at(size-1) == ';') {
		return true;
	}
	else {
		return false;
	}
}

bool needsPath(string ori) {
	if ((ori.find("while") == 0) || (ori.find("if") == 0) || (ori.find("proc") == 0) ||
			(ori.find("skip") == 0) || (ori.find("abort") == 0) || (ori == "") || (ori.find("par") == 0) || (ori.find("}") == 0) || (ori.find("{") == 0)) {
		return false;
	}
	else {
		return true;
	}
}

void addPaths(vector<string> *ori, string path) {

	string current;

	for (int i=0; i < ori->size(); i++) {

		current = ori->at(i);

		if (needsPath(current)) {

			if (endsWithSemiColon(current)) {
				//then remove it
				current.erase(current.size()-1);
			}

			if (current.find("begin") == 0) {

				//must open a new block
				string copyOfCurrent = current;

				if (path == "(lam)") {
					path = "(" + copyOfCurrent.erase(0,6) + ";)";
				}
				else {
					path = "(" + copyOfCurrent.erase(0,6) + ";" + path.substr(1, path.size());
				}
			}
			else if (current.find("end") == 0) {
				//remove the top element from the path

				int endOfFirst = path.find_first_of(";");
				path = path.erase(0,endOfFirst+1);


				if (path == ")") {
					path = "(lam)";
				}
				else {
					path = "(" + path;
				}
			}
			else {
				current = current + " " + path + ";";
				ori->at(i) = current;
			}
		}
		else {
			//does not require a change - so ignore
		}
	}
}

//automatically add brackets around expressions
void addBracketsAroundExpressions(vector<string> *original) {

	for (int i = 0; i < original->size(); i++) {
		if (((original->at(i).find(" = ") != -1) && (original->at(i).find("var") != 0)) || ((original->at(i).find(" += ") != -1) && (original->at(i).find("var") != 0))) {
//			cout << "must add brackets to " << original->at(i) << ".\n";

			//split on the first occurrence of =
			string current = original->at(i);
			int splitPoint = current.find_first_of("=");
			string head = current;
			head.erase(splitPoint+2);

			current.erase(0,splitPoint+2);
			//remove the semi colon if its there
			if (endsWithSemiColon(current)) {
				current.erase(current.length()-1);
			}

			current = "(" + current + ");";

			original->at(i) = head + current;

			//string rest = original->at(i).erase(0,original->at(i).find_first_of("="));
			//string rest = original->erase(original->find_first_of())

			//original->at(i) = original->at(i) + "MUST ADD BRACKEts";
		}
	}

}

//add the construct identifiers automatically
vector<string> addConstructIdentifiers(vector<string> original, int nextIf, int nextLoop, int nextBlock, int nextProc, int nextCall) {

	for (int i = 0; i < original.size(); i++) {
		//cout << "loop on "<< original.at(i) << "\n";
		if (original.at(i).find("if ") == 0) {
//			cout <<"found an if\n";
			//rest of string after if and the space
			string rest = original.at(i).erase(0,original.at(i).find_first_of(' '));
			original.at(i) = "if i" + intToString1(nextIf) + ".0" + rest;
			nextIf += 1;
//			cout << "UPDATED TO " << original.at(i) << ": nextint = " << intToString1(nextIf) << "\n";
		}
		else if (original.at(i).find("while ") == 0) {
//			cout << "adding loop name\n";
			//rest of string after if and the space
			string rest = original.at(i).erase(0,original.at(i).find_first_of(' '));
			original.at(i) = "while w" + intToString1(nextLoop) + ".0" + rest;
			nextLoop += 1;
		}
		else if (original.at(i).find("begin") == 0) {
//			cout << "adding block name\n";
			//rest of string after if and the space
			string rest = original.at(i).erase(0,original.at(i).find_first_of(' '));
			original.at(i) = "begin b" + intToString1(nextBlock) + ".0" + rest;
			nextBlock += 1;
		}
		else if (original.at(i).find("proc ") == 0) {
//			cout << "adding proc name\n";
			//rest of string after if and the space
			string rest = original.at(i).erase(0,original.at(i).find_first_of(' '));
			original.at(i) = "proc p" + intToString1(nextProc) + ".0" + rest;
			nextProc += 1;
		}
		else if (original.at(i).find("call ") == 0) {
//			cout << "adding call name\n";
			//rest of string after if and the space
			string rest = original.at(i).erase(0,original.at(i).find_first_of(' '));
			original.at(i) = "call c" + intToString1(nextCall) + ".0" + rest;
			nextCall += 1;
		}
	}

	return original;
}


//==============END====================================================




	fileIO::fileIO(int cur) {
		cout << "state-saving in fileIO = " << cur << "\n";
	}

	fileIO::fileIO() {

	}

	vector<string> fileIO::read(char *filename, bool autoPaths, bool autoCon) {
		vector<string> toReturn = readFile(filename);

		addBracketsAroundExpressions(&toReturn);

		if (autoCon) {
			toReturn = addConstructIdentifiers(toReturn, 0,0,0,0,0);
		}

		if (autoPaths) {
			addPaths(&toReturn,"(lam)");
		}

		return toReturn;
	}


	void fileIO::write(char* filename, linkedList *program) {
		vector<string> vecProg = listToVector(program,NULL);
		writeFile(vecProg,filename);
	}

	void fileIO::writeTrace(char *filename, vector<string> *trace) {
		writeFile(*trace,filename);
	}

}
