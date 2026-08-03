kimamp: kimamp.c
	gcc -Wall -Wextra -O3 -o kimamp kimamp.c -lSDL2 -lSDL2_mixer -lzip
	strip kimamp

clean:
	rm -f kimamp
