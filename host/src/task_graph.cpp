#include "task_graph.h"
#include "logger.h"

namespace mango {


TaskGraph::TaskGraph(
	std::initializer_list<std::shared_ptr<Kernel>> lkernels, 
	std::initializer_list<std::shared_ptr<Buffer>> lbuffers, 
	std::initializer_list<std::shared_ptr<Event>> levents) noexcept {

	for(std::shared_ptr<Event> e : levents) {
		events.push_back(e);
	}		

	for(std::shared_ptr<Kernel> k : lkernels) {
		kernels.push_back(k);
		events.push_back(k->get_termination_event());
		for(const auto e : k->get_task_events())
			events.push_back(e);
	}

	for(auto b : lbuffers) {
		mango_log->Debug("Buffer %d event %d", b->get_id(), b->get_event()->get_id());
		buffers.push_back(b);
		events.push_back(b->get_event());
	}
	
	mango_log->Info("Local Task Graph constructed: %d kernels, %d buffers, %d events", 
			kernels.size(), buffers.size(), events.size() );

}
	

TaskGraph::~TaskGraph() {
	mango_log->Info("Destructing local task graph...");
	kernels.clear();
	buffers.clear();
	events.clear();
}

} // namespace mango
