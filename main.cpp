/*

Jan Subocz s203670

Most of the structure and systems in the game are of my own design, however:

 - inspiration to use typedef struct came from the project1demo game
 - I watched a first few episode of a series "Making snake in ncurses" by Casual Coder
   in order to have a basic understanding of ncurses
 - random number generation code was taken from the Microsoft Learn platform, albeit slightly modified
 - others posts on Reddit and Stack Overflow helped me resolved some bugs

*/
// INCLUSIONS ==========================================================================================================

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <ncurses.h>

using namespace std;

// DEFINITIONS =========================================================================================================

#define H 24
#define W 31

#define NLNS (H-5) // number of traffic lines
#define TBM 3 // top and bottom margin

#define FPS 10

#define MSL 25 // max symbol length

#define CL 4 // car length

#define L (-1)
#define R 1

#define ID "Jan Subocz s203670"

#define GRN 7
#define RED 1
#define BLU 2
#define WHT 3
#define BLC 4
#define YLW 6
#define FOFF 8
#define STRK 9
#define CLP COLOR_PAIR

#define UID uniform_int_distribution<>

#define PT mvwprintw
#define RF wrefresh
#define IP init_pair
#define CON wattron
#define COFF wattroff

// STRUCTURES ==========================================================================================================

typedef struct {
    int y;
    int x;
    char * form;
    bool alive;
    int cldwn; // cooldown
    int count;
} A;

typedef struct {
    int spd[NLNS]; // speeds
    int dirs[NLNS]; // directions
    bool rtns[NLNS]; // bouncy or not
    int x[NLNS]; // x position
    int clr[NLNS]; // colour
    int tp[NLNS]; // types
    bool B[NLNS]; // boost
    bool stop[NLNS]; // frog proximity stop
    int tr[NLNS][W - 4]; // traps
    int ntp[NLNS]; // new types flash counters
    bool isl[NLNS]; // true if lane is a safe isle
    int mode = 1;
} LNS;

typedef struct {
    int dx = 0;
    int dy = 0;
    char curpress;
} IN;

typedef struct {
    int sec;
    float mil;
    float prev;
} TMR;

typedef struct {
    char frog[2];
    char trap[2];
    char stork[2];
    char fc[CL + 1]; // fast car
    char sc[CL + 1]; // slow car
    char log[CL + 1];
} SYM;

// FUNCTIONS AND METHODS ===============================================================================================

void makeF(A * frog, SYM * syms) {
    frog->alive = true;
    frog->y = H - 2;
    frog->x = W / 2;
    frog->form = (char *) malloc(sizeof(syms->frog));
    frog->form = syms->frog;
    frog->cldwn = -1; // (when cooldown < 0, the frog can move)
    frog->count = 0;
}

void upF(A * frog, IN * in, bool * gover) {
    frog->cldwn -= 1;
    if (frog->cldwn < 0) {
        if (in->dx != 0 or in->dy != 0) {frog->cldwn = 1; frog->count += 1;
}
        if (in->dx == -1) {
            if (frog->x - 1 > 1) frog->x -= 1;
            in->dx = 0;

}
        if (in->dx == 1) {
            if (frog->x + 2 < W - 1) frog->x += 1;
            in->dx = 0;

}
        if (in->dy == -1) {
            if (frog->y - 1 > 0) frog->y -= 1;
            in->dy = 0;

}
        if (in->dy == 1) {
            if (frog->y + 1 < H) frog->y += 1;
            in->dy = 0;

}
        // finish
        if (frog->y < 3) {
            *gover = true;

}

}
}

void drawF(A * frog, WINDOW * gw) {
    int col = GRN;
    if (frog->cldwn >= 0) col = FOFF;
    CON(gw, CLP(col)); // colour on
    PT(gw, frog->y, frog->x, frog->form);
    COFF(gw, CLP(col)); // colour off
}

// ---------------------------------------------------------------------------------------------------------------------

void makeS(A * stork, SYM * syms) {
    stork->x = W - 3;
    stork->y = H - 2;
    stork->form = (char *) malloc(sizeof(syms->stork));
    stork->form = syms->stork;
    stork->cldwn = 5; // (when cooldown == 0, move stork)
}

