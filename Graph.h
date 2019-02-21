#include "Graph.c"

void DrawPixel(SDL_Surface *lscreen, DWORD ColorCode,int x,int y);
SDL_Surface *load_image(char *filename) ;
void apply_surface(int x, int y, SDL_Surface* source, SDL_Surface* destination);	
