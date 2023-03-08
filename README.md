# Maze game for two players using TCP/IP communication

In this game you move with arrows, collect coins and avoid beast. You can also play with second player using two Linux terminals.

## Important library
- ncurses - used to get input from keyboard, display and format everything in terminal
- sockets - enable communication and continuous data transfer between server and client
- threads - simultaneous gameplay for two players, beast movement 
 
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



![image](https://user-images.githubusercontent.com/97180223/223841954-d52f145a-4348-42fb-8009-73e0639e26b6.png)

