
#include "profiling.h"

#include <string>
#include <sstream>

using namespace boost::accumulators;

namespace mango {


static std::string ProfilingLabel[NR_COUNTERS] = {
	"IRET",
	"CPI",
	"MEM_ACCESS",
	"CORE_CYCLES"
};


void Profiler::update_counters_peak(
		std::shared_ptr<bbque::utils::Logger> log,
		uint32_t processor_id,
		hn_stats_monitor_st & new_values) {
	auto proc_entry = per_proc_stats.find(processor_id);
	if (proc_entry == per_proc_stats.end()) {
		per_proc_stats.emplace(processor_id, std::make_shared<accumulator_array>());
		proc_entry = per_proc_stats.find(processor_id);
	}

	auto & stats = *(proc_entry->second.get()); // the array of accumulators
	if (new_values.timestamp != curr_values_peak.timestamp) {
		log->Debug("Profiling: [PEAK] core_instr=%d ",  new_values.core_instr);
		log->Debug("Profiling: [PEAK] cpi=%d ",         new_values.core_cpi);
		log->Debug("Profiling: [PEAK] mem_access=%d ",  new_values.mc_accesses);
		log->Debug("Profiling: [PEAK] core_cycles=%d ", new_values.core_cycles);
		stats[IRET] = (new_values.core_instr - curr_values_peak.core_instr);
		stats[CPI]  = (new_values.core_cpi   - curr_values_peak.core_cpi);
		stats[MEM_ACCESS]  = (new_values.mc_accesses - curr_values_peak.mc_accesses);
		stats[CORE_CYCLES] = (new_values.core_cycles - curr_values_peak.core_cycles);
		curr_values_peak = new_values;
	}
}


void Profiler::print_stats(std::shared_ptr<bbque::utils::Logger> log) const {
	log->Notice("Profiling: kernel: id=%d...[nr_stats=%d]",
		this->kernel_id, per_proc_stats.size());

	log->Notice(PROF_KERNEL_DIV1);
	log->Notice(PROF_KERNEL_HEAD1, this->kernel_id);
	log->Notice(PROF_KERNEL_DIV2);

	for (auto & proc_entry: per_proc_stats) {
		auto & stats = *(proc_entry.second.get()); // the array of values
		log->Notice(PROF_KERNEL_HEAD2, proc_entry.first);
		log->Notice(PROF_KERNEL_DIV2);
		for (uint32_t i = 0; i < NR_COUNTERS; ++i) {
			log->Notice(PROF_KERNEL_FILL, ProfilingLabel[i].c_str(), stats[i]);
		}
	}
	log->Notice(PROF_KERNEL_DIV1);
}


} // namespace mango

