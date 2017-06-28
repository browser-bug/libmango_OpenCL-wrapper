/*! \file
 *  \brief Kernel argument packing
 */
#ifndef KERNEL_ARGUMENTS_H
#define KERNEL_ARGUMENTS_H
#include "kernel.h"
#include <string>

namespace mango {

/*! \brief Kernel argument type 
 *
 * \note Arguments are typically pointers to buffers, with the appropriate size.
 * Alternately, they can be scalar values or events.
 * Subclasses are used for the three types.
 */
class Arg {
public:
	virtual ~Arg() {}

	inline mango_size_t get_value() const noexcept { return this->value; }
	inline mango_size_t get_size()  const noexcept { return this->size;  }
	inline mango_id_t   get_id()    const noexcept { return this->id;    }

	inline void set_value(mango_size_t value) noexcept { this->value = value; }
	virtual void set_value(std::shared_ptr<const TLB> tlb) noexcept {};

protected:
	/* \brief This class cannot be instantitated directly */
	Arg() {}	

	mango_size_t value; /*!< Pointer to memory buffer (for memories), or value of the parameter */
	mango_size_t size;  /*!< Size of memory object for memories, and of the type otherwise */	
	mango_id_t id; 	    /*!< Used for Events and Buffer args */
};

template <typename T>
class ScalarArg : public Arg {
public:
	ScalarArg(T arg) noexcept;
	virtual ~ScalarArg() {};
};


class BufferArg : public Arg {
public: 
	BufferArg(std::shared_ptr<const Buffer> arg) noexcept;
	virtual ~BufferArg(){};
	virtual void set_value(std::shared_ptr<const TLB> tlb) noexcept { this->value = tlb->get_virt_addr_buffer(this->id); }
};

class EventArg : public Arg {
public:
	EventArg(std::shared_ptr<const Event> arg) noexcept;
	//EventArg(std::shared_ptr<const Event> arg, std::shared_ptr<const TLB> tlb) noexcept;
	virtual ~EventArg(){};
	virtual void set_value(std::shared_ptr<const TLB> tlb) noexcept { this->value = tlb->get_virt_addr_event(this->id); }

};


/*! \brief A class describing the arguments passed to kernels.
 * Can be specialized to generate argument serialization for other platforms.
 */
class KernelArguments {
public:
	KernelArguments(const std::initializer_list<Arg *> &arguments, 
			std::shared_ptr<Kernel> kernel) noexcept;

		
	KernelArguments(
			const std::vector<Arg *> &arguments,
			std::shared_ptr<Kernel> kernel) noexcept;

	inline int get_nr_args() const noexcept {
		return args.size();
	};

	std::string get_arguments() const noexcept;

private:
	void setup(const std::vector<Arg *> &arguments,
			std::shared_ptr<Kernel> kernel) noexcept;

	std::vector<std::shared_ptr<Arg>> args;
	std::string peak_argstring;

};


}
#endif /* KERNEL_ARGUMENTS_H */