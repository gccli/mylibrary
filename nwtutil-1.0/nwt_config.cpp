#include "nwt_config.h"
#include <string.h>
#include <stdlib.h>

bool gl_daemon_enable;
bool gl_chroot_enable;
bool gl_syslog_enable;
bool gl_verbose_enable;
bool gl_rete_control_enable;
bool gl_ipmulti_loopback_enable;
bool gl_terminate;
bool gl_spec_sleep;
bool gl_process_hex_stdin;

int  gl_loop_mode;
int  gl_listen_mode;
int  gl_send_count;
int  gl_send_rate;

int  gl_ipmulti_ttl;
int  gl_source_port;
int  gl_destination_port;
int  gl_thread_pool_size;
int  gl_sleep_usec;

const char* gl_destination_ports;  // for check destination port, e.g. 20-80 
const char* gl_source_ipaddress;
const char* gl_source_ipaddresses; // for altered source ip address
const char* gl_destination_ipaddress;
const char* gl_config_file;
const char* gl_log_file;
const char* gl_default_prefix;
const char* gl_default_rootdir;

const char* gl_ipmulti_groupaddr;
const char* gl_ipmulti_outputif;
const char* gl_ipmulti_inputif;
const char* gl_sendfilename;
const char* gl_sendbuffer;


int readconf(const char* conf)
{
	return 0;
}

void string_setting(const char* p_value, const char** p_storage)
{
	char* p_curr_val = (char*) *p_storage;
	if (p_curr_val != 0) {
		free(p_curr_val);
		p_curr_val = 0;
	}
	if (p_value != 0) {
		p_value = strdup(p_value);
	}
	*p_storage = p_value;
}

void initialize_default_conf()
{
	gl_daemon_enable		   = false;
	gl_chroot_enable		   = false;
	gl_syslog_enable		   = false;
	gl_verbose_enable		   = false;
	gl_rete_control_enable	   = false;
	gl_ipmulti_loopback_enable = false;
	gl_terminate			   = false;
	gl_spec_sleep              = false;
	gl_process_hex_stdin       = false;

	gl_loop_mode   = 0;
	gl_listen_mode = 0;
	gl_send_count  = 0;
	gl_send_rate   = 0;

	gl_source_port = 3200; // default local listen port
	gl_destination_port = 0;
	gl_thread_pool_size = 8;
	gl_sleep_usec       = 1000; // 1 ms

	gl_source_ipaddress = NULL;
	gl_sendfilename     = NULL;

	gl_ipmulti_outputif = NULL;
	gl_ipmulti_inputif  = NULL;
	gl_sendbuffer = NULL;

	string_setting ("/var/log/nwtutil.log", &gl_log_file);
	string_setting ("/etc/nwt.conf", &gl_config_file);
	string_setting ("/usr/local", &gl_default_prefix);
	string_setting (gl_default_prefix, &gl_default_rootdir);
}

