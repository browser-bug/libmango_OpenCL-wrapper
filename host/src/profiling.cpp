
#include "profiling.h"

#include <string>
#include <sstream>

using namespace boost::accumulators;

namespace mango {


static std::string ProfilingLabel[NR_COUNTERS] = {
	"IRET",
	"CPI",
	"CORE_CYCLES",
	"MEM_ACCESS",
	"L2_MISSES",
	"POWER (mW)",
};


void Profiler::update_counters_peak(
		std::shared_ptr<bbque::utils::Logger> log,
		uint32_t processor_id,
		uint32_t nr_cores,
		hn_stats_monitor_st ** new_values) {

	log->Debug("Profiling: processor=%d [nr_cores=%d]", processor_id, nr_cores);

	for (uint32_t core_id = 0; core_id < nr_cores; ++core_id) {
		std::string proc_str(std::to_string(processor_id) + "." + std::to_string(core_id));

		// update per core statistics
		auto & stats = *(per_core_stats[core_id].get()); // the array of accumulators
		log->Debug("Profiling: [core:%s] timestamp=%u ",  proc_str.c_str(), new_values[core_id]->timestamp);
		if (new_values[core_id]->timestamp != per_core_prev_vals[core_id].timestamp) {
			log->Debug("Profiling: [core:%s] core_instr=%u ",  proc_str.c_str(), new_values[core_id]->core_instr);
			log->Debug("Profiling: [core:%s] cpi=%u ",         proc_str.c_str(), new_values[core_id]->core_cpi);
			log->Debug("Profiling: [core:%s] core_cycles=%u ", proc_str.c_str(), new_values[core_id]->core_cycles);
			log->Debug("Profiling: [core:%s] mem_access=%u ",  proc_str.c_str(), new_values[core_id]->mc_accesses);
			log->Debug("Profiling: [core:%s] l2_misses=%u ",   proc_str.c_str(), new_values[core_id]->l2_misses);
			log->Debug("Profiling: [core:%s] power_est=%u ",   proc_str.c_str(), (uint32_t) new_values[core_id]->power_est);
			stats[IRET]        = (new_values[core_id]->core_instr  - per_core_prev_vals[core_id].core_instr);
			stats[CPI]         = (new_values[core_id]->core_cpi    - per_core_prev_vals[core_id].core_cpi);
			stats[CORE_CYCLES] = (new_values[core_id]->core_cycles - per_core_prev_vals[core_id].core_cycles);
			stats[MEM_ACCESS]  = (new_values[core_id]->mc_accesses - per_core_prev_vals[core_id].mc_accesses);
			stats[L2_MISSES]   = (new_values[core_id]->l2_misses   - per_core_prev_vals[core_id].l2_misses);
			stats[POWER]       = (uint32_t) new_values[core_id]->power_est;

			// last counters values
			per_core_prev_vals[core_id] = *(new_values[core_id]);
		}
	}
}


void Profiler::print_stats(std::shared_ptr<bbque::utils::Logger> log) const {
	log->Notice("Profiling: kernel: id=%d...[nr_stats=%d]",
		this->kernel_id, per_core_stats.size());

	log->Notice(PROF_KERNEL_DIV1);
	log->Notice(PROF_KERNEL_HEAD1, this->kernel_id, 0);

	for (uint32_t core_id = 0; core_id < per_core_stats.size(); ++core_id) {
		auto & stats = *(per_core_stats[core_id].get()); // the array of values
		log->Notice(PROF_KERNEL_DIV2);
		log->Notice(PROF_KERNEL_HEAD2, core_id);
		log->Notice(PROF_KERNEL_DIV2);
		for (uint32_t i = 0; i < NR_COUNTERS; ++i) {
			log->Notice(PROF_KERNEL_FILL, ProfilingLabel[i].c_str(), stats[i]);
		}
	}
	log->Notice(PROF_KERNEL_DIV1);
}


} // namespace mango