void upS(A * stork, A * frog, bool * gover) {
    if (stork->x == frog->x and stork->y == frog->y) {
        frog->alive = false;
        *gover = true;

}
    else if (stork->cldwn <= 0) {
        stork->cldwn = 5;
        int dx = frog->x - stork->x;
        int dy = frog->y - stork->y;
        if (abs(dx) > abs(dy) and dx != 0) { // changed here
            stork-> x += dx / abs(dx);

}
        if (abs(dy) > abs(dx) and dy != 0) { // and here
            stork->y += dy / abs(dy);

}
        if (abs(dx) == abs(dy) and dx != 0 and dy != 0) { // here as well
            stork->x += dx / abs(dx);
            stork->y += dy / abs(dy);

}

}
    stork->cldwn -= 1;
}

void drawS(A * stork, WINDOW * gw) {
    CON(gw, CLP(STRK));
    PT(gw, stork->y, stork->x, stork->form);
    COFF(gw, CLP(STRK));
}

void drawSP(A * stork, WINDOW * gw) {
    CON(gw, CLP(YLW));
    char prop1[] = "x";
    char prop2[] = "+";
    if (stork->cldwn % 2 == 0) {prop1[0] = '+'; prop2[0] = 'x';
}
    PT(gw, stork->y - 1, stork->x - 1, prop1);
    PT(gw, stork->y + 1, stork->x + 1, prop1);
    PT(gw, stork->y - 1, stork->x + 1, prop2);
    PT(gw, stork->y + 1, stork->x - 1, prop2);
    COFF(gw, CLP(YLW));
}

// ---------------------------------------------------------------------------------------------------------------------


void getIn(IN * in, WINDOW * gw) {
    int c = wgetch(gw);
    if (c != ERR) {
        if (c == 'w') in->dy = -1;
        if (c == 's') in->dy = 1;
        if (c == 'a') in->dx = -1;
        if (c == 'd') in->dx = 1;

}
    in->curpress = c;
}

void Timer(WINDOW * sw, TMR * timer, bool * falive) {
    float d = clock() - timer->prev;
    timer->prev = clock();
    timer->mil -= d;
    if (timer->mil < 0) {
        timer->mil = 10000;
        timer->sec -= 1;

}
    char *ctm = (char *) malloc(sizeof("00"));
    int clr = WHT; // colour (clr)
    if (timer->sec < 10) clr = RED;
    if (timer->sec == 0) *falive = false;
    ctm[0] = char(int(timer->sec / 10)) + '0';
    ctm[1] = char(timer->sec % 10) + '0';
    ctm[2] = '/0';
    werase(sw);
    PT(sw, 1, 1, ID);
    CON(sw, CLP(clr));
    PT(sw, 1, W - 3, ctm);
    COFF(sw, CLP(clr));
    box(sw, 0, 0);
    wrefresh(sw);
    free(ctm);
}

void Zz(float t) {
    usleep(100000 * t);
}

void calPts(int * pts, int moves, int sec) {
    if (moves > 0) { // changed this
        int mlt = int((NLNS + 1) * 10 / moves);
        *pts = mlt * sec;
    }
}

void ixcPts(int pts, char * cpts) {
    int d1 = int(pts / 100);
    int d2 = int((pts - d1 * 100) / 10);
    int d3 = int(pts - d1 * 100 - d2 * 10);
    cpts[0] = char(d1) + '0';
    cpts[1] = char(d2) + '0';
    cpts[2] = char(d3) + '0';
}

// ---------------------------------------------------------------------------------------------------------------------

void urlStr(char url[], char * out, int l) {
    FILE *f = fopen(url, "r");
    if (f) fgets(out, l, f);
    else exit(1);
    fclose(f);
}

void FsymRead(SYM * syms) {
    char raw[MSL];
    urlStr("../SpriteSymbols.txt", raw, MSL);
    int cnum = 0;
    int idx = 0;
    for (int i = 0; i < MSL; i++) {
        char csym = raw[i];
        if (csym != ' ') {
            switch(cnum) {
                case 0: syms->frog[0] = csym; break;
                case 1: syms->stork[0] = csym; break;
                case 2: syms->trap[0] = csym; break; // they are 1 char long anyway
                case 3: syms->fc[idx] = csym; break;
                case 4: syms->sc[idx] = csym; break;
                case 5: syms->log[idx] = csym;

}

} else {
            cnum++;
            idx = -1;

}
        idx++;

}
}

void FspdRead(int * spd) {
    char raw[4];
    urlStr("../Speeds.txt", raw, 4);
    spd[0] = raw[0] - '0';
    spd[1] = raw[1] - '0';
    spd[2] = raw[2] - '0';
}

// ---------------------------------------------------------------------------------------------------------------------

