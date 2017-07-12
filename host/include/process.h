/*! \file
 *  \brief Support for GN emulation of HN node
 */
#ifndef GN_PROCESS
#define GN_PROCESS

#ifdef __cplusplus
extern "C"{
#endif
#include <sys/types.h>
#include <stdint.h>

/*! \brief fork/execvp wrapper
 * \param filename The command to run (binary kernel filename)
 * \param args The packaged args
 * \returns pid of the spawned process
 */
pid_t gn_execvp(char *filename, char **args);

/*! \brief wait for completion of the child process 
 * \param pid The pid of the child process
 * \returns The exit status of the child process
 */
uint32_t gn_wait(pid_t pid);

#ifdef __cplusplus
}
#endif
#endif /* GN_PROCESS */
