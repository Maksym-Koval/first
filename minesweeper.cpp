#include "minesweeper.h"
#include "ui_minesweeper.h"

#include <QtWidgets>
#include <QtMath>
#include <QtDebug>

MineSweeper::MineSweeper(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MineSweeper)
{
    m_ui->setupUi(this);
    m_ui->mainToolBar->hide();
    m_ui->statusBar->hide();


    m_buttonStartGame = new QPushButton(tr("start &Game"), this);
    connect(m_buttonStartGame, &QPushButton::clicked, this, &MineSweeper::startGame);

    m_buttonSetArena = new QPushButton(tr("&Set arena"), this);
    connect(m_buttonSetArena, &QPushButton::clicked, this, &MineSweeper::initArena);

    m_buttonExit = new QPushButton(tr("&Exit"), this);
    connect(m_buttonExit, &QPushButton::clicked, this, &MineSweeper::close);


    m_settingsFrame = new QFrame(this);
    m_settingsFrame->setFrameStyle(QFrame::Panel | QFrame::Plain);
    m_settingsFrame->setLineWidth(1);
    m_settingsFrame->setAutoFillBackground(true);

    m_LArenaSize = new QLabel(tr("Arena size :"), m_settingsFrame);
    m_LArenaSize->move(5,5);

    m_SBArenaSize = new QSpinBox(m_settingsFrame);
    m_SBArenaSize->move(5, m_LArenaSize->y()+m_LArenaSize->height());
    m_SBArenaSize->setRange(10,10000);

    m_LMineCount = new QLabel(tr("Mine count :"), m_settingsFrame);
    m_LMineCount->move(5, m_SBArenaSize->y()+m_SBArenaSize->height()+5);

    m_SBMineCount = new QSpinBox(m_settingsFrame);
    m_SBMineCount->move(5, m_LMineCount->y()+m_LMineCount->height());
    m_SBMineCount->setRange(1,10000);
    m_SBMineCount->setValue(10);

    m_settingsFrame->setFixedHeight(m_SBMineCount->y()+m_SBMineCount->height()+5);


    m_LFieldsProcessed = new QLabel(this);
    m_LFieldsProcessed->setText(tr("Fields Processed:"));

    m_lcdArenaProcessed = new QLCDNumber(this);
    m_lcdArenaProcessed->setSegmentStyle(QLCDNumber::Filled);
    m_lcdArenaProcessed->setMode(QLCDNumber::Dec);
    m_lcdArenaProcessed->setDigitCount(9);

    m_rightVBoxLayout = new QVBoxLayout;//(this);
    m_rightVBoxLayout->setMargin(5);
    m_rightVBoxLayout->setSpacing(10);
    m_rightVBoxLayout->addWidget(m_buttonStartGame);
    m_rightVBoxLayout->addWidget(m_settingsFrame);
    m_rightVBoxLayout->addWidget(m_buttonSetArena);
    m_rightVBoxLayout->addWidget(m_LFieldsProcessed);
    m_rightVBoxLayout->addWidget(m_lcdArenaProcessed);
    m_rightVBoxLayout->addStretch();
    m_rightVBoxLayout->addWidget(m_buttonExit);


    m_LGameResult = new QLabel(this);
    m_LGameTime = new QLabel(this);
    QFont font = m_LGameResult->font();
    font.setPointSize(16);
    font.setBold(true);
    m_LGameResult->setFont(font);
    m_LGameTime->setFont(font);
    QPalette pal;
    pal.setColor(QPalette::WindowText, QColor(Qt::red));
    m_LGameResult->setPalette(pal);


    m_bottomFrameHBoxLayout = new QHBoxLayout;
    m_bottomFrameHBoxLayout->setMargin(5);
    m_bottomFrameHBoxLayout->setSpacing(10);
    m_bottomFrameHBoxLayout->addWidget(m_LGameResult);
    m_bottomFrameHBoxLayout->addStretch();
    m_bottomFrameHBoxLayout->addWidget(m_LGameTime);

    m_bottomFrame = new QFrame(this);
    m_bottomFrame->setFrameStyle(QFrame::Panel | QFrame::Plain);
    m_bottomFrame->setLineWidth(1);
    m_bottomFrame->setAutoFillBackground(true);
    m_bottomFrame->setFixedHeight(100);
    m_bottomFrame->setLayout(m_bottomFrameHBoxLayout);


    m_gameTimer = new QTimer(this);
    m_gameTimer->setInterval(1000);
    connect(m_gameTimer, &QTimer::timeout, this, &MineSweeper::updateGameTime);

    m_arena = new MArena(this);
    connect(m_arena, &MArena::gameOver, this, &MineSweeper::gameOver);
    connect(m_arena, SIGNAL(fieldsProcessed(qint32)), m_lcdArenaProcessed, SLOT(display(int)));

    m_scrollArea = new QScrollArea(this);

    initArena();

    m_scrollArea->setWidget(m_arena);
    connect(m_scrollArea->horizontalScrollBar(), &QScrollBar::valueChanged , this, &MineSweeper::scrollAreaVisibleRectChanged);
    connect(m_scrollArea->verticalScrollBar(), &QScrollBar::valueChanged , this, &MineSweeper::scrollAreaVisibleRectChanged);

    m_leftVBoxLayout = new QVBoxLayout;//(this);
    m_leftVBoxLayout->setMargin(5);
    m_leftVBoxLayout->setSpacing(10);
    m_leftVBoxLayout->addWidget(m_scrollArea);
    m_leftVBoxLayout->addWidget(m_bottomFrame);

    m_topHBoxLayout = new QHBoxLayout;//(this);
    m_topHBoxLayout->setMargin(5);
    m_topHBoxLayout->setSpacing(10);
    m_topHBoxLayout->addLayout(m_leftVBoxLayout);
    m_topHBoxLayout->addLayout(m_rightVBoxLayout);

    centralWidget()->setLayout(m_topHBoxLayout);
}