void makeLns (LNS * lns) {
    int spd[3];
    FspdRead(spd);

    bool isle = false;

    int range1 = 0;
    int range2x2 = 3;
    int range2x16 = 17;
    int range2x48 = 49;

    for (int cln = 0; cln < NLNS; cln++) {
        if (isle and lns->mode == 1) {
            lns->isl[cln] = true;

}
        else {
            int rd = (double)rand() / RAND_MAX * (range2x2 - range1) + range1;
            if (rd > 2) rd = 2; // not sure whether "inclusive" means both sides
            switch(rd) {
                case 0: lns->rtns[cln] = true; lns->dirs[cln] = L; break;
                case 1: lns->rtns[cln] = false; lns->dirs[cln] = R; break;
                case 2: lns->rtns[cln] = false; lns->dirs[cln] = L; break;

}
            rd = (double)rand() / RAND_MAX * (range2x2 - range1) + range1;
            if (rd > 2) rd = 2;
            lns->x[cln] = ((double)rand() / RAND_MAX) * (W - CL - 1) + 1;
            lns->isl[cln] = false; // not a safe isle
            lns->spd[cln] = spd[rd];
            lns->tp[cln] = rd; // tp = type
            lns->ntp[cln] = 0; // ntp = new type counter
            if (rd != 2) lns->clr[cln] = RED; // clr = colour
            else lns->clr[cln] = WHT;
            rd = (double)rand() / RAND_MAX * (range2x16 - range1) + range1;
            if (rd == 0) lns->B[cln] = true; // B = boost
            for (int i = 0; i < W - 4; i++) {
                lns->tr[cln][i] = 0;
                rd = (double)rand() / RAND_MAX * (range2x48 - range1) + range1;
                if (rd == 0 and lns->mode == 3) lns->tr[cln][i] = i + 2;

}

}
        if (lns->mode == 1)
            isle = !isle;

}
}

void drawLns (LNS * lns, WINDOW * gw, SYM * syms) {
    for (int cln = 0; cln < NLNS; cln++) {
        int y = H - TBM - cln;
        if (lns->isl[cln]) {
            for (int i = 1; i < W - 1; i++) {
                PT(gw, y, i, "/");

}

}
        else {
            // traps
            CON(gw, CLP(YLW));
            for (int i = 0; i < W - 4; i++)
                PT(gw, y, lns->tr[cln][i], syms->trap);
            COFF(gw, CLP(YLW));
            // boost
            if (lns->B[cln]) {
                for (int i = 10; i < W - 9; i += 2) {
                    char *ar;
                    ar = ">";
                    if (lns->dirs[cln] == L) ar = "<";
                    PT(gw, y, i, ar);

}

}
            // setting appropriate colour
            int col = lns->clr[cln];
            // if the car has changed type it will flash blue (col = BLU every second frame)
            if (lns->ntp[cln] > 0) {
                if (lns->ntp[cln] % 2 == 1) col = BLU;
                lns->ntp[cln] -= 1;

}
            // drawing cars
            CON(gw, CLP(col));
            if (lns->tp[cln] == 0) {
                PT(gw, y, lns->x[cln], syms->sc);

}
            else if (lns->tp[cln] == 1) {
                PT(gw, y, lns->x[cln], syms->fc);

}
            else if (lns->tp[cln] == 2) {
                PT(gw, y, lns->x[cln], syms->log);

}
            COFF(gw, CLP(col));

}

}
}

// submethod - log collisions - stopping
void logStop(int cln, LNS * lns, A * F, int Y) {
    if (lns->tp[cln] == 2) {
        lns->stop[cln] = false;
        if (F->y >= Y - 1 and F->y <= Y + 1) {
            if (F->x >= lns->x[cln] - 1 and F->x <= lns->x[cln] + CL) {
                lns->stop[cln] = true;

}

}

}
}

// submethod - deadly car collisions and frog carry
void carCol(int cln, LNS * lns, A * F, int Y) {
    if (F->y == Y) {
        for (int i = 0; i < W - 4; i++)
            if (F->x == lns->tr[cln][i]) F->alive = false; // trap kills here
        if (F->x >= lns->x[cln] and F->x <= lns->x[cln] + CL - 1) {
            if (lns->stop[cln]) {
                lns->stop[cln] = false;
                if (lns->x[cln] >= 1 and lns->x[cln] + 1 <= W - 4) {
                    F->x = lns->x[cln] + 1 + int(0.5 + 0.5 * lns->dirs[cln]);
                    F->alive = true;

}

} else
                F->alive = false;

}

}
}

