#ifndef INPUT_MANAGER
#define INPUT_MANAGER

#define BUTTON_W 5
#define BUTTON_A 6
#define BUTTON_S 7
#define BUTTON_D 8

#define BUTTON_I 12
#define BUTTON_J 13
#define BUTTON_K 14
#define BUTTON_L 15

const uint8_t BUTTONS[] = {
    BUTTON_W,
    BUTTON_A,
    BUTTON_S,
    BUTTON_D,

    BUTTON_I,
    BUTTON_J,
    BUTTON_K,
    BUTTON_L};

#define TOTAL_BUTTONS 8

// Sets up the GPIO pins for the buttons
void InitializeButtons()
{
    for (int i = 0; i < sizeof(BUTTONS) / sizeof(BUTTONS[0]); i++)
    {
        gpio_init(BUTTONS[i]);
        gpio_set_dir(BUTTONS[i], GPIO_IN);
        gpio_pull_up(BUTTONS[i]);
    }
}

uint8_t GetButton()
{
    for (int i = 0; i < TOTAL_BUTTONS; i++)
    {
        if (gpio_get(BUTTONS[i]) == 0)
        {
            return BUTTONS[i];
        }
    }
    return 0;
}

#endif