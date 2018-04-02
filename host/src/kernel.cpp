#include "kernel.h"
#include "logger.h"

#include <fstream>
#include <libhn/hn.h>

namespace mango {

mango_exit_code_t KernelFunction::load(const std::string &kernel_file, UnitType unit,
					mango_file_type_t type) noexcept {
	mango_exit_code_t res;

	mango_log->Info("Unit type: %i", unit);
	mango_log->Info("DCT Unit type: %i", UnitType::DCT);
	mango_log->Info("GN Unit type: %i", UnitType::GN);

	switch (unit){
		case UnitType::GN:
			mango_log->Info("Loading GN");
			res = load_gn(kernel_file, type);
		break;
		case UnitType::PEAK:
		mango_log->Info("Loading PEAK");
			res = load_peak(kernel_file, type);
		break;
		case UnitType::DCT:
			mango_log->Info("Loading DCT");
			res = load_dct(kernel_file, type);
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

mango_exit_code_t KernelFunction::load_gn(const std::string &kernel_file, mango_file_type_t type)   noexcept {

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
                        // Peak kernel memory size it is not just the file size
                        // Peak kernels require 256MB of memory to store bss section, stack...
			size[UnitType::PEAK] = (1 << 28); //128 * 16 * line_count;

			mango_log->Info("Kernel PEAK file [%s] loaded with size %d",
					kernel_file.c_str(), size[UnitType::PEAK]);
			break;
		default: 
			mango_log->Error("Kernel file is not valid");
			return mango_exit_code_t::ERR_INVALID_KERNEL_FILE ;
	}

	return mango_exit_code_t::SUCCESS;
}
mango_exit_code_t KernelFunction::load_dct(const std::string &kernel_file, mango_file_type_t type) noexcept {
	
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

	this->termination_event = std::make_shared<KernelCompletionEvent>(kid);

	for(int i=0; i<3; i++)	{ // TODO Is this correct? In that case please document
		task_events.push_back(std::make_shared<Event>(kid));
	}

	thread_count = 0;

	my_tlb = std::make_shared<TLB>();
}


} // namespace mango
