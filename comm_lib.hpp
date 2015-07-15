///
 /// libcomm.h
 ///
 ///  Created on: Jul 13, 2015
 ///      Author: visteon
///

#ifndef COMM_LIB_HPP_
#define COMM_LIB_HPP_

#include "ipc_defs.hpp"

extern "C" {

void register_callback(cb_t, shared*, cid_t);
void* wait_answer(void*);

void verify_entered_string(char*);
void verify_entered_number(int*);

int add(int, int);
int subtract(int, int);
int multiply(int, int);
int divide(int, int);
char* concat(char*, char*);
int substr(char*, char*);
shared* create_shared();
shared* access_shared();
void* client_write(void*);
void* send_reply(void*);
void send_reply_wrapper(shared*);
void send_request(channel*);
void send_request_wrapper(shared*, operand1_t, operand2_t, operation_t, cid_t);
void* calculate_math(void*);
void* calculate_strings(void* sh);
void wait_for_request(shared*);
void* wait_for_request_task(void*);
void print_result(shared*);
}
#endif /* COMM_LIB_HPP_ */
