#include <stdio.h>
#include "cpu.h"
#include <string.h>  
#include "renderer.h"
#include <time.h>
#include "global.h"


int currentPressedKey;
int delayTimer;
int soundTimer;
uint8_t display[32][64];
#define ROM_START 0x200

int memory[4096];

uint8_t registers[16];

int i = 0;

int stack[16];
int sp = 0;

int pc = ROM_START;



int font[] = {
            0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
            0x20, 0x60, 0x20, 0x20, 0x70, // 1
            0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
            0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
            0x90, 0x90, 0xF0, 0x10, 0x10, // 4
            0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
            0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
            0xF0, 0x10, 0x20, 0x40, 0x40, // 7
            0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
            0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
            0xF0, 0x90, 0xF0, 0x90, 0x90, // A
            0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
            0xF0, 0x80, 0x80, 0x80, 0xF0, // C
            0xE0, 0x90, 0x90, 0x90, 0xE0, // D
            0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
            0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void LoadFont();
void ExecuteInstruction(int opcode);

int GetCurrentInstruction() {
    unsigned char firstInstruction = memory[pc];       // Asegúrate de usar unsigned char
    unsigned char secondInstruction = memory[pc + 1];  // Asegúrate de usar unsigned char

    // Combinar ambos bytes en un opcode de 16 bits
    int opcode = (firstInstruction << 8) | secondInstruction;

    return opcode;
}


void LoadRom(char* path) {
    FILE* fptr;

    fptr = fopen(path, "rb");

    fseek(fptr, 0l, SEEK_END);
    long sz = ftell(fptr);
    fseek(fptr, 0l, SEEK_SET);

    char* rom = (char*)malloc((sz+1)* sizeof(char));

    if (rom == NULL) {
        printf("Memory not allocated.\n");
        return;
    }


    fread(rom, sizeof(char), sz+1, fptr);


    for (int i = 0; i < sz; i++) {
        memory[i + ROM_START] = rom[i];
        printf("%02x ", (unsigned char)rom[i]);
    }


}

void LoadFont() {
    for (int i = 0; i < sizeof(font) / sizeof(font[0]); i++) {
        memory[i] = font[i];
    } 
}

int hex2int(char ch)
{
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    if (ch >= 'A' && ch <= 'F')
        return ch - 'A' + 10;
    if (ch >= 'a' && ch <= 'f')
        return ch - 'a' + 10;
    return -1;
}



void ExecuteInstruction(int opcode) {
    srand(time(0));
    pc += 2;
    
    int first = (opcode & 0xf000) >> 12;

    int x = (opcode & 0x0f00) >> 8;
    int y = (opcode & 0x00f0) >> 4;

    int n = opcode & 0x000f;
    int nn = opcode & 0x00ff;
    int nnn = opcode & 0x0fff;


    switch (first)
    {
    case 0:
        if (nn == 0xE0){

            memset(display, 0, sizeof(display));
            ClearScreen();
        }
        else if(nn == 0xEE){
            if (sp > 0) {
                sp--;
                pc = stack[sp];
            }
            else {
                printf("Error: Stack underflow\n");
            }
        }
        break;
    case 1:
        pc = nnn;
        break;
    case 2:
        stack[sp] = pc;
        sp++;
        pc = nnn;
        break;
    case 3:
        if (registers[x] == nn) {
            pc += 2;
        }
        break;
    case 4:
        if (registers[x] != nn) {
            pc += 2;
        }
        break;
    case 5:
        if (registers[x] == registers[y]) {
            pc += 2;
        }
        break;

    case 6:
        registers[x] = nn;
        break;
    case 7:
        registers[x] += nn;
        break;
    case 8:
        switch (n)
        {
        case 0:
            registers[x] = registers[y];
            break;
        case 1:
            registers[x] |= registers[y];
            break;
        case 2:
            registers[x] &= registers[y];
            break;
        case 3:
            registers[x] ^= registers[y];
            break;
        case 4:
            uint8_t beforeSum = registers[x];
            registers[x] += registers[y];
            if (registers[x] < registers[y] || registers[x] < beforeSum) {
                registers[0xf] = 1;
            }
            else {
                registers[0xf] = 0;
            }


            break;
        case 5: {
            uint8_t result = registers[x] - registers[y];
            if (registers[x] > (result - registers[y])) {
                registers[x] = result;
                registers[0xf] = 1;
            }
            else {
                registers[x] = result;
                registers[0xf] = 0;
            }
        }


            break;
        case 6:
        {
            uint8_t old = registers[x];
            registers[x] >>= 1;
            registers[0xf] = old & 0x1;
            break;
        }



        case 7: {
            uint8_t result = registers[y] - registers[x];
            
            if (registers[y] >= registers[x]) {
                registers[x] = result;
                registers[0xf] = 1;

            }
            else {
                registers[x] = result;
                registers[0xf] = 0;

            }
        }

            break;
        case 0xE:
        {
            uint8_t old = registers[x];
            registers[x] <<= 1;
            registers[0xf] = (old & 0x80) >> 7;
            break;
        }          
        default:
            break;
        }
        break;
    case 9:
        if (registers[x] != registers[y])
        {
            pc += 2;
        }
        break;
    case 0xA:
        i = nnn;
        break;
    case 0xB:
        pc = registers[0] + nnn;
        break;
    case 0xC:
        registers[x] = (rand() % 256) & nn;
        break;
    case 0xD:
        registers[0xF] = 0;
        for (int row = 0; row < n; row++) {

            int sprite = memory[i + row];
            for (int col = 0; col < 8; col++) {

                if ((sprite & 0x80) > 0) {
   
                    int xPos = (registers[x] + col) % 64;
                    int yPos = (registers[y] + row) % 32;

                    if (display[yPos][xPos] == 1) {
                        registers[0xF] = 1;
                    }
                    display[yPos][xPos] ^= 1;

                    Draw(xPos, yPos, 1, 1);
                }
                sprite <<= 1;
            }

        }

        break;
    case 0xE:
        if (nn == 0x9E) {
            if (registers[x] == currentPressedKey) {
                pc += 2;
            }
        }
        else if (nn == 0xA1) {
            if (registers[x] != currentPressedKey) {
                pc += 2;
            }
        }
        break;
    case 0xF:
        switch (nn)
        {
        case 0x07:
            registers[x] = delayTimer;
            break;
        case 0x0A:
            registers[x] = currentPressedKey;
            break;
        case 0x15:
            delayTimer = registers[x];
            break;
        case 0x18:
            soundTimer = registers[x];
            break;
        case 0x1E:
            i += registers[x];
            break;
        case 0x29:
            i = registers[x] * 5;
            break;
        case 0x33:
            printf("");
            int u = registers[x] % 10;
            int d = (registers[x] / 10) % 10;
            int c = (registers[x] / 100) % 10;

            memory[i] = c;
            memory[i + 1] = d;
            memory[i + 2] = u;

            break;
        case 0x55:
            for (int offset = 0; offset <= x; offset++) {
                memory[offset + i] = registers[offset];
            }
            break;
        case 0x65:
            for (int offset = 0; offset <= x; offset++) {
                registers[offset] = memory[offset + i];
            }
            break;
        default:
            
            break;
        }
        break;
    default:
        printf("\n");
        printf(opcode);
        printf("\n");
        break;
    }


}
