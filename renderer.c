#include <stdio.h>
#include "renderer.h"
#include "cpu.h"
#include "global.h"
#include <windows.h>


const int SCREEN_WIDTH = 64;
const int SCREEN_HEIGHT = 32;
SDL_Renderer* renderer = NULL;

#define SCALE 8

uint8_t keymap[16] = {
    SDLK_x, SDLK_1, SDLK_2, SDLK_3,
    SDLK_q, SDLK_w, SDLK_e, SDLK_a,
    SDLK_s, SDLK_d, SDLK_z, SDLK_c,
    SDLK_4, SDLK_r, SDLK_f, SDLK_v
};

const int INSTRUCTIONS_PER_SECOND = 700;
const int TIMER_HZ = 60;

void Cycle() {
    int opcode = GetCurrentInstruction();
    ExecuteInstruction(opcode);


}

int main(int argc, char* args[]) {

    SDL_Window* window = NULL;

    Uint32 lastCycleTime = 0;
    Uint32 lastTimerUpdate = 0;

    LoadRom("D:/projects/C/Chip8/x64/Debug/rom/tetris");
    LoadFont();

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    }
    else
    {
        window = SDL_CreateWindow("Chip8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH * SCALE, SCREEN_HEIGHT * SCALE, SDL_WINDOW_SHOWN);
        if (window == NULL)
        {
            printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        }
        else
        {
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
            if (renderer == NULL) {
                printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
                SDL_DestroyWindow(window);
                SDL_Quit();
                return -1;
            }

            SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
            SDL_RenderClear(renderer);



            SDL_Event e;
            int quit = 0;
            while (quit == 0) {
                while (SDL_PollEvent(&e)) {
                    if (e.type == SDL_QUIT) quit = 1;
                    if (e.type == SDL_KEYDOWN) {
                        for (int i = 0; i < 16; i++) {
                            if (e.key.keysym.sym == keymap[i]) {
                                currentPressedKey = i;
                                break;
                            }
                        }
                    }
                    else if (e.type == SDL_KEYUP) {
                        currentPressedKey = -1;
                    }
                }
                Uint32 currentTime = SDL_GetTicks();

                if (currentTime - lastCycleTime > 1000 / INSTRUCTIONS_PER_SECOND) {
                    Cycle();
                    lastCycleTime = currentTime;
                }

                if (currentTime - lastTimerUpdate > 1000 / TIMER_HZ) {
                    if (soundTimer != 0) {
                        printf("\a");
                    }
                    if (delayTimer > 0) delayTimer--;
                    if (soundTimer > 0) soundTimer--;
                    lastTimerUpdate = currentTime;
                }

                SDL_RenderPresent(renderer);
            }
        }
    }

    SDL_DestroyWindow(window);


    SDL_Quit();

    return 0;
}

void Draw(int Vx, int Vy, int width, int height) {
    SDL_Rect rect = {
        Vx * SCALE,
        Vy * SCALE,
        width * SCALE,
        height * SCALE
    };
    SDL_SetRenderDrawColor(renderer,
        display[Vy][Vx] ? 0x00 : 0xFF,
        display[Vy][Vx] ? 0x00 : 0xFF,
        display[Vy][Vx] ? 0x00 : 0xFF,
        0xFF);
    SDL_RenderFillRect(renderer, &rect);
}

void ClearScreen() {
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}