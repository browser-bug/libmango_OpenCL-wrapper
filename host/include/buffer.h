/*! \file
 *  \brief Buffer (memory objects)
 */
#ifndef BUFFER_H
#define BUFFER_H


#include "event.h"

namespace mango {

/*! \brief HN shared memory buffer descriptor */
class Buffer {
public:
	
	/* \brief The constructor of the Buffer class
	 * \param bid The id of the buffer
	 * \param size The size of the requested buffer
	 * \param kernels_in the list of kernel identifiers that are writing to the buffer
	 * \param kernels_out the list of kernel identifiers that are reading from the buffer
	 */
	explicit Buffer(mango_id_t bid, mango_size_t size,
		const std::vector<mango_id_t> &kernels_in={}, 
		const std::vector<mango_id_t> &kernels_out={}
		) noexcept;

	/*! \brief Memory transfer from GN to HN in DIRECT mode
	 * \param GN_buffer A pointer to memory in the GN address space
	 * This function performs a copy between a memory region in the GN address space
	 * and one in the HN address space. The copy works on the entire buffer size
	 * specified in the HN buffer descriptor.
	 * \note Current specification assumes synchronous transfer
	 */
	virtual std::shared_ptr<const Event> write(const void *GN_buffer, mango_size_t global_size=0) const noexcept;

	/*! \brief Memory transfer from HN to GN in DIRECT mode
	 * \param GN_buffer A pointer to memory in the GN address space
	 * This function performs a copy between a memory region in the HN address space
	 * and one in the GN address space. The copy works on the entire buffer size
	 * specified in the HN buffer descriptor.	
	 * \note Current specification assumes synchronous transfer
	 */
	virtual std::shared_ptr<const Event> read(void *GN_buffer, mango_size_t global_size=0) const noexcept;

	/*! \brief Check whether the buffer is read by the host
	 * \return Boolean value
	 * \note The host is represented as a kernel with id 0
	 */
	inline bool isReadByHost(){
		return isReadBy(0);
	}

	/*! \brief Check whether the buffer is read by a given kernel
	 * \return Boolean value
	 */
	inline bool isReadBy(uint32_t kid) const noexcept {
		return std::find(kernels_out.begin(), kernels_out.end(), kid) != kernels_out.end();
	}

	/* \brief Two buffers are considered the same object is the identifier is the same
	 */
	inline bool operator==(const Buffer &other) const noexcept {
		return id==other.id;
	}
	
	/* \brief Change the size of the buffer
	 * \note This function is not currently safe, check the code.
	 */
	mango_exit_code_t resize(mango_size_t size) noexcept;

	/*! 
	 * \brief Get the event identifier
	 */
	inline mango_id_t get_id() 		const noexcept { return id; }

	/*! 
	 * \brief Get the size of th buffer
	 */
	inline mango_id_t get_size() 		const noexcept { return size; }

	/*! 
	 * \brief Get the physical address of the event. This is not an actual physical address,
	 * 	  but it represents an offset in the tile register.
	 * \note Invalid data may be returned if not previously setted 
	 */
	inline mango_size_t get_phy_addr()	const noexcept { return phy_addr; }

	/*! 
	 * \brief Get the identifier of the memory tile assigned
	 * \note Invalid data may be returned if not previously setted 
	 */
	inline mango_id_t get_mem_tile() 	const noexcept { return mem_tile; }

	/*! 
	 * \brief Get the physical address of the event. This is not an actual physical address,
	 * 	  but it represents an offset in the tile register.
	 * \note Invalid data may be returned if not previously setted 
	 */
	inline void set_phy_addr(mango_size_t addr)	noexcept { phy_addr = addr; }

	/*! 
	 * \brief Get the identifier of the memory tile assigned
	 * \note Invalid data may be returned if not previously setted 
	 */
	inline void set_mem_tile(mango_id_t tile)	noexcept { mem_tile = tile; }

	/**
	 * \brief It returns the pointer to the event
	 */
	inline std::shared_ptr<Event> get_event()	noexcept { return event; }

	inline const std::vector<mango_id_t> &get_kernels_in() const noexcept {
		return this->kernels_in;
	}

	inline const std::vector<mango_id_t> &get_kernels_out() const noexcept {
		return this->kernels_out;
	}


protected:
	std::shared_ptr<Event> event;		/*!< Synchronization event */

private: 
	const mango_id_t id;  			/*!< buffer id */
	mango_size_t phy_addr;  		/*!< Physical address of buffer in HN memory */
	mango_id_t mem_tile;			/*!< The memory tile attached */
	mango_size_t size;			/*!< Size of the memory buffer */

	std::vector<mango_id_t> kernels_in;	/*!< Writer kernels */
	std::vector<mango_id_t> kernels_out;	/*!< Reader kernels */
		
};

/*! \brief FIFO-style memory buffer 
 * These buffers implement a burst-mode data transfer. The transfer is asynchronous,
 * and the entire GN-side area is transferred transparently. Of course, users should
 * avoid modifying the GN-side buffer while the transfer is ongoing.
 * \todo Implement read...
 */
class FIFOBuffer: public Buffer {
public:
	
	using Buffer::Buffer;
	
	mango_size_t synch_write(void *GN_buffer, mango_size_t global_size) const noexcept;

	mango_size_t synch_read (void *GN_buffer, mango_size_t global_size) const noexcept;
		
	/*! \brief Memory transfer from GN to HN in BURST mode
	 * \param GN_buffer A pointer to memory in the GN address space
	 * \param size The global size
	 * This function performs a copy between a memory region in the GN address space
	 * and one in the HN address space. The copy works in bursts of the size
	 * specified in the HN buffer descriptor.
	 * \note Current specification assumes asynchronous transfer
	 */
	virtual std::shared_ptr<const Event> write(void *GN_buffer, size_t global_size) const noexcept;
		
	virtual std::shared_ptr<const Event> read(void *GN_buffer, size_t global_size) const noexcept;
};


}
#endif /* BUFFER_H */
