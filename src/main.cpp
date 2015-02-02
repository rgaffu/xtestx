/**
******************************************************************************
* @file    main.cpp
*****************************************************************************/

//////////////////////////////////////////////////////////////////////////////
// I N C L U D E S                                                          //
//////////////////////////////////////////////////////////////////////////////
/*#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h> 
#include <time.h> 
#include <syslog.h>
#include <sched.h>
#include <dlfcn.h> 
#include <execinfo.h>
#include <cxxabi.h>*/

#include "version.h"
#include "appl.hpp"

#include <logging.hpp>
_INITIALIZE_EASYLOGGINGPP

//////////////////////////////////////////////////////////////////////////////
// M A C R O S    D E F I N I T I O N S                                     //
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// L O C A L S    D E F I N I T I O N S                                     //
//////////////////////////////////////////////////////////////////////////////
static APPL *theAppl = 0;
sig_atomic_t interrupted;

//////////////////////////////////////////////////////////////////////////////
// F U N C T I O N S   P R O T O T Y P I N G                                //
//////////////////////////////////////////////////////////////////////////////
static int splash(int argc, char *argv[]);
static void configureLoggers(int argc, char *argv[]);
static void setup_logger(const char *logname, const char *cfgname, easyloggingpp::Configurations *gconf);
static void install_termination_handler(void);
static void termination_handler(int signum);
static void exitFunction(void);
static void functionCyclic();

//////////////////////////////////////////////////////////////////////////////
// F U N C T I O N S                                                        //
//////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
	if (splash(argc, argv) < 0) {
		return -1;
	}
	configureLoggers(argc, argv);
	install_termination_handler();
	atexit(exitFunction);
	INF() << get_description_string() << " started";

	theAppl = new APPL();
	if (theAppl == NULL){
		FATAL() << "Cannot create main application class";
		return EXIT_FAILURE;
	}
	theAppl->init();
	
	int ii = 0;
	interrupted = 0;
	while(!interrupted) {
	    theAppl->run();
		functionCyclic();
	    sched_yield();

		if (++ii >= 1000000L) {
			//break;
	    }
	    if ((ii % 1000000L) == 0) {
			printf(".");
			fflush(stdout); 
		}
	}
	
	if (theAppl != 0)
		delete theAppl;
	
	return EXIT_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////
static void functionCyclic()
{
	static time_t prev;
	
	time_t now = time(0);
	if (now != prev) {
		prev = now;
		struct tm tm_val;
		gmtime_r( &now, &tm_val );
		//printf("\ntime %ld is: %s (gmtime)", now, asctime(&tm_val) );
		//printf("[%02d%02d%02d %02d%02d%02d]\n", tm_val.tm_mday, tm_val.tm_mon+1, tm_val.tm_year%100, tm_val.tm_hour, tm_val.tm_min, tm_val.tm_sec);
	}
}

