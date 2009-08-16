#define KOFS 0x1000

#define XK_Escape VK_ESCAPE
#define XK_Tab VK_TAB
#define XK_F1 VK_F1+KOFS
#define XK_F2 VK_F2+KOFS
#define XK_F3 VK_F3+KOFS
#define XK_F4 VK_F4+KOFS
#define XK_F5 VK_F5+KOFS
#define XK_F6 VK_F6+KOFS
#define XK_F7 VK_F7+KOFS
#define XK_F8 VK_F8+KOFS
#define XK_F9 VK_F9+KOFS
#define XK_F10 VK_F10+KOFS
#define XK_F11 VK_F11+KOFS
#define XK_F12 VK_F12+KOFS
#define XK_Print VK_PRINT+KOFS
#define XK_Scroll_Lock VK_SCROLL
#define XK_Pause VK_PAUSE
#define XK_Delete VK_DELETE + KOFS
#define XK_End VK_END + KOFS
#define XK_BackSpace VK_BACK
#define XK_Insert VK_INSERT + KOFS
#define XK_Home VK_HOME + KOFS
#define XK_Prior VK_PRIOR + KOFS
#define XK_Next VK_NEXT + KOFS
#define XK_Caps_Lock VK_CAPITAL
#define XK_Return VK_RETURN
#define XK_Num_Lock VK_NUMLOCK
#define XK_Shift_L VK_LSHIFT + KOFS
#define XK_Shift_R VK_RSHIFT + KOFS
#define XK_Control_L VK_LCONTROL + KOFS
#define XK_Control_R VK_RCONTROL + KOFS
#define XK_Alt_L VK_LMENU + KOFS
#define XK_Alt_R VK_RMENU + KOFS
#define XK_Left VK_LEFT + KOFS
#define XK_Up VK_UP + KOFS
#define XK_Down VK_DOWN + KOFS
#define XK_Right VK_RIGHT + KOFS
#define XK_KP_0 (VK_NUMPAD0 + KOFS)
#define XK_KP_1 (VK_NUMPAD1 + KOFS)
#define XK_KP_2 (VK_NUMPAD2 + KOFS)
#define XK_KP_3 (VK_NUMPAD3 + KOFS)
#define XK_KP_4 (VK_NUMPAD4 + KOFS)
#define XK_KP_5 (VK_NUMPAD5 + KOFS)
#define XK_KP_6 (VK_NUMPAD6 + KOFS)
#define XK_KP_7 (VK_NUMPAD7 + KOFS)
#define XK_KP_8 (VK_NUMPAD8 + KOFS)
#define XK_KP_9 (VK_NUMPAD9 + KOFS)

#define XK_KP_Delete VK_DECIMAL + KOFS
#define XK_KP_Insert VK_NUMPAD0 + KOFS
#define XK_KP_End VK_NUMPAD1 + KOFS
#define XK_KP_Down VK_NUMPAD2 + KOFS
#define XK_KP_Next VK_NUMPAD3 + KOFS
#define XK_KP_Left VK_NUMPAD4 + KOFS
#define XK_KP_Begin VK_NUMPAD5 + KOFS
#define XK_KP_Right VK_NUMPAD6 + KOFS
#define XK_KP_Home VK_NUMPAD7 + KOFS
#define XK_KP_Up VK_NUMPAD8 + KOFS
#define XK_KP_Prior VK_NUMPAD9 + KOFS

#define XK_KP_Add VK_ADD + KOFS
#define XK_KP_Subtract VK_SUBTRACT + KOFS
#define XK_KP_Multiply VK_MULTIPLY + KOFS
#define XK_KP_Divide VK_DIVIDE + KOFS
#define XK_KP_Enter VK_SEPARATOR + KOFS
#define XK_KP_Decimal VK_DECIMAL + KOFS
#define XK_space ' '

#define ShiftMask		(1<<0)
#define LockMask		(1<<1)
#define ControlMask		(1<<2)
#define Mod1Mask		(1<<3)  // alt
#define Mod2Mask		(1<<4)
#define Mod3Mask		(1<<5)
#define Mod4Mask		(1<<6)  // windows
#define Mod5Mask		(1<<7)  // alt
