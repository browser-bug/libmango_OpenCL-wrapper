/*! \file
 *  \brief Kernels
 */
#ifndef KERNEL_H
#define KERNEL_H

#include "buffer.h"
#include "tlb.h"

#ifdef PROFILING_MODE
#include "profiling.h"
#endif

#include <algorithm>
#include <chrono>
#include <map>
#include <string>

using namespace std::chrono;
using namespace boost::accumulators;

namespace mango {


/*! \brief Kernel function 
 *
 * This is an array of function pointers to support multiple versions of the
 * kernel.
 * \note This allows one kernel implementation per type of unit. If more are 
 * desired, we need to redesign this data structure.
 */
class KernelFunction { 
public:

	/*! \brief Load a new kernel image
	 *  \arg kernel_file The path and the filename of the kernel file to load
	 *  \arg unit The type of the accelerator unit to which the kernel refers
	 *  \arg type The type of the file
	 */
	mango_exit_code_t load(const std::string &kernel_file, mango_unit_type_t unit, 
				mango_file_type_t type) noexcept;

	/*! \brief Given a Mango unit type, it returns the matching filename
	 *  \throw std::out_of_range if that kernel type was never loaded
	 */ 
	std::string get_kernel_version(mango_unit_type_t type) const { return version.at(type); }

	void set_kernel_size(mango_unit_type_t type, mango_size_t size) { this->size[type] = size; }

	/*! \brief Given a Mango unit type, it returns the matching file size
	 *  \throw std::out_of_range if that kernel type was never loaded
	 */
	mango_size_t get_kernel_size(mango_unit_type_t type)   const { return size.at(type); }

	std::map<mango_unit_type_t, mango_size_t>::const_iterator cbegin() const noexcept {
		return size.cbegin();
	}

	std::map<mango_unit_type_t, mango_size_t>::const_iterator cend() const noexcept {
		return size.cend();
	}

	inline bool is_loaded() const noexcept {
		return this->loaded;
	}

	size_t length() const noexcept {
		assert(version.size() == size.size());
		return version.size();
	}

private:

	bool loaded = false;

	/*!< pointer to the start of the kernel, for each unit type */
	std::map<mango_unit_type_t, std::string> version;

	/*!< size, for each unit type */
	std::map<mango_unit_type_t, mango_size_t> size;

	mango_exit_code_t load_gn(const std::string &kernel_file, mango_file_type_t type)   noexcept;
	mango_exit_code_t load_peak(const std::string &kernel_file, mango_file_type_t type) noexcept;
	mango_exit_code_t load_nuplus(const std::string &kernel_file, mango_file_type_t type) noexcept;
	mango_exit_code_t load_dct(const std::string &kernel_file, mango_file_type_t type) noexcept;

};


/*! \brief Kernel descriptor */
class Kernel {

public:
	Kernel(mango_id_t kid, KernelFunction *k, std::vector<mango_id_t> buffers_in,
		std::vector<mango_id_t> buffers_out) noexcept;

	virtual ~Kernel();
	
	inline bool operator==(const Kernel &other) const noexcept {
		return id==other.id;
	}
	
	inline bool is_a_reader(mango_id_t buffer_id) const noexcept {
		return std::find(buffers_in.begin(), buffers_in.end(), buffer_id) != buffers_in.end();
	}

	inline bool is_a_writer(mango_id_t buffer_id) const noexcept {
		return std::find(buffers_out.begin(), buffers_out.end(), buffer_id) != buffers_out.end();
	}

	inline std::shared_ptr<KernelCompletionEvent> get_termination_event() noexcept {
		return this->termination_event;
	}

	inline std::shared_ptr<TLB> get_tlb() noexcept {
		return this->my_tlb;
	}

	inline std::vector<std::shared_ptr<Event>> &get_task_events() noexcept {
		return this->task_events;
	}

	inline const std::vector<std::shared_ptr<Event>> &get_task_events() const noexcept {
		return this->task_events;
	}

	inline mango_addr_t get_virtual_address() const noexcept {
		return this->vaddr;
	}

