# libpurple\_autoresponse
------

This is a straightforward plugin for Pidgin (tested in libpurple 2.10.6) that listens to each message recieved by every enabled account.  Once it recieves a message, it passes its metadata to a function in a lua script for processing.  Metadata passed in includes:

  - The text of the message
  - The protocol it was sent on
  - The group the sender is a part of
  - The username, status, and alias of the sender
  - The username, status, and alias of the reciever

Once this data is recieved by the script, it can be easily extended to process the data any anyway it Lua can.  The function must then return a single string that will be sent back to the original sender.  If no data or nil is returned by the function, no response is sent back to the sender.

An obvious use case for this plugin is a chat bot that can respond on any protocol Pidgin supports.

Tech
-----------

Various external libraries are used in order for this plugin to build:

  - libpurple
  - lua-5 (Tested with 5.2)
  - gtk2

Installation
--------------

1. ```bash
$ make && make install
```
2. Create a lua script at the install directory called 'autorespond.lua'

License
-

MIT
  
    