void upLns (LNS * lns, A * F) { // F is frog in this method
    int range1 = 0;
    int range2x2 = 3;
    int range2x48 = 49;

    for (int cln = 0; cln < NLNS; cln++) {
        if (!lns->isl[cln]) {
            // moving the cars
            if (!lns->stop[cln]) {
                int spd = 1;
                if (lns->B[cln]) {
                    if (lns->x[cln] > 10 - CL and lns->x[cln] < W - 10) {
                        spd = 2;

                    }

                }
                lns->x[cln] += lns->spd[cln] * spd * lns->dirs[cln];

            }
            // bouncing (rtn) cars bounce here
            if (lns->rtns[cln]) {
                if (lns->x[cln] <= 2) lns->dirs[cln] = 1;
                if (lns->x[cln] >= W - 2 - CL) lns->dirs[cln] = -1;
                // regular car wrapping

                } else {
                int rd1 = (double)rand() / RAND_MAX * (range2x48 - range1) + range1;
                int rd2 = (double)rand() / RAND_MAX * (range2x2 - range1) + range1;
                bool out = false;
                if (lns->x[cln] <= -CL + 2) {
                    out = true;
                    lns->x[cln] = W - 2;

}                else if (lns->x[cln] >= W - 2) {
                    out = true;
                    lns->x[cln] = -CL + 2;

                }
                if (out) {
                    if (rd1 == 0) {
                        lns->tp[cln] = rd2;
                        lns->ntp[cln] = 15;
                        if (rd2 == 2) lns->clr[cln] = WHT;
                        else lns->clr[cln] = RED;

                    }

                }

            }
            // frog collisions
            int Y = H - cln - 3; // Y is for comparison with frog y
            // log collisions - stopping
            logStop(cln, lns, F, Y);
            // deadly car collisions and frog carry
            carCol(cln, lns, F, Y);

        }

    }
}

// ---------------------------------------------------------------------------------------------------------------------

void drawSF(WINDOW * gw) { // draws start and finish
    for (int i = 1; i < W - 1; i++) {
        for (int j = 1; j < 3; j++) {
            PT(gw, j, i, "/");
            PT(gw, H - j, i, "/");
        }

    }
}

void drawTr(WINDOW * gw) { // transition
    char chars[] = ":O@O:";
    for (int sym = 0; sym < 5; sym++) {
        const char cursym = chars[sym];
        for (int i = 1; i < H; i++) {
            for (int j = 1; j < W - 1; j++) {
                PT(gw, i, j, &cursym);

    }

    }
        box(gw, 0, 0);
        RF(gw);
        Zz(1);

    }
    werase(gw);
    box(gw, 0, 0);
}

// ---------------------------------------------------------------------------------------------------------------------

void menuBtn(WINDOW * gw, int num, int count, char * text, int y, int x) {
    int col = WHT;
    if (num == count) {
        col = BLC;
        PT(gw, y, x - 2, "~|");
        PT(gw, y, x + 18, "|~");

    }
    CON(gw, CLP(col));
    PT(gw, y, x, text);
    COFF(gw, CLP(col));
}

void drawMenu(WINDOW * gw, bool * gon, bool * gplayed, LNS * lns, int * count, IN * in) {
    werase(gw);
    box(gw, 0, 0);
    // moving between btns
    if (in->dy == -1) {*count -= 1; in->dy = 0;
    }
    if (in->dy == 1) {*count += 1; in->dy = 0;
    }
    if (*count < 1) *count = 3;
    if (*count > 3) *count = 1;
    if (!*gplayed) {
        PT(gw, H / 2 - 5, W / 2 - 4, "> SPACE <");
        PT(gw, H / 2 - 3 , W / 2 - 1, "[W]");
        PT(gw, H / 2 - 2, W / 2 - 4, "[A][S][D]");

    } else {
        PT(gw, H / 2 - 3, W / 2 - 5, "PLAY AGAIN");

    }
    menuBtn(gw, 1, *count, "  LEVEL 1 (easy)  ", H / 2, W / 2 - 9);
    menuBtn(gw, 2, *count, " LEVEL 2 (medium) ", H / 2 + 2, W / 2 - 9);
    menuBtn(gw, 3, *count, "  LEVEL 3 (hard)  ", H / 2 + 4, W / 2 - 9);
    RF(gw);
    if (in->curpress == ' ') {*gon = true; lns->mode = *count; makeLns(lns); drawTr(gw); in->dx = 0, in->dy = 0;
    }
}


