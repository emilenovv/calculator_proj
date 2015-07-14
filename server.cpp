///
 /// server.cpp
 ///
 ///  Created on: Jul 13, 2015
 ///      Author: visteon
///


#include <cstdlib>
#include <iostream>
#include <pthread.h>

#include "ipc_defs.hpp"
#include "comm_lib.hpp"
using namespace std;

int main()
{
    shared* shmem = create_shared();
    wait_for_request(shmem);
    return 0;
}

