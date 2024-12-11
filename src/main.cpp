/*
	Authors			:	Daniel Franzen e Leandro Moura
	Description		:	Snake game usando opengl - freeglut-MinGW-3.0.0-1.mp
	Technology		:	C++ , OpenGl - freeglut-MinGW-3.0.0-1.mp
	Platform		:	Windows - Linux

*/
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <math.h>

std::mutex mtx;

using namespace std;

#include <stdint.h>
typedef uint32_t UNIVERSAL_COLOR_REF;
typedef uint8_t UNIVERSAL_BYTE;
typedef uint16_t UNIVERSAL_WORD;
#define UNIVERSAL_GetRValue(c) ((UNIVERSAL_BYTE)(c))
#define UNIVERSAL_GetGValue(c) ((UNIVERSAL_BYTE)(((UNIVERSAL_WORD)(c))>>8))
#define UNIVERSAL_GetBValue(c) ((UNIVERSAL_BYTE)((c)>>16))
#define UNIVERSAL_RGB(r,g,b) ((UNIVERSAL_COLOR_REF)((UNIVERSAL_BYTE)(r)|((UNIVERSAL_BYTE)(g) << 8)|((UNIVERSAL_BYTE)(b) << 16)))

class Vector2 {
    public: int X;
    public: int Y;
    public: Vector2() : X(0), Y(0) {}
    public: Vector2(int x, int y) : X(x), Y(y) {}
    public: Vector2 operator+(const Vector2& v) const {return Vector2(X + v.X, Y + v.Y);}
};

enum GAME_MAP_TYPE{
    NONE,
    WALL,
    SNAKE,
    APPLE,
    SNAKE_HEAD,
};
enum GAME_MAP_STYLE{
    WALLED,
    ANTHILL
};

static void gl_utils_draw_rectangle(Vector2 _position, Vector2 _size, UNIVERSAL_COLOR_REF color){
    glBegin(GL_TRIANGLES);
        glColor3f(UNIVERSAL_GetRValue(color) / 255.0f, UNIVERSAL_GetGValue(color) / 255.0f, UNIVERSAL_GetBValue(color) / 255.0f);

        glVertex2i(_position.X, _position.Y);
        glVertex2i(_position.X + _size.X, _position.Y);
        glVertex2i(_position.X + _size.X, _position.Y + _size.Y);

        glVertex2i(_position.X, _position.Y);
        glVertex2i(_position.X + _size.X, _position.Y + _size.Y);
        glVertex2i(_position.X, _position.Y + _size.Y);
    glEnd();
}

static void gl_utils_draw_text(float x, float y, const char* text, float scale_x, float scale_y, UNIVERSAL_COLOR_REF color, unsigned int line_width = 1, void * font = GLUT_STROKE_ROMAN, bool bold = false) {
    glPushMatrix();
    glTranslatef(x, y, 0.0);
    glScalef(scale_x, -scale_y, 1);
    glLineWidth(line_width);
    if(bold)
        glColor3f(0, 0, 0);
    else
        glColor3f(UNIVERSAL_GetRValue(color) / 255.0f, UNIVERSAL_GetGValue(color) / 255.0f, UNIVERSAL_GetBValue(color) / 255.0f);
    for (const char* c = text; *c != '\0'; ++c)
        glutStrokeCharacter(font, *c); // Desenha cada caractere
    glPopMatrix();
    if(bold)
        gl_utils_draw_text(x - line_width, y - line_width, text, scale_x, scale_y, color, line_width, font, false);
}

