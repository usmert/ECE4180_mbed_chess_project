# mbed Chess Game
An embedded chess game device with 1-player and 2-player functionality.
Team Members: Logan Starr, Mert Us, and Matthew Hannay

## Overview 
In this project we built a chess game using an mbed LPC1768 Microcontroller. The game includes 2-player mode, as well as a 1-player mode in which the AI opponent is implemented using the Minimax Alogorithm. Players have the ability to control the game using a joystick and a mobile phone app connected via bluetooth. The device is beginner-friendly, with the possible moves highlighted on the screen when a piece is selected, and a red LED to indicate if a user tries an illegal move. The game can be reset using a button or the app, and the mode can be changed using an on-board switch. The game mode change takes effect after the game is reset. An LCD Screen is used to display the board and a speaker plays tunes when the game ends.

## Components List
* LPC1768 Mbed With Mbed OS 2
* uLCD-144-G2
* Adafruit Bluefruit BLE UART
* SparkFun Mono Audio Amp Breakout
* PCB Mount Speaker
* 5-Way Tactile Switch Breakout
* 8 Position DIP Switch
* Push Button
* Red LED
* 10k Ohm Resistor
* 5V Breadboard Power Supply

## Project Demo
[<img src="https://github.com/usmert/ECE4180_mbed_chess_project/blob/main/images/chess_proj_thumbnail.png" width="600">](https://www.youtube.com/watch?v=JMscDs1BIZE&t=2s)

## Schematic
<img src="https://github.com/usmert/ECE4180_mbed_chess_project/blob/main/images/chess_proj_schematic.png" width="600">

# Instructions

There are two options for play. The first option is two player. The second option is player vs AI in order to use this option you have to flip the switch to indicate you want to play against AI. 
The player has two option for controlling the pieces. This first option is too use the joystick. The second option is to use your phone connected through ADA fruit Bluetooth app.  These both allow you to move around a colored square which will go over each piece and designate the moves that are possible for each by highlighting the squares the pieces can move too.
All other rules are the same as a normal game of chess.
