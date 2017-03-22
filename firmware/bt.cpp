#include <terminal.h>

#define BTCONF_PIN              32

void bt_init()
{
    pinMode(BTCONF_PIN, OUTPUT);
    digitalWrite(BTCONF_PIN, LOW);

    Serial3.begin(38400);
    for (int k=0; k<5; k++) {
        Serial3.write("AT+RESET\r\n\r\n");
        Serial3.write("AT+RESET\r\n");
    }
    Serial3.begin(115200);
}

static void goToConf()
{
    digitalWrite(BTCONF_PIN, HIGH);
    delay(100);
    digitalWrite(BTCONF_PIN, LOW);
}

static void bt_conf(char *name, char *pin)
{
    goToConf();
    Serial3.write("AT\r\n");
    delay(10);
    Serial3.write("AT+UART=115200,0,0\r\n");
    delay(10);
    Serial3.write("AT+NAME=");
    Serial3.write(name);
    Serial3.write("\r\n");
    delay(10);
    Serial3.write("AT+PSWD=");
    Serial3.write(pin);
    delay(10);
    Serial3.write("\r\n");
    Serial3.write("AT+RESET\r\n");
    delay(10);
}

TERMINAL_COMMAND(btconf, "Bluetooth config")
{
    if (argc != 2) {
        terminal_io()->println("Usage: btconf <name> <pin>");
    } else {
        char *name = argv[0];
        char *pin = argv[1];
        terminal_io()->println("Configuring bluetooth to name:");
        terminal_io()->println(name);
        terminal_io()->println("And pin:");
        terminal_io()->println(pin);

        for (int k=0; k<3; k++) {
            Serial3.begin(9600);
            for (int n=0; n<3; n++) bt_conf(name, pin);
            Serial3.begin(38400);
            for (int n=0; n<3; n++) bt_conf(name, pin);
            Serial3.begin(57600);
            for (int n=0; n<3; n++) bt_conf(name, pin);
            Serial3.begin(115200);
            for (int n=0; n<3; n++) bt_conf(name, pin);
            Serial3.begin(921600);
            for (int n=0; n<3; n++) bt_conf(name, pin);
        }
    }
}

TERMINAL_COMMAND(btpulse, "BT conf pulse")
{
    goToConf();
}