void drawGame(WINDOW * gw, A * frog, A * stork, bool * gover, LNS * lns, SYM * syms, int pts) {
    werase(gw);
    drawSF(gw); // draw start and finish
    drawLns(lns, gw, syms);
    // stork propellers (drawn first because otherwise they obstruct the frog)
    drawSP(stork, gw);
    // frog drawn here
    drawF(frog, gw);
    // stork body drawn here
    drawS(stork, gw);
    box(gw, 0, 0);
    // horizontal borders (empty space) so it looks more symmetrical
    for (int i = 1; i < H; i++) {
        PT(gw, i, 1, " ");
        PT(gw, i, W - 2, " ");

    }
    if (*gover) {
        RF(gw);
        Zz(10);
        drawTr(gw);
        if (frog->alive) {
            PT(gw, H / 2 + 1, W / 2 - 5, " YOU WIN!! ");
            char * cpts = (char *) malloc(sizeof("000"));
            ixcPts(pts, cpts);
            PT(gw, H / 2 - 1, W / 2 - 5, "score: ");
            PT(gw, H / 2 - 1, W / 2 + 2, cpts);
            free(cpts);

        }
        else PT(gw, H / 2, W / 2 - 5, " YOU LOOSE ");

    }
    RF(gw);
    if (*gover) {
        Zz(20);
        flushinp();
        makeF(frog, syms);
        makeS(stork, syms);

    }
}

// MAIN LOOP ===========================================================================================================

void GL(WINDOW * gw, WINDOW * sw, bool run, A * frog, A * stork, IN * in, bool * gplayed, LNS *lns, SYM * syms, TMR * timer) {
    bool gon = false;
    bool gover = false;
    int pts = 0;
    int btncount = 1;

    while (run) {
        if (gon) {
            *gplayed = true;
            Timer(sw, timer, &frog->alive);
            getIn(in, gw);
            upF(frog, in, &gover);
            upS(stork, frog, &gover);
            upLns(lns, frog);
            if (!frog->alive) gover = true;
            calPts(&pts, frog->count, timer->sec); // calculate points real time, its less complicated and not THAT bad
            drawGame(gw, frog, stork, &gover, lns, syms, pts);
            if (gover) {
                gon = false;
                gover = false;
                timer->mil = 10000;
                timer->sec = 60;
                PT(sw, 1, W - 3, "  ");
                RF(sw);

        }

        } else {
            getIn(in, gw);
            frog->count = 0;
            drawMenu(gw, &gon, gplayed, lns, &btncount, in);

        }
        Zz(1);

}
}

// MAIN FUNCTION =======================================================================================================

int main() {
    bool run = true;
    bool gplayed = false;

    // ncurses begin here ----------------------------------------------------------------------------------------------
    initscr();
    curs_set(0);
    noecho();
    cbreak();
    getch();

    // colours
    start_color(); // init_pair()
    IP(GRN, COLOR_GREEN, COLOR_GREEN);
    IP(FOFF, COLOR_BLACK, COLOR_GREEN);
    IP(STRK, COLOR_RED, COLOR_YELLOW);
    IP(RED, COLOR_RED, COLOR_BLACK);
    IP(BLU, COLOR_BLUE, COLOR_BLACK);
    IP(YLW, COLOR_YELLOW, COLOR_BLACK);
    IP(WHT, COLOR_WHITE, COLOR_BLACK);
    IP(BLC, COLOR_BLACK, COLOR_WHITE);

    // structures initiated here
    WINDOW * gw = newwin(H + 1, W, 1 + 3, 2); // gw - game window
    wclear(gw);
    nodelay(gw, true);

    WINDOW * sw = newwin(3, W, H + 1 + 1 + 3, 2); // sw - stat window
    PT(sw, 1, 1, ID);
    box(sw, 0, 0);
    RF(sw);

    SYM * syms = (SYM *) malloc(sizeof(SYM));
    FsymRead(syms);

    IN * in = (IN *) malloc(sizeof(IN));

    LNS * lns = (LNS *) malloc(sizeof(LNS)); // lns stands for lanes

    A * frog = (A *) malloc(sizeof(A));
    makeF(frog, syms);

    A * stork = (A *) malloc(sizeof(A));
    makeS(stork, syms);

    TMR * timer = (TMR *) malloc(sizeof(TMR));
    timer->sec = 60;
    timer->mil = 10000;
    timer->prev = clock();

    // running the main game loop right here
    GL(gw, sw, run, frog, stork, in, &gplayed, lns, syms, timer);

    free(frog->form);
    free(frog);
    free(in);
    free(syms);
    free(lns);
    free(timer);

    clear(); // window winddown
    delwin(gw);
    delwin(sw);
    endwin();
    // ncurses end here ------------------------------------------------------------------------------------------------
    return 0;
}

// =====================================================================================================================

