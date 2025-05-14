#include <GL/glut.h>
#include <ctime>
#include <vector>
#include <string>

const int WIDTH = 10;
const int HEIGHT = 20;
const int BLOCK_SIZE = 30;
int field[HEIGHT][WIDTH] = { 0 };

struct Point { int x, y; };
Point current[4], backup[4];

int figures[7][4] = {
    {1, 3, 5, 7}, {2, 4, 5, 7}, {3, 5, 4, 6}, {3, 5, 4, 7},
    {2, 3, 5, 7}, {3, 5, 7, 6}, {2, 3, 4, 5}
};

int dx = 0, score = 0;
bool rotate = false, gameOver = false;
int color = 1;
float timer = 0, delay = 0.3;

void drawText(float x, float y, std::string text) {
    glRasterPos2f(x, y);
    for (char c : text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }
}

void drawBlock(int x, int y, int color) {
    float colors[8][3] = {
        {0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1},
        {1, 1, 0}, {1, 0, 1}, {0, 1, 1}, {1, 0.5, 0}
    };
    glColor3f(colors[color][0], colors[color][1], colors[color][2]);
    glBegin(GL_QUADS);
    glVertex2i(x * BLOCK_SIZE, y * BLOCK_SIZE);
    glVertex2i((x + 1) * BLOCK_SIZE, y * BLOCK_SIZE);
    glVertex2i((x + 1) * BLOCK_SIZE, (y + 1) * BLOCK_SIZE);
    glVertex2i(x * BLOCK_SIZE, (y + 1) * BLOCK_SIZE);
    glEnd();
    glColor3f(0.1f, 0.1f, 0.1f);
    glBegin(GL_LINE_LOOP);
    glVertex2i(x * BLOCK_SIZE, y * BLOCK_SIZE);
    glVertex2i((x + 1) * BLOCK_SIZE, y * BLOCK_SIZE);
    glVertex2i((x + 1) * BLOCK_SIZE, (y + 1) * BLOCK_SIZE);
    glVertex2i(x * BLOCK_SIZE, (y + 1) * BLOCK_SIZE);
    glEnd();
}

bool check() {
    for (int i = 0; i < 4; ++i) {
        int x = current[i].x, y = current[i].y;
        if (x < 0 || x >= WIDTH || y >= HEIGHT) return false;
        if (y >= 0 && field[y][x]) return false;
    }
    return true;
}

void spawnPiece() {
    int n = rand() % 7;
    color = 1 + rand() % 7;
    for (int i = 0; i < 4; ++i) {
        current[i].x = figures[n][i] % 2 + WIDTH / 2 - 1;
        current[i].y = figures[n][i] / 2 - 1;
        if (field[current[i].y][current[i].x]) {
            gameOver = true;
        }
    }
}

void tick() {
    if (gameOver) return;

    for (int i = 0; i < 4; ++i) backup[i] = current[i];
    for (int i = 0; i < 4; ++i) current[i].x += dx;
    if (!check()) for (int i = 0; i < 4; ++i) current[i] = backup[i];

    if (rotate) {
        Point p = current[1];
        for (int i = 0; i < 4; ++i) {
            int x = current[i].y - p.y;
            int y = current[i].x - p.x;
            current[i].x = p.x - x;
            current[i].y = p.y + y;
        }
        if (!check()) for (int i = 0; i < 4; ++i) current[i] = backup[i];
    }

    // Move down
    for (int i = 0; i < 4; ++i) backup[i] = current[i];
    for (int i = 0; i < 4; ++i) current[i].y += 1;

    if (!check()) {
        for (int i = 0; i < 4; ++i)
            if (backup[i].y >= 0)
                field[backup[i].y][backup[i].x] = color;
        spawnPiece();
    }

    // ✅ Reset dx and rotate after applying
    dx = 0;
    rotate = false;
}

