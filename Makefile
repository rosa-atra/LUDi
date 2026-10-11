source = main/main.c


cc = gcc

cflags = -Wall -Wextra -Werror

rayflags = -lraylib -lm -ldl -lpthread -lGL -lrt -lX11

target: app


app:
	$(cc) $(cflags) $(source) -o app $(rayflags)


run:
	./app

clean:
	rm -f app