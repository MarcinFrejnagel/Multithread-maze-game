# Maze game for two players using TCP/IP communication

In this game you move with arrows, collect coins and avoid beast. You can also play with second player using two Linux terminals. Collisions between players same as between player and beast causes the respawn of player and leaving possible to collect 'D' in the place of his death in case the player was carrying any coins. When entering a bush '#' player has slowdown that makes the player move two times slower.

## Important library
- ncurses - used to get input from keyboard, display and format everything in terminal
- sockets - enable communication and continuous data transfer between server and client
- threads - simultaneous gameplay for two players, beast movement 

## Keybinds
- Arrows - move player
- c - spaw coin worth 1
- C - spaw coin worth 10
- T - spaw coin worth 50
- q/Q - finish game

## Compile and run the program
 
Enter command to compile all files:
```c
make
```
 
Nextly write the following command to start the server:
```c
./server
```

Furthermore you can start the client in second Linux terminal using:
```c
./client
```

## Screenshot of game

![image](https://user-images.githubusercontent.com/97180223/223841954-d52f145a-4348-42fb-8009-73e0639e26b6.png)

