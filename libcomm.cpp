///
/// libcomm.cpp
///
///  Created on: Jul 13, 2015
///      Author: visteon
///


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
#include "libcomm.h"
using namespace std;
/////////////////////////////To be tested

bool first_time = true;

pthread_t t;

callback_t callb;

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
            pthread_cond_wait(&cab->shmem->server_ready, &cab->shmem->mtx);
        cab->cb(cab->shmem);
        cab->shmem->answer_done[cab->cid - 1] = false;
        pthread_cond_signal(&cab->shmem->client_done[cab->cid - 1]);
        pthread_mutex_unlock(&cab->shmem->mtx);
    }
}


/////////////////////////////////////////
static queue<courier*> buffer;

void* add_request(void* c)
{
    courier* cr = (courier*)c;
    buffer.push(cr);
    cout << "Added buffer size: " << buffer.size();
}

int add(int a, int b)
{
    sleep(6);
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
    sem_t *sem = sem_open("/mysem", O_CREAT, 0644, 0);
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

    shmem->s = false;
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
    //sem_post(sem);
    while(!buffer.empty())
    {
        pthread_mutex_lock(&sh->mtx);
        while(sh->s == false && !first_time)
            pthread_cond_wait(&sh->server_ready, &sh->mtx);
        courier* cr = buffer.front();
        buffer.pop();
        sh->operation = cr->operation;
        sh->client_id = cr->client_id;
        //THIS MUST BE REFACT0RED!!!!!///////////////////
        switch(cr->operation)
        {
        case 1:
        case 2:
        case 3:
        case 4:
        {
            sh->operand1.op1_i = cr->operand1.op1_i;
            sh->operand2.op2_i = cr->operand2.op2_i;
        }break;
        case 5:
        case 6:
        {
            memcpy(sh->operand1.op1_ch, cr->operand1.op1_ch, strlen(cr->operand1.op1_ch) + 1);
            memcpy(sh->operand2.op2_ch, cr->operand1.op1_ch, strlen(cr->operand2.op2_ch) + 1);
        }break;
        }
        ////////////////////////////////////////////////
        sh->s = false;
        sh->new_request = true;
        sh->client_flag[sh->client_id - 1] = true;
        cout << "Client flag " <<  boolalpha << sh->client_flag[1] << endl;
        first_time = false;
        pthread_cond_signal(&sh->client_done[sh->client_id - 1]);
//        while(sh->s == false)
//            pthread_cond_wait(&sh->server_ready, &sh->mtx);
        pthread_mutex_unlock(&sh->mtx);
    }
}

//void* client_read(void* chm)
//{
//    channel* ch = (channel *)chm;
//    pthread_mutex_lock(&ch->dst->mtx);
//    while(ch->dst->s == false)
//        pthread_cond_wait(&ch->dst->server_ready, &ch->dst->mtx);
//    ch->dst->s = false;
//    ch->dst->c = true;
//    pthread_cond_signal(&ch->dst->client_ready);
//    pthread_mutex_unlock(&ch->dst->mtx);
//}

void send_request(channel* ch)
{
    pthread_t thread1, t1;
    pthread_create(&thread1, NULL, add_request, ch->src);
    pthread_join(thread1, NULL);
    pthread_create(&t1, NULL, client_write, ch->dst);
}

void send_request_wrapper(shared* sh, operand1_t a, operand2_t b, operation_t op, cid_t id)
{
    channel* ch = new channel;
    ch->dst = sh;
    ch->src = new courier;
    cout << "oper" << op;
    if (op == 1 || op == 2)
    {
        ch->src->operand1.op1_i = a.op1_i;
        ch->src->operand2.op2_i = b.op2_i;
    }
    else if (op == 3)
    {
        memcpy(ch->src->operand1.op1_ch, a.op1_ch, sizeof(a.op1_ch) + 1);
        memcpy(ch->src->operand2.op2_ch, b.op2_ch, sizeof(b.op2_ch) + 1);
    }
    ch->src->operation = op;
    ch->src->client_id = id;
    send_request(ch);
}


void send_reply_wrapper(shared* sh)
{
    pthread_t thread1;
    pthread_create(&thread1, NULL, send_reply, sh);
    pthread_join(thread1, NULL);
}

