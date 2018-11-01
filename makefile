CC	= gcc

space_invaders: army.o bunker.o buttons.o game_over.o gpio.o intc.o player.o red_alien.o scoreboard.o space_invaders.o text.o utils.o video.o
video: utils.o video.o
clock: buttons.o clock.o gpio.o intc.o switches.o utils.o

army.o: army.c army.h player.h scoreboard.h video.h
bunker.o: bunker.c bunker.h
buttons.o: buttons.c buttons.h gpio.h utils.h
clock.o: clock.c buttons.h fit.h intc.h switches.h utils.h
game_over.o: game_over.c game_over.h text.h video.h
gpio.o: gpio.c gpio.h uio.h utils.h
intc.o: intc.c intc.h uio.h utils.h
player.o: player.c army.h player.h red_alien.h video.h
red_alien.o: red_alien.c icons.h red_alien.h video.h
scoreboard.o: scoreboard.c text.h scoreboard.h video.h
space_invaders.o: space_invaders.c army.h bunker.h buttons.h game_over.h intc.h player.h red_alien.h scoreboard.h utils.h video.h
switches.o: switches.c gpio.h switches.h utils.h
text.o: text.c text.h video.h
utils.o: utils.c utils.h
video.o: video.c utils.h video.h

clean:
	rm *.o
