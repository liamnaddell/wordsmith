# Wordsmith: Kernel edition!

This is a kernel module which attempts to manage the hardware resource of "generated words". 
In short, it's a driver that maintains a list of generated words, and allows for a user to read, write, modify, etc those words.

## Assignment 1

Infrastructure. The goal here is to create an in-memory database of words. Then, every n seconds, generate a word, and print it to the debug log. Also, store these words in the RBtree.

Goal: Use a delayed workqueue to set up a task which runs every 5 seconds or so.

## Assignment 2

Validating words.

Save for later: 
kthread.h
