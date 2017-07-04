#include "kernel_arguments.h"
#include "logger.h"

#include <libhn/hn.h>
#include <memory>


namespace mango {

template<typename Base, typename T>
static inline bool p_instanceof(const T ptr) {
    return std::dynamic_pointer_cast<const Base>(ptr) != nullptr;
}


template <typename T>
ScalarArg<T>::ScalarArg(T arg) noexcept
{
	static_assert(sizeof(T) <= sizeof(mango_size_t), "This class supports only types "
							 "with size <= mango_addr_t bytes");

	this->value = (mango_size_t)arg;
	this->size  = sizeof(T);
}

template class ScalarArg<char>;
template class ScalarArg<unsigned char>;
template class ScalarArg<short>;
template class ScalarArg<unsigned short>;
template class ScalarArg<int>;
template class ScalarArg<unsigned int>;
//template class ScalarArg<long>;
//template class ScalarArg<unsigned long>;

BufferArg::BufferArg(std::shared_ptr<const Buffer> arg) noexcept
{
	this->id    = arg->get_id();
	this->size  = sizeof(void *);	// TODO Check this
}

EventArg::EventArg(std::shared_ptr<const Event> arg) noexcept
{
	this->id    = arg->get_id();
	this->size  = sizeof(mango_id_t);	// TODO Check this
}

KernelArguments::KernelArguments(const std::initializer_list<Arg *> &arguments,
					std::shared_ptr<Kernel> kernel) noexcept
{ 

	std::vector<Arg *> temp(arguments);
	setup(temp, kernel);

}

KernelArguments::KernelArguments(const std::vector<Arg *> &arguments,
					std::shared_ptr<Kernel> kernel) noexcept {
	setup(arguments, kernel);
}

void KernelArguments::setup(const std::vector<Arg *> &arguments,
					std::shared_ptr<Kernel> kernel) noexcept
{

	auto event = kernel->get_termination_event();
	auto tlb = kernel->get_tlb();

	std::shared_ptr<EventArg> ev_arg( new EventArg(event) );
	ev_arg->set_value(tlb);
	args.push_back(ev_arg);

	for(const auto ev : kernel->get_task_events()) {
		std::shared_ptr<EventArg> ev_arg_( new EventArg(ev) );
		ev_arg_->set_value(tlb);
		args.push_back(ev_arg_);
	}

	for (Arg *a : arguments) {
		auto p_a = std::shared_ptr<Arg>(a);
		p_a->set_value(tlb);
		args.push_back(p_a);
	}

}

std::string KernelArguments::get_arguments() const noexcept {


	std::stringstream ss;
	ss << "x";	// TODO: change the 'x' with the binary name

	for (const auto arg : args) {

		if (p_instanceof<BufferArg>(arg) || p_instanceof<EventArg>(arg)) {
			ss << " 0x" << std::hex << arg->get_value();
		}
		else if ( p_instanceof<ScalarArg<mango_size_t>>(arg) 
			 || p_instanceof<ScalarArg<char>>(arg) 
			 || p_instanceof<ScalarArg<unsigned char>>(arg) 
			 || p_instanceof<ScalarArg<short>>(arg) 
			 || p_instanceof<ScalarArg<unsigned short>>(arg) 
			 || p_instanceof<ScalarArg<int>>(arg) 
			 || p_instanceof<ScalarArg<unsigned int>>(arg) 
 		) {
			ss << " " << arg->get_value();
		}
		else {
			mango_log->Warn("Unrecognized class in argument vectors.");
		}
	}

	return ss.str();

}

} // namespace mango
