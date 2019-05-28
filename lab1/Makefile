lab1: bot UDP_server

bot: bot.o
	 cc -o bot bot.o
	 
bot.o: bot.c 
	   cc -c bot.c

UDP_server: UDP_server.o
			cc -o UDP_server UDP_server.o

UDP_server.o: UDP_server.c
			   cc -c UDP_server.c

clean:
		-rm -f bot.o bot UDP_server.o UDP_server *.core
