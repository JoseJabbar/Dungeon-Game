#ifndef GAME_H
#define GAME_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_ROOMS 5
#define MAX_ITEMS 5
#define INVENTORY_CAPACITY 5
#define MAX_INPUT_SIZE 100

// Structures
typedef struct
{
    char *description;
    int up, down, left, right;  // Room connections
    char **items;               // Items in the room
    int itemCount;              // Number of items in the room
    char *creature;             // Creature name
    int creatureHealth;         // Creature health
} Room;

typedef struct
{
    int health;
    int strength;
    int inventoryCapacity;
    char **inventory;
    int inventoryCount;
    int currentRoom;
    int hasVisitedTreasureRoom; // Flag to track if the player has visited the Treasure Room
    int hasKilledGoblin;        // Flag to track if the player has killed the goblin
    int hasArmor;               // Flag to track if the player has armor
    int hasKilledWitch;         // Flag to track if the player has killed the witch
    int hasKilledFinalBoss;     // Flag to track if the player has killed the final boss
} Player;

// Function Prototypes

void initializeGame();
void freeResources();
void loadRooms();
void save(const char *filepath);
void load(const char *filepath);
void move(Player *player, Room rooms[], const char *direction);
void look(const Player *player, const Room rooms[]);
void inventory(const Player *player);
void pickup(Player *player, Room *room, const char *itemName);
void attack(Player *player, Room *room);
void handleCommand(Player *player, Room rooms[], char *command);
void toLowerCase(char *str);
void map();
int hasItemInInventory(const char *item);

// Initialize Game Data
void initializeGame();

// Load Rooms Data
void loadRooms();

// Free allocated resources
void freeResources();

// Save game state to a file
void save(const char *filepath);

// Move the player in a specified direction
void move(Player *player, Room rooms[], const char *direction);

// Look around in the current room
void look(const Player *player, const Room rooms[]);

// Show the player's inventory
void inventory(const Player *player);

// Pick up an item in the current room
void pickup(Player *player, Room *room, const char *itemName);

// Attack a creature in the room
void attack(Player *player, Room *room);

// Handle commands entered by the player
void handleCommand(Player *player, Room rooms[], char *command);

// Convert a string to lowercase
void toLowerCase(char *str);

// Display a simple map of the game
void map();

// Check if the player has a specific item in their inventory
int hasItemInInventory(const char *item);

#endif // GAME_H
