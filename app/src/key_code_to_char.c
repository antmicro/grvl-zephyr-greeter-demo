#include "key_code_to_char.h"

char key_code_to_char(uint16_t key_code) {
    	switch (key_code) {
	case INPUT_KEY_1:           return '1';
	case INPUT_KEY_2:           return '2';
	case INPUT_KEY_3:           return '3';
	case INPUT_KEY_4:           return '4';
	case INPUT_KEY_5:           return '5';
	case INPUT_KEY_6:           return '6';
	case INPUT_KEY_7:           return '7';
	case INPUT_KEY_8:           return '8';
	case INPUT_KEY_9:           return '9';
	case INPUT_KEY_0:           return '0';
	case INPUT_KEY_MINUS:       return '-';
	case INPUT_KEY_EQUAL:       return '=';
	case INPUT_KEY_Q:           return 'q';
	case INPUT_KEY_W:           return 'w';
	case INPUT_KEY_E:           return 'e';
	case INPUT_KEY_R:           return 'r';
	case INPUT_KEY_T:           return 't';
	case INPUT_KEY_Y:           return 'y';
	case INPUT_KEY_U:           return 'u';
	case INPUT_KEY_I:           return 'i';
	case INPUT_KEY_O:           return 'o';
	case INPUT_KEY_P:           return 'p';
	case INPUT_KEY_LEFTBRACE:   return '[';
	case INPUT_KEY_RIGHTBRACE:  return ']';
	case INPUT_KEY_A:           return 'a';
	case INPUT_KEY_S:           return 's';
	case INPUT_KEY_D:           return 'd';
	case INPUT_KEY_F:           return 'f';
	case INPUT_KEY_G:           return 'g';
	case INPUT_KEY_H:           return 'h';
	case INPUT_KEY_J:           return 'j';
	case INPUT_KEY_K:           return 'k';
	case INPUT_KEY_L:           return 'l';
	case INPUT_KEY_SEMICOLON:   return ';';
	case INPUT_KEY_APOSTROPHE:  return '\'';
	case INPUT_KEY_GRAVE:       return '`';
	case INPUT_KEY_BACKSLASH:   return '\\';
	case INPUT_KEY_Z:           return 'z';
	case INPUT_KEY_X:           return 'x';
	case INPUT_KEY_C:           return 'c';
	case INPUT_KEY_V:           return 'v';
	case INPUT_KEY_B:           return 'b';
	case INPUT_KEY_N:           return 'n';
	case INPUT_KEY_M:           return 'm';
	case INPUT_KEY_COMMA:       return ',';
	case INPUT_KEY_DOT:         return '.';
	case INPUT_KEY_SLASH:       return '/';
	case INPUT_KEY_SPACE:	    return ' ';
	default:                    return '\0';
	}
}
