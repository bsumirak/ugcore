/*
 * ug.cpp
 *
 *  Created on: 10.11.2010
 *      Authors: marscher, iheppner, sreiter
 */

#include <stack>
#include <cstdlib>
#include "ug.h"
#include "common/log.h"
#include "common/util/path_provider.h"
#include "ug_script/ug_script.h"
#include "ug_bridge/ug_bridge.h"

#ifdef UG_PARALLEL
#include "pcl/pcl.h"
#endif

/** Tells whether profile-output is desired on exit.*/
//	only visible in this file!
static bool outputProfileStats = false;

namespace ug {

/**
 *  init app, script and data paths
 */
static bool InitPaths(const char* argv0) {
	//The method currently only works if the path is explicitly specified
	//during startup or if UG4_ROOT is defined.

	//	extract the application path.
	// UG_LOG("argv[0]: " << argv0 << endl);

	char* ug4Root = getenv("UG4_ROOT");
	if(ug4Root){
		std::string strRoot = ug4Root;
		PathProvider::set_path(ROOT_PATH, strRoot);
		PathProvider::set_path(APP_PATH, strRoot + "/bin");
		PathProvider::set_path(SCRIPT_PATH, strRoot + "/scripts");
		PathProvider::set_path(DATA_PATH, strRoot + "/data");
	}
	else{
		std::string tPath = argv0;
		size_t pos = tPath.find_last_of("/");
		if (pos == std::string::npos)
			pos = tPath.find_last_of("\\\\");

		if (pos != std::string::npos)
			tPath = tPath.substr(0, pos);
		else
			tPath = ".";

		PathProvider::set_path(APP_PATH, tPath);
		PathProvider::set_path(ROOT_PATH, tPath + "/..");
		PathProvider::set_path(SCRIPT_PATH, tPath + "/../scripts");
		PathProvider::set_path(DATA_PATH, tPath + "/../data");
	}

//	log the paths
	UG_DLOG(MAIN, 0, "app path set to: " << PathProvider::get_path(APP_PATH) <<
			std::endl << "script path set to: " << PathProvider::get_path(SCRIPT_PATH) <<
			std::endl << "data path set to: " << PathProvider::get_path(DATA_PATH) <<
			std::endl);

	if(!script::FileExists(PathProvider::get_path(APP_PATH).c_str()) ||
	   !script::FileExists(PathProvider::get_path(SCRIPT_PATH).c_str()) ||
	   !script::FileExists(PathProvider::get_path(DATA_PATH).c_str()))
	{
		UG_LOG("WARNING: paths were not initialized correctly.\n");
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////
///	initializes ug
/**	This method should be called at the beginning of main(...).
 *	If ug has been compiled for parallel use (UG_PARALLEL is defined)
 *	then this method will internally call pcl::Init.
 */
//int UGInit(int argc, char* argv[], int parallelOutputProcRank)
int UGInit(int *argcp, char ***argvp, int parallelOutputProcRank) {
	//	make sure that things are only initialized once
	// todo: afaik static in d methods is per-cpp-file
	static bool firstCall = true;
	if (firstCall) {
		firstCall = false;
#ifdef UG_PARALLEL
//		pcl::Init(argc, argv);
		pcl::Init(argcp, argvp);
		pcl::SetOutputProcRank(parallelOutputProcRank);
#endif

		//	initialize ug-interfaces
		bridge::RegisterStandardInterfaces(bridge::GetUGRegistry());
	}

//	bool pathsCorrect = InitPaths(argv[0]);
	bool pathsCorrect = InitPaths((*argvp)[0]);
	if (!pathsCorrect)
		return -1;

	return 0;
}

////////////////////////////////////////////////////////////////////////
///	finalizes ug
/**	If ug has been compiled for parallel use (UG_PARALLEL is defined)
 *	then this method will internally call pcl::Finalize.
 */
int UGFinalize() {
	if (outputProfileStats) {
	//	output the profiled data.
		PROFILER_UPDATE();
		#ifdef UG_PARALLEL
			if(pcl::IsOutputProc()) {
				UG_LOG("\n");
				PROFILER_OUTPUT();
			}
		#else
			UG_LOG("\n");
			PROFILER_OUTPUT();
		#endif
	}

#ifdef UG_PARALLEL
	pcl::Finalize();
#endif

	return 0;
}

void UGForceExit()
{
	UG_LOG("--- ABORTING UG EXECUTION ---\n");
	UGFinalize();
	exit(0);
}

void UGOutputProfileStatsOnExit(bool bEnable)
{
	outputProfileStats = bEnable;
}

} //end of namespace ug
