/*
 * Copyright (c) 2009 Matthew Iselin
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * This will be inserted into the Windows-specific key translation array.
 * Note that not all VK_ codes are supported, only a "logical" set is actually
 * available.
 */

// Letters
{"A",            'A',               true},
{"B",            'B',               true},
{"C",            'C',               true},
{"D",            'D',               true},
{"E",            'E',               true},
{"F",            'F',               true},
{"G",            'G',               true},
{"H",            'H',               true},
{"I",            'I',               true},
{"J",            'J',               true},
{"K",            'K',               true},
{"L",            'L',               true},
{"M",            'M',               true},
{"N",            'N',               true},
{"O",            'O',               true},
{"P",            'P',               true},
{"Q",            'Q',               true},
{"R",            'R',               true},
{"S",            'S',               true},
{"T",            'T',               true},
{"U",            'U',               true},
{"V",            'V',               true},
{"W",            'W',               true},
{"X",            'X',               true},
{"Y",            'Y',               true},
{"Z",            'Z',               true},

// Numbers
{"0",            '0',               true},
{"1",            '1',               true},
{"2",            '2',               true},
{"3",            '3',               true},
{"4",            '4',               true},
{"5",            '5',               true},
{"6",            '6',               true},
{"7",            '7',               true},
{"8",            '8',               true},
{"9",            '9',               true},

// Numpad
{"Numpad0",     VK_NUMPAD0,         true},
{"Numpad1",     VK_NUMPAD1,         true},
{"Numpad2",     VK_NUMPAD2,         true},
{"Numpad3",     VK_NUMPAD3,         true},
{"Numpad4",     VK_NUMPAD4,         true},
{"Numpad5",     VK_NUMPAD5,         true},
{"Numpad6",     VK_NUMPAD6,         true},
{"Numpad7",     VK_NUMPAD7,         true},
{"Numpad8",     VK_NUMPAD8,         true},
{"Numpad9",     VK_NUMPAD9,         true},

// Numpad operation keys
{"*",           VK_MULTIPLY,        true},
{"+",           VK_ADD,             true},
{"-",           VK_SUBTRACT,        true},
{".",           VK_DECIMAL,         true},
{"/",           VK_DIVIDE,          true},

// Special keys
{"Home",        VK_HOME,            true},
{"End",         VK_END,             true},
{"Left",        VK_LEFT,            true},
{"Right",       VK_RIGHT,           true},
{"Up",          VK_UP,              true},
{"Down",        VK_DOWN,            true},
{"Insert",      VK_INSERT,          true},
{"Delete",      VK_DELETE,          true},

// Function keys
{"F1",          VK_F1,              true},
{"F2",          VK_F2,              true},
{"F3",          VK_F3,              true},
{"F4",          VK_F4,              true},
{"F5",          VK_F5,              true},
{"F6",          VK_F6,              true},
{"F7",          VK_F7,              true},
{"F8",          VK_F8,              true},
{"F9",          VK_F9,              true},
{"F10",         VK_F10,             true},
{"F11",         VK_F11,             true},
{"F12",         VK_F12,             true},
{"F13",         VK_F13,             true},
{"F14",         VK_F14,             true},
{"F15",         VK_F15,             true},
{"F16",         VK_F16,             true},
{"F17",         VK_F17,             true},
{"F18",         VK_F18,             true},
{"F19",         VK_F19,             true},
{"F20",         VK_F20,             true},
{"F21",         VK_F21,             true},
{"F22",         VK_F22,             true},
{"F23",         VK_F23,             true},
{"F24",         VK_F24,             true},

// Shift keys
{"LShift",      VK_LSHIFT,          false},
{"RShift",      VK_RSHIFT,          false},

// Control keys
{"LCtrl",       VK_LCONTROL,        false},
{"RCtrl",       VK_RCONTROL,        false},
