import random
import time
import win32gui
import win32con
import win32api

class DVDWindowMover:
    def __init__(self):
        self.windows = []
        self.original_positions = []
        self.original_sizes = []
        self.current_positions = []
        self.directions = []
        self.screen_width = win32api.GetSystemMetrics(win32con.SM_CXSCREEN)
        self.screen_height = win32api.GetSystemMetrics(win32con.SM_CYSCREEN)
    
    def enum_windows_callback(self, hwnd, _):
        if win32gui.IsWindowVisible(hwnd) and win32gui.GetParent(hwnd) == 0:
            self.windows.append(hwnd)
        return True
    
    def get_all_windows(self):
        self.windows = []
        win32gui.EnumWindows(self.enum_windows_callback, None)
    
    def save_original_state(self):
        self.original_positions = []
        self.original_sizes = []
        for hwnd in self.windows:
            try:
                left, top, right, bottom = win32gui.GetWindowRect(hwnd)
                self.original_positions.append((left, top))
                self.original_sizes.append((right - left, bottom - top))
            except:
                continue
    
    def initialize_movement(self):
        self.current_positions = []
        self.directions = []
        for i, hwnd in enumerate(self.windows):
            if i >= len(self.original_positions):
                continue
            left, top = self.original_positions[i]
            self.current_positions.append([left, top])
            dx = random.randint(2, 6)
            dy = random.randint(2, 6)
            if random.random() > 0.5:
                dx = -dx
            if random.random() > 0.5:
                dy = -dy
            self.directions.append([dx, dy])
    
    def move_windows(self):
        try:
            while True:
                for i, hwnd in enumerate(self.windows):
                    if i >= len(self.current_positions) or i >= len(self.directions):
                        continue
                    try:
                        self.current_positions[i][0] += self.directions[i][0]
                        self.current_positions[i][1] += self.directions[i][1]
                        width, height = self.original_sizes[i]
                        x, y = self.current_positions[i]
                        if x <= 0 or x + width >= self.screen_width:
                            self.directions[i][0] = -self.directions[i][0]
                        if y <= 0 or y + height >= self.screen_height:
                            self.directions[i][1] = -self.directions[i][1]
                        win32gui.SetWindowPos(
                            hwnd,
                            win32con.HWND_TOP,
                            int(self.current_positions[i][0]),
                            int(self.current_positions[i][1]),
                            width,
                            height,
                            win32con.SWP_NOZORDER
                        )
                    except:
                        continue
                time.sleep(0.01)
        except KeyboardInterrupt:
            self.restore_original_positions()
    
    def restore_original_positions(self):
        for i, hwnd in enumerate(self.windows):
            if i >= len(self.original_positions):
                continue
            try:
                x, y = self.original_positions[i]
                width, height = self.original_sizes[i]
                win32gui.SetWindowPos(
                    hwnd,
                    win32con.HWND_TOP,
                    x,
                    y,
                    width,
                    height,
                    win32con.SWP_NOZORDER
                )
            except:
                continue
    
    def run(self):
        self.get_all_windows()
        self.save_original_state()
        self.initialize_movement()
        self.move_windows()

if __name__ == "__main__":
    mover = DVDWindowMover()
    mover.run()