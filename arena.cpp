#include "arena.h"

#include <QtMath>
#include <QtDebug>
#include <QPainter>
#include <QApplication>
#include <QPaintEvent>
#include <QThread>

MArena::MArena(QWidget *parent, const quint8 buttonSize):
    QFrame(parent)
{
    m_buttonMargin = 8;
    m_buttonSize = buttonSize;
    m_arenaEnabled = false;

    m_neighborMap.resize(0);
    m_neighborMap.append({-1,0});
    m_neighborMap.append({1,0});
    m_neighborMap.append({0,-1});
    m_neighborMap.append({0,1});
    m_neighborMap.append({-1,-1});
    m_neighborMap.append({-1,1});
    m_neighborMap.append({1,-1});
    m_neighborMap.append({1,1});

    m_flag = new QPixmap(m_buttonSize, m_buttonSize);
    m_flag->fill(Qt::transparent);
    QPainter painter;
    painter.begin(m_flag);
    int cx = m_flag->width() / 3;
    int cy = m_flag->height() / 2;
    painter.fillRect(QRect(cx, cy/2, 1.5*cx, cx), Qt::red);
    painter.setPen(Qt::black);
    painter.drawLine(QPoint(cx,cy/2), QPoint(cx, 3*cy/2));
    painter.drawLine(QPoint(cx+1,cy/2), QPoint(cx+1, 3*cy/2));
    painter.end();

    m_mine = new QPixmap (m_buttonSize, m_buttonSize);
    m_mine->fill(Qt::transparent);
    painter.begin(m_mine);
    painter.setBrush(Qt::black);
    painter.setPen(Qt::black);
    painter.drawEllipse(QPoint(cy,cy), cx, cx);
    painter.end();


    setFrameStyle(QFrame::Box | QFrame::Raised);
    setLineWidth(2);
    setPalette(QPalette(Qt::cyan));
    setAutoFillBackground(true);

//    setArena();
}

MArena::~MArena()
{
    if (!m_flag) delete m_flag;
    if (!m_mine) delete m_mine;
}

void MArena::setArena(const quint16 fieldsCount, const quint16 minesCount)
{
    m_fieldsCount = fieldsCount;
    m_minesCount = minesCount;
    m_sumFieldsProcessed = 0;
    m_fieldsTotal = m_fieldsCount * m_fieldsCount;

    setFixedSize(2*m_buttonMargin+m_fieldsCount*(m_buttonSize), 2*m_buttonMargin+m_fieldsCount*(m_buttonSize));

    int i, ii, j, jj, row, column, sum;

    m_arenaFieldsState.resize(m_fieldsCount);
    for (i=0; i<m_arenaFieldsState.size(); i++)
        m_arenaFieldsState[i].fill({0, false, false, QLatin1String("")}, m_fieldsCount);

    QTime midnight(0,0,0);
    qsrand(midnight.secsTo(QTime::currentTime()));
    i=0;
    while (i<m_minesCount)
    {
        row = qrand() % m_fieldsCount;
        column = qrand() % m_fieldsCount;
        if (!m_arenaFieldsState[row][column].mine)
        {
            m_arenaFieldsState[row][column].mine = -1;
            i++;
        }
    }

    for (i=0; i<m_arenaFieldsState.size(); i++)
    {
        for (j=0; j<m_arenaFieldsState[i].size(); j++)
        if (!m_arenaFieldsState[i][j].mine)
        {
            sum = 0;
            for (ii=i-1; ii<=i+1; ii++)
            for (jj=j-1; jj<=j+1; jj++)
            if ( (ii > -1) && (ii < m_fieldsCount)
                 && (jj > -1) && (jj < m_fieldsCount)
                ) if (m_arenaFieldsState[ii][jj].mine == -1) ++sum;
            m_arenaFieldsState[i][j].mine = sum;

//??????????????

//Setting text for all fields (10000 x 10000 for example) makes the application more slower
//but it is not main.
//In any time this simply fails the application without any messages
            if (sum) //Comment this line for looking the problem
                m_arenaFieldsState[i][j].text.setNum(sum);
        }
        if (!(i % 10))
        {emit fieldsProcessed((i+1) * m_fieldsCount); QApplication::processEvents();}
    }
    emit fieldsProcessed(m_fieldsTotal); QApplication::processEvents();

    update();
}

