///
 /// server.cpp
 ///
 ///  Created on: Jul 13, 2015
 ///      Author: visteon
///


#include <cstdlib>
#include <iostream>
#include <pthread.h>
#include "libcomm.h"
using namespace std;

pthread_t t;

int main()
{
    shared* shmem = create_shared();
//    channel* ch = new channel;
//    ch->dst = shmem;
//    ch->src = new courier;
    wait_for_request(shmem);
    return 0;
}