	inline void set_virtual_address(mango_addr_t addr) noexcept {
		this->vaddr = addr;
	}

	inline mango_addr_t get_physical_address() const noexcept {
		return this->phy_addr;
	}

	inline void set_physical_address(mango_addr_t addr) noexcept {
		this->phy_addr = addr;
	}

	inline mango_id_t get_mem_tile() const noexcept {
		return this->mem_tile;
	}

	inline void set_mem_tile(mango_id_t mem_tile) noexcept {
		this->mem_tile = mem_tile;
	}


	inline uint32_t get_cluster() const noexcept {
		return this->cluster_id;
	}

	inline void set_cluster(uint32_t cluster_id)	noexcept {
		this->cluster_id = cluster_id;
	}


	inline std::shared_ptr<Unit> get_assigned_unit() const noexcept {
		return this->unit;
	}

	inline void set_unit(std::shared_ptr<Unit> unit) noexcept {
		this->unit = unit;
	}

	inline const KernelFunction *get_kernel() const noexcept {
		return this->kernel;
	}

	inline mango_id_t get_id() const noexcept {
		return this->id;
	}

	inline std::vector<mango_id_t>::const_iterator buffers_in_cbegin() const noexcept {
		return buffers_in.cbegin();
	}

	inline std::vector<mango_id_t>::const_iterator buffers_in_cend() const noexcept {
		return buffers_in.cend();
	}

	inline std::vector<mango_id_t>::const_iterator buffers_out_cbegin() const noexcept {
		return buffers_out.cbegin();
	}

	inline std::vector<mango_id_t>::const_iterator buffers_out_cend() const noexcept {
		return buffers_out.cend();
	}

	inline void set_thread_count(int thread_count) noexcept {
		this->thread_count = thread_count;
	}

	inline int get_thread_count() const noexcept {
		return thread_count;
	}

#ifdef PROFILING_MODE

	inline void profiling_time_start() noexcept {
		this->start_time = high_resolution_clock::now();
	}

	inline void profiling_time_stop() noexcept {
		this->finish_time = high_resolution_clock::now();
		duration<int, std::micro> elapsed_time =
			duration_cast<duration<int, std::micro>>(finish_time - start_time);
		timings(elapsed_time.count()/1e3);
	}

	void update_profiling_data() noexcept;

	void print_profiling_data() noexcept;

#endif

protected:
	inline void set_event(std::shared_ptr<KernelCompletionEvent> event) noexcept {
		this->termination_event = event;
	}

private:
	const mango_id_t id;		     /*!< Kernel id */
	KernelFunction *kernel;              /*!< Pointer to kernel in GN memory */
	std::vector<mango_id_t> buffers_in;  /*!< Read buffers */
	std::vector<mango_id_t> buffers_out; /*!< Write buffers */

	std::shared_ptr<TLB> my_tlb;	/*!< The tlb for buffers and events */

	int thread_count;               /*!< Number of cores needed for this kernel; 0 means as many as possible */
	std::shared_ptr<Unit> unit;     /*!< The unit allocated to this kernel; NULL before mango_resource_allocation is invoked. */
	std::shared_ptr<KernelCompletionEvent> termination_event; /*!< Synchronization event */
	std::vector<std::shared_ptr<Event>> task_events;          /*!< Synch for tasks */
	mango_addr_t vaddr;             /*! Virtual address where the kernel binary is allocated */
	mango_addr_t phy_addr;          /*! Physical address where the kernel binary is allocated */
	mango_id_t mem_tile;            /*! Tile where the kernel binary is allocated */
	uint32_t cluster_id;            /*!< ID of the cluster of processors */

#ifdef PROFILING_MODE
	std::shared_ptr<Profiler> hwc_profiling; /*! Container of HW counters values */
	accumulator_set<float, features<tag::mean, tag::min, tag::max, tag::variance>>  timings;  /*!< Accumulator for timings statistics */
	high_resolution_clock::time_point start_time;
	high_resolution_clock::time_point finish_time;
#endif
};

}
#endif /* KERNEL_H */