void MArena::setArenaEnabled(const bool state)
{
    m_arenaEnabled = state;
}

void MArena::paintEvent(QPaintEvent *e)
{
    QFrame::paintEvent(e);

    QPainter painter;
    painter.begin(this);
    painter.setPen(Qt::black);

    int firstColumn = (e->rect().left()-m_buttonMargin) / m_buttonSize;
    if (firstColumn<0) firstColumn = 0;
    int lastColumn = ((e->rect().right()-m_buttonMargin) / m_buttonSize );
    if (lastColumn>=m_fieldsCount) lastColumn = m_fieldsCount-1;
    int firstRow = (e->rect().top()-m_buttonMargin) / m_buttonSize;
    if (firstRow<0) firstRow = 0;
    int lastRow = ((e->rect().bottom()-m_buttonMargin) / m_buttonSize );
    if (lastRow>=m_fieldsCount) lastRow = m_fieldsCount-1;

    for (int i=firstRow; i<=lastRow; i++)
    for (int j=firstColumn; j<=lastColumn; j++)
    {
        QRect fieldRect = QRect(m_buttonMargin + j*m_buttonSize, m_buttonMargin + i*m_buttonSize, m_buttonSize, m_buttonSize);

        if (m_arenaFieldsState[i][j].opened)
        {
            painter.setBrush(Qt::cyan);
            painter.drawRect(fieldRect);
            if (m_arenaFieldsState[i][j].mine == -1)
                painter.drawPixmap(fieldRect, *m_mine);
            if (m_arenaFieldsState[i][j].mine>0)
                painter.drawText(fieldRect, Qt::AlignCenter, m_arenaFieldsState[i][j].text);
        }
        else
        {
            painter.setBrush(Qt::darkCyan);
            painter.drawRect(fieldRect);
            if (m_arenaFieldsState[i][j].flag)
                painter.drawPixmap(fieldRect, *m_flag);
        }

    }
    painter.end();

//    qDebug() << "Paint Event | " << QTime::currentTime()
//             << firstRow << " | " << lastRow << " | "
//             << firstColumn << " | " << lastColumn << " | ";
}

void MArena::mouseReleaseEvent(QMouseEvent * event)
{
    QFrame::mouseReleaseEvent(event);

    if ( (m_arenaEnabled)
         && (event->x() > m_buttonMargin) && (event->x() < width()-m_buttonMargin)
         && (event->y() > m_buttonMargin) && (event->y() < height()-m_buttonMargin) )
    {
        int column = (event->x()-m_buttonMargin) / m_buttonSize ;
        int row = (event->y()-m_buttonMargin) / m_buttonSize;

        if (m_arenaFieldsState[row][column].opened) return;

        if (event->button() == Qt::RightButton)
        {
            m_arenaFieldsState[row][column].flag = !m_arenaFieldsState[row][column].flag;
            update(QRect(
                m_buttonMargin + column*m_buttonSize,
                m_buttonMargin + row*m_buttonSize,
                m_buttonSize, m_buttonSize
                ));
            return;
        }

        if ((event->button() == Qt::LeftButton) && (!m_arenaFieldsState[row][column].flag))
        {
            m_arenaFieldsState[row][column].opened = true;
            update(QRect(
                m_buttonMargin + column*m_buttonSize,
                m_buttonMargin + row*m_buttonSize,
                m_buttonSize, m_buttonSize
                ));

            emit fieldsProcessed(++m_sumFieldsProcessed); QApplication::processEvents();

            if (m_arenaFieldsState[row][column].mine == -1)
            {
                openMines();
                emit gameOver(0);
                return;
            }
            if (!m_arenaFieldsState[row][column].mine)
            {
                m_arenaEnabled = false;
                openFields(row, column);
                m_arenaEnabled = true;
            }
            checkGameOverWin();
        }       
    }
}

