import socket
import threading
import webbrowser
import subprocess
from datetime import datetime

HOST = "0.0.0.0"
PORT = 4444
TOKEN = "supersecretpassword"
BUFFER_SIZE = 1024


def log(msg: str) -> None:
    now = datetime.now().strftime("%H:%M:%S")
    print(f"[{now}] {msg}")


def open_url(url: str) -> None:
    webbrowser.open(url)


def run_command(command: str) -> None:
    subprocess.Popen(command, shell=True)


def shutdown_pc() -> None:
    subprocess.Popen("shutdown /s /t 0", shell=True)


def press_enter() -> None:
    import pyautogui
    pyautogui.press("enter")


def play_pause() -> None:
    import pyautogui
    pyautogui.press("playpause")


def alt_tab() -> None:
    import pyautogui
    pyautogui.hotkey( "tab")  # FIX: was just hotkey("tab"), alt+tab needs both keys


def refresh() -> None:
    import pyautogui
    pyautogui.press("f5")


def downSP() -> None:
    import pyautogui
    for _ in range(6):
        pyautogui.press("down")  # FIX: was hotkey("up") — wrong key and wrong function


def noTard() -> None:
    import pyautogui
    for _ in range(6):
        pyautogui.press("up")  # FIX: was hotkey("up") — hotkey() is for combos, not single keys

def FreeRam() -> None:
    import webbrowser
    import pyautogui
    import time
    page = "https://downloadmoreram.com/"
    wbbrowser = webbrowser.open(page)
    time.sleep(2)
    pyautogui.press("tab")
    time.sleep(0.5)
    pyautogui.press("enter")
    for e in range(4):
        pyautogui.press("tab")
        time.sleep(1)
    pyautogui.press("enter")

def FakeError() -> None:
    import webbrowser
    import pyautogui
    import time
    prank_site="https://geekprank.com/blue-death/"
    webbrowser.open(prank_site)
    time.sleep(2)
    pyautogui.hotkey("F11")

def vUP() -> None:
    from ctypes import POINTER, cast
    from comtypes import CLSCTX_ALL
    from pycaw.pycaw import AudioUtilities, IAudioEndpointVolume
    devices = AudioUtilities.GetSpeakers()
    interface = devices.Activate(IAudioEndpointVolume._iid_, CLSCTX_ALL, None)
    volume = cast(interface, POINTER(IAudioEndpointVolume))
    current_volume = volume.GetMasterVolumeLevelScalar()
    new_volume = current_volume + 0.05
    if new_volume > 1.0:
        new_volume = 1.0
    volume.SetMasterVolumeLevelScalar(new_volume, None) 


def vDOWN() -> None:
    from ctypes import POINTER, cast
    from comtypes import CLSCTX_ALL
    from pycaw.pycaw import AudioUtilities, IAudioEndpointVolume
    devices = AudioUtilities.GetSpeakers()
    interface = devices.Activate(IAudioEndpointVolume._iid_, CLSCTX_ALL, None)
    volume = cast(interface, POINTER(IAudioEndpointVolume))
    current_volume = volume.GetMasterVolumeLevelScalar()
    new_volume = current_volume - 0.05
    if new_volume < 0.0:
        new_volume = 0.0
    volume.SetMasterVolumeLevelScalar(new_volume, None)


def close_window() -> None:  # FIX: renamed from exit() — shadowed the built-in
    import pyautogui
    pyautogui.hotkey("alt", "f4")


COMMANDS = {
    "BTN1_SINGLE_YOUTUBE": alt_tab,
    "BTN2_SINGLE_WHATSAPP": press_enter,
    "BTN3_SINGLE_NETFLIX": downSP,
    "BTN4_SINGLE_FIREFOX": noTard,

    "BTN1_DOUBLE_ENTER": lambda: open_url("https://www.youtube.com"),
    "BTN2_DOUBLE_PLAYPAUSE": lambda: open_url("https://www.netflix.com"),
    "BTN3_DOUBLE_ALTTAB": lambda: run_command("start whatsapp://"),
    "BTN4_DOUBLE_REFRESH": refresh,

    "BTN1_LONG_vUP": vUP,
    "BTN2_LONG_vDOWN": vDOWN,
    "BTN3_LONG_EXIT": close_window,  # FIX: was exit (built-in shadow)
    "BTN4_LONG_SHUTDOWN": shutdown_pc,

#   "BTN1_EXTREME_FUNNYPRANK":
    "BTN2_EXTREME_FAKEERROR": FakeError,
    "BTN3_EXTREME_DOWNLOADRAM": FreeRam,
}


def handle_command(cmd: str) -> None:
    log(f"Received: {cmd}")
    action = COMMANDS.get(cmd)

    if action is None:
        log(f"Unknown command: {cmd}")
        return

    try:
        action()
        log(f"Executed: {cmd}")
    except Exception as e:
        log(f"Error while executing {cmd}: {e}")


def parse_message(data: str):
    if "|" not in data:
        return None

    token, cmd = data.split("|", 1)
    token = token.strip()
    cmd = cmd.strip()

    if not token or not cmd:
        return None

    return token, cmd


def handle_client(client: socket.socket, addr):
    try:
        raw = client.recv(BUFFER_SIZE)
        data = raw.decode(errors="ignore").strip()

        if not data:
            log(f"Empty packet from {addr}")
            return

        parsed = parse_message(data)
        if parsed is None:
            log(f"Invalid format from {addr}: {data!r}")
            return

        token, cmd = parsed

        if token != TOKEN:
            log(f"Invalid token from {addr}")
            return

        handle_command(cmd)

    except Exception as e:
        log(f"Client error from {addr}: {e}")
    finally:
        client.close()


def start_server():
    log(f"Listening on {HOST}:{PORT}")

    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind((HOST, PORT))
    server.listen(5)

    try:
        while True:
            client, addr = server.accept()
            thread = threading.Thread(
                target=handle_client,
                args=(client, addr),
                daemon=True,
            )
            thread.start()
    except KeyboardInterrupt:
        log("Server stopped.")
    finally:
        server.close()


if __name__ == "__main__":
    start_server()
