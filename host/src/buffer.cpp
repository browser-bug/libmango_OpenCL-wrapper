#include "buffer.h"
#include "logger.h"

#include <algorithm>
#include <memory>
#include <libhn/hn.h>

namespace mango {


Buffer::Buffer(mango_id_t bid, mango_size_t size, const std::vector<mango_id_t> &kernels_in, 
		const std::vector<mango_id_t> &kernels_out) noexcept : id(bid), size (size),
		kernels_in(kernels_in), kernels_out(kernels_out)
{
	this->event = std::make_shared<Event>(kernels_in, kernels_out);
}


std::shared_ptr<const Event> Buffer::write(const void *GN_buffer, mango_size_t global_size) const noexcept
{
	if ( global_size == 0 ) {
		global_size = size;
	}

	mango_log->Info("Buffer::write: mem_tile=%d cluster_id=%d, phy_addr=0x%x size=%u\n",
			mem_tile, cluster_id, phy_addr, global_size);

	int err = hn_write_memory(mem_tile, get_phy_addr(), global_size, (char*)GN_buffer, cluster_id);
	if (HN_SUCCEEDED != err) {
		mango_log->Error("Unable to write memory at memory tile %d [err=%d]", mem_tile, err);
	}

	return this->event;
}

std::shared_ptr<const Event> Buffer::read(void *GN_buffer, mango_size_t global_size) const noexcept {

	if ( global_size == 0 ) {
		global_size = size;
	}

	mango_log->Info("Buffer::read: mem_tile=%d cluster=%d phy_addr=0x%x size=%u\n",
			get_mem_tile(), get_cluster(), get_phy_addr(), global_size);

	int err = hn_read_memory(get_mem_tile(), get_phy_addr(), global_size, (char*)GN_buffer, get_cluster());
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

		event->wait_state(mango_event_status_t::WRITE);
		mango_log->Info("FIFOBuffer::synch_write: mem_tile=%d cluster_id=%d phy_addr=0x%x size=%u\n",
				get_mem_tile(), get_cluster(), get_phy_addr(), get_size());

		int err = hn_write_memory(get_mem_tile(), get_phy_addr(), get_size(),
							 (char*)GN_buffer+off, cluster_id);
		if (HN_SUCCEEDED != err) {
			mango_log->Error("Unable to write memory at memory tile %d [err=%d]", get_mem_tile(), err);
		}

		event->write(mango_event_status_t::READ);
	}

	return off;
}

mango_size_t FIFOBuffer::synch_read(void *GN_buffer, mango_size_t global_size) const noexcept
{
	mango_size_t off;

	for(off = 0; off < global_size; off += get_size()){
		event->wait_state(READ);

		mango_log->Info("FIFOBuffer::synch_read: mem_tile=%d cluster_id=%d phy_addr=0x%x size=%u\n",
				get_mem_tile(), get_cluster(), get_phy_addr(), get_size());

		int err = hn_read_memory(get_mem_tile(), get_phy_addr(), get_size(), (char*)GN_buffer+off, get_cluster());
		if (HN_SUCCEEDED != err) {
			mango_log->Error("Unable to read memory at memory tile %d [err=%d]", get_mem_tile(), err);
		}
		
		event->write(WRITE);
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
