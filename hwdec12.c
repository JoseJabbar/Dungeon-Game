#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <strings.h> // For strcasecmp()
#include "hwdec12.h"


#define MAX_ROOMS 5
#define MAX_ITEMS 5
#define INVENTORY_CAPACITY 5
#define MAX_INPUT_SIZE 100

// Structures
typedef struct
{
    char *description;
    int up, down, left, right; // Room connections
    char **items;              // Items in the room
    int itemCount;
    char *creature;     // Creature name
    int creatureHealth; // Creature health
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
    int hasArmor;
    int hasKilledWitch;
    int hasKilledFinalBoss;
} Player;

// Global Variables
Room rooms[MAX_ROOMS];
Player player;

// Function Prototypes
void initializeGame();
void freeResources();
void loadRooms();
void save(const char *filepath);
void move(Player *player, Room rooms[], const char *direction);
void look(const Player *player, const Room rooms[]);
void inventory(const Player *player);
void pickup(Player *player, Room *room, const char *itemName);
void attack(Player *player, Room *room);
void handleCommand(Player *player, Room rooms[], char *command);
void toLowerCase(char *str);
// Initialize Game Data
void initializeGame()
{
    printf("Welcome to the Text Adventure Game!\n");
    printf("Type 'help' for a list of commands.\n");
    player.health = 100;
    player.strength = 15;
    player.inventoryCapacity = INVENTORY_CAPACITY;
    player.inventoryCount = 0;
    player.inventory = malloc(sizeof(char *) * INVENTORY_CAPACITY);
    player.currentRoom = 0;
    player.hasVisitedTreasureRoom = 0; // Initially, the player hasn't visited the Treasure Room.
    player.hasKilledGoblin = 0;        // Initially, the player hasn't killed the goblin.
    player.hasArmor = 0;               // Initially the player hsn't have armor
    player.hasKilledWitch = 0;
    player.hasKilledFinalBoss = 0;

    loadRooms();
}

void loadRooms()
{
    for (int i = 0; i < MAX_ROOMS; i++)
    {
        rooms[i].description = malloc(100);
        sprintf(rooms[i].description, "Room %d description.", i);
        rooms[i].up = rooms[i].down = rooms[i].left = rooms[i].right = -1;
        rooms[i].items = malloc(sizeof(char *) * MAX_ITEMS);
        rooms[i].itemCount = 0;
        rooms[i].creature = NULL;
        rooms[i].creatureHealth = 0;
    }

    // Room 0: Dungeon Entrance
    strcpy(rooms[0].description, "You are in the Dungeon Entrance. A sword lies on the ground.");
    rooms[0].right = 1; // Can go to Room 1
    rooms[0].items[rooms[0].itemCount++] = strdup("Sword");

    // Room 1: Goblins' Hell
    strcpy(rooms[1].description, "You are in Goblins' Hell. A goblin is here, ready to attack!");
    rooms[1].left = 0;  // Can go back to Room 0 (left)
    rooms[1].down = 3;  // Can go down to Room 3 (Treasure Room)
    rooms[1].up = 2;    // Can go up to Room 2 (Witch's Holley)
    rooms[1].right = 4; // Can go right to Room 4 (Final Boss)
    rooms[1].creature = strdup("Goblin");
    rooms[1].creatureHealth = 60;

    // Room 2: Witch's Holley
    strcpy(rooms[2].description, "You are in Witch's Holley. A scary witch looms over you, cackling!");
    rooms[2].down = 1;   // You can go down to Room 1 (Goblins' Hell)
    rooms[2].up = -1;    // No room above
    rooms[2].left = -1;  // No room to the left
    rooms[2].right = -1; // No room to the right
    rooms[2].creature = strdup("Witch");
    rooms[2].items[rooms[2].itemCount++] = strdup("Key");
    rooms[2].creatureHealth = 100;

    // Room 3: Treasure Room
    strcpy(rooms[3].description, "You are in the Treasure Room. Glittering treasures are everywhere!");
    rooms[3].up = 1;     // You can go up to Room 1 (Goblins' Hell)
    rooms[3].down = -1;  // No room below
    rooms[3].left = -1;  // No room to the left
    rooms[3].right = -1; // No room to the right
    rooms[3].items[rooms[3].itemCount++] = strdup("Armor");

    // Room 4: Final Boss
    strcpy(rooms[4].description, "You are in the Final Boss room. The final boss awaits!");
    rooms[4].up = -1;    // No room above
    rooms[4].down = -1;  // No room below
    rooms[4].left = 1;   // Can go left to Room 1 (Goblins' Hell) to escape
    rooms[4].right = -1; // No room to the right
    rooms[4].creature = strdup("Final Boss");
    rooms[4].creatureHealth = 300;
}

