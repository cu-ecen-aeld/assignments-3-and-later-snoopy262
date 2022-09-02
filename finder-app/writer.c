#include <syslog.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char **argv)
{
	openlog("writer", LOG_CONS|LOG_NDELAY, LOG_USER);

	if(argc != 3)
	{
		syslog(LOG_ERR, "Wrong Number of Arguments (%d). Expected 2.", argc-1);
		return 1;
	}

	int fd = open(argv[1], O_CREAT | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);

	if(fd == -1)
	{
		syslog(LOG_ERR, "File %s cannot be opened", argv[1]);
		return 1;
	}
	
	int written = write(fd, argv[2], strlen(argv[2]));

	if(written == -1)
	{
		syslog(LOG_ERR, "String %s of lenght %lu cannot be written", argv[2], strlen(argv[2]));
		return 1;
	}

	syslog(LOG_ERR, "Writing %s to %s", argv[2], argv[1]);

	return 0;
}
