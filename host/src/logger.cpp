#include "logger.h"

#define BBQUE_APP_CONFIG_FILE    BBQUE_PATH_PREFIX "/" BBQUE_PATH_CONF "/libmango.conf"

namespace mango {

std::shared_ptr<bbque::utils::Logger> mango_log;

extern void mango_init_logger() {
	bbque::utils::Logger::SetConfigurationFile(BBQUE_APP_CONFIG_FILE);
	mango_log = bbque::utils::Logger::GetLogger("libmango");
}

}
