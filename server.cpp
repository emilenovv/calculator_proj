///
 /// server.cpp
 ///
 ///  Created on: Jul 10, 2015
 ///      Author: visteon
///


#include <cstdlib>
#include <iostream>
#include <pthread.h>
#include "libcomm.h"
using namespace std;
int main()
{
	//function_needing_callback();
    shared* shmem = create_shared();
    channel* ch = new channel;
    ch->dst = shmem;
    ch->src = new courier;
    while(true)
    {
    	if(ch->dst->new_request)
    	{
    		send_reply(shmem);
    		//break;
    		cout << "111";
    		ch->dst->new_request = false;
    		//break;
    	}
    }

    return 0;
}
