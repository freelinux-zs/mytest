#
# Makefile for the input core drivers.
#

# Each configuration option enables a list of files.
obj-$(CONFIG_BITD_RTP_TS)  += birdrtp_ts.o
obj-$(CONFIG_BITD_RTP_TS) += libbirdrtp.o 
$(obj)/libbirdrtp.o: $(srctree)/$(obj)/libbirdrtp
	cp $(srctree)/$(obj)/libbirdrtp $(obj)/libbirdrtp.o




# end
