#include <iostream>
#include "libcomm.h"
#include <dlfcn.h>

using namespace std;

void prnt()
{
	cout << "I am the BEST\n";
}

int main()
{
	void (*register_function)(void(*)());
	void *handle = dlopen("libcomm.h", RTLD_LAZY);
	register_function = dlsym(handle, "register_function");
	register_function(prnt);
	dlclose(handle);
	return 0;
}
