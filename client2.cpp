///
 /// client2.cpp
 ///
 ///  Created on: Jul 13, 2015
 ///      Author: visteon
///



#include <cstdlib>
#include <iostream>
#include <dlfcn.h>

#include "ipc_defs.hpp"
#include "comm_lib.hpp"

#define CLIENT_ID 2

using namespace std;

typedef shared* (*access_fn)();
typedef void (*register_fn)(cb_t, shared*, cid_t);
typedef void (*print_fn)(shared*);
typedef void (*send_fn)(shared*, operand1_t, operand2_t, operation_t, cid_t);
typedef void (*verify_str_fn)(char*);
typedef void (*verify_int_fn)(int*);

void* handle;

print_fn print_res;

void callback(shared* sh)
{
    cout << "Receiving request... \n";
    print_res(sh);
}

int main()
{
    handle = dlopen("./libcomm.so", RTLD_LAZY);
    if (handle == NULL)
        cout << "Handle error\n";

    print_res = (print_fn)dlsym(handle, "print_result");
    access_fn gain_access = (access_fn)dlsym(handle, "access_shared");
    if (gain_access == NULL)
        cout << "Gain error\n";
    shared* shmem = gain_access();

    register_fn register_cb = (register_fn)dlsym(handle, "register_callback");
    if (register_cb == NULL)
        cout << "cb error\n";
    register_cb(callback, shmem, CLIENT_ID);

    send_fn send_req = (send_fn)dlsym(handle, "send_request_wrapper");

    verify_str_fn verify_string = (verify_str_fn)dlsym(handle, "verify_entered_string");

    verify_int_fn verify_number = (verify_int_fn)dlsym(handle, "verify_entered_number");

    int cmd;
    while(true)
    {
        cout << "\t\t(1)\tSubtract 2 numbers\n";
        cout << "\t\t(2)\tDivide 2 numbers\n";
        cout << "\t\t(3)\tFind substring in a string\n";
        cout << "\t\t(4)\tExit\n";
        cout << "\tEnter command: ";
        verify_number(&cmd);
        operand1_t op1;
        operand2_t op2;
        switch(cmd)
        {
            case 1:
            case 2:
            {
                cout << "Enter number 1: ";
                verify_number(&op1.op1_i);
                cout << "Enter number 2: ";
                verify_number(&op2.op2_i);
            }break;
            case 3:
            {
                cout << "Enter substring: ";
                verify_string(op1.op1_ch);
                cout << "Enter string: ";
                verify_string(op2.op2_ch);
            }break;
            case 4:
            {
                cout << "Goodbye!\n";
                exit(0);
            }break;
            default:
            {
                cout << "Please enter a valid command!\n";
                continue;
            }break;
        }
        cout << "Sending request... \n";
        send_req(shmem, op1, op2, (operation_t)cmd, CLIENT_ID);
    }
    return 0;
}
