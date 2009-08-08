#include <windows.h>
#include "os-dep.h"
#include "win32-key.h"
void dbg(char *format, ...);

#if GCIN_IME
#include "ChewingIME.h"

KeySym vk2gcin_key(UINT key, char asc, const BYTE* keystate)
{
	bool shift = IsKeyDown(keystate[VK_SHIFT]);

	dbg("vk2gcin_key %x %x\n", key, asc);
	if (shift && asc)
		return asc;

	switch (key) {
		case VK_OEM_MINUS:
			return '-';
		case VK_OEM_COMMA:
			return ',';
		case VK_OEM_PERIOD:
			return '.';
		case VK_OEM_1:
			return ';';
		case VK_OEM_2:
			return '/';
		case VK_OEM_3:
			return '`';
		case VK_OEM_4:
			return '[';
		case VK_OEM_5:
			return '\\';
		case VK_OEM_6:
			return ']';
		case VK_OEM_7:
			return '\'';
		case VK_PRIOR:
		case VK_NEXT:
		case VK_END:
		case VK_HOME:
		case VK_LEFT:
		case VK_UP:
		case VK_RIGHT:
		case VK_DOWN:
		case VK_PRINT:
		case VK_INSERT:
		case VK_DELETE:
		case VK_HELP:
		case VK_NUMPAD0:
		case VK_NUMPAD1:
		case VK_NUMPAD2:
		case VK_NUMPAD3:
		case VK_NUMPAD4:
		case VK_NUMPAD5:
		case VK_NUMPAD6:
		case VK_NUMPAD7:
		case VK_NUMPAD8:
		case VK_NUMPAD9:
		case VK_MULTIPLY:
		case VK_ADD:
		case VK_SEPARATOR:
		case VK_SUBTRACT:
		case VK_DECIMAL:
		case VK_DIVIDE:
		case VK_F1:
		case VK_F2:
		case VK_F3:
		case VK_F4:
		case VK_F5:
		case VK_F6:
		case VK_F7:
		case VK_F8:
		case VK_F9:
		case VK_F10:
		case VK_F11:
		case VK_F12:
		case VK_LSHIFT:
		case VK_RSHIFT:
		case VK_LCONTROL:
		case VK_RCONTROL:
		case VK_LMENU:
		case VK_RMENU:
		case VK_LWIN:
		case VK_RWIN:
		case VK_APPS:
		case VK_SLEEP:
			return key + KOFS;
		default:
			if (asc) {
				dbg("return asc %c\n", asc);
				return asc;
			}

			dbg("unknown key %x\n", key);
			return key + KOFS;
	}
}

#else
void win32_FakeKey(UINT vk)
{
  if (vk >= KOFS)
    vk -= KOFS;
  else
  if (vk >= 'a' && vk <= 'z')
    vk = toupper(vk);
  else
  switch (vk) {
	case ';':
	case ':':
	  vk=VK_OEM_1;
	  break;
	case ',':
	  vk=VK_OEM_COMMA;
	  break;
	case '.':
	  vk=VK_OEM_PERIOD;
	  break;
	case '/':
	  vk=VK_OEM_2;
	  break;
	case '`':
	  vk=VK_OEM_3;
	  break;
	case '[':
	  vk=VK_OEM_4;
	  break;
	case '\\':
	  vk=VK_OEM_5;
	  break;
	case ']':
	  vk=VK_OEM_6;
	  break;
	case '\'':
	  vk=VK_OEM_7;
	  break;
  }

  dbg("win32_FakeKey %x\n", vk);

  keybd_event(vk,MapVirtualKey(vk, 0), 0 ,0);
  keybd_event(vk,MapVirtualKey(vk, 0), KEYEVENTF_KEYUP,0);
}
#endif