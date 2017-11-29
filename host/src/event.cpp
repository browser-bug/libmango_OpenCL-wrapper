#include "event.h"
#include "logger.h"


namespace mango {

std::atomic<mango_id_t> mango::Event::id_gen ;


Event::Event() noexcept : bbq_notify_callback(nullptr) {
	id=id_gen++;
}


Event::Event(mango_id_t kernel_id) noexcept : Event()
{
	kernels_in  = std::vector<mango_id_t>({kernel_id}) ;
	kernels_out = std::vector<mango_id_t>({kernel_id}) ;
}

Event::Event (const std::vector<mango_id_t> &kernel_id_in,
				const std::vector<mango_id_t> &kernel_id_out ) noexcept : Event() {
	kernels_in  = kernel_id_in;
	kernels_out = kernel_id_out;
}



void Event::wait_state(uint32_t state) const noexcept {
	uint32_t value;

	do {
		value = lock();
		write(value);

		if (value!=state){ 
			usleep(1);		// TODO: no other way to do this?
		}

	}	while(value!=state);
}

uint32_t Event::wait() const noexcept {

	if (fifo_task) {
		wait_state(mango_event_status_t::END_FIFO_OPERATION);
		fifo_task->join();
		mango_log->Debug("End FIFO Ops");
	}
	else wait_state(1);

	return 0;
}


uint32_t Event::lock() const noexcept {
	uint32_t value;
	do {
		value = read();
	} while(value==0);
	return value;
}

uint32_t Event::read() const noexcept {

		uint32_t value;

		uint32_t tile = id / 16;
		uint32_t reg_id = (id % 16) + HN_MANGO_TILEREG_SYNCH0;
		hn_read_register(tile, reg_id, &value);

		return value;
}

void Event::write(uint32_t value) const noexcept
{
	mango_log->Debug("Writing on an event: phy_addr %08x, value %u", phy_addr, value);

	hn_post(phy_addr, value);
	mango_log->Debug("Wrote to the event");

}

void KernelCompletionEvent::write(uint32_t value) const noexcept {
	Event::write(value);

	if (value==READ && bbq_notify_callback!=nullptr) {
		mango_log->Debug("Notifying buffer write callback");

		bbq_notify_callback();
	}

}

uint32_t KernelCompletionEvent::wait() const noexcept {
	uint32_t val = Event::wait();

	mango_log->Debug("Notifying end of kernel callback");


	if (bbq_notify_callback!=nullptr) {
		mango_log->Debug("Notifying buffer read callback");
		bbq_notify_callback();
	} else {
		mango_log->Debug("No callback in event %i", this->get_id());
	}

	return val;
}

} // namespace mango