// Free allocated resources
void freeResources()
{
    for (int i = 0; i < MAX_ROOMS; i++)
    {
        free(rooms[i].description);
        for (int j = 0; j < rooms[i].itemCount; j++)
            free(rooms[i].items[j]);
        free(rooms[i].items);
        if (rooms[i].creature)
            free(rooms[i].creature);
    }
    for (int i = 0; i < player.inventoryCount; i++)
        free(player.inventory[i]);
    free(player.inventory);
}

// Save game state to a file
void save(const char *filepath)
{
    FILE *file = fopen(filepath, "w");
    if (file == NULL)
    {
        printf("Error opening file for saving.\n");
        return;
    }

    // Save player data
    fprintf(file, "Player Health: %d\n", player.health);
    fprintf(file, "Player Strength: %d\n", player.strength);
    fprintf(file, "Inventory Capacity: %d\n", player.inventoryCapacity);
    fprintf(file, "Inventory Count: %d\n", player.inventoryCount);
    fprintf(file, "Current Room: %d\n", player.currentRoom);
    fprintf(file, "Has Visited Treasure Room: %d\n", player.hasVisitedTreasureRoom);
    fprintf(file, "Has Killed Goblin: %d\n", player.hasKilledGoblin);
    fprintf(file, "Has Killed Witch: %d\n", player.hasKilledWitch);
    fprintf(file, "Has Killed Final Boss: %d\n", player.hasKilledFinalBoss);
    

    // Save inventory items
    fprintf(file, "Inventory:\n");
    for (int i = 0; i < player.inventoryCount; i++)
    {
        fprintf(file, "  Item %d: %s\n", i + 1, player.inventory[i]);
    }

    // Save room data
    for (int i = 0; i < MAX_ROOMS; i++)
    {
        fprintf(file, "Room %d:\n", i);
        fprintf(file, "  Description: %s\n", rooms[i].description);
        fprintf(file, "  Creature: %s (Health: %d)\n", rooms[i].creature ? rooms[i].creature : "None", rooms[i].creatureHealth);
        fprintf(file, "  Item Count: %d\n", rooms[i].itemCount);

        // Save items in the room
        fprintf(file, "  Items:\n");
        for (int j = 0; j < rooms[i].itemCount; j++)
        {
            fprintf(file, "    %s\n", rooms[i].items[j]);
        }

        // Save room connections
        fprintf(file, "  Connections: Up: %d, Down: %d, Left: %d, Right: %d\n",
                rooms[i].up, rooms[i].down, rooms[i].left, rooms[i].right);
    }

    fclose(file);
}
void load(const char *filepath)
{
    FILE *file = fopen(filepath, "r");
    if (file == NULL)
    {
        printf("Error opening file for loading.\n");
        return;
    }

    char line[MAX_INPUT_SIZE];
    while (fgets(line, sizeof(line), file))
    {
        // Load player data
        if (strncmp(line, "Player Health:", 14) == 0)
            sscanf(line, "Player Health: %d", &player.health);
        else if (strncmp(line, "Player Strength:", 16) == 0)
            sscanf(line, "Player Strength: %d", &player.strength);
        else if (strncmp(line, "Inventory Capacity:", 19) == 0)
            sscanf(line, "Inventory Capacity: %d", &player.inventoryCapacity);
        else if (strncmp(line, "Inventory Count:", 16) == 0)
            sscanf(line, "Inventory Count: %d", &player.inventoryCount);
        else if (strncmp(line, "Current Room:", 13) == 0)
            sscanf(line, "Current Room: %d", &player.currentRoom);
        else if (strncmp(line, "Has Visited Treasure Room:", 26) == 0)
            sscanf(line, "Has Visited Treasure Room: %d", &player.hasVisitedTreasureRoom);
        else if (strncmp(line, "Has Killed Goblin:", 18) == 0)
            sscanf(line, "Has Killed Goblin: %d", &player.hasKilledGoblin);
        else if (strncmp(line, "Inventory:", 10) == 0)
        {
            // Load inventory items
            while (fgets(line, sizeof(line), file) && line[0] != '\n')
            {
                char *item = malloc(MAX_INPUT_SIZE);
                sscanf(line, "  Item %*d: %s", item);
                player.inventory[player.inventoryCount++] = item;
            }
        }
        else if (strncmp(line, "Room", 4) == 0)
        {
            int roomIndex;
            sscanf(line, "Room %d:", &roomIndex);
            fgets(line, sizeof(line), file); // Skip description line
            fgets(line, sizeof(line), file); // Skip creature line
            sscanf(line, "  Creature: %*s (Health: %d)", &rooms[roomIndex].creatureHealth);
            fgets(line, sizeof(line), file); // Skip item count line
            while (fgets(line, sizeof(line), file) && line[0] != '\n')
            {
                char *item = malloc(MAX_INPUT_SIZE);
                sscanf(line, "    Item: %s", item);
                rooms[roomIndex].items[rooms[roomIndex].itemCount++] = item;
            }
        }
    }

    fclose(file);
}
// Check if the player has a specific item in their inventory
// Helper function to check if an item is in the player's inventory
int hasItemInInventory(const char *item)
{
    for (int i = 0; i < player.inventoryCount; i++)
    {
        if (strcasecmp(player.inventory[i], item) == 0)
        {
            return 1; // Item found in inventory
        }
    }
    return 0; // Item not found
}