static void gl_utils_draw_center_text(float center_x, float center_y, const char* text, float scale_x, float scale_y, int window_height, UNIVERSAL_COLOR_REF color, unsigned int line_width = 1, void * font = GLUT_STROKE_ROMAN, bool bold = false) {
    float textWidth = 0;
    for (const char* c = text; *c != '\0'; ++c)
        textWidth += glutStrokeWidth(font, *c);
    const float adjusted_x = center_x - (textWidth * scale_x) / 2;
    const float adjusted_y = window_height - center_y;

    glPushMatrix();
    glTranslatef(adjusted_x, adjusted_y, 0.0);
    glScalef(scale_x, -scale_y, 1.0);
    glLineWidth(line_width);
    if(bold)
        glColor3f(0, 0, 0);
    else
        glColor3f(UNIVERSAL_GetRValue(color) / 255.0f, UNIVERSAL_GetGValue(color) / 255.0f, UNIVERSAL_GetBValue(color) / 255.0f);

    for (const char* c = text; *c != '\0'; ++c)
        glutStrokeCharacter(font, *c);
    glPopMatrix();

    if(bold)
        gl_utils_draw_center_text(center_x - line_width, center_y + line_width, text, scale_x, scale_y, window_height, color, line_width, font, false);
}


static int get_fps(){
    static float framesPerSecond = 0.0f;
    static int fps;
    static float lastTime = 0.0f;
    const float currentTime = glutGet(GLUT_ELAPSED_TIME) * 0.001f;
    ++framesPerSecond;
    if (currentTime - lastTime > 1.0f)
    {
        lastTime = currentTime;
        fps = (int)framesPerSecond;
        framesPerSecond = 0;
    }
    return fps;
}

class Snake{
    public: Vector2 direction;
    private: vector<Vector2> position;
    public: Snake(const Vector2 position, const Vector2 direction){
        this->position.push_back(position);
        this->direction = direction;
    };
    public: GAME_MAP_TYPE update(GAME_MAP_TYPE ** GAME_MAP, const unsigned int & GRID_COUNT) {
        Vector2 &head = this->position.front();
        const Vector2 &tail = this->position.back();
        GAME_MAP[static_cast<unsigned int>(tail.X)][static_cast<unsigned int>(tail.Y)] = GAME_MAP_TYPE::NONE;

        for (unsigned int i = this->position.size() - 1; i > 0; i--)
            this->position[i] = this->position[i - 1];

        head = head + direction;
        internal_check_teleport_to_another_side(head, GRID_COUNT);

        const GAME_MAP_TYPE collider = GAME_MAP[static_cast<unsigned int>(head.X)][static_cast<unsigned int>(head.Y)];
        const bool collided_with_apple = collider == GAME_MAP_TYPE::APPLE;
        const bool collided_with_nothing = collider == GAME_MAP_TYPE::NONE;
        if (collided_with_nothing || collided_with_apple) {
            GAME_MAP[static_cast<unsigned int>(head.X)][static_cast<unsigned int>(head.Y)] = GAME_MAP_TYPE::SNAKE_HEAD;
            if (collided_with_apple)
                this->position.push_back(this->position.back());
        }
        if(&tail != &head)///se pegou pegou ao menos uma maça, cabeça e calda são diferentes!
            GAME_MAP[static_cast<unsigned int>((&head + 1)->X)][static_cast<unsigned int>((&head + 1)->Y)] = GAME_MAP_TYPE::SNAKE;
        return collider;
    }
    private: void internal_check_teleport_to_another_side(Vector2 &head, const unsigned int& gridCount) {
        head.X = (head.X + gridCount) % gridCount;
        head.Y = (head.Y + gridCount) % gridCount;
    }
    public: unsigned int get_size(){
        return this->position.size();
    }
};

class Game{
    private: GAME_MAP_TYPE ** GAME_MAP;
    private: GAME_MAP_STYLE map_style;
    private: Snake * snake;
    private: float * GAME_DRAG;
    private: unsigned int SCREEN_SIZE, GRID_SCALE, GRID_COUNT, MAX_POINTS;
    private: bool GAME_OVER, GAME_WIN, GAME_PAUSE;
    private: bool allow_key_event;

    private: static constexpr UNIVERSAL_COLOR_REF wall_color = UNIVERSAL_RGB(244, 244, 244);
    private: static constexpr UNIVERSAL_COLOR_REF snake_color = UNIVERSAL_RGB(0, 225, 0);
    private: static constexpr UNIVERSAL_COLOR_REF snake_head_color = UNIVERSAL_RGB(181, 255, 128);
    private: static constexpr UNIVERSAL_COLOR_REF apple_color = UNIVERSAL_RGB(255, 0, 0);
    private: static constexpr UNIVERSAL_COLOR_REF none_color = UNIVERSAL_RGB(66, 93, 66);

