## This is a little ToDo-list manager.

There are three files:
 - main  : the main script
 - nodes : for generall node modifications
 - input : for input handling

All these files are very small and uh *hacky*.

Run main.wsc:
```
wolf-script main.wsc
```

Then the text "Your action please:" will appear. Now you have several options:

 * add
     - Will ask you again for a name and adds input
 * remove
     - Will ask you for a name and removes this node
 * done
     - Will ask you for a name and marks it as done
 * undone
     - Will ask you for a name and marks it as undone
 * quit
     - Exits the programm

Before it asks you for your input, all nodes will be printed.

You can't save your nodes yet, but you could try to implement that yourself! ;p <br>
Fun fact: all .wsc files have together only 102 lines and ~2.222 Bytes!