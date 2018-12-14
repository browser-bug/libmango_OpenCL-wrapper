/*! \file
 *  \brief Synchronization events
 */
#ifndef EVENT_H
#define EVENT_H

#include "mango_types.h"

#include "pmsl/app_controller.h"
#include "libhn/hn.h"

#include <functional>
#include <vector>
#include <atomic>
#include <memory>
#include <iostream>
#include <thread>
#include <cstring>  // strerror
#include <unistd.h> // usleep
#include <fcntl.h>  // O_CREAT
#include <errno.h>  // errno


namespace mango {

/*! \brief HN Event descriptor 
 *
 * These events are also used at the GN level.
 */
class Event {
public:	

	explicit Event() noexcept;
	explicit Event(mango_id_t kernel_id) noexcept;
	explicit Event(	const std::vector<mango_id_t> &kernel_id_in,
					const std::vector<mango_id_t> &kernel_id_out ) noexcept;


	virtual void wait_state(uint32_t state) const noexcept;

	/*! \brief High level wait primitive
	 */
	virtual uint32_t wait() const noexcept;

	/*! \brief Set an event value
	 * \param value An integer value
	 * write value to the TILEREG register associated with event
	 */		
	virtual void write(uint32_t value) const noexcept;

	/*! \brief Read and reset an event
	 * read value from the TILEREG register associated with event and replace it
	 * with 0
     * This one should deprecated
	 */		
	uint32_t read() const noexcept;
		

	inline bool operator==(const Event &other) const noexcept {
		return id==other.id;
	}

	/*! 
	 * \brief Get the event identifier
	 */
	inline mango_id_t get_id() 			const noexcept { return id; }

	/*! 
	 * \brief Get the physical address of the event. This is not an actual physical address,
	 * 	  but it represents an offset in the tile register. 
	 */
	inline mango_size_t get_phy_addr()		const noexcept { return phy_addr; }

	/*! 
	 * \brief Get the physical address of the event. This is not an actual physical address,
	 * 	  but it represents an offset in the tile register. 
	 */
	inline void set_phy_addr(mango_size_t addr)	noexcept { phy_addr = addr; }

	/*! 
	 * \brief Set the identifier of the cluster of tiles/processors assigned
	 * \note Invalid data may be returned if not previously setted 
	 */
	inline void set_cluster(uint32_t cluster_id)	noexcept {
		this->cluster_id = cluster_id;
	}

	/*! 
	 * \brief Get the identifier of the cluster of tiles/processors assigned
	 * \note Invalid data may be returned if not previously setted 
	 */
	inline uint32_t get_cluster() const noexcept {
		return this->cluster_id;
	}

	/*! 
	 * \brief Set a callback function for write/read data asynchronously
	 */
	inline void set_fifo_task(std::unique_ptr<std::thread> task) noexcept { 
		fifo_task = std::move(task);
	}


	inline const std::vector<mango_id_t> &get_kernels_in() const noexcept {
		return this->kernels_in;
	}

	inline const std::vector<mango_id_t> &get_kernels_out() const noexcept {
		return this->kernels_out;
	}


	template<typename A, typename B>
	inline void set_callback(A	_bbq_notify_callback, B obj, mango_id_t _id) noexcept {
		bbq_notify_callback = std::bind(_bbq_notify_callback, obj, _id);
	}


protected:
	std::function<void()> bbq_notify_callback;
	uint32_t cluster_id;                   /*!< ID of the cluster of processors */

private:
	mango_id_t id;				/*!< event id */
	mango_size_t phy_addr;			/*!< A memory offset from the start of the events area */
	std::vector<mango_id_t> kernels_in;	/*!< Writer kernels */
	std::vector<mango_id_t> kernels_out;	/*!< Reader kernels */
	std::unique_ptr<std::thread> fifo_task; /*!< Used to perform sync read and writes */ 

	static std::atomic<mango_id_t> id_gen;


	/*! \brief lock and read event 
	 * read value from the TILEREG register associated with event and replace it
	 * with 0, when it becomes != 0 (i.e., when unlocked)
	 */
	uint32_t lock() const noexcept;


};

class KernelCompletionEvent : public Event {

	public :
		KernelCompletionEvent(mango_id_t kernel) : Event(kernel) {}

	virtual uint32_t wait() const noexcept override;

	virtual void write(uint32_t value) const noexcept override;


};


}
#endif /* EVENT_H */
