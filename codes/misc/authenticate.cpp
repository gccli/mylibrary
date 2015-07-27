#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <sys/stat.h>
#include <fcntl.h>

#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <getopt.h>

#include <security/pam_appl.h>
#include <security/pam_misc.h>


static struct pam_conv conv = {
    misc_conv,
    NULL
};
/* Readme       
 * @compile this program
 * g++ -g -Wall authenticate.cpp -o myauth -lpam -lpam_misc
 * @pam comfiguration
 * create a file /etc/pam.d/myauth with the following content
   auth            include         system-auth
   account         include         system-auth
   password        include         system-auth
   session         include         system-auth
*/ 

int main(int argc, char* argv[])
{
    int retval;

    const char *user, *tty;
    char hostname[128];
    const char *temp=NULL;
    pam_handle_t *pamh=NULL;
    do {
      if (argc > 1)
	user=argv[1];
      else
	user=getlogin();
      if ((retval = pam_start(basename(argv[0]), user, &conv, &pamh)) != PAM_SUCCESS)
	break;

      gethostname(hostname, sizeof(hostname));
      if ((retval = pam_set_item(pamh, PAM_RHOST, hostname)) != PAM_SUCCESS)
	break;
      tty = ttyname(STDERR_FILENO);
      if ((retval = pam_set_item(pamh, PAM_TTY, tty)) != PAM_SUCCESS)
	break;

/* authenticate the applicant */
        if ((retval = pam_authenticate(pamh, 0)) != PAM_SUCCESS)
	    break;
        if ((retval = pam_acct_mgmt(pamh, 0)) != PAM_SUCCESS) {
	  if (retval == PAM_NEW_AUTHTOK_REQD) 
	    retval = pam_chauthtok(pamh, PAM_CHANGE_EXPIRED_AUTHTOK);
	  else
	    break;
	}

/* establish the requested credentials */
	if ((retval = pam_setcred(pamh, PAM_ESTABLISH_CRED)) != PAM_SUCCESS)
	    break;

	/* authentication succeeded; open a session */
	if ((retval = pam_open_session(pamh, 0)) != PAM_SUCCESS)
	  break;

        printf("Authenticate ok\n");
	pam_get_item(pamh, PAM_SERVICE, (const void **)&temp);
	printf("  service  %s\n", temp);
	pam_get_item(pamh, PAM_USER, (const void **)&temp);
	printf("  user     %s\n", temp);
	pam_get_item(pamh, PAM_AUTHTOK, (const void **)&temp);
	printf("  password %s\n", temp);



    }while (0);

    if (retval != PAM_SUCCESS) {
	printf("Authenticate failed: %s\n", pam_strerror(pamh,retval));
    } else {
      pam_close_session(pamh,0);
    }
    pam_end(pamh,retval);

    return retval == PAM_SUCCESS ? 0:1;
}
