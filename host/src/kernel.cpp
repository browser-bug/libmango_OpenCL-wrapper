#include "kernel.h"
#include "logger.h"

#include <fstream>
#include <libhn/hn.h>

namespace mango {

mango_exit_code_t KernelFunction::load(
		const std::string &kernel_file,
		UnitType unit,
		mango_file_type_t type) noexcept {

	mango_exit_code_t res;

	mango_log->Debug("load: unit type = <%i>", unit);
	mango_log->Debug("load: PEAK = %i", UnitType::PEAK);
	mango_log->Debug("load: NUP  = %i", UnitType::NUP);
	mango_log->Debug("load: DCT  = %i", UnitType::DCT);
	mango_log->Debug("load: GN   = %i", UnitType::GN);

	switch (unit){
		case UnitType::GN:
			mango_log->Info("Loading GN kernel");
			res = load_gn(kernel_file, type);
		break;
		case UnitType::PEAK:
		mango_log->Info("Loading PEAK kernel");
			res = load_peak(kernel_file, type);
		break;
		case UnitType::DCT:
			mango_log->Info("Loading DCT kernel");
			res = load_dct(kernel_file, type);
		break;

		case UnitType::NUP:
		  mango_log->Info("Loading NUPLUS kernel");
		  res=load_nuplus(kernel_file, type);
		break;

		default:
			mango_log->Error("The architecture is not currently supported");
			res = mango_exit_code_t::ERR_UNSUPPORTED_UNIT;
		break;
	}

	if (res == mango_exit_code_t::SUCCESS) {
		loaded = true;
	}

	return res;
}

mango_exit_code_t KernelFunction::load_gn(
		const std::string &kernel_file, mango_file_type_t type) noexcept {

	std::ifstream kernel_fd;

	switch (type) {
		case FileType::BINARY: 
			version[UnitType::GN] = kernel_file;

			kernel_fd.open(kernel_file);

			if(! kernel_fd.good()) {
				mango_log->Error("Unable to open the kernel file: %s", kernel_file.c_str());
				return mango_exit_code_t::ERR_INVALID_KERNEL_FILE;
			}

			kernel_fd.close();

			// Each lines contains 128 hex value
			size[UnitType::GN] = 0;

			mango_log->Info("Kernel GN file [%s] loaded", kernel_file.c_str());

			break;
		case FileType::STRING: 
		case FileType::SOURCE: 
			mango_log->Error("String and source not supported yet");
			return mango_exit_code_t::ERR_FEATURE_NOT_IMPLEMENTED ;
		default: 
			mango_log->Error("Kernel file is not valid");
			return mango_exit_code_t::ERR_INVALID_KERNEL_FILE ;
	}

	return mango_exit_code_t::SUCCESS;
}

mango_exit_code_t KernelFunction::load_peak(
		const std::string &kernel_file, mango_file_type_t type) noexcept {
	
	std::ifstream kernel_fd;
	unsigned int line_count;

	switch (type) {
		case FileType::BINARY: 
			version[UnitType::PEAK] = kernel_file;

			kernel_fd.open(kernel_file);

			if(! kernel_fd.good()) {
				mango_log->Error("Unable to open the kernel file: %s",
					kernel_file.c_str());
				return mango_exit_code_t::ERR_INVALID_KERNEL_FILE;
			}

			// Don't skip lines
			kernel_fd.unsetf(std::ios_base::skipws);
			line_count = std::count(std::istream_iterator<char>(kernel_fd),
						std::istream_iterator<char>(), '\n');

			kernel_fd.close();

			// Each lines contains 128 hex value
                        // Peak kernel memory size it is not just the file size
                        // Peak kernels require 256MB of memory to store bss section, stack...
			size[UnitType::PEAK] = (1 << 28); //128 * 16 * line_count;
			(void) line_count;
			// TODO Check this

			mango_log->Info("Kernel PEAK file [%s] loaded with size %d",
					kernel_file.c_str(), size[UnitType::PEAK]);
			break;
		default: 
			mango_log->Error("Kernel file is not valid");
			return mango_exit_code_t::ERR_INVALID_KERNEL_FILE ;
	}

	return mango_exit_code_t::SUCCESS;
}

mango_exit_code_t KernelFunction::load_nuplus(
		const std::string &kernel_file, mango_file_type_t type) noexcept {
	
	std::ifstream kernel_fd;
	unsigned int line_count;

	switch (type) {
		case FileType::BINARY:
			version[UnitType::NUP] = kernel_file;

			kernel_fd.open(kernel_file);

			if(! kernel_fd.good()) {
				mango_log->Error("Unable to open the kernel file: %s",
					kernel_file.c_str());
				return mango_exit_code_t::ERR_INVALID_KERNEL_FILE;
			}

			// Don't skip lines
			kernel_fd.unsetf(std::ios_base::skipws);
			line_count = std::count(std::istream_iterator<char>(kernel_fd),
						std::istream_iterator<char>(), '\n');
			kernel_fd.close();

			// Each lines contains 128 hex value
                        // Peak kernel memory size it is not just the file size
                        // Peak kernels require 256MB of memory to store bss section, stack...
			size[UnitType::NUP] = (1 << 28);

			mango_log->Info("Kernel NUPLUS file [%s] loaded with size %d",
					kernel_file.c_str(), size[UnitType::NUP]);
			break;
		default:
			mango_log->Error("Kernel file is not valid");
			return mango_exit_code_t::ERR_INVALID_KERNEL_FILE ;
	}

	UNUSED(line_count);	// Used in the previous version to compute the kernel size.
				// At present not used, but it may be useful in the future

	return mango_exit_code_t::SUCCESS;
}

mango_exit_code_t KernelFunction::load_dct(
		const std::string &kernel_file, mango_file_type_t type) noexcept {
	
	mango_log->Info("Loading DCT in load_dct");
	version[UnitType::DCT] = kernel_file;
	size[UnitType::DCT] = 1;
	switch (type) {
		case FileType::HARDWARE: 
			mango_log->Info("DCT Unit is a hardware unit and has no kernel to load.");
			break;
		default: 
			mango_log->Error("DCT has to be of hardware accelerator type.");
			return mango_exit_code_t::ERR_INVALID_KERNEL_FILE ;
	}

	return mango_exit_code_t::SUCCESS;
}

Kernel::Kernel(mango_id_t kid, KernelFunction *k, std::vector<mango_id_t> buffers_in,
		std::vector<mango_id_t> buffers_out) noexcept :
		id(kid), kernel(k), buffers_in(buffers_in), buffers_out(buffers_out) {

	assert(k && "Kernel function is null.");
	assert(k->length() > 0 && "Kernel function is empty.");

#ifdef PROFILING_MODE
	this->hwc_profiling = std::make_shared<Profiler>(kid);
#endif
	this->termination_event = std::make_shared<KernelCompletionEvent>(kid);

	for(int i=0; i<3; i++)	{ // TODO Is this correct? In that case please document
		mango_log->Debug("Kernel id=%d: appending event id=%d", kid, kid);
		task_events.push_back(std::make_shared<Event>(kid));
	}

	thread_count = 0;

	my_tlb = std::make_shared<TLB>();
}

Kernel::~Kernel() {
	mango_log->Crit("Kernel: id=%d destroying...", id);
#ifdef PROFILING_MODE
	hwc_profiling->print_stats(mango_log);
#endif
}


#ifdef PROFILING_MODE

void Kernel::update_profiling_data() noexcept {

	mango_log->Debug("Profiling: kernel id=%d updating...", id);
	if (unit == nullptr) {
		mango_log->Warn("Profiling: kernel id=%d is missing unit mapping", id);
		return;
	}

	// Assigned unit/tile/processor
	hwc_profiling->set_mapped_processor(unit->get_id());
	uint32_t err = 0;

	// PEAK
	if (unit->get_arch() ==  mango_unit_type_t::PEAK) {
		uint32_t nr_cores = 0;
		hn_stats_monitor_st * values[NR_CORES_PER_PROC];
		err = hn_stats_monitor_read(unit->get_id(), &nr_cores, values, cluster_id);
		if (err == 0) {
			mango_log->Debug("Profiling: kernel id=%d updating PEAK counters...", id);
			hwc_profiling->update_counters_peak(
				mango_log, unit->get_id(), nr_cores, values);
		}
		// not clear who is going to release the memory of 'values[]'
	}
	// NUPLUS (not stable)
#if 0
	else if (unit->get_arch() == mango_unit_type_t::NUP) {
		hn_stats_monitor_st * nup_values = new hn_stats_monitor_st;
		err = hn_nuplus_stats_read(unit->get_id(), nup_values, cluster_id);
		if (err == 0) {
			mango_log->Debug("Profiling: kernel id=%d updating NUPLUS counters...", id);
			hwc_profiling->update_counters_nuplus(
				mango_log, unit->get_id(), nup_values);
		}
		delete nup_values;
	}
#endif
	// other architectures...
	else {
		mango_log->Notice("Profiling: support missing for processor %d [arch=%d]",
			id, unit->get_id(), unit->get_arch());
	}

	if (err != 0) {
		mango_log->Error("Profiling: error %d", err);
		mango_log->Error("Profiling: check BarbequeRTRM building configuration "
				"- is MANGO Power Management enabled?");
	}
}


void Kernel::print_profiling_data() noexcept {
	hwc_profiling->print_stats(mango_log);
}

#endif


} // namespace mango
