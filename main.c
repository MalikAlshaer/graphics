#include <SDL2/SDL.h>
//#include <SDL2/SDL_events.h>
//#include <SDL2/SDL_rect.h>
//#include <SDL2/SDL_surface.h>
//#include <SDL2/SDL_video.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#define SCREEN_WIDTH    900
#define SCREEN_HEIGHT   900
#define COLOR_WHITE     0xffffffff
#define COLOR_BLACK     0x00000000
#define SQUARE_SIDE     5
#define LINE_DENSITY    1 // the lower the more dense
#define FPS             60

void wait(long ms);
int lines(FILE *readfile);
void input_loop(SDL_Event event, int *running);

typedef struct Point {
    float x;
    float y;
    float z;
} Point;

typedef struct Face {
    int a;
    int b;
} Face;

// init function for point
Point init(float x, float y, float z) {
    return (Point){ .x = x, .y = y, .z = z };
}

void draw(SDL_Window *window, SDL_Surface *surface, Point p1, Point p2) {

    //float l = sqrt((p2.x - p1.x)*(p2.x - p1.x) + (p2.y - p1.y)*(p2.y - p1.y));

    //float y = p1.y;
    if (p1.x == p2.x) { // <---
        // vertical line special case

        float step = (p2.y > p1.y) ? LINE_DENSITY : -LINE_DENSITY;
        for (float y = p1.y; (step > 0 ? y <= p2.y : y >= p2.y); y += step) {
            SDL_Rect r = {(int)(p1.x - SQUARE_SIDE/2.0), (int)(y - SQUARE_SIDE/2.0), SQUARE_SIDE, SQUARE_SIDE};
            SDL_FillRect(surface, &r, COLOR_WHITE);
        }
        SDL_UpdateWindowSurface(window);
        return;
    }

    float m = (p2.y - p1.y)/(p2.x - p1.x);
    float b =  p1.y - m*p1.x;

    // if p1.x > p2.x step = LINE_DENSITY otherwise step = -LINE_DENSITY (negative)
    float step = (p2.x > p1.x) ? LINE_DENSITY : -LINE_DENSITY;

    for (float x = p1.x; (step > 0 ? x <= p2.x : x >= p2.x); x += step) { // <--- loop condition
        float y = m * x + b;
        SDL_Rect line_point = {(int)(x - SQUARE_SIDE/2.0), (int)(y - SQUARE_SIDE/2.0), SQUARE_SIDE, SQUARE_SIDE};
        SDL_FillRect(surface, &line_point, COLOR_WHITE);
        //printf("\nx: %.2f\ny: %.2f\n\nx: %.2f\ny: %.2f\n\n", p1.x, p1.y,p2.x, p2.y);
    }
    SDL_UpdateWindowSurface(window); // update window
}

Point coords(Point p) {
    p.x = ((p.x + 1)/2)*SCREEN_WIDTH;
    p.y = ((1 - (p.y + 1)/2))*SCREEN_HEIGHT;
    return p;
}

Point mutate(Point p) {
    p.x = p.x/p.z;
    p.y = p.y/p.z;
    return p;
}

Point translate_z(Point p, float dz) {
    return (Point) { .x = p.x, .y = p.y, .z = p.z + dz };
}

Point rotate_xy(Point p, float angle) {
    float c = cos(angle);
    float s = sin(angle);
    return (Point) { .x = p.x*c - p.z*s, .y = p.y, .z = p.x*s + p.z*c };
}

void run(Point point_list[255], int point_count, int face_list[255][255], int face_count, int face_sizes[255]) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("trayracing", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0); // create window
    SDL_Surface *surface = SDL_GetWindowSurface(window); // create surface on window to draw on

    SDL_Rect erase_screen = {0 , 0, SCREEN_WIDTH, SCREEN_HEIGHT}; // rect with dimensions of screen

    int running = 1;

    SDL_Event event; // listen for events

    float dt = 1.0/FPS;
    float dz = 1.0;
    float dur = 1000.0/FPS;

    float angle = 0.0;

    while (running) {
        input_loop(event, &running);

        SDL_FillRect(surface, &erase_screen, COLOR_BLACK); // clear screen

        //dz += dt;
        angle += M_PI*dt;

        for (int j = 0; j < face_count; j++) {
            for (int i = 0; i < face_sizes[j]; i++) {
                Point a = point_list[face_list[j][i]];
                Point b = point_list[face_list[j][(i+1)%face_sizes[j]]];

                //printf("%d\n", (i+1)%(face_sizes[j]));

                //printf("%f %f %f\n%f %f %f", a.x, a.y, a.z, b.x, b.y, b.z);
                draw(window, surface,
                        coords(mutate(translate_z(rotate_xy(a, angle), dz))),
                        coords(mutate(translate_z(rotate_xy(b, angle), dz))));

            }
        }
        wait(dur);
    }
}

int main() {
    FILE *points = fopen("points.txt", "r");

    char cur_line[30];
    float x, y, z;
    int point_count = 0;
    Point point_list[255];

    while (fgets(cur_line, 29, points)) {
        sscanf(cur_line, "%f %f %f", &x, &y, &z);
        point_list[point_count] = (Point){x, y, z};
        //printf("%.2f %.2f %.2f\n", point_list[point_count].x, point_list[point_count].y, point_list[point_count].z);
        point_count++;
    }


    FILE *faces = fopen("faces.txt", "r");

    int a, b;
    int face_count = 0;
    int face_list[255][255];
    int face_sizes[255];

    while (fgets(cur_line, 29, faces)) {
        char *p = cur_line;
        char *end;
        int value;
        int col = 0;

        while (1) {
            value = strtol(p, &end, 10);

            if (p == end) {  // no more integers on this line
                break;
            }

            face_list[face_count][col++] = value;
            p = end;        // advance to next token
        }


        face_sizes[face_count] = col;  // how many ints were read on this line

        face_count++;
    }

    //printf("%d\n", face_count);

    run(point_list, point_count, face_list, face_count, face_sizes);

    fclose(points);
    fclose(faces);

    return 0;
}

void wait(long ms) { // <---
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}

void input_loop(SDL_Event event, int *running) {
    // close window
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT: // if the quit action is done
                *running = 0;
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_q: // if q is pressed
                        *running = 0;
                        break;
                    default:
                        break;
                }
            default:
                break;
        }
    }
}
