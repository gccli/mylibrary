import sys
import time
import win32api, win32con, win32gui

key_mapping = {
    '0': 48,
    '1': 49,
    '2': 50,
    '3': 51,
    '4': 52,
    '5': 53,
    '6': 54,
    '7': 55,
    '8': 56,
    '9': 57,
    'a': 65,
    'b': 66,
    'c': 67,
    'd': 68,
    'e': 69,
    'f': 70,
    'g': 71,
    'h': 72,
    'i': 73,
    'j': 74,
    'k': 75,
    'l': 76,
    'm': 77,
    'n': 78,
    'o': 79,
    'p': 80,
    'q': 81,
    'INS': 45,
    'DEL': 46,
    'LWIN': 91,
    'RWIN': 92,
    'LSHIFT': 160,
    'SHIFT': 161,
    'LCTRL': 162,
    'RCTRL': 163,
    'VOLUP': 175,
    'DOLDOWN': 174,
    'NUMLOCK': 144,
    'SCROLL': 145
}

def key_up(key):
    win32api.keybd_event(key, 0, 2, 0)

def key_down(key):
    win32api.keybd_event(key, 0, 1, 0)

def key_press(key, speed=1):
    rest_time = 0.05/speed
    if key in key_mapping:
        key = key_mapping[key]
        key_down(key)
        time.sleep(rest_time)
        key_up(key)
        return True

def left_click(x,y):
    win32api.SetCursorPos((x,y))
    win32api.mouse_event(win32con.MOUSEEVENTF_LEFTDOWN,x,y,0,0)
    win32api.mouse_event(win32con.MOUSEEVENTF_LEFTUP,x,y,0,0)

def enum_handler(hwnd, lParam):
    if win32gui.IsWindowVisible(hwnd):
        print 'window is', win32gui.GetWindowText(hwnd)
        if 'Outlook' in win32gui.GetWindowText(hwnd):
            win32gui.MoveWindow(hwnd, 0, 0, 760, 500, True)

win32gui.EnumWindows(enum_handler, None)

sys.exit(1)
input_window = [
    (1000, 395),
    (1500, 395)
]

coordinates = [
    (306, 385),
    (308, 200),
    (265, 800)
]

for x,y in coordinates:
    left_click(input_window[0][0], input_window[0][1])
    time.sleep(0.2)

    for i in str(x): key_press(i)

    left_click(input_window[1][0], input_window[1][1])
    time.sleep(0.2)
    for i in str(y): key_press(i)

    time.sleep(0.5)
