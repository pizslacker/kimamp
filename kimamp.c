#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <zip.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

// --- Helper: Load a BMP from a .wsz (zip) directly into an SDL_Texture ---
SDL_Texture* LoadTextureFromZip(SDL_Renderer* renderer, const char* zipPath, const char* fileName) {
    printf("[SKIN] Attempting to open archive: %s\n", zipPath);
    int err = 0;
    zip_t *za = zip_open(zipPath, 0, &err);
    if (za == NULL) {
        printf("[SKIN] ERROR: Failed to open zip archive: %s (Error code: %d)\n", zipPath, err);
        return NULL;
    }

    zip_stat_t sb;
    if (zip_stat(za, fileName, ZIP_FL_NOCASE, &sb) != 0) {
        printf("[SKIN] ERROR: Could not find '%s' in archive.\n", fileName);
        zip_close(za);
        return NULL;
    }
    
    printf("[SKIN] Found '%s' in archive. Uncompressed size: %llu bytes.\n", fileName, (unsigned long long)sb.size);

    zip_file_t *zf = zip_fopen(za, fileName, ZIP_FL_NOCASE);
    if (zf == NULL) {
        printf("[SKIN] ERROR: Failed to open file inside zip.\n");
        zip_close(za);
        return NULL;
    }

    printf("[SKIN] Allocating %llu bytes of memory for extraction...\n", (unsigned long long)sb.size);
    char *buffer = (char *)malloc(sb.size);
    if (buffer == NULL) {
        printf("[SKIN] ERROR: Memory allocation failed!\n");
        zip_fclose(zf);
        zip_close(za);
        return NULL;
    }
    
    zip_fread(zf, buffer, sb.size);
    zip_fclose(zf);
    zip_close(za); 
    printf("[SKIN] File extracted to memory successfully. Archive closed.\n");

    printf("[SKIN] Converting memory buffer to SDL_Surface...\n");
    SDL_RWops *rw = SDL_RWFromMem(buffer, sb.size);
    SDL_Surface *surface = SDL_LoadBMP_RW(rw, 1); 
    
    if (surface == NULL) {
        printf("[SKIN] ERROR: SDL could not load BMP from memory: %s\n", SDL_GetError());
        free(buffer);
        return NULL;
    }

    printf("[SKIN] Uploading Surface to GPU as SDL_Texture...\n");
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    free(buffer); 
    printf("[SKIN] Texture created successfully. RAM buffer freed.\n");

    return texture;
}

int main(int argc, char* argv[]) {
    // Silence unused parameter warnings
    (void)argc;
    (void)argv;

    // 1. Initialize SDL (Video and Audio)
    printf("[SYSTEM] Booting k!Mamp...\n");
    printf("[SYSTEM] Initializing SDL Video and Audio subsystems...\n");
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("[SYSTEM] FATAL ERROR: %s\n", SDL_GetError());
        return 1;
    }

    // 2. Create Window and Renderer
    printf("[SYSTEM] Creating window (275x116)...\n");
    SDL_Window* window = SDL_CreateWindow("k!Mamp - C/SDL2", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 275, 116, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("[SYSTEM] FATAL ERROR: Window creation failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    printf("[SYSTEM] Initializing hardware-accelerated renderer...\n");
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("[SYSTEM] FATAL ERROR: Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // 3. Initialize SDL_mixer
    printf("[AUDIO] Initializing SDL_mixer for MP3 playback...\n");
    int flags = MIX_INIT_MP3;
    if ((Mix_Init(flags) & flags) != flags) {
        printf("[AUDIO] WARNING: SDL_mixer MP3 init failed: %s\n", Mix_GetError());
    } 
    
    printf("[AUDIO] Opening audio device (44.1kHz, Stereo, 2048 chunk size)...\n");
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("[AUDIO] ERROR: Audio Open failed: %s\n", Mix_GetError());
    }

    // 4. Load Assets (Skin and Audio)
    printf("----------------------------------------\n");
    SDL_Texture* mainSkinTexture = LoadTextureFromZip(renderer, "base.wsz", "main.bmp");
    
    printf("[AUDIO] Loading track 'test.mp3'...\n");
    Mix_Music *bgm = Mix_LoadMUS("test.mp3");
    
    if (bgm) {
        printf("[AUDIO] Track loaded successfully. Starting playback loop...\n");
        Mix_PlayMusic(bgm, -1); 
    } else {
        printf("[AUDIO] ERROR: Could not load 'test.mp3'. Running visual-only mode.\n");
    }
    printf("----------------------------------------\n");
    printf("[SYSTEM] Entering main application loop. Press SPACE to pause, 'S' to stop.\n");

    // 5. Main Application Loop
    bool quit = false;
    SDL_Event e;

    while (!quit) {
        // Event Handling
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                printf("[EVENT] Application quit requested by user.\n");
                quit = true;
            } else if (e.type == SDL_KEYDOWN) {
                switch(e.key.keysym.sym) {
                    case SDLK_SPACE:
                        if (Mix_PausedMusic() == 1) {
                            Mix_ResumeMusic();
                            printf("[EVENT] Playback resumed.\n");
                        } else {
                            Mix_PauseMusic();
                            printf("[EVENT] Playback paused.\n");
                        }
                        break;
                    case SDLK_s:
                        Mix_HaltMusic();
                        printf("[EVENT] Playback halted.\n");
                        break;
                }
            }
        }

        // Rendering
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);

        if (mainSkinTexture) {
            int w, h;
            SDL_QueryTexture(mainSkinTexture, NULL, NULL, &w, &h);
            SDL_Rect destRect = { (275 - w) / 2, (116 - h) / 2, w, h };
            SDL_RenderCopy(renderer, mainSkinTexture, NULL, &destRect);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60 fps
    }

    // 6. Cleanup
    printf("----------------------------------------\n");
    printf("[SYSTEM] Initiating shutdown sequence...\n");
    
    if (mainSkinTexture) {
        printf("[SKIN] Destroying skin texture...\n");
        SDL_DestroyTexture(mainSkinTexture);
    }
    
    if (bgm) {
        printf("[AUDIO] Freeing music memory...\n");
        Mix_FreeMusic(bgm);
    }
    
    printf("[AUDIO] Closing audio device...\n");
    Mix_CloseAudio();
    Mix_Quit();
    
    printf("[SYSTEM] Destroying renderer and window...\n");
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    printf("[SYSTEM] Shutdown complete. Goodbye!\n");

    return 0;
}
