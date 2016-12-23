Option Explicit

Dim ProxyPACURL, ProxyPACLoc 
Dim ProxyServer, ProxyServerLoc, ProxyServerEnableLoc
Dim YesNo, ProxyMode, strMsg
Dim UserIn
Dim objShell

Set objShell = WScript.CreateObject("WScript.Shell")
On Error Resume Next

ProxyPACLoc = "HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Internet Settings\AutoConfigURL"
ProxyServerLoc = "HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Internet Settings\ProxyServer"
ProxyServerEnableLoc = "HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Internet Settings\ProxyEnable"

strMsg = vbCR
strMsg = strMsg & "Enter D for Disable Proxy" & vbCR
strMsg = strMsg & "Enter A for AutoConfigURL mode" & vbCR
strMsg = strMsg & "Enter S for ProxyServer mode" 

ProxyMode = InputBox(strMsg,"Make your selection")
ProxyMode = UCase(ProxyMode)

ProxyServer = objShell.RegRead(ProxyServerLoc)
ProxyPACURL = objShell.RegRead(ProxyPACLoc)

Select Case ProxyMode
    Case "A"        
        UserIn = Inputbox("Enter the Proxy AutoConfig URL", "PAC URL Required", ProxyPACURL)
        If UserIn = "" Then
             WScript.Quit
        End If
        YesNo = Msgbox("Your PAC URL is " & UserIn & ", Are you sure?", vbYesNo+vbInformation, "")
        If YesNo = vbYes Then
            objShell.RegWrite ProxyPACLoc,UserIn,"REG_SZ"
            MsgBox "Proxy AutoConfig is Enabled"
        End If

    Case "S"
        UserIn = Inputbox("Enter the Proxy server", "Proxy Server Required", ProxyServer)
        If UserIn = "" Then
             WScript.Quit
        End If
        YesNo = Msgbox("Your Proxy Server is " & UserIn & ", Are you sure?", vbYesNo+vbInformation, "")
        If YesNo = vbYes Then
            objShell.RegWrite ProxyServerLoc,UserIn,"REG_SZ"
            objShell.RegWrite ProxyServerEnableLoc,"1","REG_DWORD"
            MsgBox "Proxy Server is Enabled"
        End If

    Case "D"
        objShell.RegDelete ProxyPACLoc
        objShell.RegWrite ProxyServerEnableLoc,"0","REG_DWORD"
        MsgBox "Proxy is Disabled"

    Case Else
        MsgBox "Do Nothing!"
End Select

WScript.Quit

