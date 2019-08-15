#include <iostream>
#include <string>
using namespace std;

#include <boost/program_options/errors.hpp>
#include "EMApplicationInit.hpp"
using namespace EM;


// Documentation

namespace {
	// These options are referred to by value in operator()
	const char HELP[]    ="help";          const char Help='?';
	const char VER[]     ="version";
	const char USAGE[]   ="usage";
	const char ABOUT[]   ="about";
	const char EXAMPLE[] ="example";

	const char ENV[]     ="environment";   const char Env ='E';
	const char CFG[]     ="configuration"; const char Cfg ='F';

	//const char OTR[]   ="other";         const char Otr ='o';
	//const char STR[]   ="string";        const char Str ='s';
	//inline const char* concat(const char* c1, const char* c2) {return (string(c1)+c2).c_str();}


	class CheckENV {// This is to avoid the default UPPER->LOWER case conversion for environment variables
		public:
			CheckENV(string s):env(s), length(s.size()){}
			string env;
			size_t length;

			string operator()(string s) {
				if (s.substr(0,length)==env) return s.substr(length,string::npos);
				else return string();
			}
	};


};


EMApplicationInit::EMApplicationInit(EMApplication &app, int c, char **v)
	:emApp(app), argc(c), argv(v)
	,commandLineOptions("<<Command Line options>>"), options("<<General options>>")
{
	commandLineOptions.add_options()
	((string(HELP)+","+Help).c_str(),"Show all help (version, usage, about, example)")
	((string(VER)).c_str(),"Show version of this program ")
	((string(USAGE)).c_str(),"Show how to invoke this program")
	((string(ABOUT)).c_str(),"Show information about this program")
	((string(EXAMPLE)).c_str(),"Show an example invocation of this program")

	((string(ENV) +","+Env ).c_str(),PO::value< vector<string> >(),"Use prefixed environment variables")
	;

	options.add_options()
	((string(CFG) +","+Cfg ).c_str(),PO::value< vector<string> >()->composing(),"Use configuration file")
	;
	//options.add_options()
	//((string(OTR) +","+Otr ).c_str(),PO::value( &emApp.i )        ->default_value(0),"Other")
	//((string(STR) +","+Str ).c_str(),PO::value( &emApp.s )        ->default_value("x"),"String")
	//;
	//cout<<"EMApplicationInit::EMApplicationInit(EMApplication &app, int c, char **v) "<<notifyList.size()<<" "<<notifyList.capacity()<<endl;
	//cout<<"EMApplicationInit::EMApplicationInit(EMApplication &app, int c, char **v) "<<argc<<" "<<endl;
	//cout<<"EMApplicationInit::EMApplicationInit(EMApplication &app, int c, char **v) "<<commandLineOptions<<" "<<sizeof(commandLineOptions)<<endl;
	//cout<<"EMApplicationInit::EMApplicationInit(EMApplication &app, int c, char **v) "<<options<<" "<<sizeof(options)<<endl;
}
EMApplicationInit::~EMApplicationInit(){}

int EMApplicationInit::operator()() {
	commandLineOptions.add(options);

	emApp.invokedAs = string(argv[0]);

	try {
		PO::store(PO::command_line_parser(argc, argv).options(commandLineOptions).positional(positionals).run(), map);
		PO::notify(map);
	}
	catch(PO::error& e) {
		cerr << "Command line: " << e.what() << endl;
		throw EMApplicationException("Initialisation error");
	}

	bool bHelp=false, bHelpPartial=false;

	if (map.count(HELP)) {
		cout<< "Help for " << emApp.name() << " (invoked as "<<emApp.invokedAs <<")\n"
		    <<endl;
		bHelp=true;
	}

	if (bHelp || map.count(VER)) {
		if (bHelp) cout<<"Version\n-------\n";
		cout<< "Application " << emApp.name() << '\n'
		    << "Version     " << emApp.versionMajor() << '.' << emApp.versionMinor() << '.' << emApp.versionRelease() << '\n'
		    << "Author      " << emApp.author() << '\n'
		    << "Copyright   " << emApp.copyright() << '\n'
		    <<endl;
		bHelpPartial=true;
	}
	if (bHelp || map.count(USAGE)) {
		if (bHelp) cout<<"Usage\n-----\n";
		cout<< emApp.name() <<'\n'
		    <<commandLineOptions
		    <<endl;
		bHelpPartial=true;
	}
	if (bHelp || map.count(ABOUT)) {
		if (bHelp) cout<<"About\n-----\n";
		cout<< emApp.about()
		    <<endl;
		bHelpPartial=true;
	}
	if (bHelp || map.count(EXAMPLE)) {
		if (bHelp) cout<<"Example\n-------\n";
		cout<<emApp.example()
		    <<endl;
		bHelpPartial=true;
	}


	if (bHelp || bHelpPartial) return 1;

	if (map.count(ENV)) {// ENV options are not chained, unlike CFG options

		const vector<string> &v(map["environment"].as<vector <string> >());
		for (int i=0; i < v.size(); i++) {
			try {
				//PO::store(PO::parse_environment(options, v[i]), map);
				CheckENV ENV(v[i]);
				PO::store(PO::parse_environment(options, ENV), map);
			}
			catch(PO::error& e) {
				cerr << "Environment "<<v[i]<<": " << e.what() << endl;
				throw EMApplicationException("Initialisation error");
			}
		}
		PO::notify(map);
	}
	if (map.count(CFG)) {// composing() CFG option allows CFG files to be chained and possibly an infinite loop.
	// A limit is placed on the number of CFG files to avoid this.
	// Note that map.count(CFG) may change during this loop if CFG files are chained.

		const vector<string> &v(map[CFG].as<vector <string> >());
		for (int i=0; i < v.size(); i++) {
			if (i>=MAXCFG) {
				cerr<<"Too many configuration files(max="<<MAXCFG<<"). Infinite loop?"<<endl;
				throw EMApplicationException("Initialisation error");
			}
			ifstream ifs(v[i].c_str());
			if (ifs) {
				try {
					PO::store(PO::parse_config_file(ifs, options), map);
				}
				catch(PO::error& e) {
					cerr << "Configuration "<<v[i]<<": " << e.what() << endl;
					throw EMApplicationException("Initialisation error");
				}
			} else {
				cerr <<"Configuration file "<<v[i]<<" not found."<<endl;
				throw EMApplicationException("Initialisation error");
			}
		}
		PO::notify(map);
	}
//cout <<"About to notify init's"<<endl;
	vector<EMApplicationInit*>::const_iterator i=notifyList.begin();
	int ret=0;
	for(i; i != notifyList.end(); i++) {
		if ( (ret=(*i)->init()) ) break;
	}
//cout <<"Notify init's complete"<<endl;

	return ret;
};



