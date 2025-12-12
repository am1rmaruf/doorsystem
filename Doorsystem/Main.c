#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>
#define SLEEP_MS(ms) usleep((ms) * 1000)
#endif

#include "safeinput.h"

/* Lamp-lägen */
typedef enum {
    LAMP_OFF,
    LAMP_GREEN,
    LAMP_RED
} LampColor;

/* Ett kort i systemet */
typedef struct {
    int cardNumber;
    int hasAccess;
    char dateAdded[11]; /* YYYY-MM-DD */
} Card;

/* Allt state för systemet */
typedef struct {
    Card *cards;
    int count;
    int capacity;
    LampColor lamp;
} SystemState;

/* Skriver ut lampans färg */
void printLamp(const SystemState *state) {
    const char *t = "Off";
    if (state->lamp == LAMP_GREEN) t = "Green";
    else if (state->lamp == LAMP_RED) t = "Red";
    printf("CURRENTLY LAMP IS:%s\n", t);
}

/* Byter lampans färg och visar direkt */
void setLamp(SystemState *state, LampColor c) {
    state->lamp = c;
    printLamp(state);
}

/* Hämtar dagens datum som text */
void getToday(char *buffer) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t)
        strftime(buffer, 11, "%Y-%m-%d", t);
    else
        strncpy(buffer, "1970-01-01", 11);
}

/* Ser till att arrayen med kort har plats */
void ensureCapacity(SystemState *state) {
    if (state->count >= state->capacity) {
        int newCap = state->capacity == 0 ? 4 : state->capacity * 2;
        Card *tmp = realloc(state->cards, newCap * sizeof(Card));
        if (!tmp) {
            printf("Memory error.\n");
            exit(1);
        }
        state->cards = tmp;
        state->capacity = newCap;
    }
}

/* Letar efter kortnummer, returnerar index eller -1 */
int findCardIndex(const SystemState *state, int cardNumber) {
    for (int i = 0; i < state->count; i++)
        if (state->cards[i].cardNumber == cardNumber)
            return i;
    return -1;
}

/* Menyval 1: öppna dörren */
void remoteOpenDoor(SystemState *state) {
    printf("Remote open door\n");
    setLamp(state, LAMP_GREEN);
    SLEEP_MS(3000);
    setLamp(state, LAMP_OFF);
}

/* Menyval 2: lista alla kort */
void listAllCards(const SystemState *state) {
    printf("All cards in system\n");
    if (state->count == 0) {
        printf("No cards in system.\n");
    } else {
        for (int i = 0; i < state->count; i++) {
            const Card *c = &state->cards[i];
            printf("%d %s Added: %s\n",
                   c->cardNumber,
                   c->hasAccess ? "Access" : "No access",
                   c->dateAdded);
        }
    }
    char dummy[8];
    GetInput("Press key to continue", dummy, sizeof(dummy));
}

/* Menyval 3: lägg till eller ta bort access */
void addRemoveAccess(SystemState *state) {
    int cardNr;
    printf("Add/remove access\n");
    printLamp(state);

    if (!GetInputInt("Enter cardnumber: ", &cardNr)) {
        printf("Invalid number.\n");
        return;
    }

    int index = findCardIndex(state, cardNr);

    if (index == -1) {
        ensureCapacity(state);
        index = state->count++;
        state->cards[index].cardNumber = cardNr;
        state->cards[index].hasAccess = 0;
        getToday(state->cards[index].dateAdded);
        printf("New card created.\n");
    } else {
        printf(state->cards[index].hasAccess ?
               "This card has access.\n" :
               "This card has no access.\n");
    }

    int choice;
    if (!GetInputInt("1 = access, 2 = no access: ", &choice)) {
        printf("Invalid input.\n");
        return;
    }

    if (choice == 1) {
        state->cards[index].hasAccess = 1;
        printf("Access added.\n");
    } else if (choice == 2) {
        state->cards[index].hasAccess = 0;
        printf("Access removed.\n");
    } else {
        printf("No change.\n");
    }
}

/* Menyval 9: fake scanning av kort */
void fakeScanCard(SystemState *state) {
    char buf[255];
    while (1) {
        printf("Please scan card or enter X to go back.\n");
        printLamp(state);

        if (GetInput("", buf, sizeof(buf)) != INPUT_RESULT_OK) {
            printf("Invalid input.\n");
            continue;
        }

        if (buf[0] == 'X' || buf[0] == 'x')
            return;

        int cardNr;
        if (sscanf(buf, "%d", &cardNr) != 1) {
            printf("Invalid card.\n");
            continue;
        }

        int index = findCardIndex(state, cardNr);
        if (index >= 0 && state->cards[index].hasAccess)
            setLamp(state, LAMP_GREEN);
        else
            setLamp(state, LAMP_RED);
    }
}

/* Skriver ut adminmenyn */
void printMenu(const SystemState *state) {
    printf("=== Admin menu ===\n");
    printf("1. Remote open door\n");
    printf("2. List all cards\n");
    printf("3. Add/remove access\n");
    printf("4. Exit\n");
    printf("9. Fake test scan card\n");
    printLamp(state);
}

/* Mainloop för programmet */
int main(void) {
    SystemState state;
    state.cards = NULL;
    state.count = 0;
    state.capacity = 0;
    state.lamp = LAMP_OFF;

    int running = 1;
    int choice;

    while (running) {
        printMenu(&state);

        if (!GetInputInt("Choose option: ", &choice)) {
            printf("Invalid.\n");
            continue;
        }

        switch (choice) {
            case 1: remoteOpenDoor(&state); break;
            case 2: listAllCards(&state); break;
            case 3: addRemoveAccess(&state); break;
            case 4: running = 0; break;
            case 9: fakeScanCard(&state); break;
            default: printf("Invalid option.\n"); break;
        }
    }

    free(state.cards);
    return 0;
}
