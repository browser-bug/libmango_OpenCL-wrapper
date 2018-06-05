/*! \file
 *  \brief Context of the host-side runtime
 */
//#define UPV_HN_ENABLED


#ifndef CONTEXT_H
#define CONTEXT_H
#include "mm.h"
#include "kernel_arguments.h"


namespace mango {

/*! \class Context 
 * \brief A class to hold the current state of the host-side runtime.
 */
class Context {
public:

	/*! \brief Initialize runtime library */
	Context() noexcept {
	}

	/*! \brief Shutdown runtime library */ 
	virtual ~Context() {

	}


	/*! \brief Resource Allocation for a task graph of the application
	 * \param tg The task graph to allocate resources for
	 * \returns An exit code signalling the correct allocation (or not)
	 * \note Current implementation is a hack for PEAK, and a dummy for GN.
	 */
	virtual mango_exit_code_t resource_allocation(TaskGraph &tg) noexcept = 0;

	/*! \brief Resource Allocation for a task graph of the application
	 * \param tg The task graph to deallocate resources for
	 * \note Current implementation is a dummy
	 */
	virtual mango_exit_code_t resource_deallocation(TaskGraph &tg) noexcept = 0;


	/*! \brief Start a given kernel
	 * \param kernel Pointer to the Kernel to start
	 * \param args A KernelArguments object which provides the marshalling of
	 * arguments
	 * \param event A pointer to an Event, which is notified upon completion
	 * \return The same pointer to Event received as parameter
	 *
	 * \note GN mode is incompatible with HN_library use
	 */
	virtual std::shared_ptr<Event> start_kernel(
		std::shared_ptr<Kernel> kernel, 
		KernelArguments &args, 
		std::shared_ptr<Event> event = nullptr) noexcept;

	
	Context & operator+=(std::shared_ptr<Kernel> k){
		kernels.emplace(k->get_id(),k);
		return *this;
	}

	Context & operator+=(std::shared_ptr<Buffer> b){
		buffers.emplace(b->get_id(),b);
		return *this;
	}

	Context & operator+=(std::shared_ptr<Event> e){
		events.emplace(e->get_id(),e);
		return *this;
	}


	virtual std::shared_ptr<Kernel> register_kernel(mango_id_t kid, KernelFunction *k,
						std::initializer_list<mango_id_t> in_buffers={},
						std::initializer_list<mango_id_t> out_buffers={}) noexcept; 

	virtual std::shared_ptr<Kernel> register_kernel(mango_id_t kid, KernelFunction *k,
						std::vector<mango_id_t> in_buffers={},
						std::vector<mango_id_t> out_buffers={}) noexcept; 

	std::shared_ptr<Buffer> register_buffer(std::shared_ptr<Buffer> the_buffer, 
						mango_id_t bid) noexcept;

	std::shared_ptr<Buffer> register_buffer(mango_id_t bid,
						mango_size_t size,
						std::initializer_list<mango_id_t> kernels_in={},
						std::initializer_list<mango_id_t> kernels_out={},
						BufferType bt=BufferType::BUFFER) noexcept; 

	void deregister_buffer(mango_id_t bid);

	std::shared_ptr<Event> register_event(std::initializer_list<mango_id_t> in_buffers,
						std::initializer_list<mango_id_t> out_buffers) noexcept;

	std::shared_ptr<Event> register_event(std::shared_ptr<Event> event) noexcept;

	inline std::shared_ptr<Kernel> get_kernel(mango_id_t id) {
		return this->kernels.at(id);
	}

	inline std::shared_ptr<Buffer> get_buffer(mango_id_t id) {
		return this->buffers.at(id);
	}

	inline std::shared_ptr<Event> get_event(mango_id_t id) {
		return this->events.at(id);
	}

	inline const std::map<mango_id_t, std::shared_ptr<Event>> & get_events() const noexcept {
		return this->events;
	}

	static uint16_t mango_get_max_nr_buffers() noexcept {
		return HN_MAX_RSC;
	}

	static constexpr size_t get_max_nr_resources() noexcept {
		return HN_MAX_RSC;
	}

protected:
	void print_debug(const char *function, int line) const noexcept;

	std::map<mango_id_t, std::shared_ptr<Kernel>> kernels;
	std::map<mango_id_t, std::shared_ptr<Buffer>> buffers;
	std::map<mango_id_t, std::shared_ptr<Event>> events;


};



}


namespace mango {

class BBQContext : public Context {
	
public:	
	/*! \brief Initialize runtime library */
	BBQContext(std::string const & _name = "app",
			std::string const & _recipe = "generic");

	virtual ~BBQContext() noexcept;

	std::shared_ptr<bbque::TaskGraph> to_bbque(TaskGraph &tg) noexcept;
	void from_bbque(TaskGraph &tg) noexcept;

	virtual mango_exit_code_t resource_allocation(TaskGraph &tg) noexcept override;

	virtual mango_exit_code_t resource_deallocation(TaskGraph &tg) noexcept override {
		// TODO: Once we finish we need to free memory assigned to the kernel image
		//       hn_free_memory(tile, address, size) <- This function must be coded in hn_library
    // UPV -> POLIMI this should be done through the mango_platform_proxy calling UnsetPartition for the allocated partition

		// TODO: We need to remove TLB entries assigned to kernel image (entry 1) and buffers (entry 2 and successive ones)
    // UPV -> POLIMI this is not necessary at hardware level, but it could be required in the MM
		(void) tg;

		return mango_exit_code_t::SUCCESS;
	}

	virtual std::shared_ptr<Event> start_kernel(std::shared_ptr<Kernel> kernel, 
			KernelArguments &args, std::shared_ptr<Event> _e=nullptr) noexcept override;

private:
	bbque::ApplicationController bbque_app_ctrl;
	std::shared_ptr<bbque::TaskGraph> bbque_tg;


	static bbque::ArchType_t unit_to_arch_type(mango_unit_type_t t) noexcept;


	static mango_unit_type_t arch_to_unit_type(bbque::ArchType_t t) noexcept;



};

	
} //namespace mango

#endif /* CONTEXT_H */
