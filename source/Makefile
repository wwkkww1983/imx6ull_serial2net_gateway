VPATH=./src : ./inc : ./cgi

CC=arm-linux-gnueabihf-gcc
CFLAGS=-g -Wall -I./sqlite3_arm/include/ -I./inc -I./src
LDFLAGS=-L ./sqlite3_arm/lib/
SQLFLAGS= -lsqlite3 -ldl

main_src= 	./src
main_tar= 	../bin
cgi_src=	./cgi
cgi_tar=	../cgi-bin

obj=	$(main_src)/tcp.o \
	$(main_src)/udp.o \
	$(main_src)/modbus.o \
	$(main_src)/serial.o \
        $(main_src)/RTU-TCP_func.o 


main_exe=	tcp_pthrgh \
	 	udp_pthrgh \
	 	modbusTCPRTU

cgi_obj=	$(cgi_src)/index1_cgi.o \
		$(cgi_src)/ifconf_db_update_cgi.o \
		$(cgi_src)/olck_db_update_cgi.o \
		$(cgi_src)/serconf_db_update_cgi.o \
		$(cgi_src)/nettransconf_update_cgi.o \
		$(cgi_src)/dtu_cgi.o
		
cgi=	index1.cgi \
	ifconf_db_update.cgi \
	olck_db_update.cgi \
	serconf_db_update.cgi \
	nettransconf_update.cgi \
	dtu.cgi


all:	mkdir $(obj) $(main_exe) $(cgi_obj) $(cgi)

.PHONY : all

mkdir:
	mkdir $(main_tar)
	mkdir $(cgi_tar)

$(obj): %.o: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c $< -o $@ $(SQLFLAGS)

$(cgi_obj): %.o: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c $< -o $@ $(SQLFLAGS)

tcp_pthrgh: serial.o tcp.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(main_tar)/$@ $^ $(SQLFLAGS)

udp_pthrgh: udp.o serial.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(main_tar)/$@ $^ $(SQLFLAGS)

modbusTCPRTU: modbus.o serial.o RTU-TCP_func.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(main_tar)/$@ $^ $(SQLFLAGS)

$(cgi): %.cgi: %_cgi.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(cgi_tar)/$@ $^ $(SQLFLAGS)

.PHONY: clean
clean :
	rm $(main_src)/*.o
	rm $(main_tar)/*
	rm $(cgi_src)/*.o
	rm $(cgi_tar)/*
	rmdir $(main_tar)
	rmdir $(cgi_tar)
