#include "image.h"


SDL_Surface* loadImage(char* path) {
    SDL_Surface *image = IMG_Load(path);

    if (!image) {
        errx(500, "The image at the path '%s'can't be loaded: %s", path, IMG_GetError());
    }

    return image;
}

Uint32 getPixel(SDL_Surface *image, int x, int y)
{
    // Pixel reference
    Uint8 *p = (Uint8*)image->pixels + y * image->pitch + x * image->format->BytesPerPixel;;

    switch (image->format->BytesPerPixel)
    {
        case 1:
            return *p;

        case 2:
            return *(Uint16 *)p;

        case 3:
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
                return p[0] << 16 | p[1] << 8 | p[2];
            else
                return p[0] | p[1] << 8 | p[2] << 16;

        case 4:
            return *(Uint32 *)p;
    }
    return *p/24;
}

int show_matrix_to_img(struct MatrixUCHAR matrix)
{
    
    /* Find window size*/
    int coef = 1;
    if (matrix.rows < 780)
    {
        coef = floor(780/matrix.rows);
    }

    SDL_Window* window = NULL;
    window = SDL_CreateWindow
    (
        "Binarized Image", SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        matrix.columns * coef,
        matrix.rows * coef,
        SDL_WINDOW_SHOWN
    );

    // Setup renderer
    SDL_Renderer* renderer = NULL;
    renderer =  SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED);

    // Set render color to red ( background will be rendered in this color )
    SDL_SetRenderDrawColor( renderer, 255, 255, 255, 0 );

    // Clear winow
    SDL_RenderClear( renderer );


    for(int i = 0; i < matrix.rows;i++) {
        for(int j = 0; j < matrix.columns;j++) {
            Uint32 grey = 0;
            SDL_Rect r = {j * coef, i * coef, coef, coef};

            grey = matrixGetUCHAR(matrix, i, j) * 255;
            SDL_SetRenderDrawColor( renderer, grey, grey, grey, 0 );

            SDL_RenderFillRect( renderer, &r );
        }
    }

    // Render the rect to the screen
    SDL_RenderPresent(renderer);


    SDL_Event event;
    int quit = 0;
    while (quit == 0)
    {
        SDL_WaitEvent(&event);
 
        switch (event.type)
        {
        case SDL_QUIT:
            quit = 1;
            break;
        }
    }
    SDL_DestroyWindow(window);

    return EXIT_SUCCESS;
}

int ShowImg(SDL_Surface *image)
{
     
    /* Find window size*/
    int coef = 1;
    if (image->h < 780)
    {
        coef = floor(780/image->h);
    }

    SDL_Window* window = NULL;
    window = SDL_CreateWindow
    (
        "Binarized Image", SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        image->w * coef,
        image->h * coef,
        SDL_WINDOW_SHOWN
    );

    // Setup renderer
    SDL_Renderer* renderer = NULL;
    renderer =  SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED);

    // Set render color to red ( background will be rendered in this color )
    SDL_SetRenderDrawColor( renderer, 255, 255, 255, 0 );

    // Clear winow
    SDL_RenderClear( renderer );


    for(int i = 0; i < image->h;i++) {
        for(int j = 0; j < image->w;j++) {
            Uint32 pixel = 0;
            SDL_Rect r = {j * coef, i * coef, coef, coef};
            SDL_Color color;

            pixel = getPixel(image, j, i);
            SDL_GetRGB(pixel, image->format, &color.r, &color.g, &color.b);
            SDL_SetRenderDrawColor( renderer, color.r, color.g, color.b, 0 );

            SDL_RenderFillRect( renderer, &r );
        }
    }

    // Render the rect to the screen
    SDL_RenderPresent(renderer);


    SDL_Event event;
    int quit = 0;
    while (quit == 0)
    {
        SDL_WaitEvent(&event);
 
        switch (event.type)
        {
        case SDL_QUIT:
            quit = 1;
            break;
        }
    }
    SDL_DestroyWindow(window);

    return EXIT_SUCCESS;
}

