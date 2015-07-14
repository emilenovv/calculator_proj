///
/// libcomm.cpp
///
///  Created on: Jul 13, 2015
///      Author: visteon
///

#include <cstdlib>
#include <semaphore.h>
#include <pthread.h>
#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <queue>

#include "comm_lib.hpp"
#include "ipc_defs.hpp"

using namespace std;

bool first_time = true;

pthread_t t;
callback_t callb;
queue<courier*> buffer;

void register_callback(cb_t cb, shared* sh, cid_t id)
{
    callb.cb = cb;
    callb.shmem = sh;
    callb.cid = id;
    pthread_create(&t, NULL, wait_answer, (void*)&callb);
}

void* wait_answer(void* sh)
{
    callback_t* cab = (callback_t*)sh;
    while(true)
    {
        pthread_mutex_lock(&cab->shmem->mtx);
        while(cab->shmem->answer_done[cab->cid - 1] == false)
            pthread_cond_wait(&cab->shmem->answer_calculated[cab->cid - 1], &cab->shmem->mtx);
        cab->cb(cab->shmem);
        if (cab->shmem->operation == 1)
            sem_post(&cab->shmem->sem1);
        cab->shmem->answer_done[cab->cid - 1] = false;
        pthread_mutex_unlock(&cab->shmem->mtx);
    }
    return NULL;
}

void* add_request(void* c)
{
    courier* cr = (courier*)c;
    buffer.push(cr);
    return NULL;
}

