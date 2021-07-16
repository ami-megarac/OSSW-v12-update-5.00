AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/common -I$(SPXINC)/video_vnc
vncserver=vncserver
vncserver_LDADD=-L../libvncserver/ -lvncserver -lm
vncserver_LDADD+=-L${SPXLIB}/video_vnc -lvideo_vnc

vncserver_SOURCES= vncserver.c hid.c
noinst_HEADERS= vncserver.h hid.h 

noinst_PROGRAMS=$(vncserver)
