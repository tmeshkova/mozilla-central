/* Copyright 2012 Mozilla Foundation and Mozilla contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef GONKKEYMAPPING_H
#define GONKKEYMAPPING_H

/* See libui/KeycodeLabels.h for the mapping */
static const unsigned long kKeyMapping[] = {
    0,
    NS_VK_ESCAPE,
    NS_VK_1,
    NS_VK_2,
    NS_VK_3,
    NS_VK_4,
    NS_VK_5,
    NS_VK_6,
    NS_VK_7,
    NS_VK_8,
    NS_VK_9,
    NS_VK_0,
    NS_VK_HYPHEN_MINUS,
    NS_VK_EQUALS,
    NS_VK_BACK,
    NS_VK_TAB,
    NS_VK_Q,
    NS_VK_E,
    NS_VK_R,
    NS_VK_T,
    NS_VK_Y,
    NS_VK_U,
    NS_VK_I,
    NS_VK_O,
    NS_VK_P,
    0, //NS_VK_LEFTBRACE,
    0, //NS_VK_RIGHTBRACE,
    NS_VK_ENTER,
    0, //NS_VK_LEFTCTRL,
    NS_VK_A,
    NS_VK_S,
    NS_VK_D,
    NS_VK_F,
    NS_VK_G,
    NS_VK_H,
    NS_VK_J,
    NS_VK_K,
    NS_VK_L,
    NS_VK_SEMICOLON,
    0, //NS_VK_APOSTROPHE,
    0, //NS_VK_GRAVE,
    0, //NS_VK_LEFTSHIFT,
    0, //NS_VK_BACKSLASH,
    NS_VK_Z,
    NS_VK_X,
    NS_VK_C,
    NS_VK_V,
    NS_VK_B,
    NS_VK_N,
    NS_VK_M,
    NS_VK_COMMA,
    0, //NS_VK_DOT,
    NS_VK_SLASH,
    0, //NS_VK_RIGHTSHIFT,
    0, //NS_VK_KPASTERISK,
    0, //NS_VK_LEFTALT,
    NS_VK_SPACE,
    0, //NS_VK_CAPSLOCK,
    NS_VK_F1,
    NS_VK_F2,
    NS_VK_F3,
    NS_VK_F4,
    NS_VK_F5,
    NS_VK_F6,
    NS_VK_F7,
    NS_VK_F8,
    NS_VK_F9,
    NS_VK_F10,
    // There are more but we don't map them
};
#endif /* GONKKEYMAPPING_H */
