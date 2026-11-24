// Read-only facts. No switching desktops, changing DWM, showing windows or input.
using System;
using System.Runtime.InteropServices;
using System.Text;

public sealed class CgpuiDx12DesktopReport {
    public uint ActiveConsoleSession;
    public bool RemoteSession;
    public bool CompositionEnabled;
    public int CompositionResult;
    public bool WorkAreaAvailable;
    public int WorkWidth;
    public int WorkHeight;
    public string InputDesktop;
    public int InputDesktopError;
}

public static class CgpuiDx12DesktopFacts {
    [StructLayout(LayoutKind.Sequential)]
    private struct Rect { public int Left, Top, Right, Bottom; }
    [DllImport("kernel32.dll")] private static extern uint WTSGetActiveConsoleSessionId();
    [DllImport("user32.dll")] private static extern int GetSystemMetrics(int index);
    [DllImport("dwmapi.dll")] private static extern int DwmIsCompositionEnabled(out bool enabled);
    [DllImport("user32.dll", SetLastError = true)]
    private static extern bool SystemParametersInfo(uint action, uint parameter, out Rect value, uint flags);
    [DllImport("user32.dll", SetLastError = true)]
    private static extern IntPtr OpenInputDesktop(uint flags, bool inherit, uint access);
    [DllImport("user32.dll", SetLastError = true, CharSet = CharSet.Unicode)]
    private static extern bool GetUserObjectInformation(IntPtr handle, int index, StringBuilder value,
                                                       uint length, out uint needed);
    [DllImport("user32.dll", SetLastError = true)]
    private static extern bool CloseDesktop(IntPtr handle);

    public static CgpuiDx12DesktopReport Capture() {
        var result = new CgpuiDx12DesktopReport();
        result.ActiveConsoleSession = WTSGetActiveConsoleSessionId();
        result.RemoteSession = GetSystemMetrics(0x1000) != 0;
        bool enabled;
        result.CompositionResult = DwmIsCompositionEnabled(out enabled);
        result.CompositionEnabled = enabled;
        Rect area;
        result.WorkAreaAvailable = SystemParametersInfo(0x0030, 0, out area, 0);
        result.WorkWidth = area.Right - area.Left;
        result.WorkHeight = area.Bottom - area.Top;
        // DESKTOP_READOBJECTS only: inspect the actual input desktop, never switch.
        var desktop = OpenInputDesktop(0, false, 0x0001);
        if (desktop == IntPtr.Zero) {
            result.InputDesktopError = Marshal.GetLastWin32Error();
            return result;
        }
        try {
            var name = new StringBuilder(256);
            uint needed;
            if (GetUserObjectInformation(desktop, 2, name, (uint)(name.Capacity * 2), out needed))
                result.InputDesktop = name.ToString();
            else result.InputDesktopError = Marshal.GetLastWin32Error();
        } finally { CloseDesktop(desktop); }
        return result;
    }
}
