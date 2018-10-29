#ifndef LIBMANGO_PROFILING_H
#define LIBMANGO_PROFILING_H

#include <array>
#include <iostream>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/count.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/variance.hpp>

#define _NR_COUNTERS    3

using namespace boost::accumulators;


namespace mango {


enum ProfilingCounter {
	// Hardware counters
	IRET = 0,
	IPC,
	MEM_ACCESS,

	NR_COUNTERS
};


class Profiler {

public:
	Profiler(uint32_t _id): id(_id) { }

	void add_sample(ProfilingCounter counter_key, uint32_t value) {
		if (counter_key < NR_COUNTERS)
			counter_stats[counter_key](value);
	}

	void print_to_console() const;

	void print_to_file() const {};

private:

	uint32_t id; // Set to task id for debug purposes

	std::array<accumulator_set<double, features<tag::mean, tag::min, tag::max>>, _NR_COUNTERS> counter_stats;
};


} // namespace mango

#endif //  LIBMANGO_PROFILING_H

