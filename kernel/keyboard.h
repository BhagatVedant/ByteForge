#ifndef KEYBOARD_H
#define KEYBOARD_H

void keyboard_init(void);
void keyboard_poll(void);
void keyboard_push_char(char c);
int keyboard_has_char(void);
char keyboard_get_char(void);

#endif