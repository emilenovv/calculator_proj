///
 /// client1.cpp
 ///
 ///  Created on: Jul 13, 2015
 ///      Author: visteon
///


#include <cstdlib>
#include <iostream>
#include <pthread.h>

#include "ipc_defs.hpp"
#include "comm_lib.hpp"

#define CLIENT_ID 1

using namespace std;

void callback(shared* sh)
{
    cout << "Receiving request... \n";
    print_result(sh);
}

int main()
{
    shared* shmem = access_shared();
    register_callback(callback, shmem, CLIENT_ID);
    int cmd;
    while(true)
    {
        cout << "\t\t(1)\tAdd 2 numbers\n";
        cout << "\t\t(2)\tMultiply 2 numbers\n";
        cout << "\t\t(3)\tConcatenate 2 strings\n";
        cout << "\t\t(4)\tExit\n";
        cout << "\tEnter command: ";
        cin >> cmd;
        operand1_t op1;
        operand2_t op2;
        switch(cmd)
        {
            case 1:
            case 2:
            {
                cout << "Enter number 1: ";
                cin >> op1.op1_i;
                cout << "Enter number 2: ";
                cin >> op2.op2_i;
            }break;
            case 3:
            {
                cout << "Enter string 1: ";
                cin >> op1.op1_ch;
                cout << "Enter string 2: ";
                cin >> op2.op2_ch;
            }break;
            case 4:
            {
                cout << "Goodbye!\n";
                exit(0);
            }break;
            default:
                cout << "Please enter a valid command!\n";break;
        }
        cout << "Sending request... \n";
        send_request_wrapper(shmem, op1, op2, (operation_t)cmd, CLIENT_ID);
    }
    return 0;
}