    public: Game(const unsigned int SCREEN_SIZE, const unsigned int GRID_SCALE, const GAME_MAP_STYLE map_style, float * GAME_DRAG){
        srand (time(NULL));
        internal_start_game(SCREEN_SIZE, GRID_SCALE, map_style, GAME_DRAG);
    };

    private: void internal_start_game(
        const unsigned int SCREEN_SIZE, const unsigned int GRID_SCALE, const GAME_MAP_STYLE map_style, float * GAME_DRAG
    ){
        this->GAME_DRAG = GAME_DRAG;
        this->GAME_MAP = NULL;
        internal_destructor();
        this->allow_key_event = true;
        this->GAME_WIN = this->GAME_OVER  = false;
        this->GAME_PAUSE = true;
        this->SCREEN_SIZE = SCREEN_SIZE;
        this->GRID_SCALE = GRID_SCALE;
        this->GRID_COUNT = SCREEN_SIZE/GRID_SCALE;
        GAME_MAP = (GAME_MAP_TYPE **) calloc(this->GRID_COUNT, sizeof(GAME_MAP_TYPE *));
        for(unsigned int i =0; i < this->GRID_COUNT; i++)
            GAME_MAP[i] = (GAME_MAP_TYPE *) calloc(this->GRID_COUNT, sizeof(GAME_MAP_TYPE) );
        this->map_style = map_style;

        this->internal_generate_game_map(map_style);
        this->internal_generate_snake();
        this->internal_generate_apple();

        this->MAX_POINTS = this->internal_calc_max_points();
    }
    private: void internal_generate_game_map(GAME_MAP_STYLE map_style){
        switch(map_style){
            case GAME_MAP_STYLE::WALLED:{
                for(unsigned int i =0; i < this->GRID_COUNT; i++){
                    GAME_MAP[0][i] = GAME_MAP_TYPE::WALL;
                    GAME_MAP[this->GRID_COUNT - 1][i] = GAME_MAP_TYPE::WALL;
                    GAME_MAP[i][0] = GAME_MAP_TYPE::WALL;
                    GAME_MAP[i][this->GRID_COUNT - 1] = GAME_MAP_TYPE::WALL;
                }
                break;
            }
            case GAME_MAP_STYLE::ANTHILL: {
                constexpr unsigned int border_distance = 2;
                constexpr unsigned int minimum_number_of_walls = 6;
                unsigned int maximum_number_of_walls = minimum_number_of_walls * (1 + rand() % 2);
                vector<Vector2> wall_points;
                while(maximum_number_of_walls > 0){
                    constexpr unsigned int r_min_mod = border_distance + 1;
                    const unsigned int r_max_mod = this->GRID_COUNT - 2*(border_distance + 1);
                    const unsigned int r_px = r_min_mod + rand() % r_max_mod;
                    const unsigned int r_py = r_min_mod + rand() % r_max_mod;
                    if(GAME_MAP[r_px][r_py] != GAME_MAP_TYPE::NONE)
                        continue;
                    bool allow_point = true;
                    for(unsigned int i = 0; i < wall_points.size(); i++){
                        const unsigned int distance = sqrt(pow(wall_points[i].X - r_px, 2) + pow(wall_points[i].Y - r_py, 2));
                        if(distance <= border_distance){
                            allow_point = false;
                            break;
                        }
                    }
                    if(!allow_point)
                        continue;

                    GAME_MAP[r_px][r_py] = GAME_MAP_TYPE::WALL;
                    wall_points.push_back(Vector2(r_px, r_py));
                    maximum_number_of_walls--;
                }
                break;
            }
        }

    }

