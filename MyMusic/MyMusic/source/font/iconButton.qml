import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Window {
    width: 400
    height: 300
    visible: true
    title: "Iconfont按钮控件示例"
    color: "#f5f7fa"

    // 加载字体
    FontLoader {
        id: iconFont
        source: "./iconfont.ttf" // 替换为你的字体文件路径
    }

    // Iconfont按钮控件
    Component {
        id: iconButtonComponent
        
        Rectangle {
            id: buttonRoot
            property string iconCode: "\ue65b" // 默认图标
            property string buttonText: "按钮"
            property color buttonColor: "#4a6cf7"
            property color hoverColor: Qt.darker(buttonColor, 1.1)
            property color pressColor: Qt.darker(buttonColor, 1.2)
            property int iconSize: 20
            property int borderRadius: 8
            
            signal clicked()
            
            implicitWidth: 150
            implicitHeight: 50
            radius: borderRadius
            color: mouseArea.pressed ? pressColor : 
                   mouseArea.containsMouse ? hoverColor : buttonColor
            
            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 12
                
                // Iconfont图标
                Text {
                    id: icon
                    text: iconCode
                    font.family: iconFont.name
                    font.pixelSize: buttonRoot.iconSize
                    color: "white"
                    Layout.alignment: Qt.AlignVCenter
                }
                
                // 按钮文本
                Text {
                    text: buttonRoot.buttonText
                    font.pixelSize: 16
                    font.weight: Font.Medium
                    color: "white"
                    Layout.alignment: Qt.AlignVCenter
                    Layout.fillWidth: true
                }
            }
            
            // 鼠标区域
            MouseArea {
                id: mouseArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    buttonRoot.clicked();
                    
                    // 点击动画
                    scaleAnimation.start();
                }
            }
            
            // 点击动画
            ScaleAnimator {
                id: scaleAnimation
                target: buttonRoot
                from: 1
                to: 0.95
                duration: 100
                easing.type: Easing.OutCubic
                
                onFinished: {
                    resetScale.start();
                }
            }
            
            ScaleAnimator {
                id: resetScale
                target: buttonRoot
                from: 0.95
                to: 1
                duration: 100
                easing.type: Easing.OutCubic
            }
            
            // 悬停动画
            Behavior on color {
                ColorAnimation {
                    duration: 200
                    easing.type: Easing.OutCubic
                }
            }
        }
    }

    // 主界面
    ColumnLayout {
        anchors.centerIn: parent
        spacing: 20
        
        // 标题
        Text {
            text: "Iconfont按钮控件"
            font.pixelSize: 24
            font.bold: true
            color: "#1d2b3a"
            Layout.alignment: Qt.AlignHCenter
        }
        
        // 按钮1
        Loader {
            sourceComponent: iconButtonComponent
            onLoaded: {
                item.iconCode = "\ue88a";
                item.buttonText = "主页";
                item.buttonColor = "#4a6cf7";
                item.clicked.connect(function() {
                    console.log("主页按钮被点击");
                });
            }
        }
        
        // 按钮2
        Loader {
            sourceComponent: iconButtonComponent
            onLoaded: {
                item.iconCode = "\ue87d";
                item.buttonText = "收藏";
                item.buttonColor = "#27ae60";
                item.clicked.connect(function() {
                    console.log("收藏按钮被点击");
                });
            }
        }
        
        // 按钮3
        Loader {
            sourceComponent: iconButtonComponent
            onLoaded: {
                item.iconCode = "\ue65b";
                item.buttonText = "搜索";
                item.buttonColor = "#e74c3c";
                item.clicked.connect(function() {
                    console.log("搜索按钮被点击");
                });
            }
        }
        
        // 按钮4
        Loader {
            sourceComponent: iconButtonComponent
            onLoaded: {
                item.iconCode = "\ue7fd";
                item.buttonText = "个人中心";
                item.buttonColor = "#9b59b6";
                item.clicked.connect(function() {
                    console.log("个人中心按钮被点击");
                });
            }
        }
    }
}