SDL_Surface* MedianFilter(SDL_Surface *image, int px)
{
    //Verify px parameter
    int border = 0;
    switch (px)
    {
    case 3:
        border = 1;
        break;
    case 5:
        border = 2;
        break;
    case 7:
        border = 3;
        break;
    
    default:
        return image;
    }


    SDL_Surface *outImage = SDL_CreateRGBSurface(image->flags,image->w,image->h,image->format->BitsPerPixel,image->format->Rmask,image->format->Gmask,image->format->Bmask,image->format->Amask);
    for (int i = 1; i < image->w - border; i++)
    {
        for (int j = 1; j < image->h - border; j++)
        {
            //Create neighbours pixels matrix
            int neighboursR[px*px];
            int neighboursG[px*px];
            int neighboursB[px*px];

            //Set neighbours pixels
            int index = 0;
            for (int x = 0; x < px; x++)
            {
                for (int y = 0; y < px; y++)
                {                                     
                    Uint32 pixel = getPixel(image, i - 1 + x, j - 1 + y);
                    SDL_Color color;
                    SDL_GetRGB(pixel, image->format, &color.r, &color.g, &color.b);
                    neighboursR[index] = color.r;
                    neighboursG[index] = color.g;
                    neighboursB[index] = color.b;
                    index += 1;
                }
                
            }

            

            // Search Median value
            int medianValueR = MedianValueINT(neighboursR, sizeof(neighboursR) / sizeof(neighboursR[0]));
            int medianValueG = MedianValueINT(neighboursG, sizeof(neighboursG) / sizeof(neighboursG[0]));
            int medianValueB = MedianValueINT(neighboursB, sizeof(neighboursB) / sizeof(neighboursB[0]));

            // Set pixel value
            SDL_Rect surface_rect = {i, j, 1, 1};
            SDL_FillRect(outImage, &surface_rect, SDL_MapRGB(image->format, medianValueR, medianValueG, medianValueB));
        }
        
    }
    
    return outImage;
}

SDL_Surface* Grayscale(SDL_Surface *image)
{
    SDL_Surface *outImage = SDL_CreateRGBSurface(image->flags,image->w,image->h,image->format->BitsPerPixel,image->format->Rmask,image->format->Gmask,image->format->Bmask,image->format->Amask);
    for (int i = 1; i < image->w - 1; i++)
    {
        for (int j = 1; j < image->h - 1; j++)
        {
            SDL_Color color;
            Uint32 pixel = getPixel(image, i, j);
            SDL_GetRGB(pixel, image->format, &color.r, &color.g, &color.b);

            Uint8 newcolor = (0.2126*color.r) + (0.7152*color.g) + (0.0722*color.b);

            // Set pixel value
            SDL_Rect surface_rect = {i, j, 1, 1};
            SDL_FillRect(outImage, &surface_rect, SDL_MapRGB(image->format, newcolor, newcolor, newcolor));
        }
    }
    return outImage;
}

SDL_Surface* laplacien(SDL_Surface *image)
{
    SDL_Surface *outImage = SDL_CreateRGBSurface(image->flags,image->w,image->h,image->format->BitsPerPixel,image->format->Rmask,image->format->Gmask,image->format->Bmask,image->format->Amask);
    for (int i = 1; i < image->w - 1; i++)
    {
        for (int j = 1; j < image->h - 1; j++)
        {
            SDL_Color finalColor;
            SDL_Color color1;
            SDL_Color color2;
            SDL_Color color3;
            SDL_Color color4;
            SDL_Color color5;

            Uint32 pixel1 = getPixel(image, i, j - 1);
            Uint32 pixel2 = getPixel(image, i - 1, j);
            Uint32 pixel3 = getPixel(image, i + 1, j);
            Uint32 pixel4 = getPixel(image, i, j + 1);
            Uint32 pixel5 = getPixel(image, i, j);

            SDL_GetRGB(pixel1, image->format, &color1.r, &color1.g, &color1.b);
            SDL_GetRGB(pixel2, image->format, &color2.r, &color2.g, &color2.b);
            SDL_GetRGB(pixel3, image->format, &color3.r, &color3.g, &color3.b);
            SDL_GetRGB(pixel4, image->format, &color4.r, &color4.g, &color4.b);
            SDL_GetRGB(pixel5, image->format, &color5.r, &color5.g, &color5.b);

            finalColor.r = color1.r + color2.r + color3.r + color4.r - (4*color5.r);
            finalColor.g = color1.g + color2.g + color3.g + color4.g - (4*color5.g);
            finalColor.b = color1.b + color2.b + color3.b + color4.b - (4*color5.b);

            // Set pixel value
            SDL_Rect surface_rect = {i, j, 1, 1};
            SDL_FillRect(outImage, &surface_rect, SDL_MapRGB(image->format, finalColor.r, finalColor.r, finalColor.r));
        }
    }
    return outImage;
}