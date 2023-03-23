#!/usr/bin/python3
# -*- coding: utf-8 -*-

import sys
import socket
import time
import pypinyin
import distance
import threading
from PyQt5 import QtWidgets
from PyQt5.QtGui import QPainter, QColor, QFont, QTextOption, QTextCursor
from PyQt5.QtCore import Qt
from PyQt5 import QtCore

txtcont = open("/home/leohearts/Downloads/text.txt", 'r').read()
pinyinLocMap = {}


def defferedJumpAction(object, target):
    best = {
        "id": 0,
        "d": 999999
    }

    for key in pinyinLocMap:
        to_compare = pinyinLocMap[key]
        d = distance.levenshtein(target, to_compare)    # 编辑距离
        if d < best['d']:
            best['d'] = d
            best['id'] = key

    print(best, pinyinLocMap[best['id']])
    if best['d'] > 14:
        print("Skipping...")
        return
    txtPos = best['id']
    cur = QTextCursor(object.document().findBlockByLineNumber(0))
    cur.setPosition(min(len(txtcont) - 1, txtPos + 50))
    object.setTextCursor(cur)

def handleConn(conn, object):
    buff = []
    while (True):
        curr = conn.recv(1024).decode('utf8')
        print(curr)
        buff += pypinyin.lazy_pinyin(curr, neutral_tone_with_five=True, errors='ignore')
        if (len(buff) >= 25):
            buff = buff[-20:]
            threading.Thread(target=defferedJumpAction, args=(object, buff)).start()
    

def startListenForContent(object):
    s = socket.socket()         # 创建 socket 对象
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    host = "127.0.0.1"
    port = 8964                # 设置端口
    s.bind((host, port))        # 绑定端口
    s.listen(5)                 # 等待客户端连接
    while True:
        c, addr = s.accept()     # 建立客户端连接
        print("Accepting from " + str(addr) + "...")
        threading.Thread(target=handleConn, args=(c, object)).start()


class MainWindow(QtWidgets.QMainWindow):

    def __init__(self):
        super().__init__()
        self.setAttribute(Qt.WA_TranslucentBackground, True)
        self.initUI()

    def initUI(self):
        # Scroll Area which contains the widgets, set as the centralWidget
        self.scroll = QtWidgets.QScrollArea()
        # Widget that contains the collection of Vertical Box
        self.widget = QtWidgets.QWidget()

        # The Vertical Box that contains the Horizontal Boxes of  labels and buttons
        self.vbox = QtWidgets.QHBoxLayout()

        object = QtWidgets.QTextEdit()
        font = QFont()
        font.setPointSize(32)
        object.setFont(font)
        object.setTextColor(QColor(255, 255, 255))
        object.setAcceptRichText(False)
        object.setPlainText(txtcont)
        object.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOff)

        threading.Thread(target=startListenForContent, args=(object,)).start()

        object.setWordWrapMode(QTextOption.WrapMode.WrapAnywhere)
        self.vbox.addWidget(object)

        self.widget.setLayout(self.vbox)

        # Scroll Area Properties
        self.scroll.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.scroll.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.scroll.setWidgetResizable(True)
        self.scroll.setWidget(self.widget)
        object.setStyleSheet("background-color: rgba(0,0,0,0);")
        self.scroll.setStyleSheet("background-color: rgba(0,0,0,0);")

        self.setCentralWidget(self.scroll)

        self.setGeometry(600, 100, 800, 400)
        self.setWindowTitle('Scroll Area Demonstration')

        return


class TextCont(QtWidgets.QWidget):

    def __init__(self):
        self.lastTime = time.time()
        self.color = 10
        super().__init__()
        threading.Thread(target=self.getUpdate)
        self.initUI()

    def getUpdate(self):
        pass

    def initUI(self):
        self.setAttribute(Qt.WA_TranslucentBackground, True)
        self.setStyleSheet("background-color: yellow;")
        self.text = u"qwq 这是一段测试文字"

        self.setGeometry(300, 300, 280, 170)
        self.setWindowTitle('Drawing text')
        # self.show()

    def paintEvent(self, event):
        # print(self.text)
        qp = QPainter()

        qp.begin(self)
        self.drawText(event, qp)
        qp.end()

    def drawText(self, event, qp):
        qp.setPen(QColor(255, 10, 10))
        qp.setFont(QFont('dfgirl', 30))
        qp.drawText(event.rect(), Qt.AlignBottom, self.text)


if __name__ == '__main__':


    # preprocess
    for i in range(0, len(txtcont), 20):
        currBlock = pypinyin.lazy_pinyin(txtcont[i:min(len(txtcont)-1,i+20)], neutral_tone_with_five=True, errors='ignore')

        if len(currBlock) < 10:
            continue
        pinyinLocMap[i] = currBlock

    app = QtWidgets.QApplication(sys.argv)
    ex = MainWindow()
    ex.show()

    sys.exit(app.exec_())
