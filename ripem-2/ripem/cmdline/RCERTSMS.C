/* Put USAGE_MESSAGE_LINE1 in a separate buffer so that main can append the
     RIPEM_VERSION string.
 */
char USAGE_MESSAGE_LINE1[80] = 
 "rcerts: Certificate Manager for RIPEM version ";

char *RCERTS_USAGE_MESSAGE[] = {
 USAGE_MESSAGE_LINE1,
 "Menu-driven utility for managing certificates and CRLs.",
 "rcerts is in the public domain, but requires agreeing to the RSAREF license",
 "from RSA Data Security; contact rsaref-info@rsa.com.  It's free but limited.",
 "Usage:  rcerts [-u myusername] [-H home_dir]",
 "  [-p publickey_infile(s)] [-s privatekey_infile] [-k key_to_private_key or -]",
 "  [-y pub_key_server_name] [-Y key_sources]",
 "  [-D debug_level] [-Z debug_out_file]          ",
 "where:",
 "  key_sources is a string of one or more of \"fgs\", which tell ripem to look",
 "    for public keys from a File, finGer, or Server, in the order specified.",
 "Relevant environment variables:   RIPEM_HOME_DIR,",
 "  RIPEM_PUBLIC_KEY_FILE, RIPEM_PRIVATE_KEY_FILE, RIPEM_KEY_TO_PRIVATE_KEY,",
 "  RIPEM_USER_NAME, RIPEM_SERVER_NAME, RIPEM_ARGS",
(char *)0
};