void MArena::openMines()
{
    for (int i=0; i<m_fieldsCount; i++)
    for (int j=0; j<m_fieldsCount; j++)
    if (m_arenaFieldsState[i][j].mine == -1)
    {
        m_arenaFieldsState[i][j].opened = true;

        if (  (i >= m_firstVisibleRow) && (i <= m_lastVisibleRow)
            &&(j >= m_firstVisibleColumn) && (j <= m_lastVisibleColumn) )
        update(QRect(
            m_buttonMargin + j*m_buttonSize,
            m_buttonMargin + i*m_buttonSize,
            m_buttonSize, m_buttonSize
            ));
    }
}

void MArena::openFields(const int row, const int column)
{
    int i, j, r, c;

    QVector<FieldPoint> newOpened(1, {row,column});
    QVector<FieldPoint> oldOpened;

    while (!newOpened.isEmpty())
    {
        oldOpened.resize(0);
        newOpened.swap(oldOpened);

        for (i=0; i<oldOpened.size(); i++)
        for (j=0; j<m_neighborMap.size(); j++)
        {
            r = oldOpened[i].row + m_neighborMap[j].row;
            c = oldOpened[i].column + m_neighborMap[j].column;

            if (    (r > -1) && (r < m_fieldsCount)
                 && (c > -1) && (c < m_fieldsCount) )
//Separately checking the conditions to avoid possible rangecheck error
               if (   (!m_arenaFieldsState[r][c].opened)
                    &&(m_arenaFieldsState[r][c].mine > -1) )
               {
                   m_arenaFieldsState[r][c].opened = true;
                   if (!m_arenaFieldsState[r][c].mine)
                       newOpened.append({r,c});
                   else ++m_sumFieldsProcessed;
//Cut unvisible updates
                   if (  (r >= m_firstVisibleRow) && (r <= m_lastVisibleRow)
                       &&(c >= m_firstVisibleColumn) && (c <= m_lastVisibleColumn) )
                   update(QRect(
                       m_buttonMargin + c*m_buttonSize,
                       m_buttonMargin + r*m_buttonSize,
                       m_buttonSize, m_buttonSize
                       ));
//                   else qDebug() << "Skip Update for " << r << " | " << c;
               }
        }

        m_sumFieldsProcessed += newOpened.size();
        emit fieldsProcessed(m_sumFieldsProcessed); QApplication::processEvents();
    }
}

void MArena::checkGameOverWin()
{
    if ( (m_fieldsTotal - m_sumFieldsProcessed) > m_minesCount) return;

    for (int i=0; i<m_fieldsCount; i++)
    for (int j=0; j<m_fieldsCount; j++)
    if (m_arenaFieldsState[i][j].mine == -1)
    {
        m_arenaFieldsState[i][j].flag = true;

        if (  (i >= m_firstVisibleRow) && (i <= m_lastVisibleRow)
            &&(j >= m_firstVisibleColumn) && (j <= m_lastVisibleColumn) )
        update(QRect(
            m_buttonMargin + j*m_buttonSize,
            m_buttonMargin + i*m_buttonSize,
            m_buttonSize, m_buttonSize
            ));
    }

    emit gameOver(1);
}

void MArena::setArenaVisibleRect(const QRect& rect)
{
    m_firstVisibleColumn = (rect.left()-m_buttonMargin) / m_buttonSize;
    if (m_firstVisibleColumn<0) m_firstVisibleColumn = 0;
    m_lastVisibleColumn = ((rect.right()-m_buttonMargin) / m_buttonSize );
    if (m_lastVisibleColumn>=m_fieldsCount) m_lastVisibleColumn = m_fieldsCount-1;
    m_firstVisibleRow = (rect.top()-m_buttonMargin) / m_buttonSize;
    if (m_firstVisibleRow<0) m_firstVisibleRow = 0;
    m_lastVisibleRow = ((rect.bottom()-m_buttonMargin) / m_buttonSize );
    if (m_lastVisibleRow>=m_fieldsCount) m_lastVisibleRow = m_fieldsCount-1;

//    qDebug() << "Visible part of Arena : ("
//             << m_firstVisibleRow << ";" << m_lastVisibleRow << ") - ("
//             << m_firstVisibleColumn << ";" << m_lastVisibleColumn << ")";
}