// Function to handle movement
void move(Player *player, Room rooms[], const char *direction)
{
    // Get the current room
    Room *currentRoom = &rooms[player->currentRoom];
    int nextRoom = -1;

    // Check the direction and determine the next room
    if (strcasecmp(direction, "right") == 0)
    {
        nextRoom = currentRoom->right;
    }
    else if (strcasecmp(direction, "left") == 0)
    {
        nextRoom = currentRoom->left;
    }
    else if (strcasecmp(direction, "up") == 0)
    {
        nextRoom = currentRoom->up;
    }
    else if (strcasecmp(direction, "down") == 0)
    {
        nextRoom = currentRoom->down;
    }

    // Check if the direction is valid
    if (nextRoom == -1)
    {
        printf("You can't go that way.\n");
        return;
    }

    // Check conditions for moving to Room 1 (Goblin's Hell) from Room 0
    if (player->currentRoom == 0 && nextRoom == 1)
    {
        if (!hasItemInInventory("Sword"))
        {
            printf("You cannot move to Goblin's Hell without a sword.\n");
            return;
        }
        else
        {
            player->strength += 10;
        }
    }

    // Check conditions for moving to Room 3 (Treasure Room) from Room 1
    if (player->currentRoom == 1 && nextRoom == 3)
    {
        if (!hasItemInInventory("Key"))
        {
            printf("You cannot move to the Treasure Room without the key.\n");
            return;
        }
    }

    // Check conditions for moving to Room 2 (Witch's Alley) from Room 1
    if (player->currentRoom == 1 && nextRoom == 2)
    {
        if (!player->hasKilledGoblin)
        {
            printf("You cannot move to Witch's Alley until you kill the goblin!\n");
            return;
        }
    }

    if (player->currentRoom == 1 && nextRoom == 4)
    {
        if (!hasItemInInventory("Armor"))
        {
            printf("You cannot move to the Final Boss room without equipping the armor!\n");
            return;
        }
    }

    // Move the player to the next room
    player->currentRoom = nextRoom;
    printf("You move to Room %d.\n", player->currentRoom);
}

// Look command: display room description and items
// Look command: display room description and itemsmove
void look(const Player *player, const Room rooms[])
{
    Room current = rooms[player->currentRoom];

    // Update Goblin's message after death
    if (player->hasKilledGoblin && player->currentRoom == 1 && current.creature == NULL)
    {
        current.description = "You are in Goblins' Hell. The goblin's dead body lies on the ground.";
    }
    else if (!player->hasKilledWitch && player->currentRoom == 2 && current.creature == NULL)
    {
        current.description = "You are in Witch's Halley. Witch's ashes flying in the room.";
    }
    else if (!player->hasKilledFinalBoss && player->currentRoom == 4 && current.creature == NULL)
    {
        current.description = "You are in Final Boss's room. His dead body lies on the ground. Who is BOSS now?!";
    }

    printf("%s\n", current.description);
    if (current.itemCount > 0)
    {
        printf("Items: ");
        for (int i = 0; i < current.itemCount; i++)
        {
            printf("%s ", current.items[i]);
        }
        printf("\n");
    }
    if (current.creature)
    {
        printf("Creature: %s (Health: %d)\n", current.creature, current.creatureHealth);
    }
}

void map()
{
  
    printf("           -----------------\n");
    printf("          | Witchs' Halley  |\n");
    printf("           ------- && ------\n");
    printf("          |                 |\n");
    printf("----------     Goblins'     |-------------- \n");
    printf(" Entrance &      Hell        &  FINAL BOSS  |\n");
    printf("----------                  |--------------\n");
    printf("          |                 |\n");
    printf("           ------ && -------\n");
    printf("          |       ???       |\n");
    printf("           -----------------\n");
  
}

