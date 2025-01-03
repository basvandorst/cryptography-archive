# Gopher protocol client interface

import string

# Default selector, host and port
DEF_SELECTOR = '1/'
DEF_HOST     = 'gopher.micro.umn.edu'
DEF_PORT     = 70

# Recognized file types
A_TEXT       = '0'
A_MENU       = '1'
A_CSO        = '2'
A_ERROR      = '3'
A_MACBINHEX  = '4'
A_PCBINHEX   = '5'
A_UUENCODED  = '6'
A_INDEX      = '7'
A_TELNET     = '8'
A_BINARY     = '9'
A_DUPLICATE  = '+'
A_SOUND      = 's'
A_EVENT      = 'e'
A_CALENDAR   = 'c'
A_HTML       = 'h'
A_TN3270     = 'T'
A_MIME       = 'M'
A_IMAGE      = 'I'
A_WHOIS      = 'w'
A_QUERY      = 'q'
A_GIF        = 'g'
A_HTML       = 'h'			# HTML file
A_WWW        = 'w'			# WWW address
A_PLUS_IMAGE = ':'
A_PLUS_MOVIE = ';'
A_PLUS_SOUND = '<'


# Function mapping all file types to strings; unknown types become TYPE='x'
_names = dir()
_type_to_name_map = None
def type_to_name(gtype):
	global _type_to_name_map
	if not _type_to_name_map:
		for name in _names:
			if name[:2] == 'A_':
				_type_to_name_map[eval(name)] = name[2:]
	if _type_to_name_map.has_key(gtype):
		return _type_to_name_map[gtype]
	return 'TYPE=' + `gtype`

# Names for characters and strings
CRLF = '\r\n'
TAB = '\t'

# Send a selector to a given host and port, return a file with the reply
def send_selector(selector, host, *args):
	import socket
	import string
	if args:
		if args[1:]: raise TypeError, 'too many args'
		port = args[0]
	else:
		port = None
		i = string.find(host, ':')
		if i >= 0:
			host, port = host[:i], string.atoi(host[i+1:])
	if not port:
		port = DEF_PORT
	elif type(port) == type(''):
		port = string.atoi(port)
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.connect(host, port)
	s.send(selector + CRLF)
	s.shutdown(1)
	return s.makefile('r')

# Send a selector and a query string
def send_query(selector, query, host, *args):
	return apply(send_selector, (selector + '\t' + query, host) + args)

# The following functions interpret the data returned by the gopher
# server according to the expected type, e.g. textfile or directory

# Get a directory in the form of a list of entries
def get_directory(f):
	import string
	list = []
	while 1:
		line = f.readline()
		if not line:
			print '(Unexpected EOF from server)'
			break
		if line[-2:] == CRLF:
			line = line[:-2]
		elif line[-1:] in CRLF:
			line = line[:-1]
		if line == '.':
			break
		if not line:
			print '(Empty line from server)'
			continue
		gtype = line[0]
		parts = string.splitfields(line[1:], TAB)
		if len(parts) < 4:
			print '(Bad line from server:', `line`, ')'
			continue
		if len(parts) > 4:
			if parts[4:] != ['+']:
			    print '(Extra info from server:', parts[4:], ')'
		else:
			parts.append('')
		parts.insert(0, gtype)
		list.append(parts)
	return list

# Get a text file as a list of lines, with trailing CRLF stripped
def get_textfile(f):
	list = []
	get_alt_textfile(f, list.append)
	return list

# Get a text file and pass each line to a function, with trailing CRLF stripped
def get_alt_textfile(f, func):
	while 1:
		line = f.readline()
		if not line:
			print '(Unexpected EOF from server)'
			break
		if line[-2:] == CRLF:
			line = line[:-2]
		elif line[-1:] in CRLF:
			line = line[:-1]
		if line == '.':
			break
		if line[:2] == '..':
			line = line[1:]
		func(line)

# Get a binary file as one solid data block
def get_binary(f):
	data = f.read()
	return data

# Get a binary file and pass each block to a function
def get_alt_binary(f, func, blocksize):
	while 1:
		data = f.read(blocksize)
		if not data:
			break
		func(data)

# Trivial test program
def test():
	import sys
	import getopt
	opts, args = getopt.getopt(sys.argv[1:], '')
	selector = DEF_SELECTOR
	type = selector[0]
	host = DEF_HOST
	port = DEF_PORT
	if args:
		host = args[0]
		args = args[1:]
	if args:
		type = args[0]
		args = args[1:]
		if len(type) > 1:
			type, selector = type[0], type
		else:
			selector = ''
			if args:
				selector = args[0]
				args = args[1:]
		query = ''
		if args:
			query = args[0]
			args = args[1:]
	if type == A_INDEX:
		f = send_query(selector, query, host)
	else:
		f = send_selector(selector, host)
	if type == A_TEXT:
		list = get_textfile(f)
		for item in list: print item
	elif type in (A_MENU, A_INDEX):
		list = get_directory(f)
		for item in list: print item
	else:
		data = get_binary(f)
		print 'binary data:', len(data), 'bytes:', `data[:100]`[:40]

# Run the test when run as script
if __name__ == '__main__':
	test()
