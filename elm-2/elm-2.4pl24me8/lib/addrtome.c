extern char *username;

int
addressed_to_me (char *s) {
	char *ptr;
	char c;
	int len;

	c = username[0];
	len = strlen (username);

	ptr = index (s, c);
	if (ptr == 0)
		return 0;
	while (*ptr != '\0') {
		if (strncmp (ptr, username, len) == 0)
			return 1;
		else
			while (*p != c && *p != '\0')
				p++;
	}
	return 0;
}
