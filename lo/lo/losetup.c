/*
 * losetup.c - setup and control loop devices
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <module/loop.h>

static char *progname;


struct crypt_type_struct {
	int id;
	char *name;
} crypt_type_tbl[] = {
	{ LO_CRYPT_NONE,"no" },
	{ LO_CRYPT_NONE,"none" },
	{ LO_CRYPT_XOR,	"xor" },
	{ LO_CRYPT_DES,	"DES" },
	{ -1,		NULL }
};


char *crypt_name(int id)
{
	int i;

	for (i = 0; crypt_type_tbl[i].id != -1; i++)
		if (id == crypt_type_tbl[i].id)
			return crypt_type_tbl[i].name;
	return "undefined";
}


int crypt_type(char *name)
{
	int i;

	for (i = 0; crypt_type_tbl[i].id != -1; i++)
		if (!strcasecmp(name, crypt_type_tbl[i].name))
			return crypt_type_tbl[i].id;
	return -1;
}


void show_loop(char *device)
{
	struct	loop_info	loopinfo;
	int			fd;
	
	if ((fd = open(device, O_RDWR)) < 0) {
		perror(device);
		return;
	}
	if (ioctl(fd, LOOP_GET_STATUS, &loopinfo) < 0) {
		perror("Cannot get loop info");
		close(fd);
		return;
	}
	printf("%s: [%04x]:%ld (%s) offset %d, %s encryption\n",
	       device, loopinfo.lo_device, loopinfo.lo_inode,
	       loopinfo.lo_name, loopinfo.lo_offset,
	       crypt_name(loopinfo.lo_encrypt_type));
	close(fd);
}


void set_loop(char *device, char *file, int offset, char *encryption)
{
	struct loop_info	loopinfo;
	int	fd, ffd, i;
	char	*pass;
	
	if ((fd = open(device, O_RDWR)) < 0) {
		perror(device);
		exit(1);
	}
	if ((ffd = open(file, O_RDWR)) < 0) {
		perror(file);
		exit(1);
	}
	memset(&loopinfo, 0, sizeof(loopinfo));
	strncpy(loopinfo.lo_name, file, LO_NAME_SIZE);
	loopinfo.lo_name[LO_NAME_SIZE-1] = 0;
	if (encryption && (loopinfo.lo_encrypt_type = crypt_type(encryption))
	    < 0) {
		fprintf(stderr,"Unsupported encryption type %s",encryption);
		exit(1);
	}
	loopinfo.lo_offset = offset;
	switch (loopinfo.lo_encrypt_type) {
	case LO_CRYPT_NONE:
		loopinfo.lo_encrypt_key_size = 0;
		break;
	case LO_CRYPT_XOR:
		pass = getpass("Password: ");
		strncpy(loopinfo.lo_encrypt_key, pass, LO_KEY_SIZE);
		loopinfo.lo_encrypt_key[LO_KEY_SIZE-1] = 0;
		loopinfo.lo_encrypt_key_size = strlen(loopinfo.lo_encrypt_key);
		break;
	case LO_CRYPT_DES:
		pass = getpass("Password: ");
		strncpy(loopinfo.lo_encrypt_key, pass, 8);
		loopinfo.lo_encrypt_key[8] = 0;
		loopinfo.lo_encrypt_key_size = 8;
		pass = getpass("Init (up to 16 hex digits): ");
		for (i = 0; i < 16 && pass[i]; i++)
			if (isxdigit(pass[i]))
				loopinfo.lo_init[i >> 3] |= (pass[i] > '9' ?
				    (islower(pass[i]) ? toupper(pass[i]) :
				    pass[i])-'A'+10 : pass[i]-'0') << (i & 7)*4;
			else {
				fprintf(stderr,"Non-hex digit '%c'.\n",pass[i]);
				exit(1);
			}
		break;
	default:
		fprintf(stderr,
			"Don't know how to get key for encryption system %d\n",
			loopinfo.lo_encrypt_type);
		exit(1);
	}
	if (ioctl(fd, LOOP_SET_FD, ffd) < 0) {
		perror("ioctl: LOOP_SET_FD");
		exit(1);
	}
	if (ioctl(fd, LOOP_SET_STATUS, &loopinfo) < 0) {
		(void) ioctl(fd, LOOP_CLR_FD, 0);
		perror("ioctl: LOOP_SET_STATUS");
		exit(1);
	}
	close(fd);
	close(ffd);
}

void del_loop(char *device)
{
	int fd;

	if ((fd = open(device, O_RDONLY)) < 0) {
		perror(device);
		exit(1);
	}
	if (ioctl(fd, LOOP_CLR_FD, 0) < 0) {
		perror("ioctl: LOOP_CLR_FD");
		exit(1);
	}
	exit(0);
}


int usage()
{
	fprintf(stderr, "usage: %s [ -d ] loop_device\n", progname);
	fprintf(stderr,"%15s[ -e encryption ] [ -o offset ] loop_device "
	    "file\n","");
	exit(1);
}

int main(int argc, char **argv)
{
	char *offset,*encryption;
	int delete,off,c;

	delete = off = 0;
	offset = encryption = NULL;
	progname = argv[0];
	while ((c = getopt(argc,argv,"de:o:")) != EOF) {
		switch (c) {
			case 'd':
				delete = 1;
				break;
			case 'e':
				encryption = optarg;
				break;
			case 'o':
				offset = optarg;
				break;
			default:
				usage();
		}
	}
	if (argc == 1) usage();
	if ((delete && (argc != optind+1 || encryption || offset)) ||
	    (!delete && (argc < optind+1 || argc > optind+2)))
		usage();
	if (argc == optind+1)
		if (delete)
			del_loop(argv[optind]);
		else
			show_loop(argv[optind]);
	else {
		if (offset && sscanf(offset,"%d",&off) != 1)
			usage();
		set_loop(argv[optind],argv[optind+1],off,encryption);
	}
	return 0;
}
