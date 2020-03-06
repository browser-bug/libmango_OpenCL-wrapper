#include "mango.h"
#include "logger.h"
#include <stdarg.h>
#include <assert.h>

using namespace std;

static mango::BBQContext *cxt;

extern "C"
{

	mango_exit_t mango_init(const char *application_name, const char *recipe)
	{

		assert(application_name != NULL && "Application name must be a valid pointer!");
		assert(recipe != NULL && "Recipe name must be a valid pointer!");
		assert(strlen(application_name) > 0 && "Application name must be at least 1 character!");
		assert(strlen(recipe) > 0 && "Recipe name must be at least 1 character!");

		mango::mango_init_logger();
		cxt = new mango::BBQContext(application_name, recipe);
		return SUCCESS;
	}

	mango_exit_t mango_release()
	{
		delete cxt;
		return SUCCESS;
	}

	kernelfunction *mango_kernelfunction_init()
	{
		mango::KernelFunction *k = new mango::KernelFunction();
		return (kernelfunction *)k;
	}

	mango_exit_t mango_load_kernel(
		const char *kname, kernelfunction *kernel,
		mango_unit_type_t unit, filetype t)
	{

		assert(kname != NULL && "Kernel name must be a valid pointer!");
		assert(kernel != NULL && "Kernel function must be a valid pointer!");

		assert(unit < mango_unit_type_t::STOP && "Invalid unit type");

		mango::mango_exit_code_t err = ((mango::KernelFunction *)kernel)->load(kname, (mango::UnitType)unit, (mango::FileType)t);

		return err == mango::mango_exit_code_t::SUCCESS ? SUCCESS : ERR_INVALID_KERNEL_FILE;
	}

	mango_kernel_t mango_register_kernel(uint32_t kernel_id,
										 kernelfunction *kernel, unsigned int nbuffers_in, unsigned int nbuffers_out, ...)
	{

		assert(kernel != NULL && "Kernel function must be a valid pointer!");
		assert(kernel_id > 0 && "Kernel id must be positive");

		mango::KernelFunction *t = (mango::KernelFunction *)kernel;

		assert(t->is_loaded() && "You must load the kernel file before register it.");

		std::vector<uint32_t> in;
		std::vector<uint32_t> out;
		va_list list;
		va_start(list, nbuffers_out);
		for (unsigned int i = 0; i < nbuffers_in; i++)
			in.push_back(va_arg(list, mango_buffer_t));
		for (unsigned int i = 0; i < nbuffers_out; i++)
			out.push_back(va_arg(list, mango_buffer_t));
		va_end(list);
		cxt->register_kernel(kernel_id, t, in, out);

		return kernel_id;
	}

	mango_kernel_t mango_register_kernel_with_buffers(uint32_t kernel_id, kernelfunction *kernel, void *_in, void *_out)
	{

		assert(kernel != NULL && "Kernel function must be a valid pointer!");
		assert(kernel_id > 0 && "Kernel id must be positive");

		mango::KernelFunction *t = (mango::KernelFunction *)kernel;

		assert(t->is_loaded() && "You must load the kernel file before register it.");

		std::vector<uint32_t> *in = (std::vector<uint32_t> *)_in;
		std::vector<uint32_t> *out = (std::vector<uint32_t> *)_out;
		cxt->register_kernel(kernel_id, t, *in, *out);

		return kernel_id;
	}

	mango_buffer_t mango_register_memory(uint32_t buffer_id, size_t size, mango_buffer_type_t mode,
										 unsigned int nkernels_in, unsigned int nkernels_out, ...)
	{

		assert(buffer_id > 0 && "Buffer id must be positive");
		assert(size > 0 && "Size must be positive");

		std::vector<uint32_t> in;
		std::vector<uint32_t> out;
		va_list list;
		va_start(list, nkernels_out);
		for (unsigned int i = 0; i < nkernels_in; i++)
			in.push_back(va_arg(list, mango_kernel_t));
		for (unsigned int i = 0; i < nkernels_out; i++)
			out.push_back(va_arg(list, mango_kernel_t));
		va_end(list);

		//

		if (mode == FIFO)
		{
			std::shared_ptr<mango::FIFOBuffer> buff = std::make_shared<mango::FIFOBuffer>(buffer_id, size, in, out);
			cxt->register_buffer(buff, buffer_id);
		}
		else
		{
			std::shared_ptr<mango::Buffer> buff = std::make_shared<mango::Buffer>(buffer_id, size, in, out);
			cxt->register_buffer(buff, buffer_id);
		}
		return buffer_id;
	}

	mango_buffer_t mango_register_memory_with_kernels(uint32_t buffer_id, size_t size, mango_buffer_type_t mode, void *_in, void *_out)
	{
		assert(buffer_id > 0 && "Buffer id must be positive");
		assert(size > 0 && "Size must be positive");

		std::vector<uint32_t> in;
		std::vector<uint32_t> out;

		if (_in != NULL)
			in.swap(*(std::vector<mango_kernel_t> *)_in);

		if (_out != NULL)
			out.swap(*(std::vector<mango_kernel_t> *)_out);

		//

		if (mode == FIFO)
		{
			std::shared_ptr<mango::FIFOBuffer> buff = std::make_shared<mango::FIFOBuffer>(buffer_id, size, in, out);
			cxt->register_buffer(buff, buffer_id);
		}
		else
		{
			std::shared_ptr<mango::Buffer> buff = std::make_shared<mango::Buffer>(buffer_id, size, in, out);
			cxt->register_buffer(buff, buffer_id);
		}
		return buffer_id;
	}

	mango_event_t mango_register_event(unsigned int nkernels_in, unsigned int nkernels_out, ...)
	{

		assert(((nkernels_in + nkernels_out) > 0) && "An event must have at least one kernel tied.");

		std::vector<uint32_t> in;
		std::vector<uint32_t> out;
		va_list list;
		va_start(list, nkernels_out);
		for (unsigned int i = 0; i < nkernels_in; i++)
			in.push_back(va_arg(list, mango_kernel_t));
		for (unsigned int i = 0; i < nkernels_out; i++)
			out.push_back(va_arg(list, mango_kernel_t));
		va_end(list);

		std::shared_ptr<mango::Event> event = std::make_shared<mango::Event>(in, out);

		assert(event && "Cannot create a new event.");

		cxt->register_event(event);

		assert(event->get_id() > 0 && "Wrong event id returned.");

		return event->get_id();
	}

	void mango_deregister_memory(mango_buffer_t mem)
	{
		cxt->deregister_buffer(mem);
	}

	void mango_resize_memory(mango_buffer_t mem, size_t size)
	{
		(void)mem;
		(void)size;
		assert(false);

		// TODO not implemented
		//cxt->buffers[mem]->resize(size);
	}

	mango_event_t mango_get_buffer_event(mango_buffer_t buffer)
	{

		auto buffer_obj = cxt->get_buffer(buffer);

		assert(buffer_obj && "The buffer does not exist");

		auto event = buffer_obj->get_event();

		assert(event && "The buffer does not have an event");
		assert(event->get_id() > 0 && "Invalid event id");

		return event->get_id();
	}

	mango_task_graph_t *mango_task_graph_create(int k, int b, int e, ...)
	{

		assert(k > 0 && "You need at least 1 kernel");
		assert(b > 0 && "You need at least 1 buffer");

		va_list list;
		va_start(list, e);
		mango::TaskGraph *tg = new mango::TaskGraph();
		for (int i = 0; i < k; i++)
			*tg += cxt->get_kernel(va_arg(list, mango_kernel_t));
		for (int i = 0; i < b; i++)
			*tg += cxt->get_buffer(va_arg(list, mango_buffer_t));
		for (int i = 0; i < e; i++)
			*tg += cxt->get_event(va_arg(list, mango_event_t));
		va_end(list);
		for (auto &e : cxt->get_events())
			*tg += e.second;
		return (TaskGraph *)tg;
	}

	void mango_task_graph_destroy_all(mango_task_graph_t *task_graph)
	{

		assert(task_graph != NULL && "Invalid task graph pointer.");

		delete (mango::TaskGraph *)task_graph;
	}

	mango_exit_t mango_resource_allocation(mango_task_graph_t *tg)
	{

		assert(tg != NULL && "Invalid task graph pointer.");

		return (mango_exit_t)cxt->resource_allocation(*((mango::TaskGraph *)tg));
	}

	void mango_resource_deallocation(mango_task_graph_t *tg)
	{

		assert(tg != NULL && "Invalid task graph pointer.");

		cxt->resource_deallocation(*((mango::TaskGraph *)tg));
	}

	mango_event_t mango_write(const void *GN_buffer, mango_buffer_t HN_buffer,
							  mango_communication_mode_t mode, size_t global_size)
	{

		assert(GN_buffer != NULL && "Invalid buffer pointer.");
		assert((mode != BURST || global_size > 0) && "Positive size required for BURST mode.");

		std::shared_ptr<const mango::Event> e;

		auto b = cxt->get_buffer(HN_buffer);

		assert(b && "Invalid buffer");

		if (mode == BURST)
		{
			auto f = std::static_pointer_cast<mango::FIFOBuffer>(b);
			e = f->write(GN_buffer, global_size);
		}
		else
		{
			e = cxt->get_buffer(HN_buffer)->write(GN_buffer, global_size);
		}

		assert(e && "Error getting buffer event");
		assert(e->get_id() > 0 && "Error getting buffer event");

		return e->get_id();
	}

	mango_event_t mango_read(void *GN_buffer, mango_buffer_t HN_buffer,
							 mango_communication_mode_t mode, size_t global_size)
	{
		std::shared_ptr<const mango::Event> e;

		assert(GN_buffer != NULL && "Invalid buffer pointer.");
		assert((mode != BURST || global_size > 0) && "Positive size required for BURST mode.");

		auto b = cxt->get_buffer(HN_buffer);

		assert(b && "Invalid buffer");

		if (mode == BURST)
		{
			auto f =
				std::static_pointer_cast<mango::FIFOBuffer>(cxt->get_buffer(HN_buffer));
			e = f->read(GN_buffer, global_size);
		}
		else
		{
			e = b->read(GN_buffer, global_size);
		}

		assert(e && "Error getting buffer event");
		assert(e->get_id() > 0 && "Error getting buffer event");

		return e->get_id();
	}

	mango_arg_t *mango_arg(mango_kernel_t kernel, const void *value, size_t size, mango_buffer_type_t t)
	{
		mango::Arg *a = NULL;
		auto k = cxt->get_kernel(kernel);

		assert(value != NULL && "Invalid value pointer.");
		assert(k && "Invalid kernel.");

		switch (t)
		{
		case SCALAR:
			switch (size)
			{
			case sizeof(uint64_t):
				// TODO Loses precision
				a = new mango::ScalarArg<uint32_t>(*(uint64_t *)value);
				break;
			case sizeof(uint32_t):
				a = new mango::ScalarArg<uint32_t>(*(uint32_t *)value);
				break;
			case sizeof(uint16_t):
				a = new mango::ScalarArg<uint16_t>(*(uint16_t *)value);
				break;
			case sizeof(uint8_t):
				a = new mango::ScalarArg<uint8_t>(*(uint8_t *)value);
				break;
			}
			break;
		case EVENT:
			a = new mango::EventArg(cxt->get_event(*(uint32_t *)value));
			break;
		case BUFFER:
		case FIFO:
			a = new mango::BufferArg(cxt->get_buffer(*(uint32_t *)value));
			break;
		default:
			assert(false && "Buffer type unknown or not supported.");
			break;
		}

		assert(a && "Something failed in argument allocation.");

		return (mango_arg_t *)a;
	}

	mango_args_t *mango_set_args(mango_kernel_t kernel, int argc, ...)
	{

		assert(argc >= 0 && "argc must be non-negative");

		std::vector<mango::Arg *> arguments;
		va_list list;
		va_start(list, argc);
		for (int i = 0; i < argc; i++)
			arguments.push_back((mango::Arg *)va_arg(list, mango_arg_t *));
		va_end(list);
		auto k = cxt->get_kernel(kernel);

		std::cout << "[DEBUG] Correctly retrieved the kernel : " << kernel << std::endl;

		assert(k && "Kernel not found.");

		return (mango_args_t *)new mango::KernelArguments(arguments, k);
	}

	mango_args_t *mango_set_args_from_vector(mango_kernel_t kernel, void *_arguments)
	{

		assert(_arguments != NULL && "argc must be non-negative");

		std::vector<mango::Arg *> *arguments;
		arguments = (std::vector<mango::Arg *> *)_arguments;

		auto k = cxt->get_kernel(kernel);

		std::cout << "[DEBUG] Correctly retrieved the kernel : " << kernel << std::endl;

		assert(k && "Kernel not found.");

		return (mango_args_t *)new mango::KernelArguments(*arguments, k);
	}

	mango_event_t mango_start_kernel(mango_kernel_t kernel, mango_args_t *args, mango_event_t event)
	{
		auto k = cxt->get_kernel(kernel);
		auto e = cxt->get_event(event);

		assert(args != NULL && "args argument invalid");
		assert(k && "Kernel not found");
		assert(e && "Event not found");

		return cxt->start_kernel(k, *((mango::KernelArguments *)args), e)->get_id();
	}

	void mango_wait(mango_event_t e)
	{

		auto event = cxt->get_event(e);
		assert(event && "Event not found");
		event->wait();
	}

	void mango_wait_state(mango_event_t e, uint32_t state)
	{
		auto event = cxt->get_event(e);
		assert(event && "Event not found");
		event->wait_state(state);
	}

	uint32_t mango_get_unit_id(mango_kernel_t kernel)
	{
		auto k = cxt->get_kernel(kernel);
		return k->get_assigned_unit()->get_id();
	}

	mango_unit_type_t mango_get_unit_arch(mango_kernel_t kernel)
	{
		auto k = cxt->get_kernel(kernel);
		return (mango_unit_type_t)k->get_assigned_unit()->get_arch();
	}

	void mango_write_synchronization(mango_event_t event, uint32_t value)
	{
		auto event_obj = cxt->get_event(event);
		assert(event_obj && "Event not found");
		event_obj->write(value);
	}

	uint32_t mango_read_synchronization(mango_event_t event)
	{
		auto event_obj = cxt->get_event(event);
		assert(event_obj && "Event not found");
		return event_obj->read();
	}

	uint16_t mango_get_max_nr_buffers(void)
	{
		return mango::Context::mango_get_max_nr_buffers();
	}

	/* OpenCL Wrapper Utils Implementations */

	mango_task_graph_t *mango_task_graph_add_buffer(mango_task_graph_t *tg, mango_buffer_t *buffer)
	{
		printf("[BUFFER] adding buffer: %d\n", *buffer);
		// *((mango::TaskGraph *)tg)
		assert(buffer != NULL && "Buffer must be a valid pointer!");
		mango::TaskGraph *tgp = (mango::TaskGraph *)tg;
		if (tgp == NULL)
		{
			tgp = new mango::TaskGraph();
			printf("[BUFFER] creating a new task_graph object\n");
		}

		*tgp += cxt->get_buffer(*buffer);
		return (TaskGraph *)tgp;
	}

	mango_task_graph_t *mango_task_graph_add_kernel(mango_task_graph_t *tg, mango_kernel_t *kernel)
	{
		printf("[KERNEL] adding kernel: %d\n", *kernel);
		// *((mango::TaskGraph *)tg)
		assert(kernel != NULL && "Kernel must be a valid pointer!");
		mango::TaskGraph *tgp = (mango::TaskGraph *)tg;
		if (tgp == NULL)
		{
			tgp = new mango::TaskGraph();
			printf("[KERNEL] creating a new task_graph object\n");
		}

		*tgp += cxt->get_kernel(*kernel);
		return (TaskGraph *)tgp;
	}

	mango_task_graph_t *mango_task_graph_add_event(mango_task_graph_t *tg, mango_event_t *event)
	{
		mango::TaskGraph *tgp = (mango::TaskGraph *)tg;
		if (tgp == NULL)
		{
			tgp = new mango::TaskGraph();
			printf("[EVENT] creating a new task_graph object\n");
		}

		if (event != NULL)
		{
			printf("[EVENT] adding event: %d\n", *event);
			*tgp += cxt->get_event(*event);
		}

		// FIX: this is not properly its function, so this must be moved somewhere else
		for (auto &e : cxt->get_events())
			*tgp += e.second;
		return (TaskGraph *)tgp;
	}
}
