#ifndef _SINGLETONPROCESS_H_20190104_
#define _SINGLETONPROCESS_H_20190104_

#include <iostream>
#include <string>

class SingletonProcess
{
public:
	SingletonProcess(uint16_t port0);
	~SingletonProcess();

	bool operator()();

	std::string GetLockFileName();

private:
	int socket_fd;
	int rc;
	uint16_t port;
};

#endif
