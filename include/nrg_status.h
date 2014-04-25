#ifndef NRG_STATUS_H
#define NRG_STATUS_H
#include <errno.h>
#include <string.h>
#include <type_traits>

namespace nrg {

struct Status {

	Status() : Status(true){}

	Status(bool ok, int err = 0)
	: type(ok ? Ok : SystemError)
	, sys_errno(ok ? 0 : err)
	, desc(ok ? "No error." : "System call returned error."){
	
	}

	Status(const char* custom)
	: type(InternalError)
	, sys_errno(0)
	, desc(custom){
	
	}
	
	enum : int {
		Ok,
		InternalError,
		SystemError
	} type;
	int sys_errno;
	const char* desc;
	
	operator bool() const {
		return type == Ok;
	}
};

struct StatusOK : Status {
	StatusOK() : Status(true){}
};

struct StatusErr : Status {
	StatusErr(int e = errno) : Status(false, e){}
};

// portability wrapper for both GNU and XSI strerror_r versions.
static inline const char* strerr_r(int eno, char* buf, size_t sz){
	auto r = strerror_r(eno, buf, sz);
	
	if(std::is_same<decltype(r), int>::value){
		return buf;
	} else {
		return r;
	}
}

}

#endif
