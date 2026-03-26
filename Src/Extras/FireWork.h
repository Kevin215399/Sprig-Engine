#ifndef FIREWORK

//// Currently unused, uses too much sram, is removed

/*
#define FIREWORK
#include <stdio.h>
#include "pico/stdlib.h"
#include <math.h>

#include "TFT.h"
#include "SmartRender.h"
#include "EngineStructs.h"
#include "Editor.h"

uint16_t colors[12];

void SetupColors()
{
    colors[0] = RGBTo16(255, 0, 0);
    colors[1] = RGBTo16(255, 35, 0);
    colors[2] = RGBTo16(255, 55, 0);
    colors[3] = RGBTo16(255, 255, 0);
    colors[4] = RGBTo16(200, 255, 0);
    colors[5] = RGBTo16(0, 255, 0);
    colors[6] = RGBTo16(0, 255, 100);
    colors[7] = RGBTo16(0, 255, 255);
    colors[8] = RGBTo16(0, 0, 255);
    colors[9] = RGBTo16(160, 0, 255);
    colors[10] = RGBTo16(255, 0, 200);
    colors[11] = RGBTo16(255, 255, 255);
}

typedef struct
{
    bool root;
    float velocity;
    int direction;
    Vector2 pos;
    uint16_t color;
} Particle;

Particle particles[600];
bool particlesUsed[600];

int MakeParticle(bool root, float velocity, int dir, Vector2 pos, uint16_t color)
{
    int particleIndex = 0;
    while (particlesUsed[particleIndex])
    {
        particleIndex++;
    }

    particles[particleIndex].root = root;
    particles[particleIndex].velocity = velocity;
    particles[particleIndex].direction = dir;
    particles[particleIndex].pos.x = pos.x;
    particles[particleIndex].pos.y = pos.y;
    particles[particleIndex].color = color;

    particlesUsed[particleIndex] = true;
}

void RunParticle(int index)
{
    if (!particlesUsed[index])
        return;
    particles[index].pos.x += cos((particles[index].direction - 90) * (3.1415 / 180)) * particles[index].velocity;
    particles[index].pos.y += sin((particles[index].direction - 90) * (3.1415 / 180)) * particles[index].velocity;

    particles[index].velocity *= .9;

    printf("(%f,%f), %f\n", particles[index].pos.x, particles[index].pos.y, particles[index].velocity);

    SmartRect(particles[index].color, floor(particles[index].pos.x - 1), floor(particles[index].pos.y - 1), 2, 2);

    if (particles[index].velocity <= .5)
    {
        if (particles[index].root)
        {
            printf("%s\n", "spawning more");
            for (int i = 0; i < 180; i++)
            {
                MakeParticle(false, rand() % 12 + 5, i * 2, particles[index].pos, particles[index].color);
            }
            particlesUsed[index] = false;
        }
        else
        {
            particlesUsed[index] = false;
        }
    }
}

void LaunchFirework(int x)
{

    while (1)
    {
        Vector2 pos;
        pos.x = rand() % 140 + 10;
        pos.y = 128;
        MakeParticle(true, 10, 0, pos, colors[rand() % 12]);

        for (int i = 0; i < 30; i++)
        {
            SmartClear();
            for (int i = 0; i < 200; i++)
            {
                RunParticle(i);
            }
            SmartShow();
            sleep_ms(20);
        }
    }
}
void FireworkShow()
{
    SetupColors();
    memset(particles, 0, sizeof(particles));
    memset(particlesUsed, 0, sizeof(particlesUsed));

    LaunchFirework(80);
}*/

#endif