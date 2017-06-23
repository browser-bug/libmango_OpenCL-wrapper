/*! \file
 *  \brief Synchronization events
 */
#ifndef TLB_H
#define TLB_H

#include "buffer.h"
#include "event.h"
#include "mango_types.h"

#include <map>


namespace mango {

/*! \brief HN Event descriptor 
 *
 * These events are also used at the GN level.
 */
class TLB {
public:	

	TLB() noexcept {}

	inline mango_size_t get_virt_addr(const Buffer &buffer) const { 
		return this->tlb_buffers.at(buffer.get_id());
	}

	inline mango_size_t get_virt_addr(const Event &event)   const {
		return this->tlb_events.at(event.get_id());
	}

	inline mango_size_t get_virt_addr_buffer(const mango_id_t buffer_id) const { 
		return this->tlb_buffers.at(buffer_id);
	}

	inline mango_size_t get_virt_addr_event(const mango_id_t event_id)   const {
		return this->tlb_events.at(event_id);
	}

	inline void set_virt_addr(const Buffer &buffer, mango_size_t virt_addr) noexcept {
		this->tlb_buffers[buffer.get_id()] = virt_addr;
	}

	inline void set_virt_addr(const Event &event, mango_size_t virt_addr)   noexcept {
		this->tlb_events[event.get_id()] = virt_addr;
	}


private:
	std::map<mango_id_t, mango_size_t> tlb_buffers; /*!< map addresses of buffers */
	std::map<mango_id_t, mango_size_t> tlb_events;  /*!< map addresses of events */

};


}
#endif /* TLB_H */
