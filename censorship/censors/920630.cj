edited by DJB, 930722, for the public record
note that the three pages of attached Snuffle information are omitted
for obvious reasons

					[address]
					30 June 1992

PM/DTC
Room 228, SA-6
Department of State
Washington, DC 20522-0602

Subject: Commodity Jurisdiction Request for snuffle 5.0 software

Dear Sirs:

     My name is Daniel J. Bernstein. I would like to export the software
known as ``snuffle 5.0'', described below, together with the
documentation of my ``Snuffle'' encryption system, also described below.
It is my understanding that the Department of State can decide to
transfer jurisdiction of this software to the Commerce Department
through a Commodity Jurisdiction Procedure. I'd like that to happen. I
apologize in advance for any errors I may have made in this procedure;
if necessary you can leave a message for me at [phone], but I may
not receive the message for some time. I can be reached on the Internet
computer network as [email].

     The Snuffle encryption system is designed to convert any one-way
hash function into a zero-delay private-key encryption system. This
means that it can be used, for instance, to exchange text between two
people who have previously exchanged keys, provided that it is combined
with a one-way hash function. ``Zero-delay'' means that Snuffle can be
used for interactive conversations: each character typed by one person
can be encrypted, sent to the other person, and decrypted by the other
person immediately. A precise definition of Snuffle appears on the
attached sheet ``The Snuffle Encryption System.''

     snuffle 5.0 is my implementation of Snuffle. It consists of the two
C language source files snuffle.c and unsnuffle.c, for encryption and
decryption respectively. When combined with an implementation of a
one-way hash function, snuffle.c and unsnuffle.c can be used for various
applications requiring private-key cryptography, including the example
above of interactive text exchange. The files are designed to interface
smoothly, with minimal effort upon the user's part, with the Xerox
Secure Hash Function, known as Snefru 2.0, which has been distributed
for free by Xerox. Complete listings of snuffle.c and unsnuffle.c are
attached.

     The three possibly controlled items which I would like to export
are (1) the document ``The Snuffle Encryption System,'' attached; (2)
the snuffle.c source file, attached; (3) the unsnuffle.c source file,
attached. Snuffle and snuffle 5.0 result from fundamental research and,
since last year, have been available for free to U.S. citizens within
the U.S. I would like to publish these items in a widely read
international electronic conference known as ``sci.crypt'' for
discussion by the worldwide academic community.

								page 2

     The portions of snuffle.c and unsnuffle.c which actually perform
encryption and decryption contain just 15 lines each of C code with no
cryptographic technology per se. All the work is done by the one-way
hash function code. Similarly, the description of Snuffle is short; the
system itself does not contain appreciable complexity. In effect what I
want to export is a description of a way to use existing technology in a
more effective manner. I do not foresee military or commercial use of
Snuffle by anyone who does not already have access to the cryptographic
technology contained in, e.g., the Xerox Secure Hash Function. I do
foresee practical use of Snuffle by those who do have such access, in
particular for the purpose of interactively exchanging encrypted text.

     Thank you for your kind attention.

					Sincerely,

                                        [signature]


					Daniel J. Bernstein
