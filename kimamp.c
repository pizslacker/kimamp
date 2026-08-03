#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

int main(int argc, char* argv[]) {
    // 1. Initialize SDL Video and Audio
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        return 1;
    }

    // 2. Create a basic Window (for our UI later)
    SDL_Window* window = SDL_CreateWindow(
        "Winamp Clone Minimal", 
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
        400, 200, 
        SDL_WINDOW_SHOWN
    );

    if (window == NULL) {
        printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // 3. Initialize SDL_mixer
    // We want MP3 support. You can add MIX_INIT_OGG if needed.
    int flags = MIX_INIT_MP3;
    if ((Mix_Init(flags) & flags) != flags) {
        printf("SDL_mixer could not initialize! Mix Error: %s\n", Mix_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // 4. Open the Audio Device
    // 44100 Hz, Standard 16-bit format, 2 channels (stereo), 2048 byte chunk size
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer could not open audio! Mix Error: %s\n", Mix_GetError());
        Mix_Quit();
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // 5. Load an MP3 file (Ensure you have a file named 'test.mp3' in the same folder)
    Mix_Music *bgm = Mix_LoadMUS("test.mp3");
    if (bgm == NULL) {
        printf("Failed to load beat music! Mix Error: %s\n", Mix_GetError());
        // We won't quit here, just continue so the window stays open
    } else {
        // Play the music: -1 means loop infinitely. 1 means play once.
        Mix_PlayMusic(bgm, 1);
        printf("Playing audio...\n");
    }

    // 6. Application Loop
    bool quit = false;
    SDL_Event e;

    while (!quit) {
        // Handle events on queue
        while (SDL_PollEvent(&e) != 0) {
            // User requests quit (clicks the 'X' on the window)
            if (e.type == SDL_EVENT_QUIT) {
                quit = true;
            }
            // Basic Keyboard controls
            else if (e.type == SDL_KEYDOWN) {
                switch(e.key.keysym.sym) {
                    case SDLK_SPACE:
                        // Toggle pause/play
                        if (Mix_PausedMusic() == 1) {
                            Mix_ResumeMusic();
                            printf("Resumed.\n");
                        } else {
                            Mix_PauseMusic();
                            printf("Paused.\n");
                        }
                        break;
                    case SDLK_s:
                        // Stop music
                        Mix_HaltMusic();
                        printf("Stopped.\n");
                        break;
                }
            }
        }
        
        // Small delay to prevent maxing out the CPU loop
        SDL_Delay(16); 
    }

    // 7. Clean up and Exit
    if (bgm != NULL) {
        Mix_FreeMusic(bgm);
    }
    Mix_CloseAudio();
    Mix_Quit();
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
