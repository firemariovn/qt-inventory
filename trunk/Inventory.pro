#-------------------------------------------------
#
# Project created by QtCreator 2011-05-25T12:57:03
#
#-------------------------------------------------

QT       += core gui sql
RC_FILE  = Inventory.rc

TARGET = Inventory
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    dbconnect.cpp \
    tableform.cpp \
    inventorytableview.cpp \
    rights.cpp \
    users.cpp \
    items.cpp \
    allocations.cpp \
    headerview.cpp \
    changepassword.cpp \
    inventorycombobox.cpp \
    logger.cpp \
    settings.cpp \
    inventorylineedit.cpp \
    browser.cpp \
    connectionwidget.cpp \
    checkdelegate.cpp \
    scrap.cpp

HEADERS  += mainwindow.h \
    dbconnect.h \
    tableform.h \
    inventorytableview.h \
    rights.h \
    users.h \
    items.h \
    version.h \
    allocations.h \
    headerview.h \
    changepassword.h \
    inventorycombobox.h \
    logger.h \
    settings.h \
    inventorylineedit.h \
    browser.h \
    connectionwidget.h \
    checkdelegate.h \
    scrap.h \
    interface.h

FORMS    += mainwindow.ui \
    dbconnect.ui \
    tableform.ui \
    rights.ui \
    users.ui \
    items.ui \
    allocations.ui \
    changepassword.ui \
    settings.ui \
    browserwidget.ui \
    scrap.ui

