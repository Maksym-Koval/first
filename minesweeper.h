#ifndef MINESWEEPER_H
#define MINESWEEPER_H

#include <QMainWindow>

#include "arena.h"

namespace Ui {
class MineSweeper;
}

class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
class QLCDNumber;
class QScrollArea;
class QFrame;
class QMediaPlayer;
class QLabel;
class QSpinBox;

class MArena;

class MineSweeper : public QMainWindow
{
    Q_OBJECT

public:
    explicit MineSweeper(QWidget *parent = 0);
    ~MineSweeper();

protected:
    virtual void showEvent(QShowEvent * event);
    virtual void resizeEvent(QResizeEvent * event);

private:
    Ui::MineSweeper *m_ui;

    QVBoxLayout* m_rightVBoxLayout;
    QVBoxLayout* m_leftVBoxLayout;
    QHBoxLayout* m_topHBoxLayout;
    QHBoxLayout* m_bottomFrameHBoxLayout;

    QPushButton* m_buttonExit;
    QPushButton* m_buttonStartGame;
    QPushButton* m_buttonSetArena;

    QLabel* m_LFieldsProcessed;
    QLCDNumber* m_lcdArenaProcessed;

    QLabel* m_LArenaSize;
    QSpinBox* m_SBArenaSize;
    QLabel* m_LMineCount;
    QSpinBox* m_SBMineCount;
    QFrame* m_settingsFrame;

    QScrollArea* m_scrollArea;
    QFrame* m_bottomFrame;
    QLabel* m_LGameResult;
    QLabel* m_LGameTime;

    QTimer* m_gameTimer;
    QTime m_gameTime;

    MArena* m_arena;

protected slots:
    void initArena();
    void startGame();
    void gameOver(const int gameResult);
    void updateGameTime();
    void scrollAreaVisibleRectChanged();
};

#endif // MINESWEEPER_H
