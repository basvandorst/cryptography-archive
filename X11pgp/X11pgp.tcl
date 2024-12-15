#!/usr/X386/bin/wish -f
# For tcl 7.3/tk 3.6.
#
# X11pgp 0.9
# This program is public domain. <m@mbsks.franken.de>
# (or <ms1207@cd4680fs.rrze.uni-erlangen.de> or 2:2490/1301.3@FidoNet)
#
# This program requires that pgp is allready installed.
# The menue Option/Packet-Version is only for PGP 2.6ui, but I haven'd tried it.
# If you have an other version you should not use the radiobuttons or delete them
# in the script.
# You can set the userid-variable with your user-ID at the beginning of the script
# or leave it blank, but you must set it.
# You can use the pgppause.sh-script. You copy it e.g. to /usr/local/bin, then
# you must set pgpfile to "/usr/local/bin/pgppause.sh". If you want directly
# start pgp, you must set it to pgp, with or without the path. The script appends
# only a pause of 2 seconds.
# If you press the Encrypt- or Decrypt-button you get the options for encrypting
# and decrypting. To process a file, click only twice on the file in the listbox.
#
# TODO
# - armor_version
# - enable files with spaces (and maybe other not common characters)
# - make pgp-window to current or selected or what-ever-you-call-this window
#
# pgppause.sh :
# ----------------------
# #!/bin/sh
# pgp $*
# sleep2
# ----------------------
#
# "He smiled at the axe ... and the axe smiled back." - Peter Claus Gutmann
#

#-------- Setup ----------

set userid {}
#set userid 0xb8d00f
set pgpfile /usr/local/bin/pgppause.sh
#set pgpfile /usr/local/bin/pgp
set icon /usr/local/pgp/X11pgp.xbm

#-------- Procedure: updatelist ------------

proc updatelist {} {
  global nowdir
  set statuslist [.listbox0 size]
  while {$statuslist != "0"} {
    .listbox0 delete end
    set statuslist [.listbox0 size]
  }
  set dircont [lsort [glob -nocomplain *]]
  set nowdir [pwd]
  if {$nowdir != "/"} {
    .listbox0 insert end {../}
  }
  if {$dircont != ""} {
    foreach addfile $dircont {
      if [file isdirectory $addfile] {
        set addfile ${addfile}/
      }
      .listbox0 insert end $addfile
    }
  }
}

#---------- Procedure: pgp ------------

proc pgp {arg} {
  global pgpfile
  set code [catch {eval exec xterm -T PGP -e $pgpfile $arg}]
  return code
}

#----------- Procedure: encrypt ----------

proc encrypt {} {
  global userid wipe pgpversion sign ascii text signonly receiver selected
  set pgpargs "-${sign}${signonly}${wipe}${ascii}${text} ${pgpversion} ${selected} "
  if {$userid != {}} {
    set pgpargs "${pgpargs} -u \"${userid}\""
  }
  if {$signonly == {e}} {
    set pgpargs "${pgpargs} ${receiver}"
  }
  pgp ${pgpargs}
  updatelist
}

#----------- Procedure: decrypt ----------

proc decrypt {} {
  global view origname selected
  set pgpargs "-${view}${origname} ${selected}"
  pgp ${pgpargs}
  updatelist
}

#----------- Procedure: enterid ----------

proc enterid {} {
  global userid oldFocus

  toplevel .enterid 
  wm title .enterid {Enter UserID}
  entry .enterid.entry -width 30 -relief sunken -bd 2 -textvariable userid
  pack .enterid.entry -side left -padx 1c -pady 1c
  set oldFocus [focus]
  focus .enterid.entry
  bind .enterid.entry <Return> {
    global oldFocus
    destroy .enterid
    focus $oldFocus
    return
  }
  bind .enterid.entry <Double-Button-1> {
    global oldFocus
    destroy .enterid
    focus $oldFocus
    return
  }
}

#----------- Procedure: EncryptFrame ----------

proc encryptframe {} {
  global pgpfunc
  set pgpfunc e
  pack forget .frame5
  pack configure .frame4 -side top -padx 2m -pady 3m
  pack configure .frame4.check0 -expand 1 -fill x
  pack configure .frame4.check3 -expand 1 -fill x
  pack configure .frame4.check1 -expand 1 -fill x
  pack configure .frame4.check2 -expand 1 -fill x
  pack configure .frame4.label0
  pack configure .frame4.label1 -expand 1 -fill x
  pack configure .frame4.entry0 -expand 1 -fill x
}

#----------- Procedure: DecrpytFrame ----------

proc decryptframe {} {
  global pgpfunc
  set pgpfunc d
  pack forget .frame4
  pack configure .frame5 -side top -padx 2m -pady 3m
  pack configure .frame5.check0 -expand 1 -fill x
  pack configure .frame5.check1 -expand 1 -fill x
}

#----------- Procedure: startup ----------