// Inventory command: display items in the player's inventory
void inventory(const Player *player)
{
    printf("Inventory:\n");
    for (int i = 0; i < player->inventoryCount; i++)
    {
        printf("- %s\n", player->inventory[i]);
    }
}

// Pickup command: allow the player to pick up items

void pickup(Player *player, Room *room, const char *itemName)
{
    for (int i = 0; i < room->itemCount; i++)
    {
        if (strcasecmp(room->items[i], itemName) == 0)
        {
            if (player->inventoryCount < player->inventoryCapacity)
            {
                player->inventory[player->inventoryCount++] = strdup(room->items[i]);
                printf("You picked up %s.\n", room->items[i]);

                // Update Room 0 description after the item is picked up
                if (player->currentRoom == 0)
                {
                    strcpy(room->description, "You are in the Dungeon Entrance. Let's go!"); // Updated pickup function with Room 0 description change
                }

                free(room->items[i]);
                room->items[i] = room->items[room->itemCount - 1];
                room->itemCount--;
                return;
            }
            else
            {
                printf("You can't carry more items!\n");
                return;
            }
        }
    }
    printf("Item not found.\n");
}


// Attack command: simulate combat
void attack(Player *player, Room *room)
{
    if (!room->creature || room->creatureHealth <= 0)
    {
        printf("No creature to attack!\n");
        return;
    }

    // Battle logic
    while (player->health > 0 && room->creatureHealth > 0)
    {
        room->creatureHealth -= player->strength;
        printf("You hit the %s. Its health is now %d.\n", room->creature, room->creatureHealth);

        if (room->creatureHealth > 0)
        {
            player->health -= 5;
            printf("The %s hits you. Your health is now %d.\n", room->creature, player->health);
        }
    }

    if (player->health <= 0)
    {
        printf("You died. Game over!\n");
        exit(0);
    }
    else
    {
        printf("You defeated the %s!\n", room->creature);
        free(room->creature);
        room->creature = NULL;

        

        // Mark goblin as killed
        player->hasKilledGoblin = 1;
    }
}

// Function to convert string to lowercase
void toLowerCase(char *str)
{
    for (int i = 0; str[i]; i++)
    {
        str[i] = tolower((unsigned char)str[i]);
    }
}

// Handle commands from the player
void handleCommand(Player *player, Room rooms[], char *command)
{
    if (strncmp(command, "save ", 5) == 0)
    {
        // Save game
        char *filepath = command + 5;
        save(filepath);
    }
    else if (strncmp(command, "load ", 5) == 0)
    {
        // Load game
        char *filepath = command + 5;
        load(filepath);  // Load from the specified file
    }
    else if (strcasecmp(command, "quit") == 0)
    {
        printf("Thank you for playing. Goodbye!\n");
        exit(0);
    }
    else if (strcasecmp(command, "help") == 0)
    {
        printf("Available commands:\n");
        printf("  move <direction>  - Move in a direction (up, down, left, right).\n");
        printf("  look              - Look around the room.\n");
        printf("  map               - Shows map\n");
        printf("  inventory         - View your inventory.\n");
        printf("  pickup <item>    - Pick up an item in the room.\n");
        printf("  attack            - Attack a creature in the room.\n");
        printf("  save <filepath>   - Save the game state to a file.\n");
        printf("  load <filepath>   - Load the game state from a file.\n");
        printf("  quit              - Quit the game.\n");
    }
    else if (strncmp(command, "move ", 5) == 0)
    {
        move(player, rooms, command + 5);
    }
    else if (strcasecmp(command, "look") == 0)
    {
        look(player, rooms);
    }
    else if (strcasecmp(command, "inventory") == 0)
    {
        inventory(player);
    }
    else if (strncmp(command, "pickup ", 7) == 0)
    {
        pickup(player, &rooms[player->currentRoom], command + 7);
    }
    else if (strcasecmp(command, "attack") == 0)
    {
        attack(player, &rooms[player->currentRoom]);
    }else if (strcasecmp(command, "map") == 0)
    {
        map();
    }
    else
    {
        printf("Unknown command. Type 'help' for a list of commands.\n");
    }
}


int main()
{
    char command[MAX_INPUT_SIZE];

    // Initialize game
    initializeGame();
    

    while (1)
    {
        printf("\n> ");
        if (fgets(command, MAX_INPUT_SIZE, stdin) != NULL)
        {
            // Remove trailing newline character
            command[strcspn(command, "\n")] = '\0';

            // Convert input to lowercase for consistency
            toLowerCase(command);

            handleCommand(&player, rooms, command);
        }
    }

    // Free allocated resources
    freeResources();

    return 0;
}