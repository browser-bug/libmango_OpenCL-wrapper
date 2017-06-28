#include "kernel.h"
#include "logger.h"

#include <fstream>
#include <libhn/hn.h>

namespace mango {

mango_exit_code_t KernelFunction::load(const std::string &kernel_file, UnitType unit,
					mango_file_type_t type) noexcept {
	mango_exit_code_t res;
	switch (unit){
		case UnitType::GN:
			res = load_gn(kernel_file, type);
		break;
		case UnitType::PEAK:
			res = load_peak(kernel_file, type);
		break;
		default:
			mango_log->Error("The architecture is not currently supported");
			res = mango_exit_code_t::ERR_UNSUPPORTED_UNIT;
		break;
	}
	return res;
}

mango_exit_code_t KernelFunction::load_gn(const std::string &kernel_file, mango_file_type_t type)   noexcept {

// TODO Check if this is necessary
/*	switch (type) {
		case FileType::BINARY: 
			version[UnitType::GN] = (kernelfp)strdup(kname);
			size[UnitType::GN] = sizeof(char *); 
			break;
		case FileType::STRING: 
		case FileType::SOURCE: 
			return ExitCode::ERR_FEATURE_NOT_IMPLEMENTED ;
		default: 
			mango_log->Error("Kernel file is not valid");
			return ExitCode::ERR_INVALID_KERNEL_FILE ;
	}
*/
	return mango_exit_code_t::ERR_FEATURE_NOT_IMPLEMENTED;
}

mango_exit_code_t KernelFunction::load_peak(const std::string &kernel_file, mango_file_type_t type) noexcept {
	
	std::ifstream kernel_fd;
	unsigned int line_count;

	switch (type) {
		case FileType::BINARY: 
			version[UnitType::PEAK] = kernel_file;

			kernel_fd.open(kernel_file);

			if(! kernel_fd.good()) {
				mango_log->Error("Unable to open the kernel file: %s", kernel_file.c_str());
				return mango_exit_code_t::ERR_INVALID_KERNEL_FILE;
			}

			// Don't skip lines
			kernel_fd.unsetf(std::ios_base::skipws);
			line_count = std::count(std::istream_iterator<char>(kernel_fd),
						std::istream_iterator<char>(), '\n');

			kernel_fd.close();

			// Each lines contains 128 hex value
			size[UnitType::PEAK] = 128 * 16 * line_count;

			mango_log->Info("Kernel PEAK file [%s] loaded with size %d",
					kernel_file.c_str(), size[UnitType::PEAK]);
			break;
		default: 
			mango_log->Error("Kernel file is not valid");
			return mango_exit_code_t::ERR_INVALID_KERNEL_FILE ;
	}

	return mango_exit_code_t::SUCCESS;
}

Kernel::Kernel(mango_id_t kid, KernelFunction *k, std::vector<mango_id_t> buffers_in,
		std::vector<mango_id_t> buffers_out) noexcept :
		id(kid), kernel(k), buffers_in(buffers_in), buffers_out(buffers_out) {

	this->termination_event = std::make_shared<KernelCompletionEvent>(kid);

	for(int i=0; i<3; i++)	{ // TODO Is this correct? In that case please document
		task_events.push_back(std::make_shared<Event>(kid));
	}

	thread_count = 0;

	my_tlb = std::make_shared<TLB>();
}


} // namespace mango