proc startup {} {

  global icon
  wm title . {X11pgp}
  wm iconbitmap . @$icon

#----------- Menue ---------------

  # build widget .frame0
  frame .frame0 -borderwidth {2} -relief {raised}

#----- Menue Files --------

  # build widget .frame0.menubutton1
  menubutton .frame0.menubutton1 -menu {.frame0.menubutton1.m} -text {File}

  # build widget .frame0.menubutton1.m
  menu .frame0.menubutton1.m
  .frame0.menubutton1.m add command -command {exit} -label {Quit}

#----- Menue Options -------

  # build widget .frame0.menubutton2
  menubutton .frame0.menubutton2 -menu {.frame0.menubutton2.m} -text {Options}

  # build widget .frame0.menubutton2.m
  menu .frame0.menubutton2.m
  .frame0.menubutton2.m add command -command {enterid} -label {UserID ...}
  .frame0.menubutton2.m add separator
  .frame0.menubutton2.m add checkbutton -label {Wipe file} -variable wipe \
    -offvalue {} -onvalue {w} -state normal
  .frame0.menubutton2.m add separator
  .frame0.menubutton2.m add cascade -label {Packet-Version} \
    -menu .frame0.menubutton2.m.version

  # build widget .frame0.menubutton2.m.version
  menu .frame0.menubutton2.m.version
  .frame0.menubutton2.m.version add radiobutton -label "Default" -value { } \
    -variable pgpversion -state active
  .frame0.menubutton2.m.version add radiobutton -label "Packet-Version 2 (2.3a)" -value {+version_byte=2} \
    -variable pgpversion
  .frame0.menubutton2.m.version add radiobutton -label "Packet-Version 3 (2.6)" -value {+version_byte=3} \
    -variable pgpversion

  # build widget .frame1
  frame .frame1

  # build widget .frame2
  frame .frame2

  # build widget .listbox0
  listbox .listbox0 -geometry {20x10} -relief {raised} \
    -yscrollcommand {.scrollbar1 set}
  # bindings
  bind .listbox0 <Double-Button-1> {
    global selected pgpfunc
    set code [catch {set selected [selection get]}]
    if {$code == 0} {
      if [file isdirectory "$selected"] {
        cd $selected
        updatelist
      } elseif {[file isfile "$selected"] && [file readable "$selected"]} {
        if {$pgpfunc == {e}} {
          encrypt
        } elseif {$pgpfunc == {d}} {
          decrypt
        }
      }
    }
  }

  # build widget .scrollbar1
  scrollbar .scrollbar1 -command {.listbox0 yview} -relief {raised}

  # build message .msg0
  message .msg0 -borderwidth 2 -relief raised -width 5c -textvariable nowdir

  # build widget .frame3
  frame .frame3

  # build buttons .frame3.encrypt .frame3.decrypt
  button .frame3.encrypt -text {Encrypt} -command encryptframe
  button .frame3.decrypt -text {Decrypt} -command decryptframe

  # build widgets .frame4 .frame5
  frame .frame4
  frame .frame5

  # build widgets .frame4.check0 .check1 .check2 .label0 .label1 .entry0
  checkbutton .frame4.check0 -text {Sign} -variable sign -offvalue {} -onvalue {s} \
    -anchor w
  bind .frame4.check0 <Button-1> {
    .frame4.check0 toggle
    if {$sign == {}} {
      .frame4.check3 deselect
    }
  }
  checkbutton .frame4.check1 -text {ASCII-Armor} -variable ascii -offvalue {} \
    -onvalue {a} -anchor w
  checkbutton .frame4.check2 -text {Text-File} -variable text -offvalue {} \
    -onvalue {t} -anchor w
  checkbutton .frame4.check3 -text {Sign only} -variable signonly -offvalue {e} \
    -onvalue {} -anchor w
  bind .frame4.check3 <Button-1> {
    .frame4.check3 toggle
    if {$signonly == {}} {
      .frame4.check0 select
    }
  }
  label .frame4.label0 -text { }
  label .frame4.label1 -text {UserID of receiver:} -anchor w
  entry .frame4.entry0 -width 20 -relief sunken -textvariable receiver

  # build widgets .frame5.*
  checkbutton .frame5.check0 -text {View} -variable view -offvalue {} -onvalue {m} \
    -anchor w -width 17
  bind .frame5.check0 <Button-1> {
    .frame5.check0 toggle
    if {$origname == {p}} {
      .frame5.check1 deselect
    }
  }
  checkbutton .frame5.check1 -text {Original filename} -variable origname -offvalue {} \
    -onvalue {p} -anchor w
  bind .frame5.check1 <Button-1> {
    .frame5.check1 toggle
    if {$view == {m}} {
      .frame5.check0 deselect
    }
  }

  # pack master .
  pack configure .frame0 -fill x
  pack configure .frame2 -fill y -fill x -side left
  pack configure .frame1 -in .frame2 -fill y -fill x

  # pack master .frame0
  pack configure .frame0.menubutton1 -fill x -padx 3 -side left

  # pack master .frame1
  pack configure .listbox0 -in .frame1 -fill x -side left
  pack configure .scrollbar1 -in .frame1 -fill x -fill y -side left
  pack configure .frame0.menubutton2 -fill x -padx 3 -side left

  #pack .msg0
  pack configure .msg0 -in .frame2 -fill x -side bottom

  #pack .frame3.??crypt
  pack configure .frame3 -side top -fill x
  pack configure .frame3.encrypt -side left -expand 1 -fill x
  pack configure .frame3.decrypt -side left -expand 1 -fill x

  #pack .frame4 .check0 .check1 .check2 .label0 .label1 .entry0
  pack configure .frame4 -side top -padx 2m -pady 3m
  pack configure .frame4.check0 -expand 1 -fill x
  pack configure .frame4.check3 -expand 1 -fill x
  pack configure .frame4.check1 -expand 1 -fill x
  pack configure .frame4.check2 -expand 1 -fill x
  pack configure .frame4.label0
  pack configure .frame4.label1 -expand 1 -fill x
  pack configure .frame4.entry0 -expand 1 -fill x

  updatelist

  global pgpversion pgpfunc userid sign ascii text signonly receiver view origname
  set pgpversion { }
  set pgpfunc e
#  set userid {}
  set sign {}
  set ascii {}
  set text {}
  set signonly {e}
  set receiver {}
  set view {}
  set origname {}
}

set nowdir [pwd]

startup

