/*
 * client2.cpp
 *
 *  Created on: Jul 12, 2015
 *      Author: emil
 */

#include <cstdlib>

#include <iostream>
#include <pthread.h>
#include <dlfcn.h>

#include "libcomm.h"

using namespace std;

typedef shared* (*fn_t)();


void ortabudala()
{
	cout << "I am the BEST!";
}

int main()
{
	void *handle = dlopen("./libcomm.so", RTLD_LAZY);
	//register_function = (void (*)(void(*)()))dlsym(handle, "register_function");
	//register_function(ortabudala);
	//shared* shmem = func;
//	while(true)
//	{
//		cout << "\t\t(1)\tSubtract 2 numbers\n";
//		cout << "\t\t(2)\tDivide 2 numbers\n";
//		cout << "\t\t(3)\tFind substring in a string\n";
//		cout << "\t\t(4)\tExit\n";
//		cout << "\tEnter command: ";
//		int cmd;
//		cin >> cmd;
//		operand1_t op1;
//		operand2_t op2;
//		switch(cmd)
//		{
//			case 1:
//			case 2:
//			{
//				cout << "Enter number 1: ";
//				cin >> op1.op1_i;
//				cout << "Enter number 2: ";
//				cin >> op2.op2_i;
//			}break;
//			case 3:
//			{
//				cout << "Enter string 1: ";
//				cin >> op1.op1_ch;
//				cout << "Enter substring 2: ";
//				cin >> op2.op2_ch;
//			}break;
//
//			case 4:
//				cout << "Goodbye";break;
//			default:
//				cout << "Please enter a valid command!\n";break;
//		}
//		send_request_wrapper(shmem, op1, op2, (operation_t)cmd);
//	}
	return 0;
}


