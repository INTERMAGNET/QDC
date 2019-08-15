#ifndef EMApplicationInit_hpp
#define EMApplicationInit_hpp

#include <iostream>
#include <vector>
#include <fstream>

#include "EMApplication.hpp"

#include <boost/program_options.hpp>
namespace  PO=boost::program_options;

// Documentation

namespace EM {
class EMApplicationInit {
	#define MAXCFG 16

	public:
		EMApplicationInit(EMApplication &app, int argc, char *argv[]);
		virtual ~EMApplicationInit();
		int operator()();

		PO::options_description commandLineOptions;
		PO::options_description options;
		PO::positional_options_description positionals;
		PO::variables_map map;

	protected:
		EMApplication &emApp;
		int argc;
		char **argv;

		std::vector<EMApplicationInit *> notifyList;
		virtual int init() {return 0;};
		void notify(){notifyList.push_back(this);} //must be called by subclass::ctor() if subclass::init() defined/needed

};


} // namespcace EM

// The MACRO MAIN standardises the way Applications are initialised
// and passes control to the application main().

#define MAIN \
static EM_APPLICATION_INIT::InitialisedClass app;\
\
int main(int argc, char *argv[]) { \
	try { \
		EM_APPLICATION_INIT init(app, argc, argv); \
		int i; \
		if ( (i=init()) ) {return i;} \
	} \
	catch (EM::EMApplicationException& e) { \
		std::cerr << e.what() << std::endl; \
		return EXIT_FAILURE; \
	} \
\
	try { \
		return app.main(); \
	} \
	catch (EM::EMApplicationException& e) { \
		std::cerr << e.what() << std::endl; \
		return EXIT_FAILURE; \
	} \
	return EXIT_SUCCESS; \
}

#endif

