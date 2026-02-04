#ifndef STARTUP_LOGO
#define STARTUP_LOGO

#include "SmartRender.h"
#include "pico/multicore.h"

void SuperCoolLogo()
{

    for (int x = 0; x <= 330; x++)
    {
        SmartClear();
        int s = min(max(0, x), 200);
        int p = min(max(15, x), 215) - 15;
        int r = min(max(30, x), 230) - 30;
        int i = min(max(45, x), 245) - 45;
        int g = min(max(60, x), 260) - 60;

        int move = min(max(0, x - 300), 15);

        SmartLetter('S', cos(((float)s * .9 - 90) * (3.1415 / 180)) * 50 + 4, sin(((float)s * .9 - 90) * (3.1415 / 180)) * 50 - move, 4, WHITE, TRANSPARENT);
        SmartLetter('P', cos(((float)p * .9 - 90) * (3.1415 / 180)) * 50 + 34, sin(((float)p * .9 - 90) * (3.1415 / 180)) * 50 - move, 4, WHITE, TRANSPARENT);
        SmartLetter('R', cos(((float)r * .9 - 90) * (3.1415 / 180)) * 50 + 64, sin(((float)r * .9 - 90) * (3.1415 / 180)) * 50 - move, 4, WHITE, TRANSPARENT);
        SmartLetter('I', cos(((float)i * .9 - 90) * (3.1415 / 180)) * 50 + 94, sin(((float)i * .9 - 90) * (3.1415 / 180)) * 50 - move, 4, WHITE, TRANSPARENT);
        SmartLetter('G', cos(((float)g * .9 - 90) * (3.1415 / 180)) * 50 + 124, sin(((float)g * .9 - 90) * (3.1415 / 180)) * 50 - move, 4, WHITE, TRANSPARENT);

        if (x >= 330)
            SmartWord("E N G I N E", strlen("E N G I N E"), 5, 70, 2, BLACK, GREEN);

        SmartShow();
    }
    sleep_ms(1500);
}

void LoadingScreen()
{
    Clear();
    SmartClear();
    SmartLetter('S', 4, 35, 4, WHITE, TRANSPARENT);
    SmartLetter('P', 34, 35, 4, WHITE, TRANSPARENT);
    SmartLetter('R', 64, 35, 4, WHITE, TRANSPARENT);
    SmartLetter('I', 94, 35, 4, WHITE, TRANSPARENT);
    SmartLetter('G', 124, 35, 4, WHITE, TRANSPARENT);

    SmartWord("Loading...", strlen("Loading..."), 5, 70, 2, BLACK, GREEN);

    SmartShowAll();
}

#endif
