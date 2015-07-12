///
 /// libcomm.cpp
 ///
 ///  Created on: Jul 10, 2015
 ///      Author: visteon
///

#include <pthread.h>
#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include "libcomm.h"

using namespace std;



void (*callback)() = NULL;

void register_function(void(*ortabudala)())
{
    callback = ortabudala;
}

void function_needing_callback()
{
     callback();
}


int add(int a, int b)
{
	usleep(1000000);
    return a + b;
}

int subtract(int a, int b)
{

    return a - b;
}

int multiply(int a, int b)
{

    return a * b;
}

int divide(int a, int b)
{
    return a / b;
}

char* concat(char* a, char* b)
{
    char* result = new char[33];
    strcat(result, a);
    strcat(result, b);
    return result;
}

int substr(char* needle, char* haystack)
{
    return 1;
}

shared* create_shared()
{
    cout << "Initializing... \n";
    int fd = shm_open("/mymem", O_CREAT | O_RDWR, 0666);
    ftruncate(fd, sizeof(shared));
    shared* shmem = (shared*)mmap(0, sizeof(shared), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);

    pthread_mutexattr_t shared;
    pthread_mutexattr_init(&shared);
    pthread_mutexattr_setpshared(&shared, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&(shmem->mtx), &shared);

    pthread_condattr_t condAttr;
    pthread_condattr_setpshared(&condAttr, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(&shmem->server_ready, &condAttr);

    pthread_condattr_t condAttr2;
    pthread_condattr_setpshared(&condAttr2, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(&shmem->client_ready, &condAttr2);

    pthread_condattr_t condAttr3;
    pthread_condattr_setpshared(&condAttr3, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(&shmem->client2_ready, &condAttr3);

    shmem->s = true;
    shmem->c = false;
    shmem->new_request = false;
    cout << "Initialized!\n";
    return shmem;
}

shared* access_shared()
{
    int fd = shm_open("/mymem", O_CREAT | O_RDWR, 0666);
    ftruncate(fd, sizeof(shared));
    shared* shmem = (shared*)mmap(0, sizeof(shared), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    return shmem;
}

void* client_write(void* chn)
{
    channel* ch = (channel*)chn;
    pthread_mutex_lock(&ch->dst->mtx);
    while(ch->dst->s == false)
        pthread_cond_wait(&ch->dst->server_ready, &ch->dst->mtx);
    ch->dst->operation = ch->src->operation;
    ch->dst->client_id = ch->src->client_id;
    switch(ch->src->operation)
    {
        case 1:
        case 2:
        case 3:
        case 4:
        {
            ch->dst->operand1.op1_i = ch->src->operand1.op1_i;
            ch->dst->operand2.op2_i = ch->src->operand2.op2_i;
        }break;
        case 5:
        case 6:
        {
            memcpy(ch->dst->operand1.op1_ch, ch->src->operand1.op1_ch, strlen(ch->src->operand1.op1_ch) + 1);
            memcpy(ch->dst->operand2.op2_ch, ch->src->operand1.op1_ch, strlen(ch->src->operand2.op2_ch) + 1);
        }break;
    }
    ch->dst->s = false;
    ch->dst->new_request = true;
    ch->dst->c = true;
    pthread_cond_signal(&ch->dst->client_ready);
    while(ch->dst->s == false)
            pthread_cond_wait(&ch->dst->server_ready, &ch->dst->mtx);
    pthread_mutex_unlock(&ch->dst->mtx);
}


void* client_read(void* chm)
{
    channel* ch = (channel *)chm;
    pthread_mutex_lock(&ch->dst->mtx);
    while(ch->dst->s == false)
        pthread_cond_wait(&ch->dst->server_ready, &ch->dst->mtx);

    ch->dst->s = false;
    ch->dst->c = true;
    pthread_cond_signal(&ch->dst->client_ready);
    pthread_mutex_unlock(&ch->dst->mtx);
}

void send_request(channel* ch)
{
    pthread_t thread1;
    pthread_create(&thread1, NULL, client_write, ch);
    pthread_join(thread1, NULL);
}

void send_request_wrapper(shared* sh, operand1_t a, operand2_t b, operation_t op, cid_t id)
{
	channel* ch = new channel;
	ch->dst = sh;
	ch->src = new courier;
	ch->src->operand1.op1_i = a.op1_i;
	ch->src->operand2.op2_i = b.op2_i;
	ch->src->operation = op;
	ch->src->client_id = id;
	send_request(ch);
}

//void recieve_request(channel* ch)
//{
//    pthread_t thread;
//    pthread_create(&thread, NULL, server_read, ch);
//    pthread_join(thread, NULL);
//}

//void* server_read(void* chn)
//{
//    channel* ch = (channel*)chn;
//    pthread_mutex_lock(&ch->dst->mtx);
//    while(ch->dst->c == false)
//        pthread_cond_wait(&ch->dst->client_ready, &ch->dst->mtx);
//
//    switch(ch->dst->operation)
//    {
//        case 1:
//        case 2:
//        case 3:
//        case 4:
//        {
//        	cout << "dadada";
//            ch->src->operand1.op1_i = ch->dst->operand1.op1_i;
//            ch->src->operand2.op2_i = ch->dst->operand2.op2_i;
//            cout << ch->src->operand1.op1_i << endl;
//            cout << ch->src->operand2.op2_i << endl;
//
//        }break;
//        case 5:
//        case 6:
//        {
//            memcpy(ch->src->operand1.op1_ch, ch->dst->operand1.op1_ch, strlen(ch->dst->operand1.op1_ch) + 1);
//            memcpy(ch->src->operand2.op2_ch, ch->dst->operand1.op1_ch, strlen(ch->dst->operand2.op2_ch) + 1);
//        }break;
//    }
//    ch->dst->s = true;
//    ch->dst->c = false;
//
//    pthread_cond_signal(&ch->dst->server_ready);
//    pthread_mutex_unlock(&ch->dst->mtx);
//
//}

void* server_write(void *chn)
{
    channel* ch = (channel*) chn;
    pthread_mutex_lock(&ch->dst->mtx);
    while(ch->dst->c == false)
        pthread_cond_wait(&ch->dst->client_ready, &ch->dst->mtx);

    switch(ch->dst->operation)
    {
    case 1: cout << (ch->dst->result.result_int = add(ch->dst->operand1.op1_i, ch->dst->operand2.op2_i)); break;
    case 2: cout << (ch->dst->result.result_int = subtract(ch->dst->operand1.op1_i, ch->dst->operand2.op2_i)); break;
    case 3: cout << (ch->dst->result.result_int = multiply(ch->dst->operand1.op1_i, ch->dst->operand2.op2_i)); break;
    case 4: cout << (ch->dst->result.result_int = divide(ch->dst->operand1.op1_i, ch->dst->operand2.op2_i)); break;
    case 6: cout << (ch->dst->result.result_int = substr(ch->dst->operand1.op1_ch, ch->dst->operand2.op2_ch)); break;
    case 5:
    {
        concat(ch->dst->operand1.op1_ch, ch->dst->operand2.op2_ch);
        memcpy(ch->dst->result.result_char, concat(ch->dst->operand1.op1_ch, ch->dst->operand2.op2_ch), strlen(ch->src->result.result_char) + 1);

    }break;
    }
    ch->dst->s = true;
    ch->dst->c = false;
    ch->dst->new_request = false;
    pthread_cond_signal(&ch->dst->server_ready);
    pthread_mutex_unlock(&ch->dst->mtx);
}

void send_reply(shared* sh) //synchronously
{
	pthread_mutex_lock(&sh->mtx);
	while(sh->c == false)
		pthread_cond_wait(&sh->client_ready, &sh->mtx);
	if (sh->operation == 1 || sh->operation == 2)
	{
		pthread_t thread1;
		pthread_create(&thread1, NULL, calculate_math, sh);
		pthread_join(thread1, NULL);
	}
	else if (sh->operation == 3)
	{
		pthread_t thread2;
		pthread_create(&thread2, NULL, calculate_strings, sh);
		pthread_join(thread2, NULL);
	}
	sh->s = true;
	sh->c = false;
	sh->new_request = false;
	pthread_cond_signal(&sh->server_ready);
	pthread_mutex_unlock(&sh->mtx);
}


void* calculate_math(void* shm)
{
	shared* sh = (shared*)shm;
	operation_t op = sh->operation;
	cid_t id = sh->client_id;
	cout << id;
	if (id == 1)
	{
		if (op == 1)
			sh->result.result_int = add(sh->operand1.op1_i, sh->operand2.op2_i);
		else if (op == 2)
			sh->result.result_int = multiply(sh->operand1.op1_i, sh->operand2.op2_i);
		//callback(client1);
		cout << sh->result.result_int;
	}
	else if (id == 2)
	{
		if (op == 1)
			sh->result.result_int = add(sh->operand1.op1_i, sh->operand2.op2_i);
		else if (op == 2)
			sh->result.result_int = multiply(sh->operand1.op1_i, sh->operand2.op2_i);
		//callback(client2);
	}
}

void* calculate_strings(void* shm)
{
	shared* sh = (shared*)shm;
	operation_t op = sh->operation;
	cid_t id = sh->client_id;
	if (id == 1)
	{
		sh->result.result_char = concat(sh->operand1.op1_ch, sh->operand2.op2_ch);
		//callback(client1);
	}
	else if (id == 2)
	{
		sh->result.result_int = substr(sh->operand1.op1_ch, sh->operand2.op2_ch);
		//callback(client2);
	}
}

void receive_reply(channel* ch)
{
     pthread_t thread2;
     pthread_create(&thread2, NULL, client_read, ch);
     pthread_join(thread2, NULL);
}