#if 0

#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>
using namespace std;

#include "EMApplicationInit.hpp"
using namespace EM;
namespace {
	// These options are referred to by value in operator()
	const char HELP[]  ="help";
	const char VER[]   ="version";
	const char ENV[]   ="environment";
	const char PRI[]   ="priority";
	const char CFG[]   ="configuration";
	const char DAEMON[]="daemon";
	inline const char* concat(const char* c1, const char* c2) {return (string(c1)+c2).c_str();}
};

EMApplicationInit::EMApplicationInit(EMApplication &theApplication)
	:commandLineOptions("<<Command Line options>>"), options("<<General options>>")
	,app_(theApplication)
{
	commandLineOptions.add_options()
	(concat(HELP  ,",?"),"Show this help message")
	(concat(VER   ,",V"),"Show program version")
	;
	options.add_options()
	(concat(ENV   ,",E"),PO::value< vector<string> >(),"Use prefixed environment variables")
	(concat(CFG   ,",F"),PO::value< vector<string> >()->composing(),"Use configuration file")

	(concat(PRI   ,",P"),PO::value<int>(),"Priority")
	(concat(DAEMON,",D"),PO::bool_switch(),"Make the process a daemon")

	("verbose,v",PO::value(&app_.verbose_)->default_value(0),"Level of messages")
	("size,Z",PO::value(&app_.logFileSize_)->default_value(0),"Maximum log file size")
	("log,L",PO::value(&app_.logFileHint_)->default_value(app_.program_+".log")
        	,"Log file name hint(tail Day.log uses {Mon..Sun}.log)")
	;
}

EMApplicationInit::~EMApplicationInit()
{
}

const int MAXCFG = 3;

int EMApplicationInit::operator()(int argc, char *argv[])
{
	// positionals.add("positional",-1); // add positional parameters at derived levels
	commandLineOptions.add(options);

	PO::store(PO::command_line_parser(argc, argv).options(commandLineOptions).positional(positionals).run(), map);
	PO::notify(map);

	if (map.count(HELP)) {
		cout << "Usage : "<<app_.program_<<" [options]"<<endl<<commandLineOptions<<endl;
		return 1;
	}
	if (map.count(VER)) {
		cout<<"Application:"<<app_.program_ <<" Version:"<< app_.version_ <<" Author:"<< app_.author_ <<endl;
		return 1;
	}


	if (map.count(ENV)) {

	// ENV options are not chained, unlike CFG options

		const vector<string> &v(map["environment"].as<vector <string> >());
		for (int i=0; i < v.size(); i++) {
			PO::store(PO::parse_environment(options, v[i]), map);
			cerr <<"environment="<<v[i]<<" : read"<<endl;
		}
		PO::notify(map);
	}

	if (map.count(CFG)) {

	// composing() CFG option allows CFG files to be chained and possibly an infinite loop.
	// A limit is placed on the number of CFG files to avoid this.
	// Note that map.count(CFG) may change during this loop if CFG files are chained.

		const vector<string> &v(map["configuration"].as<vector <string> >());
		for (int i=0; i < v.size(); i++) {
			cerr <<"configuration="<<v[i]<<" : ";
			if (i>=MAXCFG) {
				cerr<<"Too many configuration files(max="<<MAXCFG<<"). Infinite loop?"<<endl;
				return 1;
			}
			ifstream ifs(v[i].c_str());
			if (ifs) {
				PO::store(PO::parse_config_file(ifs, options), map);
				cerr <<"read."<<endl;
			} else {
				cerr <<"could not be found."<<endl;
				return 1;
			}
		}
		PO::notify(map);
	}

	if (map.count(PRI)) {
		cout <<"Prority!"<<map["priority"].as<int>()<<endl;
	}

	if (map[DAEMON].as<bool>()) {
		cout <<"Daemon!"<<endl;
	}

	{	app_.logMethod_ = EMApplication::SizedFile;

		const string Daylog = "Day.log";
		if (app_.logFileHint_.rfind(Daylog)==(app_.logFileHint_.size()-Daylog.size()))
			app_.logMethod_ = EMApplication::DailyFile;
	}
	return 0;
}

#endif
