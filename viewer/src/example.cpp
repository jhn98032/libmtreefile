
#include "mainwindow.h"
#include <QScreen>


#if QT_VERSION < 0x050000
#include <QtGui/QApplication>
#endif
#include <QDir>
#include <QDebug>

#include "config.h"  


void center(QWidget &widget ,int WIDTH , int HEIGHT)
{
    int x, y;
    int screenWidth;
    int screenHeight;

    QScreen* screen = QGuiApplication::primaryScreen();
    if (!screen) 
    {
        qWarning() << "Unable to access the primary screen.";
    }
    
    QRect area = screen->geometry();
    screenWidth = area.width();
    screenHeight = area.height();

    x = (screenWidth - WIDTH) / 2;
    y = (screenHeight - HEIGHT) / 2;
 
    widget.setGeometry(x, y, WIDTH, HEIGHT);
}


int showGui(QApplication &app)
{
    MainWindow w(NULL);

    //center(w, 1200,600);

    if(app.arguments().size() >= 2)
    {
        w.load(app.arguments()[1]);
    }
    w.show();

    return app.exec();
}

int dumpUsage()
{
    printf("Usage: %s \n", PROGRAM_NAME);
    return 1;
}


        
/**
 * @brief Main program entry.
 */
int main(int argc, char *argv[])
{

    QApplication app(argc, argv);
    app.setStyle("cleanlooks");

    return showGui(app);
}
