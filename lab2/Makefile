lab1: bot server

bot: bot.o
	 cc -o bot bot.o
	 
bot.o: bot.c 
	   cc -c bot.c

server: server.o
			cc -o server server.o

server.o: server.c
			   cc -c server.c

clean:
		-rm -f bot.o bot server.o server *.core
