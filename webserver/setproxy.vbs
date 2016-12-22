strTitle = "Answer Box"

strMsg = vbCR
strMsg = strMsg & "Enter D for Disable Proxy" & vbCR
strMsg = strMsg & "Enter A for AutoConfigURL mode" & vbCR
strMsg = strMsg & "Enter S for ProxyServer mode"

mode = InputBox(strMsg,"Make your selection")
mode = UCase(mode)


ProxyServer = objShell.RegRead(ProxyServerLoc)
ProxyPACURL = objShell.RegRead(ProxyPACLoc)

Select Case mode
    Case "A"
        UserIn = Inputbox("Enter the Proxy AutoConfig URL", "Proxy Server Required", ProxyPACURL)
	objShell.RegWrite ProxyPACLoc,UserIn,"REG_SZ"
	objShell.RegWrite ProxyServerEnable,"1","REG_DWORD"
	MsgBox "Proxy is Enabled"
    Case "S"
        UserIn = Inputbox("Enter the Proxy server", "Proxy Server Required", ProxyServer)
	objShell.RegWrite ProxyServerLoc,UserIn,"REG_SZ"
	objShell.RegWrite ProxyServerEnable,"1","REG_DWORD"
	MsgBox "Proxy is Enabled"

    Case "D"
	objShell.RegWrite ProxyServerEnable,"0","REG_DWORD"
	MsgBox "Proxy is Disabled"
    Case Else
        MsgBox "Do Nothing!"
End Select



WScript.Quit



YesNo =  MsgBox("Enable System Proxy?", vbYesNoCancel, "Enable Proxy")
If YesNo=vbYes Then
    Dim aOpt(2)
    aOpt(0) = "Using ProxyServer"
    aOpt(1) = "Using Proxy PAC URL"

    Sel = SelectBox("Select which mode: ", aOpt)

    ProxyServer = objShell.RegRead(ProxyServerLoc)
	UserIn = Inputbox("Enter the Proxy server", "Proxy Server Required", ProxyServer)
	objShell.RegWrite ProxyServerLoc,UserIn,"REG_SZ"
	objShell.RegWrite ProxyServerEnable,"1","REG_DWORD"
	MsgBox "Proxy is Enabled"
ElseIf YesNo=vbNo Then
	objShell.RegWrite ProxyServerEnable,"0","REG_DWORD"
	MsgBox "Proxy is Disabled"
Else
	MsgBox "Do nothing"
End If
WScript.Quit