/////////////////////////////////////////////////////////////////////////////
static int splash(int argc, char *argv[])
{
	printf("%s %s [%s]\n", get_description_string(), get_version_string(), get_build_date());
	return 0;
	printf("\n");
	printf("%s\n", &argv[0][2]);
	printf("Usage: %s [OPTION]", argv[0]);
	printf("\n");
	printf("-m, --memshared            execute without expecting shared memory already created\n");
	printf("\n");
	
	for(int ii = 1; ii < argc; ii++)
	{
		//bOptionMem = ((strcmp(argv[ii],"-m") == 0) || (strcmp(argv[ii],"--memshared") == 0));
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
static void configureLoggers(int argc, char *argv[])
{
#define CONFIG_DIR   			"config/"
#define FILENAME_LOGCONFIG    	(const char *)("log.cfg")
	
	_START_EASYLOGGINGPP(argc, argv);
	easyloggingpp::Configurations * generic_conf = new easyloggingpp::Configurations();
	easyloggingpp::Logger* defaultLogger = easyloggingpp::Loggers::getLogger(DEFAULT_LOG_ID);
	if (generic_conf->parseFromFile(FILENAME_LOGCONFIG) == false) {
		delete generic_conf;
		generic_conf = 0;
	} else {
		defaultLogger->configure(*generic_conf);
	}
	
	setup_logger("timers", CONFIG_DIR"log/log_tmr.cfg", generic_conf);
	
	/*setup_logger("memory", CONFIG_DIR"log/log_mem.cfg", generic_conf);
	setup_logger("conf",   CONFIG_DIR"log/log_cfg.cfg", generic_conf);

	setup_logger("cmdsrv", CONFIG_DIR"log/log_cmd.cfg", generic_conf);
	setup_logger("udpsrv", CONFIG_DIR"log/log_udp.cfg", generic_conf);*/

	if (generic_conf)
		delete generic_conf;
}
static void setup_logger(const char *logname, const char *cfgname, easyloggingpp::Configurations *gconf)
{
	easyloggingpp::Configurations conf;
	easyloggingpp::Logger* logger = easyloggingpp::Loggers::getLogger(logname);
	if (conf.parseFromFile(cfgname, gconf))
		logger->configure(conf);
	else
	if (gconf)
		logger->configure(*gconf);
}

/////////////////////////////////////////////////////////////////////////////
static void install_termination_handler(void)
{
	struct sigaction new_action, old_action;

	sigset_t sigset;
	sigfillset(&sigset);
	
	if (sigprocmask(SIG_SETMASK, &sigset, NULL) == -1) {
		fprintf(stderr, "!!! %s ", __func__); 
		perror("sigprocmask error");
		exit(-1);
	}

	/* Set up the structure to specify the new action. */
	new_action.sa_handler = termination_handler;	
	sigemptyset (&new_action.sa_mask);
	new_action.sa_flags = 0;

	sigaction (SIGINT, NULL, &old_action);
	if (old_action.sa_handler != SIG_IGN)
		sigaction (SIGINT, &new_action, NULL);
	
	sigaction (SIGHUP, NULL, &old_action);
	if (old_action.sa_handler != SIG_IGN)
		sigaction (SIGHUP, &new_action, NULL);
	
	sigaction (SIGTERM, NULL, &old_action);
	if (old_action.sa_handler != SIG_IGN)
		sigaction (SIGTERM, &new_action, NULL);

	sigdelset(&sigset, SIGINT);
	sigdelset(&sigset, SIGHUP);
	sigdelset(&sigset, SIGTERM);
	if (sigprocmask(SIG_SETMASK, &sigset, NULL) == -1) {
		fprintf(stderr, "!!! %s ", __func__); 
		perror("sigprocmask error");
		exit(-1);
	}
}

/////////////////////////////////////////////////////////////////////////////
static void termination_handler(int signum)
{
 	if (signum == SIGTERM) printf("Terminated\n");
 	if (signum == SIGINT)  printf("Interrupted\n");
 	if (signum == SIGHUP)  printf("Hung up\n");
	interrupted = 1;
}

/////////////////////////////////////////////////////////////////////////////
static void exitFunction(void)
{
	INF() << get_description_string() << " ended";
}


#if 0
// TODO valutare implementazione per gestione crash
/////////////////////////////////////////////////////////////////////////////
void crash_handler(int signum)
{
	using namespace abi;
	enum{ MAX_DEPTH = 10 }; 
	void *trace[MAX_DEPTH];
	Dl_info dlinfo;

	int status;
	const char *symname;
	char *demangled;

	int trace_size = backtrace(trace, MAX_DEPTH);

	printf("* CALL STACK *\n");
	printf("compiled %s - %s\n",__DATE__, __TIME__);

	for (int ii = 0; ii < trace_size; ++ii) {  
		if (!dladdr(trace[ii], &dlinfo))
			continue;

		symname = dlinfo.dli_sname;

		demangled = __cxa_demangle(symname, NULL, 0, &status);
		if(status == 0 && demangled)
			symname = demangled;

		printf("object: %s, function: %s + %d\n", dlinfo.dli_fname, symname, (unsigned int)(((unsigned long)trace[ii]) - ((unsigned long)dlinfo.dli_saddr)));
		if (demangled)
			free(demangled);
	}  
}

/////////////////////////////////////////////////////////////////////////////
void install_crash_handler(void)
{
	signal(SIGABRT, crash_handler);
	signal(SIGSEGV, crash_handler);
	signal(SIGILL,  crash_handler);
	signal(SIGFPE,  crash_handler);	
}
#endif