void clearLines() {
    for (int i = HEIGHT - 1; i >= 0; --i) {
        bool full = true;
        for (int j = 0; j < WIDTH; ++j)
            if (field[i][j] == 0) full = false;
        if (full) {
            for (int row = i; row > 0; --row)
                for (int col = 0; col < WIDTH; ++col)
                    field[row][col] = field[row - 1][col];
            for (int j = 0; j < WIDTH; ++j)
                field[0][j] = 0;
            score += 100;
            ++i;
        }
    }
}


void drawFog() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Semi-transparent white/gray fog
    glColor4f(0.5f, 0.5f, 0.5f, 0.2f);  // Gray fog with some transparency
    glBegin(GL_QUADS);
        glVertex2i(0, 0);
        glVertex2i(WIDTH * BLOCK_SIZE, 0);
        glVertex2i(WIDTH * BLOCK_SIZE, HEIGHT * BLOCK_SIZE);
        glVertex2i(0, HEIGHT * BLOCK_SIZE);
    glEnd();

    glDisable(GL_BLEND); // Disable blending after drawing the fog
}

void drawGlow() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Soft red glow at the top (like a blood-red moon glow)
    glColor4f(0.8f, 0.1f, 0.1f, 0.3f); // Red glow with transparency
    glBegin(GL_QUADS);
        glVertex2i(0, 0);
        glVertex2i(WIDTH * BLOCK_SIZE, 0);
        glVertex2i(WIDTH * BLOCK_SIZE, HEIGHT * BLOCK_SIZE / 2);
        glVertex2i(0, HEIGHT * BLOCK_SIZE / 2);
    glEnd();

    glDisable(GL_BLEND);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw background effects (fog, glow)
    drawFog();
    drawGlow();

    // Draw the game field and current pieces
    for (int i = 0; i < HEIGHT; ++i)
        for (int j = 0; j < WIDTH; ++j)
            if (field[i][j])
                drawBlock(j, i, field[i][j]);

    for (int i = 0; i < 4; ++i)
        if (current[i].y >= 0)
            drawBlock(current[i].x, current[i].y, color);

    glColor3f(1, 1, 1);  // White text color
    drawText(10, 30, "Score: " + std::to_string(score));

    if (gameOver)
        drawText(60, HEIGHT * BLOCK_SIZE / 2, "GAME OVER");

    glutSwapBuffers();
}

void timerFunc(int) {
    glutPostRedisplay();
    if (gameOver) return;

    timer += 0.05;
    if (timer > delay) {
        tick();
        clearLines();
        timer = 0;
    }
    glutTimerFunc(50, timerFunc, 0);
}

void keyboard(int key, int, int) {
    dx = 0; rotate = false;
    if (gameOver) return;
    if (key == GLUT_KEY_LEFT) dx = -1;
    else if (key == GLUT_KEY_RIGHT) dx = 1;
    else if (key == GLUT_KEY_UP) rotate = true;
    else if (key == GLUT_KEY_DOWN) delay = 0.05;
}

void keyboardUp(int key, int, int) {
    if (key == GLUT_KEY_DOWN) delay = 0.3;
}

void init() {
    // Set background color to deep purple, giving a dark and eerie vibe
    glClearColor(0.2f, 0.1f, 0.3f, 1.0f); // Dark purple background color
    glEnable(GL_TEXTURE_2D);  // Enable textures if you want to use one
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WIDTH * BLOCK_SIZE, HEIGHT * BLOCK_SIZE, 0);
    srand(time(0));
    spawnPiece();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WIDTH * BLOCK_SIZE, HEIGHT * BLOCK_SIZE);
    glutCreateWindow("Tetris - OpenGL");

    init();
    glutDisplayFunc(display);
    glutSpecialFunc(keyboard);
    glutSpecialUpFunc(keyboardUp);
    glutTimerFunc(50, timerFunc, 0);
    glutMainLoop();
    return 0;
}