OTHER_FILES += \
    icons/3.png \
    icons/2.png \
    icons/1.png \
    icons/0.png \
    icons/8.png \
    icons/7.png \
    icons/6.png \
    icons/5.png \
    icons/4.png \
    icons/Application.png \
    icons/Ampersand.png \
    icons/Alarme.png \
    icons/9.png \
    icons/Arrow1 Left.png \
    icons/Arrow1 DownRight.png \
    icons/Arrow1 DownLeft.png \
    icons/Arrow1 Down.png \
    icons/Applications.png \
    icons/Arrow2 Down.png \
    icons/Arrow1 UpRight.png \
    icons/Arrow1 UpLeft.png \
    icons/Arrow1 Up.png \
    icons/Arrow1 Right.png \
    icons/Arrow2 Up.png \
    icons/Arrow2 Right.png \
    icons/Arrow2 Left.png \
    icons/Arrow2 DownRight.png \
    icons/Arrow2 DownLeft.png \
    icons/Arrow3 Left.png \
    icons/Arrow3 Down.png \
    icons/Arrow2 UpRight.png \
    icons/Arrow2 UpLeft.png \
    icons/Back Top.png \
    icons/Audio Message.png \
    icons/Attach.png \
    icons/Arrow3 Up.png \
    icons/Arrow3 Right.png \
    icons/Calc.png \
    icons/Burn.png \
    icons/Bubble 3.png \
    icons/Bubble 1.png \
    icons/Back.png \
    icons/Card2.png \
    icons/Card1.png \
    icons/Car.png \
    icons/Cancel.png \
    icons/Calendar.png \
    icons/Cart2.png \
    icons/Cart.png \
    icons/Card4.png \
    icons/Card3.png \
    icons/Clipboard Paste.png \
    icons/Clipboard Cut.png \
    icons/Clipboard Copy.png \
    icons/Cd.png \
    icons/Contact.png \
    icons/Computer.png \
    icons/Clock.png \
    icons/Currency Dollar.png \
    icons/Cube.png \
    icons/Copyright.png \
    icons/Direction Diag2.png \
    icons/Direction Diag1.png \
    icons/Database.png \
    icons/Currency Pound.png \
    icons/Currency Euro.png \
    icons/Document New.png \
    icons/Discuss.png \
    icons/Directions.png \
    icons/Direction Vert.png \
    icons/Direction Horz.png \
    icons/Dots Up.png \
    icons/Dots Down.png \
    icons/Document2.png \
    icons/Document.png \
    icons/Fbook.png \
    icons/Exclamation.png \
    icons/Email.png \
    icons/Download.png \
    icons/Dots.png \
    icons/Folder2.png \
    icons/Folder.png \
    icons/Flag.png \
    icons/Forbidden.png \
    icons/Footprint.png \
    icons/Folder3.png \
    icons/Globe.png \
    icons/Gear.png \
    icons/Game.png \
    icons/Full Size.png \
    icons/Full Screen.png \
    icons/Hdd Network.png \
    icons/Hand.png \
    icons/Graph.png \
    icons/Go Out.png \
    icons/Go In.png \
    icons/Home.png \
    icons/Heart.png \
    icons/Health.png \
    icons/Hdd.png \
    icons/Ipod.png \
    icons/Info2.png \
    icons/Info.png \
    icons/Home2.png \
    icons/Link.png \
    icons/Light.png \
    icons/Key.png \
    icons/Loop.png \
    icons/Lock.png \
    icons/Lock Open.png \
    icons/Microphone.png \
    icons/Man.png \
    icons/Mail.png \
    icons/Luggage.png \
    icons/Movie.png \
    icons/Mouse.png \
    icons/Mobile.png \
    icons/Minus.png \
    icons/Paragraph.png \
    icons/Ok.png \
    icons/Nuke.png \
    icons/Music2.png \
    icons/Music.png \
    icons/Picture.png \
    icons/Photo.png \
    icons/Phone.png \
    icons/Percent.png \
    icons/Player Pause.png \
    icons/Player Next.png \
    icons/Player FastRev.png \
    icons/Player FastFwd.png \
    icons/Player Eject.png \
    icons/Plus.png \
    icons/Player Stop.png \
    icons/Player Record.png \
    icons/Player Previous.png \
    icons/Player Play.png \
    icons/Puzzle.png \
    icons/Printer.png \
    icons/Poll.png \
    icons/Pointer.png \
    icons/Podcast.png \
    icons/Rss 1.png \
    icons/Refresh.png \
    icons/Reduced Size.png \
    icons/Question.png \
    icons/Security.png \
    icons/Search.png \
    icons/Screen.png \
    icons/Save.png \
    icons/Rss 2.png \
    icons/Size Vert.png \
    icons/Size Horz.png \
    icons/Size Diag2.png \
    icons/Size Diag1.png \
    icons/Sitemap.png \
    icons/Sound Minus.png \
    icons/Smiley3.png \
    icons/Smiley2.png \
    icons/Smiley1.png \
    icons/Sleep.png \
    icons/Star.png \
    icons/Standby.png \
    icons/Sound Plus.png \
    icons/Sound On.png \
    icons/Sound Off.png \
    icons/Table.png \
    icons/Stats2.png \
    icons/Stats.png \
    icons/Stats 3.png \
    icons/Start.png \
    icons/Text Large.png \
    icons/Target.png \
    icons/Tape.png \
    icons/Tag.png \
    icons/Thumb Down.png \
    icons/Text Small.png \
    icons/Text Plus.png \
    icons/Text Minus.png \
    icons/Text Meduim.png \
    icons/Trackback.png \
    icons/Tool2.png \
    icons/Tool.png \
    icons/Thumb Up.png \
    icons/User.png \
    icons/Tv.png \
    icons/Tree.png \
    icons/Travel.png \
    icons/Trash.png \
    icons/Weather Could Sun.png \
    icons/Weather Cloud.png \
    icons/Warning.png \
    icons/Wait.png \
    icons/Video.png \
    icons/Wizard.png \
    icons/Weather Sun.png \
    icons/Weather Snow.png \
    icons/Weather Rain.png \
    icons/Write2.png \
    icons/Write.png \
    icons/Wordpress.png \
    icons/Woman.png \
    icons/Zoom Out.png \
    icons/Zoom In.png \
    icons/Write3.png \
    ../Inventory/store.db \
    Inventory.rc \
    Inventory.ico \
    inventory.sqlite \
    Inventory_bg.ts \
    Inventory_bg.qm \
    Inventory_en.ts \
    Inventory_en.qm

RESOURCES += \
    resources.qrc























