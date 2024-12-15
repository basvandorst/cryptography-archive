import news
import stdwin
import gwin
import listwin
import string

from stdwinevents import *

stdwin.setfgcolor(stdwin.fetchcolor('black'))
stdwin.setbgcolor(stdwin.fetchcolor('gray80'))

def server(host):
  return news.server(host)

def subs_action(w, string, i, detail):
  (h, v), clicks, button, mask = detail
  if clicks == 2:
    w2 = view_art(w.server, w.group, w.ovs[i][6])

def groups_action(w, string, i, detail):
  (h, v), clicks, button, mask = detail
  if clicks == 2:
    w2 = view_subs(w.server, w.groups[i][0])

def view_groups(server):
  groups = server.list()
  print 'after server.list()'
  group_names = map (lambda (x): x[0], groups)
  print 'after map'
  groupwin = listwin.open('News!', group_names)
  print 'after listwin.open()'
  groupwin.server = server
  groupwin.action = groups_action
  groupwin.groups = groups
  return groupwin

def view_subs(server, group):
  print 'grabbing overviews for ' + group
  low, high, estnum = server.group(group)
  if (estnum > 50):
    low = high - 50
  ovs = server.overviews(group, low, high)
  subs = map (lambda (x): x[0], ovs)
  subwin = listwin.open(group + ' subjects', subs)
  subwin.action = subs_action
  subwin.server = server
  subwin.group = group
  subwin.ovs = ovs
  return subwin

def view_art(server, group, artnum):
  art = server.article(group, artnum)
  print art[0]
  art_lines = string.splitfields(art[0], '\012')
  title = group + ':' + str(artnum)
  win = listwin.open(title, art_lines)
  
def go_nntp(name):
  server = news.nntp(name, None, None)
  mainwin = view_groups(server)
  gwin.mainloop()

def go_file():
  server = news.file ('/var/spool/news', '/usr/lib/news2', None, 'fnord', '/')
  mainwin = view_groups(server)
  gwin.mainloop()

