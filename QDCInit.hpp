#ifndef QDCInit_hpp
#define QDCInit_hpp

#include "EMApplicationInit.hpp"
#include "QDC.hpp"

// Documentation


class QDCInit : public EMApplicationInit {
	public:
		typedef QDC InitialisedClass;
		QDCInit(QDC &app, int argc, char *argv[]);
		int init();
};

#define EM_APPLICATION_INIT QDCInit

#endif