    private: void internal_generate_snake(){
        constexpr unsigned int border_distance = 3;
        ///gerando posição aleatória
        constexpr unsigned int r_min_mod = border_distance + 1;
        const unsigned int r_max_mod = this->GRID_COUNT - 2*(border_distance + 1);
        const unsigned int r_px = r_min_mod + rand() % r_max_mod;
        const unsigned int r_py = r_min_mod + rand() % r_max_mod;
        GAME_MAP[r_px][r_py] = GAME_MAP_TYPE::SNAKE_HEAD;

        ///gerando direção aleatória
        constexpr int dir_arr[2] = {-1, 1};
        int r_dir_arr[2] = {0, 0};
        r_dir_arr[rand() %2] = dir_arr[rand() %2];

        this->snake = new Snake(Vector2(r_px, r_py), Vector2(r_dir_arr[0], r_dir_arr[1]));
    }

    private: void internal_generate_apple(){
        if(this->snake->get_size() == (pow(this->GRID_COUNT - 2, 2)) - 1){
            this->GAME_WIN = true;
            return;
        }

        constexpr unsigned int border_distance = 0;
        while(true){///fazer para parar quando o tamanho da cobra é igual a quantidade de casas para ocupar
            constexpr unsigned int r_min_mod = border_distance + 1;
            const unsigned int r_max_mod = this->GRID_COUNT - 2*(border_distance + 1);
            const unsigned int r_px = r_min_mod + rand() % r_max_mod;
            const unsigned int r_py = r_min_mod + rand() % r_max_mod;
            if(GAME_MAP[r_px][r_py] == GAME_MAP_TYPE::NONE){
                GAME_MAP[r_px][r_py] = GAME_MAP_TYPE::APPLE;
                break;
            }
        }
    }

    private: unsigned int internal_calc_max_points(){
        unsigned int max_points = 0;
        for(unsigned int i =0; i< this->GRID_COUNT; i++)
            for(unsigned int j=0; j< this->GRID_COUNT; j++)
                if(this->GAME_MAP[j][i] == GAME_MAP_TYPE::NONE)
                    max_points++;
        return max_points;
    }

    private: void debug_show_game_map(){
        for(unsigned int i =0; i< this->GRID_COUNT; i++){
            for(unsigned int j=0; j< this->GRID_COUNT; j++)
                cout << ((int) this->GAME_MAP[j][i]);
            cout << endl;
        }
    }

