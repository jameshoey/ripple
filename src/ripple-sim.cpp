//============================================================================
// Name        : ripple-sim.cpp
// Author      : James Hoey
// Version     :
// Copyright   : Copyright 2018
// Description : Main file of the ripple simulator - initialises the simulator and begins the interface
//============================================================================

#include <iostream>
#include <vector>
#include <stack>
#include <stdlib.h>
#include <iterator>
#include <utility>
#include <tr1/unordered_map>
#include "parser.h"
#include "linkedList.h"
#include "simulator.h"
#include "fileIO.h"
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <sys/time.h>
#include <ctime>

using namespace std;

int main() {
	simulator s = simulator(); //create an instance of the simulator
	s.interface(); //begin the interface
	return 0;
}
