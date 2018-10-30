
#include "profiling.h"

#include <string>
#include <sstream>

using namespace boost::accumulators;

namespace mango {


static std::string ProfilingLabel[NR_COUNTERS] = {
	"iret",
	"cpi",
	"mem_access",
};


void Profiler::update_counters_peak(uint32_t processor_id, hn_stats_monitor_st & new_values) {
	auto proc_entry = per_proc_stats.find(processor_id);
	if (proc_entry == per_proc_stats.end())
		per_proc_stats.emplace(processor_id, std::make_shared<accumulator_array>());

	auto & stats(*(proc_entry->second.get())); // the array of accumulators
	if (new_values.timestamp != curr_values_peak.timestamp) {
		stats[IRET](new_values.core_instr - curr_values_peak.core_instr);
		stats[CPI](new_values.core_cpi - curr_values_peak.core_cpi);
		stats[MEM_ACCESS](new_values.mc_accesses - curr_values_peak.mc_accesses);
		curr_values_peak = new_values;
	}
}


void Profiler::print_stats(std::shared_ptr<bbque::utils::Logger> log) const {
	std::string kern_str("Profiling: kernel_id=");
	kern_str += std::to_string( this->kernel_id);
	if (log)
		log->Info("Profiling: %s", kern_str.c_str());
	else
		std::cout << kern_str << std::endl;

	std::stringstream ss;
	for (auto & proc_entry: per_proc_stats) {
		auto & stats = *(proc_entry.second.get()); // the array of accumulators
		for (uint32_t i = 0; i < NR_COUNTERS; ++i) {
			ss << "Profiling: processor=" << proc_entry.first <<
				" " << ProfilingLabel[i] << ":"
				" avg=" << mean(stats[i]) <<
				" min=" << min(stats[i])  <<
				" max=" << max(stats[i])  << std::endl;
			if (log)
				log->Info("Profiling: %s", ss.str().c_str());
			else
				std::cout << ss.str();
		}
	}
}


} // namespace mango

