#pragma once

#include <QFrame>
#include <QVector>
#include <QTime>

class QPixmap;

struct ArenaFieldState
{
    int mine;//-1|0|>0
    bool opened;
    bool flag;
    QString text;
};

struct FieldPoint
{
    int row;
    int column;
};

class MArena : public QFrame
{
    Q_OBJECT

public:
    MArena(QWidget *parent = 0, const quint8 buttonSize = 40);
    ~MArena();
    void setArena(const quint16 fieldsCount = 10, const quint16 minesCount = 10);
    void setArenaEnabled(const bool state);
    void setArenaVisibleRect(const QRect& rect);

protected:
    virtual void paintEvent(QPaintEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent * event);

    void openMines();
    void openFields(const int row, const int column);
    void checkGameOverWin();

private:
    quint8 m_buttonMargin;
    quint8 m_buttonSize;
    quint16 m_fieldsCount;
    quint16 m_minesCount;
    bool m_arenaEnabled;
    int m_sumFieldsProcessed;
    int m_fieldsTotal;

    int m_firstVisibleColumn;
    int m_lastVisibleColumn;
    int m_firstVisibleRow;
    int m_lastVisibleRow;

    QVector<FieldPoint> m_neighborMap;

    QPixmap* m_flag;
    QPixmap* m_mine;

    QVector< QVector<ArenaFieldState> > m_arenaFieldsState;

signals:
    void fieldsProcessed (const qint32 count);
    void gameOver(const int gameRusult);
};


