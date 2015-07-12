///
 /// libcomm.h
 ///
 ///  Created on: Jul 10, 2015
 ///      Author: visteon
///

#ifndef LIBCOMM_H_
#define LIBCOMM_H_

#include<string.h>
#include <queue>

typedef int cid_t;
//typedef std::queue<courier*> buffe_t;

//extern "C" void (*register_function)(void(*)(void (*)()));
//extern "C" void function_needing_callback();

typedef union operand1_t
    {
        int op1_i;
        char op1_ch[17];
        operand1_t() {memset(this, 0, sizeof(operand1_t));}
        operand1_t(int _op1_i):op1_i(_op1_i){}
        operand1_t(char* _op1_ch)
        {
        	strcpy(op1_ch, _op1_ch);
        }
    }operand1_t;

typedef union operand2_t
    {
        int op2_i;
        char op2_ch[17];
        operand2_t() {memset(this, 0, sizeof(operand2_t));}
        operand2_t(int _op2_i):op2_i(_op2_i){}
        operand2_t(char* _op2_ch)
		{
			strcpy(op2_ch, _op2_ch);
		}
    }operand2_t;

typedef union result_t
	{
		int result_int;
		char* result_char;
		result_t() {memset(this, 0, sizeof(result_t));}
		result_t(int _op2_i):result_int(_op2_i){}
		result_t(char* _result_char)
		{
			strcpy(result_char, _result_char);
		}
	}result_t;

typedef enum operation_t
{
	ADD = 1, SUBSTRACT,
	MULTIPLY, DIVIDE,
	CONCAT, SUBSTR
}operation_t;

typedef struct shared
{
	operand1_t operand1;
	operand2_t operand2;
	operation_t operation;
    result_t result;
    cid_t client_id;

    pthread_mutex_t mtx;
    pthread_cond_t server_ready;
    pthread_cond_t client_ready;
    pthread_cond_t client2_ready;
    bool c, s, new_request; //c = true when the client is has completed its job with the shared memory, respectively when server is ready
                // s = true
}shared;

typedef struct courier
{
	operand1_t operand1;
	operand2_t operand2;
	operation_t operation;
	result_t result;
	cid_t client_id;
}courier;

typedef struct channel
{
    courier* src;
    shared* dst;
}channel;

// Needs to be tested
void function_needing_callback();
void (*register_function)(void(*)());
//#end

extern "C" int add(int, int);
int subtract(int, int);
int multiply(int, int);
int divide(int, int);
char* concat(char*, char*);
shared* create_shared();
extern "C" shared* access_shared();
void init();
void* client_read(void*);
void* client_write(void*);
void* server_read(void*);
void* server_write(void*);
void send_reply(shared*);
void receive_reply(channel*);
void send_request(channel*);
void send_request_wrapper(shared*, operand1_t, operand2_t, operation_t, cid_t);
//void recieve_request(channel*);
void* calculate_math(void*);
void* calculate_strings(void* sh);




#endif /* LIBCOMM_H_ */
