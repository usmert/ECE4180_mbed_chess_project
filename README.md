# chess_proj
An embedded chess game device with 1-player and 2-player functionality.
# Overview 
The goal of our project was to make a completely function chess board on the mbed. Our chess has the ability to play against an ai which can see three moves into the feature. The chess game also has the a two player function. The chess game is controlled either Bluetooth or a joystick. The chess game is displayed on a uLCD.
Team Members: Logan Starr, Mert Us, and Matthew Hannay
# Components List 

LPC1768 Mbed With Mbed OS 2
*Adafruit Bluefruit LE UART Module to control the chess board using Bluetooth
*Sparkfun PCB or breadboard mount Speaker 8ohm .1W for game winning sounds
*5v votlage cord
*uLCD-144-G2 128 to display the chess game
# Video Demonstration

# Schematic
![image](https://github.gatech.edu/storage/user/65657/files/9f0fb8db-7ca2-422b-abbc-5c56ffe5838a)

# Instructions

There are two options for play. The first option is two player. The second option is player vs AI in order to use this option you have to flip the switch to indicate you want to play against AI. 
The player has two option for controlling the pieces. This first option is too use the joystick. The second option is to use your phone connected through ADA fruit Bluetooth app.  These both allow you to move around a colored square which will go over each piece and designate the moves that are possible for each by highlighting the squares the pieces can move too.
All other rules are the same as a normal game of chess.
