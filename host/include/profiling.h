#ifndef LIBMANGO_PROFILING_H
#define LIBMANGO_PROFILING_H

#include <array>
#include <cstring>
#include <iostream>
#include <map>
#include <memory>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/count.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/variance.hpp>

#include <libhn/hn.h>

#include "logger.h"



#define PROF_BUFFER_DIV1  "=======================+=========================================== "
#define PROF_BUFFER_HEAD1 "| Buffer %3d           |            Transfer time (ms)            | "
#define PROF_BUFFER_DIV2  "|------------+---------+------------------------------------------| "
#define PROF_BUFFER_HEAD2 "| Operation  |  count  |      min       max       avg      var    | "
#define PROF_BUFFER_FILL  "| %-10s | %6d  | %8.2f   %8.2f   %8.2f   %6.2f  | "


#define PROF_KERNEL_DIV1  "===============+========================= "
#define PROF_KERNEL_HEAD1 "| Kernel %3d   |        Values          | "
#define PROF_KERNEL_DIV2  "|--------------+------------------------| "
#define PROF_KERNEL_HEAD2 "| HW counter   |     processor:%3d      | "

#define PROF_KERNEL_FILL  "| %-12s | %-22d | "

using namespace boost::accumulators;


namespace mango {

/*! \class ProfilerCounter
 *  \brief Enumerate the available HW counters
 */
enum ProfilingCounter {
	IRET = 0,
	CPI,
	MEM_ACCESS,
	CORE_CYCLES,

	NR_COUNTERS
};

#define _NR_COUNTERS    4

enum ProfilingOperation {
	PROF_READ = 0,
	PROF_WRITE,
	PROF_SYNC_READ,
	PROF_SYNC_WRITE,

	NR_OPERATIONS
};

#define _NR_TIMINGS NR_OPERATIONS

using time_accumul_array = std::array<accumulator_set<int, features<tag::mean, tag::min, tag::max, tag::variance> >, _NR_TIMINGS>;


/*! \class Profiler
 *  \brief Store kernel specific profiling data
 */
class Profiler {

//using accumulator_array = std::array<accumulator_set<int, features<tag::mean, tag::min, tag::max>>, _NR_COUNTERS>;
using accumulator_array = std::array<int , _NR_COUNTERS>;

public:
	/*! \brief Constructor
	 *  \param id the id of the kernel to profile
	 */
	Profiler(uint32_t id): kernel_id(id) {
		memset(&curr_values_peak, 0, sizeof(hn_stats_monitor_st));
	}

	/*! \brief Destructor
	 */
	virtual ~Profiler() {
	}

	/*! \brief Update current counters values and statistics for PEAK processors
	 *  \param log the logger object, if null print to console
	 *  \param processor_id the id of the processing unit
	 *  \param new_values updated hw counters values for
	 */
	void update_counters_peak(
		std::shared_ptr<bbque::utils::Logger> log,
		uint32_t processor_id,
		hn_stats_monitor_st & new_values);

	/*! \brief Print the counters statistics
	 *  \param log the logger object, if null print to console
	 */
	void print_stats(std::shared_ptr<bbque::utils::Logger> log) const;

private:
	//! Set to kernel id for debug purposes
	uint32_t kernel_id;

	//! PEAK hardware counters last values
	hn_stats_monitor_st curr_values_peak;

	//! Statistics on hardware counters
	std::map<uint32_t, std::shared_ptr<accumulator_array>> per_proc_stats;
};


} // namespace mango

#endif //  LIBMANGO_PROFILING_H

