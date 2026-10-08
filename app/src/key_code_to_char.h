#ifndef HID_DEMO_KEY_CODE_TO_CHAR_H
#define HID_DEMO_KEY_CODE_TO_CHAR_H

#include <zephyr/input/input.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

char key_code_to_char(uint16_t key_code);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* HID_DEMO_KEY_CODE_TO_CHAR_H */
