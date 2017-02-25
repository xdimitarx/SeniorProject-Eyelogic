using UnityEngine;
using System;
using System.Threading;
using System.Runtime.InteropServices;

public class MouseControl : MonoBehaviour
{
    private const uint MOUSEEVENTF_LEFTDOWN = 0x02;
    private const uint MOUSEEVENTF_LEFTUP = 0x04;
    private const uint MOUSEEVENTF_RIGHTDOWN = 0x08;
    private const uint MOUSEEVENTF_RIGHTUP = 0x10;
    private const uint MOUSEEVENTF_ABSOLUTE = 0x8000;

    [System.Runtime.InteropServices.StructLayout(LayoutKind.Sequential)]
    public struct POINT
    {
        public int X;
        public int Y;
    }

    [DllImport("user32.dll")]
    public static extern bool SetCursorPos(int X, int Y);

    [DllImport("user32.dll")]
    public static extern bool GetCursorPos(out POINT pos);

    [DllImport("user32.dll")]
    static extern void mouse_event(uint dwFlags, uint dx, uint dy, uint dwData, UIntPtr dwExtraInfo);

    public static MouseControl handle;
    public static Resolution screenResolution;
    public static POINT pastLoc;

    //Starting from bottome left in 0-100%;
    public void MoveMousePercent(float x, float y)
    {
        int screenX = (int)Mathf.Round(x);
        int screenY = (int)Mathf.Round(y);
        if(screenX >= 0 && screenX <= 100 && screenY >= 0 && screenY <= 100)
        {
            SetCursorPos((int)Mathf.Round((float)screenX * screenResolution.width), (int)Mathf.Round((float)screenResolution.height - screenY * screenResolution.height));
        }
    }

    public void MoveMousePixel(int x, int y)
    {
        SetCursorPos(x, screenResolution.height - y);
    }

    public void click()
    {
        mouse_event(MOUSEEVENTF_LEFTDOWN,0,0,0,UIntPtr.Zero);
        mouse_event(MOUSEEVENTF_LEFTUP,0,0,0,UIntPtr.Zero);
    }

    public void startdrag()
    {
        mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, UIntPtr.Zero);
    }

    public void stopdrag()
    {
        mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, UIntPtr.Zero);
    }

    public void rightclick()
    {
        mouse_event(MOUSEEVENTF_RIGHTDOWN,0,0,0,UIntPtr.Zero);
        mouse_event(MOUSEEVENTF_RIGHTUP,0,0,0,UIntPtr.Zero);
    }

    private void Start()
    {
        handle = this;
        Application.runInBackground = true;
    }
}