int add(int a, int b)
{
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
    bool flag = true;
    int current_index = 0;
    for (int i = 0; i < strlen(haystack); ++i)
    {
        if (haystack[i] == needle[current_index])
        {
            for (int j = i + 1; j < strlen(needle); ++j)
            {
                if (haystack[j] != needle[++current_index])
                    flag = false;
            }
            if (flag)
                return i;
        }
    }
    return NULL;
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
    pthread_cond_init(&shmem->client_done[0], &condAttr2);

    pthread_condattr_t condAttr3;
    pthread_condattr_setpshared(&condAttr3, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(&shmem->client_done[1], &condAttr3);

    pthread_condattr_t condAttr4;
    pthread_condattr_setpshared(&condAttr4, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(&shmem->answer_calculated[0], &condAttr4);

    pthread_condattr_t condAttr5;
    pthread_condattr_setpshared(&condAttr5, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(&shmem->answer_calculated[1], &condAttr5);

    sem_init(&shmem->sem1, 1, 0);

    shmem->server_flag = false;
    shmem->client_flag[0] = false;
    shmem->client_flag[1] = false;
    shmem->new_request = false;
    shmem->answer_done[0] = false;
    shmem->answer_done[1] = false;
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

void* client_write(void* shm)
{
    shared* sh = (shared*)shm;
    pthread_mutex_lock(&sh->mtx);
    while(sh->server_flag == false && !first_time)
        pthread_cond_wait(&sh->server_ready, &sh->mtx);
    courier* cr = buffer.front();
    buffer.pop();
    sh->operation = cr->operation;
    sh->client_id = cr->client_id;
    if (cr->operation == 1 || cr->operation == 2)
    {
        sh->operand1.op1_i = cr->operand1.op1_i;
        sh->operand2.op2_i = cr->operand2.op2_i;
    }
    else if (cr->operation == 3)
    {
        strcpy(sh->operand1.op1_ch, cr->operand1.op1_ch);
        strcpy(sh->operand2.op2_ch, cr->operand2.op2_ch);
    }
    sh->server_flag = false;
    sh->new_request = true;
    sh->client_flag[sh->client_id - 1] = true;
    first_time = false;
    pthread_cond_signal(&sh->client_done[sh->client_id - 1]);
    pthread_mutex_unlock(&sh->mtx);
    return NULL;
}

void send_request(channel* ch)
{
    pthread_t add, write;
    pthread_create(&add, NULL, add_request, ch->src);
    pthread_join(add, NULL);
    pthread_create(&write, NULL, client_write, ch->dst);
}

void send_request_wrapper(shared* sh, operand1_t a, operand2_t b, operation_t op, cid_t id)
{
    channel* ch = new channel;
    ch->dst = sh;
    ch->src = new courier;
    if (op == 1 || op == 2)
    {
        ch->src->operand1.op1_i = a.op1_i;
        ch->src->operand2.op2_i = b.op2_i;
    }
    else if (op == 3)
    {
        strcpy(ch->src->operand1.op1_ch, a.op1_ch);
        strcpy(ch->src->operand2.op2_ch, b.op2_ch);
    }
    ch->src->operation = op;
    ch->src->client_id = id;
    send_request(ch);
    if (op == 1)
        sem_wait(&sh->sem1);
}


void send_reply_wrapper(shared* sh)
{
    pthread_create(&t, NULL, send_reply, sh);
    pthread_join(t, NULL);
}

void* send_reply(void* shm)
{
    shared* sh = (shared*)shm;
    pthread_mutex_lock(&sh->mtx);
    while(!sh->client_flag[sh->client_id - 1])
    {
        pthread_cond_wait(&sh->client_done[sh->client_id - 1], &sh->mtx);
    }
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
    sh->server_flag = true;
    sh->client_flag[sh->client_id - 1] = false;
    sh->new_request = false;
    sh->answer_done[sh->client_id - 1] = true;
    cout << "Sending to client_id : " << sh->client_id << endl;
    pthread_cond_signal(&sh->answer_calculated[sh->client_id - 1]);
    pthread_cond_signal(&sh->server_ready);
    pthread_mutex_unlock(&sh->mtx);
    return NULL;
}

void* calculate_math(void* shm)
{
    shared* sh = (shared*)shm;
    operation_t op = sh->operation;
    cid_t id = sh->client_id;
    if (id == 1)
    {
        if (op == 1)
            sh->result.result_int = add(sh->operand1.op1_i, sh->operand2.op2_i);
        else if (op == 2)
            sh->result.result_int = multiply(sh->operand1.op1_i, sh->operand2.op2_i);
        cout << "Result: " << sh->result.result_int << endl;
    }
    else if (id == 2)
    {
        if (op == 1)
            sh->result.result_int = subtract(sh->operand1.op1_i, sh->operand2.op2_i);
        else if (op == 2)
            sh->result.result_int = divide(sh->operand1.op1_i, sh->operand2.op2_i);
        cout << "Result: " << sh->result.result_int << endl;
    }
    return NULL;
}

void* calculate_strings(void* shm)
{
    shared* sh = (shared*)shm;
    cid_t id = sh->client_id;
    if (id == 1)
    {
        strcpy(sh->result.result_char, concat(sh->operand1.op1_ch, sh->operand2.op2_ch));
        cout << "Result: " << sh->result.result_char << endl;
    }
    else if (id == 2)
    {
        sh->result.result_int = substr(sh->operand1.op1_ch, sh->operand2.op2_ch);
        cout << "Result: " << sh->result.result_int << endl;
    }
    return NULL;
}

void* wait_for_request_task(void* sh)
{
    shared* shmem = (shared*)sh;
    while(1)
    {
        if (shmem->new_request)
        {
            cout << "Receving request from client: " << shmem->client_id << endl;
            send_reply_wrapper(shmem);
        }
    }
    return NULL;
}

void wait_for_request(shared* sh)
{
    pthread_create(&t, NULL, wait_for_request_task, sh);
    pthread_join(t, NULL);
}

void print_result(shared* sh)
{
    if (sh->client_id == 1)
    {
        if (sh->operation == 1)
        {
            cout << "Result from (" << sh->operand1.op1_i << "+" << sh->operand2.op2_i << ") is ";
            cout << sh->result.result_int << endl;
        }
        else if (sh->operation == 2)
        {
            cout << "Result from (" << sh->operand1.op1_i << "*" << sh->operand2.op2_i << ") is ";
            cout << sh->result.result_int << endl;
        }
        else if (sh->operation == 3)
        {
            cout << "Result from concat(" << sh->operand1.op1_ch << ", " << sh->operand2.op2_ch << ") is ";
            cout << sh->result.result_char << endl;
        }
    }
    else if (sh->client_id == 2)
    {
        if (sh->operation == 1)
        {
            cout << "Result from (" << sh->operand1.op1_i << "-" << sh->operand2.op2_i << ") is ";
            cout << sh->result.result_int << endl;
        }
        else if (sh->operation == 2)
        {
            cout << "Result from (" << sh->operand1.op1_i << "/" << sh->operand2.op2_i << ") is ";
            cout << sh->result.result_int << endl;
        }
        else if (sh->operation == 3)
        {
            cout << "Result from substr(" << sh->operand1.op1_ch << ", " << sh->operand2.op2_ch << ") is ";
            cout << sh->result.result_int << endl;
        }
    }
}