void* send_reply(void* shm) //synchronously
{
    shared* sh = (shared*)shm;
    pthread_t thread1;
    pthread_mutex_lock(&sh->mtx);

    while(sh->client_flag[0] == false && sh->client_flag[1] == false)
    {
        if (sh->client_flag[0])
            pthread_cond_wait(&sh->client_done[0], &sh->mtx);
        else
            pthread_cond_wait(&sh->client_done[1], &sh->mtx);
    }
    cout << "Sending to client << " << sh->client_id << endl;
    if (sh->operation == 1 || sh->operation == 2)
    {
        pthread_create(&thread1, NULL, calculate_math, sh);
        pthread_join(thread1, NULL);
        // sh->s = true;
        // sh->c = false;
        // sh->new_request = false;
        // pthread_cond_signal(&sh->server_ready);
        // pthread_mutex_unlock(&sh->mtx);
    }
    else if (sh->operation == 3)
    {
        pthread_t thread2;
        pthread_create(&thread2, NULL, calculate_strings, sh);
        pthread_join(thread2, NULL);
    }
    sh->s = true;
    sh->client_flag[sh->client_id - 1] = false;
    sh->new_request = false;
    sh->answer_done[sh->client_id - 1] = true;
    cout << "Calculated!\n";
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
        cout << "Result: " << sh->result.result_int;
    }
    else if (id == 2)
    {
        if (op == 1)
            sh->result.result_int = subtract(sh->operand1.op1_i, sh->operand2.op2_i);
        else if (op == 2)
            sh->result.result_int = divide(sh->operand1.op1_i, sh->operand2.op2_i);
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
    }
    else if (id == 2)
    {
        sh->result.result_int = substr(sh->operand1.op1_ch, sh->operand2.op2_ch);
    }
}

//void receive_reply(channel* ch)
//{
//    pthread_t thread2;
//    pthread_create(&thread2, NULL, client_read, ch);
//    pthread_join(thread2, NULL);
//}

void* wait_for_request_task(void* sh)
{
    shared* shmem = (shared*)sh;
    while(1)
    {
        if (shmem->new_request)
        {
            cout << "Receving request from client: " << shmem->client_id << " arguments: " << shmem->operand1.op1_i << " " << shmem->operand2.op2_i << endl;
            send_reply_wrapper(shmem);
        }
    }
}

void wait_for_request(shared* sh)
{
    pthread_t t;
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
            cout << sh->result.result_char << endl;
        }
    }
}


//void recieve_request(channel* ch)
//{
// pthread_t thread;
// pthread_create(&thread, NULL, server_read, ch);
// pthread_join(thread, NULL);
//}
//void* server_read(void* chn)
//{
// channel* ch = (channel*)chn;
// pthread_mutex_lock(&ch->dst->mtx);
// while(ch->dst->c == false)
// pthread_cond_wait(&ch->dst->client_ready, &ch->dst->mtx);
//
// switch(ch->dst->operation)
// {
// case 1:
// case 2:
// case 3:
// case 4:
// {
// cout << "dadada";
// ch->src->operand1.op1_i = ch->dst->operand1.op1_i;
// ch->src->operand2.op2_i = ch->dst->operand2.op2_i;
// cout << ch->src->operand1.op1_i << endl;
// cout << ch->src->operand2.op2_i << endl;
//
// }break;
// case 5:
// case 6:
// {
// memcpy(ch->src->operand1.op1_ch, ch->dst->operand1.op1_ch, strlen(ch->dst->operand1.op1_ch) + 1);
// memcpy(ch->src->operand2.op2_ch, ch->dst->operand1.op1_ch, strlen(ch->dst->operand2.op2_ch) + 1);
// }break;
// }
// ch->dst->s = true;
// ch->dst->c = false;
//
// pthread_cond_signal(&ch->dst->server_ready);
// pthread_mutex_unlock(&ch->dst->mtx);
//
//}
//void* server_write(void *chn)
//{
// channel* ch = (channel*) chn;
// pthread_mutex_lock(&ch->dst->mtx);
// while(ch->dst->c == false)
// pthread_cond_wait(&ch->dst->client_ready, &ch->dst->mtx);
//
// switch(ch->dst->operation)
// {
// case 1: cout << (ch->dst->result.result_int = add(ch->dst->operand1.op1_i, ch->dst->operand2.op2_i)); break;
// case 2: cout << (ch->dst->result.result_int = subtract(ch->dst->operand1.op1_i, ch->dst->operand2.op2_i)); break;
// case 3: cout << (ch->dst->result.result_int = multiply(ch->dst->operand1.op1_i, ch->dst->operand2.op2_i)); break;
// case 4: cout << (ch->dst->result.result_int = divide(ch->dst->operand1.op1_i, ch->dst->operand2.op2_i)); break;
// case 6: cout << (ch->dst->result.result_int = substr(ch->dst->operand1.op1_ch, ch->dst->operand2.op2_ch)); break;
// case 5:
// {
// concat(ch->dst->operand1.op1_ch, ch->dst->operand2.op2_ch);
// memcpy(ch->dst->result.result_char, concat(ch->dst->operand1.op1_ch, ch->dst->operand2.op2_ch), strlen(ch->src->result.result_char) + 1);
//
// }break;
// }
// ch->dst->s = true;
// ch->dst->c = false;
// ch->dst->new_request = false;
// pthread_cond_signal(&ch->dst->server_ready);
// pthread_mutex_unlock(&ch->dst->mtx);
//}
