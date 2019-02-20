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

#define _NR_COUNTERS    3

using namespace boost::accumulators;


namespace mango {

/*! \class ProfilerCounter
 *  \brief Enumerate the available HW counters
 */
enum ProfilingCounter {
	IRET = 0,
	CPI,
	MEM_ACCESS,

	NR_COUNTERS
};


/*! \class Profiler
 *  \brief Store kernel specific profiling data
 */
class Profiler {

using accumulator_array = std::array<accumulator_set<float, features<tag::mean, tag::min, tag::max>>, _NR_COUNTERS>;

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
	 *  \param processor_id the id of the processing unit
	 *  \param new_values updated hw counters values for
	 */
	void update_counters_peak(uint32_t processor_id, hn_stats_monitor_st & new_values);

	/*! \brief Print the counters statistics
	 *  \param log the logger object, if null print to console
	 */
	void print_stats(std::shared_ptr<bbque::utils::Logger> log = nullptr) const;

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