    public: void draw(){
        for(unsigned int i =0; i< this->GRID_COUNT; i++){
            for(unsigned int j=0; j< this->GRID_COUNT; j++){
                UNIVERSAL_COLOR_REF draw_color;
                switch(GAME_MAP[i][j]){
                    case GAME_MAP_TYPE::WALL:{
                        draw_color = Game::wall_color;
                        break;
                    }
                    case GAME_MAP_TYPE::SNAKE:{
                        draw_color = Game::snake_color;
                        break;
                    }
                    case GAME_MAP_TYPE::SNAKE_HEAD:{
                        draw_color = Game::snake_head_color;
                        break;
                    }
                    case GAME_MAP_TYPE::APPLE:{
                        draw_color = Game::apple_color;
                        break;
                    }
                    default:{
                        draw_color = Game::none_color;
                        break;
                    }
                }
                const unsigned int grid_diff = static_cast<unsigned int>(draw_color != Game::wall_color || this->map_style != GAME_MAP_STYLE::WALLED);
                gl_utils_draw_rectangle(Vector2(this->GRID_SCALE * i, this->GRID_SCALE *j), Vector2(this->GRID_SCALE - grid_diff, this->GRID_SCALE - grid_diff), draw_color);
            }
        }

        gl_utils_draw_text(this->GRID_SCALE, this->GRID_SCALE - 2, string("Pontos: " + std::to_string(this->snake->get_size() - 1)).c_str(), 0.15, 0.15, Game::wall_color, 2, GLUT_STROKE_ROMAN, true);
        if(this->GAME_OVER){
            gl_utils_draw_center_text(this->SCREEN_SIZE/2, this->SCREEN_SIZE/2, "Game Over", 0.5, 0.5, this->SCREEN_SIZE, Game::apple_color, 3, GLUT_STROKE_ROMAN, true);
            gl_utils_draw_center_text(this->SCREEN_SIZE/2, this->SCREEN_SIZE/2 - 50, "Press 'R' to reload", 0.25, 0.25, this->SCREEN_SIZE, Game::apple_color, 2, GLUT_STROKE_ROMAN, true);
        }else if(this->GAME_WIN){
            gl_utils_draw_center_text(this->SCREEN_SIZE/2, this->SCREEN_SIZE/2, "Game Win", 0.5, 0.5, this->SCREEN_SIZE, Game::wall_color, 3, GLUT_STROKE_ROMAN, true);
            gl_utils_draw_center_text(this->SCREEN_SIZE/2, this->SCREEN_SIZE/2 - 50, "Press 'R' to reload", 0.25, 0.25, this->SCREEN_SIZE, Game::wall_color, 2, GLUT_STROKE_ROMAN, true);
        }else if(this->GAME_PAUSE){
            gl_utils_draw_center_text(this->SCREEN_SIZE/2, this->SCREEN_SIZE/2 + 40, "Game Pause", 0.2, 0.2, this->SCREEN_SIZE, Game::snake_color, 3, GLUT_STROKE_ROMAN, true);
            gl_utils_draw_center_text(this->SCREEN_SIZE/2, this->SCREEN_SIZE/2 + 40- 30, "Press key", 0.14, 0.14, this->SCREEN_SIZE, Game::snake_color, 2, GLUT_STROKE_ROMAN, true);
            gl_utils_draw_center_text(this->SCREEN_SIZE/2, this->SCREEN_SIZE/2 + 40- 30 - 30, "to continue", 0.14, 0.14, this->SCREEN_SIZE, Game::snake_color, 2, GLUT_STROKE_ROMAN, true);
        }

        gl_utils_draw_text(this->GRID_SCALE + 150, this->GRID_SCALE - 2, string("FPS: " + std::to_string(get_fps())).c_str(), 0.15, 0.15, Game::wall_color, 2, GLUT_STROKE_ROMAN, true);
        gl_utils_draw_text(this->GRID_SCALE, this->SCREEN_SIZE  - 5, "Jogue com: W, S, D, A", 0.075, 0.075, Game::wall_color, 1, GLUT_STROKE_MONO_ROMAN, true);
        gl_utils_draw_text(this->GRID_SCALE + 210, this->SCREEN_SIZE  - 5, "Pause com: P", 0.075, 0.075, Game::wall_color, 1, GLUT_STROKE_MONO_ROMAN, true);
        gl_utils_draw_text(this->GRID_SCALE + 210 + 130, this->SCREEN_SIZE  - 5, "Reload com: R", 0.078, 0.078, Game::wall_color, 1, GLUT_STROKE_MONO_ROMAN, true);

    }
    public: void update(){
        this->allow_key_event = true;
        if(this->GAME_OVER || this->GAME_WIN || this->GAME_PAUSE)
            return;
        GAME_MAP_TYPE collider = this->snake->update(this->GAME_MAP, this->GRID_COUNT);
        if(collider == GAME_MAP_TYPE::NONE || collider == GAME_MAP_TYPE::APPLE){
            if(collider == GAME_MAP_TYPE::APPLE){
                this->internal_generate_apple();
            }
        }else
            this->GAME_OVER = true;
        //debug_show_game_map();
    }
    public: void key_controls(unsigned char key){
        if(!this->allow_key_event)
            return;
        this->GAME_PAUSE = false;
        switch (key)
        {
            case 'd':
                if(this->snake->direction.X == 0 || this->snake->get_size() == 1){
                    this->snake->direction.X = 1;
                    this->snake->direction.Y = 0;
                }
                break;
            case 'a':
                if(this->snake->direction.X == 0 || this->snake->get_size() == 1){
                    this->snake->direction.X = -1;
                    this->snake->direction.Y = 0;
                }
                break;
            case 'w':
                if(this->snake->direction.Y == 0 || this->snake->get_size() == 1){
                    this->snake->direction.X = 0;
                    this->snake->direction.Y = -1;
                }
                break;
            case 's':
                if(this->snake->direction.Y == 0 || this->snake->get_size() == 1){
                    this->snake->direction.X = 0;
                    this->snake->direction.Y = 1;
                }
                break;
            case 'p':
                this->GAME_PAUSE = !this->GAME_PAUSE;
                break;
            case 'z':
                *this->GAME_DRAG -= 0.1f;
                if(*this->GAME_DRAG < 0.1f)
                    *this->GAME_DRAG = 0.1f;
                break;
            case 'x':
                *this->GAME_DRAG += 0.1f;
                if(*this->GAME_DRAG > 3.5f)
                    *this->GAME_DRAG = 3.5f;
                break;
            case 'r':
                internal_start_game(this->SCREEN_SIZE, this->GRID_SCALE, this->map_style, this->GAME_DRAG);
                break;
            case 'q':
                exit(0);
                break;
            case 27:
                exit(0);
                break;
        }
        this->allow_key_event = false;
    }
    private: void internal_destructor(){
        if(this->GAME_MAP == NULL)
            return;
        for(unsigned int i =0; i < this->GRID_COUNT; i++)
            free(GAME_MAP[i]);
        free(GAME_MAP);
        GAME_MAP = NULL;
        delete this->snake;
        this->snake = NULL;
    }

