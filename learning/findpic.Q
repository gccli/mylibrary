[General]
SyntaxVersion=2
BeginHotkey=121
BeginHotkeyMod=0
PauseHotkey=0
PauseHotkeyMod=0
StopHotkey=123
StopHotkeyMod=0
RunOnce=1
EnableWindow=
MacroID=9f1e4eb3-6346-40bb-8296-5656d0eaa076
Description=findpic
Enable=0
AutoRun=0
[Repeat]
Type=0
Number=1
[SetupUI]
Type=2
QUI=
[Relative]
SetupOCXFile=
[Comment]

[Script]
Dim fso, out
Set fso = CreateObject("Scripting.FileSystemObject")
Set out = fso.OpenTextFile("D:\coordinate.txt", 2, True)

WindowW = 580
WindowH = 1040
CenterX = 264
CenterY = 480

Hwnd = Plugin.Window.Find("kaopu001_kpzsEmuFrame8", "靠谱助手 - 天天模拟器")
If Hwnd > 0 Then 
	Call Plugin.Window.Move(Hwnd, 0, 0)
	Delay 200
	Call Plugin.Window.Size(Hwnd, WindowW, WindowH)
	Delay 200
	Call Plugin.Window.Show(Hwnd)	
	Delay 200
	Call Plugin.Window.Top(Hwnd, 0)
	Delay 200
End If

Function GetCoordinate(x, y, xx, yy, debug)
	Dim Similarity
	
	MoveTo x, y
	Delay 500	
	
	Similarity = 0.70
	FindPic 204, 429, 330, 572, "D:\pic\si.bmp", Similarity, intX, intY
	If intX < 0 and intY < 0 Then 
		FindPic 204, 429, 330, 572, "D:\pic\si1.bmp", Similarity, intX, intY
	End If
	If intX < 0 and intY < 0 Then 
		FindPic 204, 429, 330, 572, "D:\pic\si2.bmp", Similarity, intX, intY
	End If	
	If intX < 0 and intY < 0 Then 
		FindPic 204, 429, 330, 572, "D:\pic\si3.bmp", Similarity, intX, intY
	End If		
	If intX < 0 and intY < 0 Then 
		Exit Function
	End If
	
	LeftClick 1
	Delay 750
	FindPic 255, 434, 308, 470, "D:\pic\lv60.bmp", Similarity, intX, intY
	Delay 750
	KeyPress "X", 1
	
	If intX > 0 and intY > 0 Then
		out.WriteLine (xx & " " & yy)
		If debug Then 
			MsgBox "找到60级的矿 " & xx & " " & yy, 0, "找矿"
		End If
	End If
End Function


Function InputX(x)
	Dim x0, x1, x2
	MoveTo 250, 72              '移动到x输入框
	Delay 100
	LeftClick 1	
	
	x0 = x \ 100
	x1 = (x - x0 * 100) \ 10
	x2 = (x - x0*100 - x1*10)
	' MsgBox "x " & x & " x0 " & x0 & " x1 " & x1 & " x2 " & x2
	
	If x0 > 0 Then 
		KeyPress CStr(x0), 1
	End If
	KeyPress CStr(x1), 1
	KeyPress CStr(x2), 1	

	KeyPress "Y", 1
	Delay 100	
End Function


Function InputY(y)
	Dim y0, y1, y2
	Dim yc

	MoveTo 370, 72              '移动到y输入框
	Delay 50
	LeftClick 1
	
	y0 = y \ 100
	y1 = (y - y0 * 100) \ 10
	y2 = (y - y0*100 - y1*10)

	' MsgBox "y " & y & " y0 " & y0 & " y1 " & y1 & " y2 " & y2
	If y0 > 0 Then 
		yc = Chr(y0  + 65)
		KeyPress CStr(yc), 1
	End If
	yc = Chr(y1  + 65)
	KeyPress CStr(yc), 1
	yc = Chr(y2  + 65)
	KeyPress CStr(yc), 1
	
	KeyPress "Z", 1
	Delay 50	
End Function


Function GotoCoordinate(xii, yii)
	Call InputX(xii)
	Call InputY(yii)
	
	MoveTo 464, 72               '查找
	LeftClick 1
	Delay 100
	
	Call GetCoordinate(CenterX, CenterY, xii, yii, 1)
End Function


' For Test
' si
Call GotoCoordinate(1, 245)
Delay 1000
Call GotoCoordinate(37, 282)
Delay 1000

/*
Call GotoCoordinate(131, 3)
Delay 1000

' diamond
Call GotoCoordinate(30, 262)
Delay 1000

' iron
Call GotoCoordinate(9, 49)
Delay 1000
Call GotoCoordinate(86, 4)
Delay 1000
Call GotoCoordinate(38, 257)
Delay 1000

' False
Call GotoCoordinate(38, 281)
'Call GetCoordinate(CenterX, CenterY, 131, 3, 1)

*/