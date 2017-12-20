#include "mm.h"
#include "logger.h"

#include <assert.h>
#include <libhn/hn.h>

#define TLB_BASE_SHRD  0x50000000
#define TLB_BASE_SYNCH 0x5e000000
#define TLB_BASE_KERN  0x30000000
#define TLB_ENTRY_PEAKOS 0
#define TLB_ENTRY_KERNEL 1
#define TLB_ENTRY_EVENTS 2
#define TLB_ENTRY_START  3



namespace mango {

void MM::set_tlb_kb(mango_id_t unit, mango_id_t mem_bank, mango_addr_t starting_addr,
			mango_size_t size, mango_addr_t phy_addr, int entry) const noexcept {

	hn_set_tlb(unit, entry, starting_addr, starting_addr + size - 1,	// TODO Check if -1 is correct 
			phy_addr, 1, 0, 0, mem_bank);
}

MM::MM() noexcept {

	mango::mango_init_logger();
	mango_log->Info("Local Memory Manager initializing...");
	
	this->next_virtual_address = TLB_BASE_SHRD; 	// Could be one per kernel
	this->last_address = 1024;			// TODO CHECK

	mango_log->Info("TLB Information");
	mango_log->Info(" -> Base virtual address for shared data: %p", TLB_BASE_SHRD);
	mango_log->Info(" -> Base virtual address for events: %p", TLB_BASE_SYNCH);
	mango_log->Info(" -> Base virtual address for kernels: %p", TLB_BASE_KERN);
	mango_log->Info(" -> Default entry nr. peakos/kernel/events/others: %d/%d/%d/%d+",
			TLB_ENTRY_PEAKOS, TLB_ENTRY_KERNEL, TLB_ENTRY_EVENTS,
			TLB_ENTRY_START);
}
MM::~MM() noexcept {
	// Nothing to do
}

mango_exit_code_t MM::set_vaddr_kernels(TaskGraph &tg) noexcept {

	entries.clear();

	for(auto& k : tg.get_kernels()){

		const auto unit = k->get_assigned_unit();

		mango_addr_t virt_addr = TLB_BASE_KERN;
		mango_addr_t phy_addr  = k->get_physical_address();
		mango_id_t   mem_bank  = k->get_mem_tile();
		mango_id_t   tile_unit = unit->get_id();
		mango_size_t kern_size = k->get_kernel()->get_kernel_size(unit->get_arch());

		// We have only one kernel per unit, so the virtual address is fixed
		k->set_virtual_address(virt_addr);

		mango_size_t tlb_area_size;

		switch(unit->get_arch()) {
			case mango_unit_type_t::PEAK:
				// In PEAK the stack area is fixed and we do not have paging,
				// thus we have to allocate a fixed 256MB area for the kernel
				// @see issue#9
				// https://bitbucket.org/mango_developers/mangolibs/issues/9/
				tlb_area_size = 256 * 1024 * 1024;
			break;

			default:
				tlb_area_size = kern_size;
			break;
		}

		set_tlb_kb(tile_unit, mem_bank, virt_addr, tlb_area_size, phy_addr, TLB_ENTRY_KERNEL);


		mango_log->Notice("Mapped kernel image. [tile=%d, mem_bank=%d, phy_addr=%p, "
				  "virt_addr=%p, size=%d]", tile_unit, mem_bank, phy_addr,
				  virt_addr, tlb_area_size);

		this->entries[tile_unit] = TLB_ENTRY_START;

	}
	return ExitCode::SUCCESS;
}

void MM::set_buff_tlb(std::shared_ptr<Kernel> k, std::shared_ptr<Buffer> b) noexcept {

	auto tlb = k->get_tlb();

	mango_log->Debug("Adding TLB entry for buffer %d address 0x%x", b->get_id(), next_virtual_address);
	tlb->set_virt_addr(*b, next_virtual_address);

	const auto unit = k->get_assigned_unit();
	mango_id_t   mem_bank  = b->get_mem_tile();
	mango_id_t   tile_unit = unit->get_id();
	mango_addr_t phy_addr  = b->get_phy_addr();
	mango_size_t buff_size = b->get_size();
	int entry = this->entries.at(tile_unit);

	set_tlb_kb(tile_unit, mem_bank, next_virtual_address, buff_size, phy_addr, entry); 

	this->entries[tile_unit] = entry + 1;

	mango_log->Notice("Mapped buffer %d to kernel %d [virt_addr=%p] [entry=%d]",
				b->get_id(), k->get_id(), next_virtual_address, entry);

	// Update the base address for the next buffer to allocate in the TLB
	next_virtual_address += b->get_size();

	if ((next_virtual_address % 64) != 0)	// TODO Check this requirement with UPV
		next_virtual_address += 64 - (next_virtual_address % 64);

}

void MM::set_event_tlb(std::shared_ptr<Kernel> k, std::shared_ptr<Event> e) noexcept {

	// TODO Not sure if we have to add this offset, we have to ask UPV about this
	const uint32_t offset = 0x40;

	auto tlb = k->get_tlb();
	tlb->set_virt_addr(*e, TLB_BASE_SYNCH + offset + e->get_phy_addr());
}

mango_exit_code_t MM::set_vaddr_buffers(TaskGraph &tg) noexcept {

	for(auto& b : tg.get_buffers()){

		assert(b != nullptr);
		mango_log->Info("Mapping input buffers...");

		for(auto k_id : b->get_kernels_in()) {

			if (k_id==0) continue;		// TODO What?
			auto k = tg.get_kernel_by_id(k_id);
			assert(k != nullptr);

			set_buff_tlb(k, b);
		}

		mango_log->Info("Mapping output buffers...");
		for(auto k_id : b->get_kernels_out()) {

			if (k_id==0) continue;		// TODO What?
			auto k = tg.get_kernel_by_id(k_id);
			assert(k != nullptr);

			set_buff_tlb(k, b);
		}


	}
	return ExitCode::SUCCESS;

}

mango_exit_code_t MM::set_vaddr_events(TaskGraph &tg) noexcept {
	mango_log->Info("Mapping events...");


	for(auto& e : tg.get_events()) {

		for(auto &k_id : e->get_kernels_in()) {
			if (k_id==0)
				continue;	 // TODO Check 

			set_event_tlb(tg.get_kernel_by_id(k_id), e);
		}

		for(auto &k_id : e->get_kernels_out()) {			
			if (k_id==0)
				continue;	 // TODO Check 

			set_event_tlb(tg.get_kernel_by_id(k_id), e);
		}
	}

	for(auto& k : tg.get_kernels()) {

		const auto unit = k->get_assigned_unit();

		if (unit->get_arch() == mango_unit_type_t::PEAK) {
			mango_id_t   tile_unit  = unit->get_id();
			mango_addr_t start_addr = TLB_BASE_SYNCH;
			mango_addr_t end_addr   = TLB_BASE_SYNCH + 0xffffff;

			mango_log->Notice("Configured TLB for events of tile %d [%p - %p]", tile_unit,
						start_addr, end_addr);

			/* Single instance of tlb for all events */
			hn_set_tlb(tile_unit, TLB_ENTRY_EVENTS, start_addr, end_addr, 0, 0, 1, 0, 0);
		}

		for(auto &e : k->get_task_events()){

			set_event_tlb(k, e);
		}
	}

	return ExitCode::SUCCESS;
}



void MM_GN::set_buff_tlb(std::shared_ptr<Kernel> k, std::shared_ptr<Buffer> b) noexcept {

	auto tlb = k->get_tlb();

	mango_log->Debug("Adding TLB entry for buffer %d address 0x%x", b->get_id(), b->get_phy_addr());
	tlb->set_virt_addr(*b, b->get_phy_addr());

}

void MM_GN::set_event_tlb(std::shared_ptr<Kernel> k, std::shared_ptr<Event> e) noexcept {

	auto tlb = k->get_tlb();
	mango_log->Info("Adding TLB entry for event %d address 0x%x", e->get_id(), e->get_phy_addr());
	tlb->set_virt_addr(*e, e->get_phy_addr());
}


} // namespace mango


