#include "context.h"
#include "logger.h"

#include "pmsl/app_controller.h"
#include "tg/task_graph.h"
#include "tg/hw.h"

#include <string>

#include <libhn/hn.h>

namespace mango {

void Context::print_debug(const char *function, int line) const noexcept {
	mango_log->Debug("Printing debug info in function %s line %d", function, line);

	std::string temp;
	temp = "Kernels:";
	for (const auto k : this->kernels) {
		temp += " " + std::to_string(k.first) + " (";
		temp += std::to_string(k.second.use_count()) + ")";
	}

	mango_log->Debug("%s", temp.c_str());

	temp = "Buffers:";
	for (const auto b : this->buffers) {
		temp += " " + std::to_string(b.first) + " (";
		temp += std::to_string(b.second.use_count()) + ")";
	}

	mango_log->Debug("%s", temp.c_str());

	temp = "Events:";
	for (const auto e : this->events) {
		temp += " " + std::to_string(e.first) + " (";
		temp += std::to_string(e.second.use_count()) + ")";
	}

	mango_log->Debug("%s", temp.c_str());

}

std::shared_ptr<Kernel> Context::register_kernel(mango_id_t kid, KernelFunction *k,
					std::vector<mango_id_t> in_buffers,
					std::vector<mango_id_t> out_buffers) noexcept {

	assert(nullptr != k && "Kernel function is nullptr");

	if(k->length() <= 0) {
		mango_log->Error("You cannot register a kernel with an empty KernelFunction");
		return nullptr;
	}

	std::shared_ptr<Kernel> the_kernel = std::make_shared<mango::Kernel>(kid, k, in_buffers, out_buffers);

	assert(the_kernel && "Internal error: kernel construction FAILED");
	assert(the_kernel->get_termination_event() && "Invalid termination event");

	this->kernels.emplace(kid, the_kernel);

	this->events.emplace(the_kernel->get_termination_event()->get_id(), the_kernel->get_termination_event());

	for(const auto e : the_kernel->get_task_events()) {
		this->events.emplace(e->get_id(),e);
	}
	
	print_debug(__FUNCTION__,__LINE__);			

	return the_kernel;
}

std::shared_ptr<Kernel> Context::register_kernel(mango_id_t kid, KernelFunction *k,
					std::initializer_list<mango_id_t> in_buffers,
					std::initializer_list<mango_id_t> out_buffers) noexcept {

	return this->register_kernel(kid, k, std::vector<mango_id_t>(in_buffers), 
					std::vector<mango_id_t>(out_buffers));

}

std::shared_ptr<Buffer> Context::register_buffer(std::shared_ptr<Buffer> the_buffer, mango_id_t bid) noexcept {

	buffers.emplace(bid, the_buffer);

	events.emplace(the_buffer->get_event()->get_id(),the_buffer->get_event());

	print_debug(__FUNCTION__,__LINE__);
	return the_buffer;
}

std::shared_ptr<Buffer> Context::register_buffer(mango_id_t bid, mango_size_t size,
					std::initializer_list<mango_id_t> kernels_in,
					std::initializer_list<mango_id_t> kernels_out,
					BufferType bt) noexcept {
	
	assert(size > 0 && "Unable to create a buffer of size 0");

	std::shared_ptr<Buffer> the_buffer;
	switch (bt) {
		case BufferType::FIFO : 
			the_buffer = std::make_shared<mango::FIFOBuffer>(
					bid, size,
					std::vector<mango_id_t>(kernels_in),
					std::vector<mango_id_t>(kernels_out)
			);
			break;
		case BufferType::BUFFER : 
		default :
			the_buffer = std::make_shared<mango::Buffer>(
					bid, size,
					std::vector<mango_id_t>(kernels_in),
					std::vector<mango_id_t>(kernels_out)
			);
			break;
	}
	this->register_buffer(the_buffer, bid);
	return the_buffer;
} 


void Context::deregister_buffer(mango_id_t bid)
{
	events.erase(buffers[bid]->get_event()->get_id());
	buffers.erase(bid);
	print_debug(__FUNCTION__,__LINE__);			
}

std::shared_ptr<Event> Context::register_event(std::initializer_list<mango_id_t> in_buffers,
						std::initializer_list<mango_id_t> out_buffers) noexcept {

	std::shared_ptr<Event> the_event = std::make_shared<mango::Event>(
					std::vector<mango_id_t>(in_buffers),
					std::vector<mango_id_t>(out_buffers)
	);

	this->events.emplace(the_event->get_id(), the_event);
	return the_event;
}

std::shared_ptr<Event> Context::register_event(std::shared_ptr<Event> event) noexcept {
	this->events.emplace(event->get_id(), event);
	return event;
}

std::shared_ptr<Event> Context::start_kernel(std::shared_ptr<Kernel> kernel,
		KernelArguments &args, std::shared_ptr<Event> event) noexcept {
	
	(void) event; // TODO Check why not used
	int err;
	
	if (kernel->get_assigned_unit()->get_arch()==mango_unit_type_t::DCT) {
		mango_log->Info("Not writing image into memory.");
	}
	else {
		/*! Load kernel image to device memory */
		mango_log->Info("Write image into memory to tile %d address 0x%x", kernel->get_mem_tile(), kernel->get_physical_address());

		err = hn_write_image_into_memory(
						(char *)kernel->get_kernel()->get_kernel_version(kernel->get_assigned_unit()->get_arch()).c_str(),
						kernel->get_mem_tile(), kernel->get_physical_address());
		if (err != HN_SUCCEEDED) {
			mango_log->Error("Unable to write the kernel image memory.");
			return nullptr;
		}
	}

	/*! Get argument string */
	std::string str_arguments = args.get_arguments(kernel->get_assigned_unit()->get_arch());
	assert(str_arguments.length() > 0 && "Empty argument string makes no sense.");
	char * arguments=(char *)str_arguments.c_str();

	char str_args[MANGO_ARGS_STRLEN_MAX];
	strncpy(str_args, arguments, MANGO_ARGS_STRLEN_MAX);
	mango_log->Info ("Argument string: %s...\n", str_args);

	/*! Run kernel 
	 * \note We assume that hn_run_kernel is non-blocking
	 */
	mango_log->Debug ("Kernel ID %d VADDR 0x%x \n", kernel->get_assigned_unit()->get_id(), kernel->get_virtual_address());

	std::shared_ptr<Event> re = kernel->get_termination_event();
	re->write(0);

	mango_size_t kernel_address;
	if (kernel->get_assigned_unit()->get_arch() == mango_unit_type_t::NUP) {
		kernel_address = kernel->get_physical_address();
	}
	else {
		kernel_address = kernel->get_virtual_address();
	}

	err = hn_run_kernel(kernel->get_assigned_unit()->get_id(), kernel_address, arguments);
	if (err != HN_SUCCEEDED) {
		mango_log->Error("Unable to launch kernel id=%d err=%d", kernel->get_assigned_unit()->get_id(), err);
		return nullptr;
	}

	return re;
}

BBQContext::BBQContext(std::string const & _name, std::string const & _recipe):
				Context(), bbque_app_ctrl(_name,_recipe) {
	assert(mango_log && "MANGO Logger not initialized!");
	mango_log->Debug("Initializing the application controller...");
	bbque_app_ctrl.Init(); 

	// Now let's setup the connection with the HN daemon
	hn_filter_t filter;
	filter.target = HN_FILTER_TARGET_MANGO;
	filter.mode = HN_FILTER_APPL_MODE_SYNC_READS;
	filter.tile = 999;
	filter.core = 999;

	// We initialize the hn library with the filter and the UPV's partition strategy
	// FIXME: UPV -> POLIMI: The application cannot capture signals? Last
	// argument is to allow the application to receive SIGINT or SIGTERM to
	// terminate
	int rv = hn_initialize(filter, UPV_PARTITION_STRATEGY, 1, 0, 0);
	// TODO: Check UPV_PARTITION_STRATEGY
	if (rv != HN_SUCCEEDED) {
		const char error[] = "Unable to initialize HN library";
		throw std::runtime_error(error);
	}
}

BBQContext::~BBQContext() noexcept {
	bbque_tg->Print();	// TODO Debugging purposes, maybe we want to delete this?
	bbque_app_ctrl.WaitForTermination();

	hn_end();
}

bbque::ArchType_t BBQContext::unit_to_arch_type(mango_unit_type_t t) noexcept {
	switch(t) {
		case UnitType::STOP:
			return bbque::ArchType_t::STOP;
		case UnitType::GN:
			return bbque::ArchType_t::GN;
		case UnitType::DCT:
			return bbque::ArchType_t::DCT;
		case UnitType::GPU:
			return bbque::ArchType_t::GPU;
		case UnitType::ARM:
			return bbque::ArchType_t::ARM;
		case UnitType::PEAK:
			return bbque::ArchType_t::PEAK;
		case UnitType::NUP:
			return bbque::ArchType_t::NUPLUS;
		default:
			return bbque::ArchType_t::NONE;
	}
}

mango_unit_type_t BBQContext::arch_to_unit_type(bbque::ArchType_t t) noexcept {
	switch(t) {
		case bbque::ArchType_t::NONE:
		case bbque::ArchType_t::STOP:
			return UnitType::STOP;
		case bbque::ArchType_t::DCT:
			return UnitType::DCT;
		case bbque::ArchType_t::GN:
			return UnitType::GN;
		case bbque::ArchType_t::GPU:
			return UnitType::GPU;
		case bbque::ArchType_t::ARM:
			return UnitType::ARM;
		case bbque::ArchType_t::PEAK:
			return UnitType::PEAK;
		case bbque::ArchType_t::NUPLUS:
			return UnitType::NUP;
		default:
			return UnitType::STOP;
	};
}


std::shared_ptr<bbque::TaskGraph> BBQContext::to_bbque(TaskGraph &tg) noexcept {
	auto kl = bbque::TaskMap_t();
	auto bl = bbque::BufferMap_t();
	auto el = bbque::EventMap_t();

	assert(tg.get_kernels().size() > 0 && "At least 1 kernel is required to ask resources");

	for(auto &k : tg.get_kernels()){
		std::list<mango_id_t> inl(k->buffers_in_cbegin(), k->buffers_in_cend());
		std::list<mango_id_t> outl(k->buffers_out_cbegin(), k->buffers_out_cend());
		assert((inl.size() + outl.size() > 0) && "At least 1 buffer is required per kernel");
		assert(k->get_id() > 0 && "Kernel id should be greater than 0");

		// TODO core counts not implemented
//		assert(k->get_thread_count() > 0 && "Thread count should be at least 1");
		auto bk = boost::make_shared<bbque::Task>(k->get_id(), inl, outl, k->get_thread_count());
		assert(k->get_kernel() != NULL && "Internal error: kernel invalid pointer");

		const auto &kernels_func = *k->get_kernel();
		for (auto s = kernels_func.cbegin(); s != kernels_func.cend(); s++ ) {
			// TODO Check arguments
			assert(s->first < mango_unit_type_t::STOP && "Invalid archtype");
			auto bbque_archtype = unit_to_arch_type(s->first);
			assert(bbque_archtype != bbque::ArchType_t::STOP && "Invalid BarbequeRTRM ArchType");
			assert(bbque_archtype != bbque::ArchType_t::NONE && "Invalid BarbequeRTRM ArchType");
			bk->AddTarget(bbque_archtype, 0, 0, s->second, 0);
		}

		kl[k->get_id()] = bk;
	}

	for(auto &b : tg.get_buffers()) {
		auto bb = boost::make_shared<bbque::Buffer>(b->get_id(), b->get_size());
		bb->SetEvent(b->get_event()->get_id());
		bl[b->get_id()]=bb;
	}

	for(auto &e : tg.get_events()) {
		auto be = boost::make_shared<bbque::Event>(e->get_id());
		el[e->get_id()]=be;
	}


	bbque_tg = std::make_shared<bbque::TaskGraph>(kl, bl, el);
	for(auto &b : tg.get_buffers()) {
		/*! \note Here we set multiple output buffers. We need to take this into account
		 *        in BarbequeRTRM */
		if (b->get_kernels_out().size() == 0 || b->isReadByHost()) {
			mango_log->Debug("Setting bbque output buffer %d", b->get_id());
			bbque_tg->SetOutputBuffer(b->get_id());

			b->get_event()->set_callback(
				&bbque::ApplicationController::NotifyEvent,
				bbque_app_ctrl,
				b->get_id());
		}
	}
#ifdef DEBUG
	bbque_tg->Print();
#endif
	return bbque_tg;
}


void BBQContext::from_bbque(TaskGraph &tg) noexcept {
	assert(bbque_tg && "Internal error: invalid TG from BarbequeRTRM");
	assert(bbque_tg->Tasks().size() ==
		tg.get_kernels().size() && "Internal error: TG from BarbequeRTRM contains no sufficient tasks");
	assert(bbque_tg->Buffers().size() ==
		tg.get_buffers().size() && "Internal error: TG from BarbequeRTRM contains no sufficient buffers");

	for(auto k : bbque_tg->Tasks()){
		mango_id_t pid = k.second->GetMappedProcessor();
		auto bbque_arch_type = k.second->GetAssignedArch();
		int ncores = k.second->GetMappedCores();
		// TODO: cores not handled
		// assert(ncores > 0 && "Internal error: less than 1 core from BarbequeRTRM");

		mango_log->Debug("Assigning kernel %d to archtype %d", k.first, bbque_arch_type);
		for(auto &kt : tg.get_kernels())
			if (k.first == kt->get_id()) {
				mango_unit_type_t arch_type = arch_to_unit_type(bbque_arch_type);
				assert(arch_type != mango_unit_type_t::STOP && "Internal error: invalid archtype");
				kt->set_unit(std::make_shared<Unit>(pid, arch_type, ncores));

				auto arch_info = k.second->Targets().at(bbque_arch_type);
				kt->set_mem_tile(arch_info->MemoryBank());
				kt->set_physical_address(arch_info->Address());
				mango_log->Debug("Memory tile %d address %p", kt->get_mem_tile(),
						kt->get_physical_address());

			}
	}

	for(auto e : bbque_tg->Events()){
		for(auto &et : tg.get_events())
			if(et->get_id() == e.first) {
				assert(e.second && "Internal error: null event from BarbequeRTRM");
				et->set_phy_addr(e.second->PhysicalAddress());
				// It follows a strange pattern:
				// - We read the value (this should change to zero the register)
				et->read();
				// - We re-read the value and now must be zero
				int value = et->read();
				assert( 0 == value );
			}
	}

	for(auto b : bbque_tg->Buffers()){
		for(auto &bt : tg.get_buffers())
			if(bt->get_id() == b.first){
				assert(b.second && "Internal error: null buffer from BarbequeRTRM");
				bt->set_mem_tile(b.second->MemoryBank());
				bt->set_phy_addr(b.second->PhysicalAddress());

				auto et = bt->get_event();
				assert(et->get_phy_addr() != 0);
				et->read();	// Read the event BEFORE writing it to allow the initialization
						// in case of write-accumulare register
				int value = et->read();
				assert( 0 == value );

				et->write(WRITE);
			}
	}

}


mango_exit_code_t BBQContext::resource_allocation(TaskGraph &tg) noexcept {

	this->to_bbque(tg);
	this->bbque_app_ctrl.GetResourceAllocation(bbque_tg);
	this->from_bbque(tg);

	std::shared_ptr<MM> mm;

	if ( tg.get_kernels()[0]->get_assigned_unit()->get_arch() == mango_unit_type_t::GN ) {
		// Simulated mode
		mm = std::make_shared<MM_GN>();
		mango_log->Warn("Simulated GN mode");
	} else {
		mm = std::make_shared<MM>();
	}

	/* TLB management */
	mm->set_vaddr_kernels(tg);
	mm->set_vaddr_buffers(tg);
	mm->set_vaddr_events(tg);

#if 0
	// TODO: We cannot do this here, we should move after the resourse allocation

	/* Initialize all buffer locks to WRITE */
	for (auto &b : tg.get_buffers()) 
		b->get_event()->write(2);
#endif
	return ExitCode::SUCCESS;
}


std::shared_ptr<Event> BBQContext::start_kernel(std::shared_ptr<Kernel> kernel, 
			KernelArguments &args, std::shared_ptr<Event> _e) noexcept {

	(void) _e; // TODO Check why not used

	this->bbque_app_ctrl.NotifyTaskStart(kernel->get_id());
	auto e = Context::start_kernel(kernel, args);

	e->set_callback(&BBQContext::on_kernel_termination, this, kernel->get_id());
#ifdef DEBUG
	bbque_tg->Print();
#endif
	print_debug(__FUNCTION__,__LINE__);			
	return e;
}

void BBQContext::on_kernel_termination(mango_id_t kernel_id) noexcept {
#ifdef LIBMANGO_PROFILING_MODE
	Profiler task_profile & per_task_profiling[kernel_id];
	task_profile.print_to_console();
#endif
	this->bbque_app_ctrl.NotifyTaskStop(kernel_id);
}


} // namespace mango
