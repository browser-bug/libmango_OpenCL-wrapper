#ifndef MM_H
#define MM_H

#include "mango_types.h"
#include "task_graph.h"

/*! \file mm.h
 * \brief Local memory manager for virtual address resolution
 */ 


namespace mango {

/*! \brief Stub of a Memory Manager
 * Currently, it only manages in a very rough way the virtual addresses
 */
class MM {


public:
	MM() noexcept;

	~MM();


	mango_exit_code_t allocate_events(TaskGraph &tg) noexcept;


	mango_exit_code_t set_vaddr_kernels(TaskGraph &tg) noexcept;
		
	mango_exit_code_t set_vaddr_buffers(TaskGraph &tg) noexcept;

	mango_exit_code_t set_vaddr_events(TaskGraph &tg) noexcept;

	mango_exit_code_t allocate_buffers(TaskGraph &tg) noexcept;

	mango_exit_code_t deallocate_buffers(TaskGraph &tg) noexcept {
		return ExitCode::ERR_FEATURE_NOT_IMPLEMENTED;
	}

private:
	mango_addr_t last_address;
	mango_addr_t next_virtual_address;
	std::map<mango_id_t, int> entries;

	void set_tlb_kb(mango_id_t unit, mango_id_t mem_bank, mango_addr_t starting_addr,
			mango_size_t size, mango_addr_t phy_addr, int entry) const noexcept;

	void set_buff_tlb(std::shared_ptr<Kernel> k, std::shared_ptr<Buffer> b) noexcept;

};

}
#endif /* MM */
