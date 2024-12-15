
import news
import string
import sys
import regex

server = ''

def print_art(art):
  for x in string.splitfields(art[0], '\n'):
    print x
  print '<return> to continue...',
  sys.stdout.flush()
  temp = sys.stdin.readline()[:-1]

def pick_article (groupname, info):
  global server
  arts = []
  print 'Enter a regex to match: ',
  sys.stdout.flush()
  reg = regex.compile(sys.stdin.readline()[:-1])
  print 'Grabbing overviews...\n'
  sys.stdout.flush()
  artnum = ''
  ov = server.overviews (groupname, info[0], info[1])
  for x in ov:
    if reg.match(x[0]) != -1:
      print 'grabbing article # ', x[6]
      art = server.article(groupname, x[6])
      arts.append(art)
      print_art (art)
  return arts

def pick_group():
  print 'What group ? : ',
  sys.stdout.flush()
  groupname = sys.stdin.readline()[:-1]
  print 'You chose: ' + groupname
  print server.group(groupname)
  info = ''
  try:
    info = server.group(groupname)
  except news.error:
    print 'No such group'
  if info != '':
    return pick_article (groupname, info)

def go_file():
  global server
  server = news.file('/var/spool/news', '/usr/lib/news2', None, 'fnord', '/')
  return pick_group()

def go_nntp(name):
  global server
  server = news.nntp(name, None, None)
  return pick_group()

