#ifndef NOPROTO
extern int speed_main(int argc,char *argv[]);
extern int ca_main(int argc,char *argv[]);
extern int enc_main(int argc,char *argv[]);
extern int dgst_main(int argc,char *argv[]);
extern int verify_main(int argc,char *argv[]);
extern int asn1parse_main(int argc,char *argv[]);
extern int crl_main(int argc,char *argv[]);
extern int rsa_main(int argc,char *argv[]);
extern int x509_main(int argc,char *argv[]);
extern int genrsa_main(int argc,char *argv[]);
extern int gendh_main(int argc,char *argv[]);
extern int pem2bin_main(int argc,char *argv[]);
extern int bin2pem_main(int argc,char *argv[]);
extern int s_server_main(int argc,char *argv[]);
extern int s_client_main(int argc,char *argv[]);
extern int s_filter_main(int argc,char *argv[]);
extern int req_main(int argc,char *argv[]);
extern int dh_main(int argc,char *argv[]);
extern int errstr_main(int argc,char *argv[]);
extern int hashdir_main(int argc,char *argv[]);
extern int s_time_main(int argc,char *argv[]);
#else
extern int speed_main();
extern int ca_main();
extern int enc_main();
extern int dgst_main();
extern int verify_main();
extern int asn1parse_main();
extern int crl_main();
extern int rsa_main();
extern int x509_main();
extern int genrsa_main();
extern int gendh_main();
extern int pem2bin_main();
extern int bin2pem_main();
extern int s_server_main();
extern int s_client_main();
extern int s_filter_main();
extern int req_main();
extern int dh_main();
extern int errstr_main();
extern int hashdir_main();
extern int s_time_main();
#endif

typedef struct {
	char *name;
	int (*func)();
	} FUNCTION;

FUNCTION functions[] = {
	{"speed",speed_main},
	{"ca",ca_main},
	{"enc",enc_main},
	{"dgst",dgst_main},
	{"verify",verify_main},
	{"asn1parse",asn1parse_main},
	{"crl",crl_main},
	{"rsa",rsa_main},
	{"x509",x509_main},
	{"genrsa",genrsa_main},
	{"gendh",gendh_main},
	{"pem2bin",pem2bin_main},
	{"bin2pem",bin2pem_main},
	{"s_server",s_server_main},
	{"s_client",s_client_main},
	{"s_filter",s_filter_main},
	{"req",req_main},
	{"dh",dh_main},
	{"errstr",errstr_main},
	{"hashdir",hashdir_main},
	{"s_time",s_time_main},
	{"md2",dgst_main},
	{"md5",dgst_main},
	{"sha",dgst_main},
	{"sha1",dgst_main},
	{"des",enc_main},
	{"des3",enc_main},
	{"idea",enc_main},
	{"rc4",enc_main},
	{"des-cbc",enc_main},
	{"des-ede-cbc",enc_main},
	{"des-ede3-cbc",enc_main},
	{"des-ecb",enc_main},
	{"des-ede",enc_main},
	{"des-ede3",enc_main},
	{"des-cfb",enc_main},
	{"des-ede-cfb",enc_main},
	{"des-ede3-cfb",enc_main},
	{"des-ofb",enc_main},
	{"des-ede-ofb",enc_main},
	{"des-ede3-ofb",enc_main},
	{"idea-cbc",enc_main},
	{"idea-ecb",enc_main},
	{"idea-cfb",enc_main},
	{"idea-ofb",enc_main},
	{NULL,NULL}
	};
