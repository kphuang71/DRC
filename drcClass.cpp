// drcClass.cpp : Defines the entry point for the console application.
//

#include "drc.h"
#include <math.h>
#include<iostream>

using namespace std;

const int dataLen = 128;
double dataS[dataLen] = {1,1,1,1,1,1,1,1,1,1,1}; // creat the pulse of len: dataLen

drc drc01;


void main()
{
	drc01.setTypical();
	for (int i = 0; i < dataLen; i++) {
		cout << endl << dataS[i]*drc01.xGained(dataS[i]) ;
	}

}

