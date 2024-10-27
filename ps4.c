#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>

#define DEVICE_PATH "/dev/input/event24"
#define DEADZONE 10

int main()
{
    int fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0)
    {
        perror("Failed to open device");
        return 1;
    }

    struct ff_effect effect;
    memset(&effect, 0, sizeof(effect));
    effect.type = FF_RUMBLE;
    effect.id = -1;
    effect.u.rumble.strong_magnitude = 0x8000;
    effect.u.rumble.weak_magnitude = 0x4000;
    effect.replay.length = 2000;
    effect.replay.delay = 0;

    if (ioctl(fd, EVIOCSFF, &effect) == -1)
    {
        perror("Error creating force feedback effect");
        close(fd);
        return 1;
    }

    struct input_event play, stop;
    memset(&play, 0, sizeof(play));
    play.type = EV_FF;
    play.code = effect.id;
    play.value = 1;

    memset(&stop, 0, sizeof(stop));
    stop.type = EV_FF;
    stop.code = effect.id;
    stop.value = 0;

    struct input_event ev;
    while (1)
    {
        ssize_t bytes = read(fd, &ev, sizeof(ev));
        if (bytes < (ssize_t)sizeof(ev))
        {
            if (bytes == -1 && errno != EAGAIN)
            {
                perror("Error reading event");
                break;
            }
            usleep(5000);
            continue;
        }

        if (ev.type == EV_KEY)
        {
            switch (ev.code)
            {
            case BTN_SOUTH:
                printf("Cross (X) Button %s\n", ev.value ? "pressed" : "released");
                if (ev.value == 1)
                {
                    printf("Starting vibration...\n");
                    if (write(fd, &play, sizeof(play)) == -1)
                    {
                        perror("Error starting vibration");
                    }
                }
                else if (ev.value == 0)
                {
                    printf("Stopping vibration...\n");
                    if (write(fd, &stop, sizeof(stop)) == -1)
                    {
                        perror("Error stopping vibration");
                    }
                }
                break;
            case BTN_EAST:
                printf("Circle Button %s\n", ev.value ? "pressed" : "released");
                break;
            case BTN_NORTH:
                printf("Triangle Button %s\n", ev.value ? "pressed" : "released");
                break;
            case BTN_WEST:
                printf("Square Button %s\n", ev.value ? "pressed" : "released");
                break;
            case BTN_TL:
                printf("L1 (Left Bumper) %s\n", ev.value ? "pressed" : "released");
                break;
            case BTN_TR:
                printf("R1 (Right Bumper) %s\n", ev.value ? "pressed" : "released");
                break;
            case BTN_SELECT:
                printf("Share Button %s\n", ev.value ? "pressed" : "released");
                break;
            case BTN_START:
                printf("Options Button %s\n", ev.value ? "pressed" : "released");
                break;
            case BTN_THUMBL:
                printf("Left Joystick (L3) %s\n", ev.value ? "pressed" : "released");
                break;
            case BTN_THUMBR:
                printf("Right Joystick (R3) %s\n", ev.value ? "pressed" : "released");
                break;
            case BTN_MODE:
                printf("PlayStation (PS) Button %s\n", ev.value ? "pressed" : "released");
                break;
            case BTN_TOUCH:
                printf("Touchpad Button %s\n", ev.value ? "pressed" : "released");
                break;
            default:
                printf("Button %d %s\n", ev.code, ev.value ? "pressed" : "released");
                break;
            }
        }
        else if (ev.type == EV_ABS)
        {
            int value = ev.value;

            switch (ev.code)
            {
            case ABS_X:
                if (abs(value - 128) > DEADZONE)
                    printf("Left Joystick Horizontal: %d\n", value);
                break;
            case ABS_Y:
                if (abs(value - 128) > DEADZONE)
                    printf("Left Joystick Vertical: %d\n", value);
                break;
            case ABS_RX:
                if (abs(value - 128) > DEADZONE)
                    printf("Right Joystick Horizontal: %d\n", value);
                break;
            case ABS_RY:
                if (abs(value - 128) > DEADZONE)
                    printf("Right Joystick Vertical: %d\n", value);
                break;
            case ABS_Z:
                if (value > DEADZONE)
                    printf("L2 (Left Trigger) Pressure: %d\n", value);
                break;
            case ABS_RZ:
                if (value > DEADZONE)
                    printf("R2 (Right Trigger) Pressure: %d\n", value);
                break;
            case ABS_HAT0X:
                printf("D-Pad Horizontal Raw Value: %d\n", value);
                if (value == -1)
                    printf("D-Pad Left\n");
                else if (value == 1)
                    printf("D-Pad Right\n");
                else if (value == 0)
                    printf("D-Pad Horizontal Centered\n");
                break;
            case ABS_HAT0Y:
                printf("D-Pad Vertical Raw Value: %d\n", value);
                if (value == -1)
                    printf("D-Pad Up\n");
                else if (value == 1)
                    printf("D-Pad Down\n");
                else if (value == 0)
                    printf("D-Pad Vertical Centered\n");
                break;
            default:
                printf("Axis %d value: %d\n", ev.code, value);
                break;
            }
        }
        else if (ev.type == EV_SYN)
        {
            continue;
        }
    }

    close(fd);
    return 0;
}