    public: ~Game(){
        internal_destructor();
    }
};



constexpr unsigned int SCREEN_SIZE = 500;
constexpr unsigned int FIXED_FPS = 60;
float GAME_DRAG = 2.0f;///atrito / arrasto

Game * snake_game = new Game(SCREEN_SIZE, 20, GAME_MAP_STYLE::ANTHILL, &GAME_DRAG);
#ifndef __linux__
    ///para windows
    extern "C"{
        __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001; ///habilitar placa nvidea
        __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;///habilitar placa AMD
    }
#endif // WIN32

static void resize(int w, int h){
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, w, 0, h, -1, 1);
    glScalef(1, -1, 1);
    glTranslatef(0, -h, 0);
    glutReshapeWindow(SCREEN_SIZE, SCREEN_SIZE);
}

bool allow_buffer_to_draw = false;
static void fixed_update(int time){
    snake_game->update();
    mtx.lock();
        allow_buffer_to_draw = true;
    mtx.unlock();
    glutTimerFunc(FIXED_FPS * GAME_DRAG, fixed_update, 0);
}

static void update(void){
    mtx.lock();
        const bool _allow_buffer_to_draw = allow_buffer_to_draw;
    mtx.unlock();
    if(!_allow_buffer_to_draw) return;

    constexpr int t_sleep = 1000/ FIXED_FPS;
    std::chrono::time_point<std::chrono::high_resolution_clock> start = std::chrono::high_resolution_clock::now();

    glClear(GL_COLOR_BUFFER_BIT);
    snake_game->draw();

    //glFlush();
    glutSwapBuffers();

    std::chrono::time_point<std::chrono::high_resolution_clock> end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> duration = end - start;
    const float delta_time = duration.count() * 1000;
    const int sleep_time = std::max(0, t_sleep - static_cast<int>(delta_time));
    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
    mtx.lock();
        allow_buffer_to_draw = false;
    mtx.unlock();
}

static void key(unsigned char key, int x, int y){
    snake_game->key_controls(key);
    glutPostRedisplay();
}

static void idle(void){
    glutPostRedisplay();
}

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    glutInitWindowSize(SCREEN_SIZE,SCREEN_SIZE);
    glutInitWindowPosition((glutGet(GLUT_SCREEN_WIDTH)-SCREEN_SIZE)/2,(glutGet(GLUT_SCREEN_HEIGHT)-SCREEN_SIZE)/2);
    glutCreateWindow("Snake Game");
    glutInitDisplayMode(GLUT_DOUBLE| GLUT_RGB);
    //glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutReshapeFunc(resize);
    glutDisplayFunc(update);
    glutKeyboardFunc(key);
    glutIdleFunc(idle);
    glutTimerFunc(FIXED_FPS, fixed_update, 0);
    glClearColor(66/255.0f,99/255.0f,66/255.0f,1);
    glutMainLoop();

    return EXIT_SUCCESS;
}
