.TH DBATCH L "December 4, 1992"
.GN 2
.SH NAME
dbatch \- distributed batch-job system
.SH SYNOPSIS
.B dbatch
[
.B \-jun
]
.br
.B dbatch \-s
.I numHours
.br
.B dbatch
.B \-d
\fS{\fP
.I jobId
\fS}\fP
|
.B ALL
.br
.B dbatch
[
.B \-h 
\fS"\fP\fIhostList\fP\fS"\fR
] [
.I ansProgram
]
\fS"\fP\fIjobDesc\fP\fS"\fP
.br

.br
.I hostList =
\fS{\fP
.I hostName
[ 
.B ..
.I hostName
]
\fS}\fP
.br
.I jobDesc =
.I program
\fS{\fP
.I parameter
\fS} {\fP
.I redirection
\fS}\fP
.br
.I redirection = 
.B <
.I inFile
|
.B >
.I outFile
|
.B 2>
.I errFile
|
.B >&
.I outErrFile
.SH DESCRIPTION
.I Dbatch
is a batch job processing system in a distributed environment. It manages 
a queue of batch jobs, monitors the state of the hosts in the network 
and accounts for the cpu\-time per user. Batch jobs sent to 
.I dbatch
are executed among free hosts. 
Upon termination of a batch job, the user is informed via email or, 
if 
.I ansProgram
is specified, via the execution of this program. The returned 
informations consists of the exit state of the program, the used 
cpu\-time (in seconds), the name of the used host and the executed command.
The option 
.B \-s 
allows the user to interrupt all batch jobs been executed 
on his or her host for a specified time-period.
.SH OPTIONS
.TP
.B -d
Delete jobs from job-queue. Either a list of job\-ID's is given or the 
special word 
.B ALL\c
\&.
In the second case all jobs belonging to a specific user are deleted from 
the job-queue.
.TP
.B -h
Inserts a job in the job-queue and allows execution only on the hosts given 
in the list of host\-names.
.TP
.B \-j
Shows state of job-queue. For each job, its number, its owner, its
state, its host informations and its command are displayed. If a job is in a 
waiting state, the host informations consists of a list of host-names (only 
last letters) on which the command can be executed; otherwise the host 
informations consists of the name and the state of the host on which the 
command is executed and of the command process\-ID on this host.
.TP
.B -n
Shows state of network. The name and the usage of each host in the network 
are displayed. The usage shows either the average percentage of not idle
cpu-time or the number of batch jobs running on that host.
.TP
.B -s
Interrupts execution of batch-jobs on your host for the specified time-period.
.TP
.B -u
Shows state of user-list. For each user, the used cpu\-time (in seconds), 
the number of jobs in the waiting state and the total number of jobs in 
the job\-queue are displayed.
.SH I/O REDIRECTION
.TP
.B <
The standard input is redirected to the given file. Default is /dev/null.
.TP
.B >
The standard output is redirected to the given file. Default is /dev/null.
.TP
.B 2>
The standard error is redirected to the given file. Default is /dev/null.
.TP
.B >&
The standard output and standard error are redirected to the given file.
.SH ANSWER PROGRAM
.TP
.I ansProgram
Upon termination of the batch job this program is executed. If for example 
the program 
.B 'simulation 5'
terminates with an exit state of 
.B 1
and a usage of 
.B 28
seconds on the host 
.B beaune,
the response would be the same as if you had typed:
.br
.I ansProgram
.B 1 28 beaune simulation 5 \fS"<"\fP 
.B /dev/null \fS">"\fP 
.B /dev/null \fS"\fB2\fP>"\fP 
.B /dev/null
.SH FILE NAME COMPLETION
.TP
.I ansProgram, program:
If not a full path\-name is specified, 
.I dbatch 
looks through all directories given by the environment variable 
.B PATH
to find the corresponding executable files.
.TP
.I inFile, outFile, errFile, outErrFile:
If not a full path\-name is specified, 
.I dbatch 
completes these file-names 
with the name of the current working directory.
.SH EXAMPLES
To show the content of the batch\-job-queue:
.LP
.RS
.nf
.ft B
dbatch
.ft R
.fi
.RE
.br
.ne 10
.LP
To show job, user and network informations:
.LP
.RS
.nf
.ft B
dbatch -jun
.ft R
.fi
.RE
.br
.ne 10
.LP
To delete the batch\-jobs with job\-ID's 37 and 38:
.LP
.RS
.nf
.ft B
dbatch -d 37 38
.ft R
.fi
.RE
.br
.ne 10
.LP
To delete all jobs from the job\-queue belonging to a specific user:
.LP
.RS
.nf
.ft B
dbatch -d ALL
.ft R
.fi
.RE
.br
.ne 10
.LP
To insert the command 'simulation 6 >& out6' in the job\-queue:
.LP
.RS
.nf
.ft B
dbatch "simulation 6 >& out6"
.ft R
.fi
.RE
.br
.ne 10
.LP
To insert a job in the job\-queue which can be executed only on the hosts
beaune and chablis:
.LP
.RS
.nf
.ft B
dbatch -h "beaune chablis" "simulation 7 < in7 > out7 2> err7"
.ft R
.fi
.RE
.br
.ne 10
.LP
To insert a job in job-queue for which, upon termination, the program 'result' is executed:
.LP
.RS
.nf
.ft B
dbatch result "simulation 8"
.ft R
.fi
.RE
.br
.ne 10
.LP
To interrupt execution of batch-jobs on your host for the next 4 hours:
.LP
.RS
.nf
.ft B
dbatch -s 4
.ft R
.fi
.RE
.br
.ne 10
.LP
To reallow execution of batch-jobs on your host:
.LP
.RS
.nf
.ft B
dbatch -s 0
.ft R
.fi
.RE
.SH FILES
.PD 0
.TP 20
.B dbatch
user command
.TP
.B dbatchd
daemon managing all informations
.TP
.B dbatchd.adr
address for inter\-process communication
.TP
.B dbatchd.sav
file with all state informations
.PD
.SH AUTHOR
Richard De Moliner (demoliner@isi.ee.ethz.ch)
.br
Signal and Information Processing Laboratory
.br
Swiss Federal Institute of Technology
.br
CH-8092 Zurich, Switzerland
