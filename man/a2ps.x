[NAME]
a2ps \- format files for printing on a PostScript printer

[DESCRIPTION]
.\" Add any additional description here

[FILES]
a2ps reads several files before the command line options.  In order,
they are:

.IP 1.
The system configuration file (usually \fI/usr/local/etc/a2ps.cfg\fR)
unless you have defined the environment variable \fIA2PS_CONFIG\fR, in
which case a2ps reads the file it points to;

.IP 2.
the user's home configuration file (\fI$HOME/.a2ps/a2psrc\fR)

.IP 3.
the file \fI.a2psrc\fR in the current directory.

See the info manual for more information, including a description of the
configuration file format.
