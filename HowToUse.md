## Server ##

The server is the computer which will receive keypresses from clients. To run the server, simply run:
```
netkeys --client
```

The un-intuitive use of "client" here will be removed in a future release.

## Client ##

The client is the computer which sends keypresses to the server. To run the client, use the following command:
```
netkeys --ip <ip-of-server> <keys to send, separated by spaces>
```

## Example ##

This example will send the A, B, and C keys to the computer with IP address "10.0.0.1".

On the system to receive the keys, the following command is run:
```
netkeys --client
```

It's now listening for keys, so let's run the client and send some keys:
```
netkeys --ip 10.0.0.1 A B C
```

Now any press of the A, B, or C keys will be sent to the server computer, as if they had been typed on the server's keyboard.

## Multiple Clients ##

It is possible to run one server and have several different computers sending keys to that server. However, it is not yet possible to have one keyboard to many computers. This is something that is definitely planned for the future.