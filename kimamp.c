#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <zip.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

// --- Helper: Load a BMP from a .wsz (zip) directly into an SDL_Texture ---
SDL_Texture* LoadTextureFromZip(SDL_Renderer* renderer, const char* zipPath, const char* fileName) {
    int err = 0;
    zip_t *za = zip_open(zipPath, 0, &err);
    if (za == NULL) {
        printf("Failed to open zip archive: %s\n", zipPath);
        return NULL;
    }

    zip_stat_t sb;
    if (zip_stat(za, fileName, ZIP_FL_NOCASE, &sb) != 0) {
        printf("Could not find %s in archive.\n", fileName);
        zip_close(za);
        return NULL;
    }

    zip_file_t *zf = zip_fopen(za, fileName, ZIP_FL_NOCASE);
    if (zf == NULL) {
        printf("Failed to open file inside zip.\n");
        zip_close(za);
        return NULL;
    }

    char *buffer = (char *)malloc(sb.size);
    if (buffer == NULL) {
        printf("Memory allocation failed!\n");
        zip_fclose(zf);
        zip_close(za);
        return NULL;
    }
    
    zip_fread(zf, buffer, sb.size);
    zip_fclose(zf);
    zip_close(za); 

    SDL_RWops *rw = SDL_RWFromMem(buffer, sb.size);
    SDL_Surface *surface = SDL_LoadBMP_RW(rw, 1); 
    
    if (surface == NULL) {
        printf("SDL could not load BMP from memory: %s\n", SDL_GetError());
        free(buffer);
        return NULL;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    free(buffer); 

    return texture;
}

int main(int argc, char* argv[]) {
    // Silence unused parameter warnings
    (void)argc;
    (void)argv;

    // 1. Initialize SDL (Video and Audio)
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("SDL Error: %s\n", SDL_GetError());
        return 1;
    }

    // 2. Create Window and Renderer
    SDL_Window* window = SDL_CreateWindow("k!Mamp - C/SDL2", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 275, 116, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Window Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Renderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // 3. Initialize SDL_mixer
    int flags = MIX_INIT_MP3;
    if ((Mix_Init(flags) & flags) != flags) {
        printf("SDL_mixer Error: %s\n", Mix_GetError());
    } else if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("Audio Open Error: %s\n", Mix_GetError());
    }

    // 4. Load Assets (Skin and Audio)
    SDL_Texture* mainSkinTexture = LoadTextureFromZip(renderer, "base.wsz", "main.bmp");
    Mix_Music *bgm = Mix_LoadMUS("test.mp3");
    
    if (bgm) {
        Mix_PlayMusic(bgm, -1); // -1 loops infinitely
        printf("Playing audio... Press SPACE to pause/resume, 'S' to stop.\n");
    } else {
        printf("Could not load test.mp3. Running without audio.\n");
    }

    // 5. Main Application Loop
    bool quit = false;
    SDL_Event e;

    while (!quit) {
        // Event Handling
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_KEYDOWN) {
                switch(e.key.keysym.sym) {
                    case SDLK_SPACE:
                        if (Mix_PausedMusic() == 1) Mix_ResumeMusic();
                        else Mix_PauseMusic();
                        break;
                    case SDLK_s:
                        Mix_HaltMusic();
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
    if (mainSkinTexture) SDL_DestroyTexture(mainSkinTexture);
    if (bgm) Mix_FreeMusic(bgm);
    
    Mix_CloseAudio();
    Mix_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}