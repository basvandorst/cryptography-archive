# pdb.py -- finally, a Python debugger!

# (See pdb.doc for documentation.)

import string
import sys
import linecache
import cmd
import bdb
import repr


class Pdb(bdb.Bdb, cmd.Cmd):
	
	def __init__(self):
		bdb.Bdb.__init__(self)
		cmd.Cmd.__init__(self)
		self.prompt = '(Pdb) '
	
	def reset(self):
		bdb.Bdb.reset(self)
		self.forget()
	
	def forget(self):
		self.lineno = None
		self.stack = []
		self.curindex = 0
		self.curframe = None
	
	def setup(self, f, t):
		self.forget()
		self.stack, self.curindex = self.get_stack(f, t)
		self.curframe = self.stack[self.curindex][0]
	
	# Override Bdb methods (except user_call, for now)
	
	def user_line(self, frame):
		# This function is called when we stop or break at this line
		self.interaction(frame, None)
	
	def user_return(self, frame, return_value):
		# This function is called when a return trap is set here
		frame.f_locals['__return__'] = return_value
		print '--Return--'
		self.interaction(frame, None)
	
	def user_exception(self, frame, (exc_type, exc_value, exc_traceback)):
		# This function is called if an exception occurs,
		# but only if we are to stop at or just below this level
		frame.f_locals['__exception__'] = exc_type, exc_value
		print exc_type + ':', repr.repr(exc_value)
		self.interaction(frame, exc_traceback)
	
	# General interaction function
	
	def interaction(self, frame, traceback):
		self.setup(frame, traceback)
		self.print_stack_entry(self.stack[self.curindex])
		self.cmdloop()
		self.forget()

	def default(self, line):
		if line[:1] == '!': line = line[1:]
		locals = self.curframe.f_locals
		globals = self.curframe.f_globals
		globals['__privileged__'] = 1
		try:
			exec(line + '\n', globals, locals)
		except:
			print '***', sys.exc_type + ':', sys.exc_value

	# Command definitions, called by cmdloop()
	# The argument is the remaining string on the command line
	# Return true to exit from the command loop 
	
	do_h = cmd.Cmd.do_help

	def do_break(self, arg):
		if not arg:
			print self.get_all_breaks() # XXX
			return
		# Try line number as argument
		try:	
			lineno = int(eval(arg))
			filename = self.curframe.f_code.co_filename
		except:
			# Try function name as the argument
			import codehack
			g_frame = self.curframe.f_globals
			try:
				code = eval(arg, g_frame).func_code 
			except:
				print '*** Could not eval argument:', arg
				return
			lineno = codehack.getlineno(code)
			filename = code.co_filename

		# now set the break point
		err = self.set_break(filename, lineno)
		if err: print '***', err
	do_b = do_break
	
	def do_clear(self, arg):
		if not arg:
			try:
				reply = raw_input('Clear all breaks? ')
			except EOFError:
				reply = 'no'
			reply = string.lower(string.strip(reply))
			if reply in ('y', 'yes'):
				self.clear_all_breaks()
			return
		try:
			lineno = int(eval(arg))
		except:
			print '*** Error in argument:', `arg`
			return
		filename = self.curframe.f_code.co_filename
		err = self.clear_break(filename, lineno)
		if err: print '***', err
	do_cl = do_clear # 'c' is already an abbreviation for 'continue'
	
	def do_where(self, arg):
		self.print_stack_trace()
	do_w = do_where
	
	def do_up(self, arg):
		if self.curindex == 0:
			print '*** Oldest frame'
		else:
			self.curindex = self.curindex - 1
			self.curframe = self.stack[self.curindex][0]
			self.print_stack_entry(self.stack[self.curindex])
			self.lineno = None
	do_u = do_up
	
	def do_down(self, arg):
		if self.curindex + 1 == len(self.stack):
			print '*** Newest frame'
		else:
			self.curindex = self.curindex + 1
			self.curframe = self.stack[self.curindex][0]
			self.print_stack_entry(self.stack[self.curindex])
			self.lineno = None
	do_d = do_down
	
	def do_step(self, arg):
		self.set_step()
		return 1
	do_s = do_step
	
	def do_next(self, arg):
		self.set_next(self.curframe)
		return 1
	do_n = do_next
	
	def do_return(self, arg):
		self.set_return(self.curframe)
		return 1
	do_r = do_return
	
	def do_continue(self, arg):
		self.set_continue()
		return 1
	do_c = do_cont = do_continue
	
	def do_quit(self, arg):
		self.set_quit()
		return 1
	do_q = do_quit
	
	def do_args(self, arg):
		if self.curframe.f_locals.has_key('__return__'):
			print `self.curframe.f_locals['__return__']`
		else:
			print '*** Not arguments?!'
	do_a = do_args
	
	def do_retval(self, arg):
		if self.curframe.f_locals.has_key('__return__'):
			print self.curframe.f_locals['__return__']
		else:
			print '*** Not yet returned!'
	do_rv = do_retval
	
	def do_p(self, arg):
		self.curframe.f_globals['__privileged__'] = 1
		try:
			value = eval(arg, self.curframe.f_globals, \
					self.curframe.f_locals)
		except:
			print '***', sys.exc_type + ':', `sys.exc_value`
			return

		print `value`

	def do_list(self, arg):
		self.lastcmd = 'list'
		last = None
		if arg:
			try:
				x = eval(arg, {}, {})
				if type(x) == type(()):
					first, last = x
					first = int(first)
					last = int(last)
					if last < first:
						# Assume it's a count
						last = first + last
				else:
					first = max(1, int(x) - 5)
			except:
				print '*** Error in argument:', `arg`
				return
		elif self.lineno is None:
			first = max(1, self.curframe.f_lineno - 5)
		else:
			first = self.lineno + 1
		if last == None:
			last = first + 10
		filename = self.curframe.f_code.co_filename
		breaklist = self.get_file_breaks(filename)
		try:
			for lineno in range(first, last+1):
				line = linecache.getline(filename, lineno)
				if not line:
					print '[EOF]'
					break
				else:
					s = string.rjust(`lineno`, 3)
					if len(s) < 4: s = s + ' '
					if lineno in breaklist: s = s + 'B'
					else: s = s + ' '
					if lineno == self.curframe.f_lineno:
						s = s + '->'
					print s + '\t' + line,
					self.lineno = lineno
		except KeyboardInterrupt:
			pass
	do_l = do_list

	def do_whatis(self, arg):
		try:
			value = eval(arg, self.curframe.f_globals, \
					self.curframe.f_locals)
		except:
			print '***', sys.exc_type + ':', `sys.exc_value`
			return
		code = None
		# Is it a function?
		try: code = value.func_code
		except: pass
		if code:
			print 'Function', code.co_name
			return
		# Is it an instance method?
		try: code = value.im_func.func_code
		except: pass
		if code:
			print 'Method', code.co_name
			return
		# None of the above...
		print type(value)
	
	# Print a traceback starting at the top stack frame.
	# The most recently entered frame is printed last;
	# this is different from dbx and gdb, but consistent with
	# the Python interpreter's stack trace.
	# It is also consistent with the up/down commands (which are
	# compatible with dbx and gdb: up moves towards 'main()'
	# and down moves towards the most recent stack frame).
	
	def print_stack_trace(self):
		try:
			for frame_lineno in self.stack:
				self.print_stack_entry(frame_lineno)
		except KeyboardInterrupt:
			pass
	
	def print_stack_entry(self, frame_lineno):
		frame, lineno = frame_lineno
		if frame is self.curframe:
			print '>',
		else:
			print ' ',
		print self.format_stack_entry(frame_lineno)


# Simplified interface

def run(statement):
	Pdb().run(statement)

def runctx(statement, globals, locals):
	Pdb().runctx(statement, globals, locals)

def runcall(*args):
	apply(Pdb().runcall, args)


# Post-Mortem interface

def post_mortem(t):
	p = Pdb()
	p.reset()
	while t.tb_next <> None: t = t.tb_next
	p.interaction(t.tb_frame, t)

def pm():
	import sys
	post_mortem(sys.last_traceback)


# Main program for testing

TESTCMD = 'import x; x.main()'

def test():
	run(TESTCMD)

# print help
def help():
	import os
	for dirname in sys.path:
		fullname = os.path.join(dirname, 'pdb.doc')
		if os.path.exists(fullname):
			sts = os.system('${PAGER-more} '+fullname)
			if sts: print '*** Pager exit status:', sts
			break
	else:
		print 'Sorry, can\'t find the help file "pdb.doc"',
		print 'along the Python search path'
