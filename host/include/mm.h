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

	virtual ~MM();


	virtual mango_exit_code_t set_vaddr_kernels(TaskGraph &tg) noexcept;
		
	virtual mango_exit_code_t set_vaddr_buffers(TaskGraph &tg) noexcept;

	virtual mango_exit_code_t set_vaddr_events(TaskGraph &tg) noexcept;


private:
	mango_addr_t last_address;
	mango_addr_t next_virtual_address;
	std::map<mango_id_t, int> entries;

	void set_tlb_kb(mango_id_t unit, mango_id_t mem_bank, mango_addr_t starting_addr,
			mango_size_t size, mango_addr_t phy_addr, int entry) const noexcept;

	virtual void set_buff_tlb(std::shared_ptr<Kernel> k, std::shared_ptr<Buffer> b) noexcept;
	virtual void set_event_tlb(std::shared_ptr<Kernel> k, std::shared_ptr<Event> e) noexcept;

};

class MM_GN : public MM {


public:
	MM_GN() noexcept {};

	virtual ~MM_GN() override {};

private:
	mango_addr_t last_address;
	mango_addr_t next_virtual_address;
	std::map<mango_id_t, int> entries;

	void set_tlb_kb(mango_id_t unit, mango_id_t mem_bank, mango_addr_t starting_addr,
			mango_size_t size, mango_addr_t phy_addr, int entry) const noexcept = delete;

	virtual void set_buff_tlb(std::shared_ptr<Kernel> k, std::shared_ptr<Buffer> b) noexcept override;
	virtual void set_event_tlb(std::shared_ptr<Kernel> k, std::shared_ptr<Event> e) noexcept override;
};

}
#endif /* MM */
