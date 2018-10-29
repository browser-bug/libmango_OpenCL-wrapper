
#include "profiling.h"

#include <string>

using namespace boost::accumulators;

namespace mango {


static std::string ProfilingLabel[NR_COUNTERS] = {
	"iret",
	"ipc",
	"mem_access",
};

void Profiler::print_to_console() const {
	for (uint32_t i = 0; i < NR_COUNTERS; ++i) {
		std::cout << "id=" << this->id << " " << ProfilingLabel[i] << ":"
			" avg=" << mean(counter_stats[i]) <<
			" min=" << min(counter_stats[i])  <<
			" max=" << max(counter_stats[i])  << std::endl;
	}
}

} // namespace mango