MineSweeper::~MineSweeper()
{
    delete m_ui;
}

void MineSweeper::showEvent(QShowEvent * event)
{
    Q_UNUSED(event);

    int h = m_arena->height()
            +m_scrollArea->horizontalScrollBar()->height();

    m_scrollArea->setMinimumSize(h,h);
    h += m_bottomFrame->height()+m_leftVBoxLayout->margin();

    int w = m_scrollArea->minimumWidth() + 150 + m_rightVBoxLayout->geometry().width() + 4*m_topHBoxLayout->margin();

    setMinimumSize(w+20,h+20);
    setGeometry(100,100,1000,600);
}

void MineSweeper::resizeEvent(QResizeEvent * event)
{
    QMainWindow::resizeEvent(event);
    scrollAreaVisibleRectChanged();
}

void MineSweeper::initArena()
{
    m_gameTimer->stop();

    m_LGameResult->setText(tr("Initializing Arena"));

    m_buttonStartGame->setEnabled(false);

    m_scrollArea->setEnabled(false);
    m_scrollArea->horizontalScrollBar()->setValue(m_scrollArea->horizontalScrollBar()->minimum());
    m_scrollArea->verticalScrollBar()->setValue(m_scrollArea->verticalScrollBar()->minimum());

    m_arena->setArenaEnabled(false);
    m_arena->setArena(m_SBArenaSize->value(), m_SBMineCount->value());

    m_scrollArea->setEnabled(true);

    scrollAreaVisibleRectChanged();

    m_buttonStartGame->setEnabled(true);

    m_LGameResult->setText(tr("Waiting for Start"));
    m_LGameTime->setText(tr(""));
}

void MineSweeper::startGame()
{
    m_arena->setArenaEnabled(true);
    m_buttonStartGame->setEnabled(false);

    m_LGameResult->setText(tr("Playing ..."));
    m_gameTime = QTime::currentTime();
    m_gameTimer->start();
}

void MineSweeper::gameOver(const int gameResult)
{
    m_gameTimer->stop();

    if (!gameResult)
        m_LGameResult->setText(tr("Game over. LOSE"));
    else m_LGameResult->setText(tr("Game over. WIN"));

    m_arena->setArenaEnabled(false);
}

void MineSweeper::updateGameTime()
{
    int gameTime = m_gameTime.secsTo(QTime::currentTime());
    m_LGameTime->setText( QString(tr("Round time : %1h %2m %3s"))
                          .arg(gameTime / 3600).arg(gameTime / 60).arg(gameTime % 60)
                         );
}

void MineSweeper::scrollAreaVisibleRectChanged()
{
    m_arena->setArenaVisibleRect(QRect(
             m_scrollArea->horizontalScrollBar()->value(),
             m_scrollArea->verticalScrollBar()->value(),
             m_scrollArea->viewport()->width(),
             m_scrollArea->viewport()->height()
             ));
}





