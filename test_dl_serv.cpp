/*
 * test_dl_serv.cpp
 *
 *  Created on: Jul 12, 2015
 *      Author: emil
 */


#include "libcomm.h"
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

int main()
{
	function_needing_callback();
	return 0;
}
