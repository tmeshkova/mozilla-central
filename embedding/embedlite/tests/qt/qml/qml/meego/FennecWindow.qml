import Qt 4.7
import QtMozilla 1.0
import QtQuick 1.1
import com.nokia.meego 1.0
import com.nokia.extras 1.0

QDeclarativeMozView {
    id: webViewport
    anchors.fill: parent
    
    signal urlChanged(string url)
    signal titleChanged(string title)
    signal holdOnUrl(string url)
    
    property string title: webViewport.child().title
    property string url: webViewport.child().url
    property string address: ""
    
    Connections {
	target: webViewport.child()
	onViewInitialized: {
	    print("QML View Initialized first");
	    if (address!="")
		webViewport.child().load(address);
	}
	onTitleChanged: {
	    print("onTitleChanged: " + webViewport.child().title)
	    titleChanged(webViewport.child().title)
	}
	onUrlChanged: {
	    print("onUrlChanged: " + webViewport.child().url)
	    urlChanged(webViewport.child().url)
	}
	onRecvAsyncMessage: {
	    print("onRecvAsyncMessage:" + message + ", data:" + data);
	    if (message == "LongTap:aHRef")
		holdOnUrl(data);
	}
	onRecvSyncMessage: {
	    print("onRecvSyncMessage:" + message + ", data:" + data);
	    if (message == "browser-element-api:get-fullscreen-allowed") {
		response.message = true;
	    } else if (message == "browser-element-api:get-name") {
		response.message = true;
	    }
	}
	onAlert: {
	    print("onAlert: title:" + title + ", msg:" + message + " winid:" + winid);
	    webViewport.visible = false;
	    alertDlg.winId = winid;
	    alertDlg.titleText = title;
	    alertDlg.message = message;
	    alertDlg.open();
	}
	onConfirm: {
	    print("onConfirm: title:" + title + ", msg:" + message);
	    webViewport.visible = false;
	    confirmDlg.winId = winid;
	    confirmDlg.titleText = title;
	    confirmDlg.message = message;
	    confirmDlg.open();
	}
	onPrompt: {
	    print("onPrompt: title:" + title + ", msg:" + message);
	    webViewport.visible = false;
	    promptDlg.winId = winid;
	    promptDlg.titleText = title;
	    promptDlg.messageText = message;
	    promptDlg.valueText = defaultValue;
	    promptDlg.open();
	}
	onAuthRequired: {
	    print("onAuthRequired: title:" + title + ", msg:" + message + ", winid:" + winid);
	    webViewport.visible = false;
	    authDlg.winId = winid;
	    authDlg.titleText = title;
	    authDlg.messageText = message;
	    authDlg.usernameText = defaultUsername;
	    authDlg.open();
	}
    }
    
    QueryDialog {
	id: alertDlg
	property int winId: 0
	acceptButtonText: "OK"
	
	onAccepted: {
	    webViewport.visible = true;
	    webViewport.child().unblockPrompt(winId, 0, true, "", "", "");
	}
    }
    
    QueryDialog {
	id: confirmDlg
	property int winId: 0
	acceptButtonText: "OK"
	rejectButtonText: "Cancel"
	
	onAccepted: {
	    webViewport.visible = true;
	    webViewport.child().unblockPrompt(winId, 0, true, "", "", "");
	}
	onRejected: {
	    webViewport.visible = true;
	    webViewport.child().unblockPrompt(winId, 0, false, "", "", "");
	}
    }
    
    Dialog {
	id: promptDlg
	property alias titleText: promptTitleString.text
	property alias messageText: promptMessageString.text
	property alias valueText: valueField.text
	property int winId: 0
	title: Label { 
	    text: "Prompt dialog"
	    anchors.verticalCenter: parent.verticalCenter
	    font.pixelSize: 30
	    color: "white"
	}
	content: Column {
	    width: parent.width
	    spacing: 5
	    
	    Item { height: 40; width: parent.width }
	    
	    Label { 
		id: promptTitleString
		color: "white"
	    }
	    
	    Item { height: 20; width: parent.width }
	    
	    Label {
		id: promptMessageString 
		color: "white"
	    }
	    
	    TextField {
		id: valueField
		width: parent.width
		placeholderText: "value"
		
		Keys.onReturnPressed: promptDlg.accept();
	    }
	    
	    Item { height: 40; width: parent.width }
	}
	buttons: Column {
	    anchors.verticalCenter: parent.verticalCenter
	    spacing: 5
	    
	    Button {text: "OK"; onClicked: promptDlg.accept(); }
	    Button {text: "Cancel"; onClicked: promptDlg.reject(); }
	}
	onAccepted: {
	    webViewport.visible = true;
	    webViewport.child().unblockPrompt(winId, 0, true, valueField.text, "", "");
	}
	onRejected: {
	    webViewport.visible = true;
	    webViewport.child().unblockPrompt(winId, 0, false, "", "", "");
	}
    }
    
    Dialog {
	id: authDlg
	property alias titleText: authTitleString.text
	property alias messageText: authMessageString.text
	property alias usernameText: usernameField.text
	property int winId: 0
	title: Label { 
	    text: "Authentication required"
	    anchors.verticalCenter: parent.verticalCenter
	    font.pixelSize: 30
	    color: "white"
	}
	content: Column {
	    width: parent.width
	    spacing: 5
	    
	    Item { height: 40; width: parent.width }
	    
	    Label { 
		id: authTitleString
		color: "white"
	    }
	    
	    Label { 
		id: authMessageString 
		color: "white"
	    }
	    
	    Item { height: 20; width: parent.width }
	    
	    Label {
		text: "Username"
		color: "white"
	    }
	    
	    TextField {
		id: usernameField
		width: parent.width
		placeholderText: "username"
		
		Keys.onReturnPressed: passwordField.focus = true;
	    }
	    
	    Label {
		text: "Password"
		color: "white"
	    }
	    
	    TextField {
		id: passwordField
		width: parent.width
		placeholderText: "password"
		echoMode: TextInput.Password
		
		Keys.onReturnPressed: authDlg.accept();
	    }
	    
	    Item { height: 40; width: parent.width }
	}
	buttons: Column {
	    anchors.verticalCenter: parent.verticalCenter
	    spacing: 5
	    
	    Button {text: "OK"; onClicked: authDlg.accept(); }
	    Button {text: "Cancel"; onClicked: authDlg.reject();}
	}
	onAccepted: {
	    webViewport.visible = true;
	    webViewport.child().unblockPrompt(winId, 0, true, "", usernameField.text, passwordField.text);
	}
	onRejected: {
	    webViewport.visible = true;
	    webViewport.child().unblockPrompt(winId, 0, false, "", "", "");
	}
    }
}
