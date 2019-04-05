#include "buffer.h"
#include "logger.h"

#include <algorithm>
#include <chrono>
#include <memory>
#include <libhn/hn.h>


using namespace std::chrono;
using namespace boost::accumulators;

namespace mango {


#ifdef PROFILING_MODE
static std::string ProfilingLabelOp[NR_OPERATIONS] = {
	"READ",
	"WRITE",
	"SYNC_READ",
	"SYNC_WRITE"
};
#endif

Buffer::Buffer(mango_id_t bid, mango_size_t size, const std::vector<mango_id_t> &kernels_in,
		const std::vector<mango_id_t> &kernels_out) noexcept : id(bid), size (size),
		kernels_in(kernels_in), kernels_out(kernels_out)
{
#ifdef PROFILING_MODE
	timings = std::make_shared<time_accumul_array>();
#endif
	this->event = std::make_shared<Event>(kernels_in, kernels_out);
}


Buffer::~Buffer() {
#ifdef PROFILING_MODE
	mango_log->Notice(PROF_BUFFER_DIV1);
	mango_log->Notice(PROF_BUFFER_HEAD1, id);
	mango_log->Notice(PROF_BUFFER_DIV2);
	mango_log->Notice(PROF_BUFFER_HEAD2);
	mango_log->Notice(PROF_BUFFER_DIV2);

	auto & accs = *(timings.get());
	for (int i=PROF_READ; i < NR_OPERATIONS; ++i) {
		if (count(accs[i]) == 0)
			continue;
		mango_log->Notice(PROF_BUFFER_FILL,
			ProfilingLabelOp[i].c_str(),
			boost::accumulators::count(accs[i]),
			boost::accumulators::min(accs[i]),
			boost::accumulators::max(accs[i]),
			boost::accumulators::mean(accs[i]),
			boost::accumulators::variance(accs[i]));
	}

	mango_log->Notice(PROF_BUFFER_DIV1);
#endif
}


std::shared_ptr<const Event> Buffer::write(const void *GN_buffer, mango_size_t global_size) const noexcept
{
	if ( global_size == 0 ) {
		global_size = size;
	}

	mango_log->Info("Buffer::write: id=%d mem_tile=%d cluster_id=%d phy_addr=0x%x size=%u",
			id, mem_tile, cluster_id, phy_addr, global_size);

#ifdef PROFILING_MODE
	high_resolution_clock::time_point start_time = high_resolution_clock::now();
#endif
	int err = hn_write_memory(mem_tile, get_phy_addr(), global_size, (char*)GN_buffer, cluster_id);

#ifdef PROFILING_MODE
	high_resolution_clock::time_point finish_time = high_resolution_clock::now();
	duration<int, std::micro> elapsed_time = duration_cast<duration<int, std::micro>>(finish_time - start_time);
	auto & acc = *(timings.get());
	acc[PROF_WRITE](elapsed_time.count()/1e3); // accumulate value in milliseconds
#endif
	if (HN_SUCCEEDED != err) {
		mango_log->Error("Unable to write memory at memory tile %d [err=%d]", mem_tile, err);
	}

	return this->event;
}

std::shared_ptr<const Event> Buffer::read(void *GN_buffer, mango_size_t global_size) const noexcept {

	if ( global_size == 0 ) {
		global_size = size;
	}

	mango_log->Info("Buffer::read: id=%d mem_tile=%d cluster=%d phy_addr=0x%x size=%u",
			id, get_mem_tile(), get_cluster(), get_phy_addr(), global_size);
#ifdef PROFILING_MODE
	high_resolution_clock::time_point start_time = high_resolution_clock::now();
#endif
	int err = hn_read_memory(get_mem_tile(), get_phy_addr(), global_size, (char*)GN_buffer, get_cluster());

#ifdef PROFILING_MODE
	high_resolution_clock::time_point finish_time = high_resolution_clock::now();
	duration<int, std::micro> elapsed_time = duration_cast<duration<int, std::micro>>(finish_time - start_time);
	auto & acc = *(timings.get());
	acc[PROF_READ](elapsed_time.count()/1e3); // accumulate value in milliseconds
#endif
	if (HN_SUCCEEDED != err) {
		mango_log->Error("Unable to read memory at memory tile %d [err=%d]", get_mem_tile(), err);
	}

	return event;
}

mango_exit_code_t Buffer::resize(mango_size_t size) noexcept {
	this->size = size;

	mango_log->Warn("Buffer::resize: Calls to this function may currently cause race conditions");

	/*! \TODO fix this, as it should not be possible to resize while buffer
			is in use */
	return mango_exit_code_t::SUCCESS;
}

mango_size_t FIFOBuffer::synch_write(const void *GN_buffer, mango_size_t global_size) const noexcept {
	mango_size_t off;

	for(off = 0; off < global_size; off += get_size()){
#ifdef PROFILING_MODE
		high_resolution_clock::time_point start_time = high_resolution_clock::now();
#endif
		event->wait_state(mango_event_status_t::WRITE);
		mango_log->Info("FIFOBuffer::synch_write: id=%d mem_tile=%d cluster_id=%d phy_addr=0x%x size=%u\n",
				id, get_mem_tile(), get_cluster(), get_phy_addr(), get_size());

		int err = hn_write_memory(get_mem_tile(), get_phy_addr(), get_size(),
							 (char*)GN_buffer+off, cluster_id);
		if (HN_SUCCEEDED != err) {
			mango_log->Error("Unable to write memory at memory tile %d [err=%d]", get_mem_tile(), err);
		}

		event->write(mango_event_status_t::READ);

#ifdef PROFILING_MODE
		high_resolution_clock::time_point finish_time = high_resolution_clock::now();
		duration<int, std::micro> elapsed_time = duration_cast<duration<int, std::micro>>(finish_time - start_time);
		auto & acc = *(timings.get());
		acc[PROF_SYNC_WRITE](elapsed_time.count()/1e3); // accumulate value in milliseconds

#endif
	}

	return off;
}

mango_size_t FIFOBuffer::synch_read(void *GN_buffer, mango_size_t global_size) const noexcept
{
	mango_size_t off;

	for(off = 0; off < global_size; off += get_size()){
#ifdef PROFILING_MODE
		high_resolution_clock::time_point start_time = high_resolution_clock::now();
#endif
		event->wait_state(READ);

		mango_log->Info("FIFOBuffer::synch_read: id=%d mem_tile=%d cluster_id=%d phy_addr=0x%x size=%u\n",
				id, get_mem_tile(), get_cluster(), get_phy_addr(), get_size());

		int err = hn_read_memory(get_mem_tile(), get_phy_addr(), get_size(), (char*)GN_buffer+off, get_cluster());
		if (HN_SUCCEEDED != err) {
			mango_log->Error("Unable to read memory at memory tile %d [err=%d]", get_mem_tile(), err);
		}

		event->write(WRITE);
#ifdef PROFILING_MODE
		high_resolution_clock::time_point finish_time = high_resolution_clock::now();
		duration<int, std::micro> elapsed_time = duration_cast<duration<int, std::micro>>(finish_time - start_time);
		auto & acc = *(timings.get());
		acc[PROF_SYNC_READ](elapsed_time.count()/1e3); // accumulate value in milliseconds
#endif
	}
	return off;
}

std::shared_ptr<const Event> FIFOBuffer::write(const void *GN_buffer, mango_size_t global_size) const noexcept {
	event->set_fifo_task( std::make_unique<std::thread>(
			&FIFOBuffer::synch_write,
			this, GN_buffer, global_size));
	return event;
}

std::shared_ptr<const Event> FIFOBuffer::read(void *GN_buffer, mango_size_t global_size) const noexcept {
	event->set_fifo_task( std::make_unique<std::thread>(
			&FIFOBuffer::synch_read,
			this, GN_buffer, global_size));
	return event;
}

} // namespace mango
