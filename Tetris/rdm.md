Report bugs

install
Windows

    Install MinGW:
        Download MinGW from the official MinGW website.
        Follow the installation instructions to set up MinGW.
        Add MinGW to your system PATH.

    Install ncurses:
        Download and install the pre-compiled ncurses library for Windows from pdcurses.
        Follow the instructions to set up ncurses.

    Clone the Repository:
        Open a terminal and run:
        sh

    git clone https://github.com/yourusername/tetris-game.git
    cd tetris-game

Compile the Game:

    In the terminal, run:

    sh

        g++ -o tetris tetris.cpp -lncurses

Linux

    Install ncurses:
        Open a terminal and run:

        sh

    sudo apt-get install libncurses5-dev libncursesw5-dev

Clone the Repository:

    In the terminal, run:

    sh

    git clone https://github.com/yourusername/tetris-game.git
    cd tetris-game

Compile the Game:

    In the terminal, run:

    sh

        g++ -o tetris tetris.cpp -lncurses

Running the Game
Windows

    Open a terminal (cmd, PowerShell, or Git Bash).
    Navigate to the tetris-game directory:

    sh

cd path\to\tetris-game

Run the game:

sh

    tetris.exe

Linux

    Open a terminal.
    Navigate to the tetris-game directory:

    sh

cd path/to/tetris-game

Run the game:

sh

    ./tetris

Controls

    Left Arrow: Move the block left.
    Right Arrow: Move the block right.
    Down Arrow: Move the block down faster.
    Up Arrow: Rotate the block.
    Q: Quit the game.

Gameplay

    The game starts with an empty grid.
    Random blocks will fall from the top of the grid.
    Your goal is to position and rotate the blocks to form complete horizontal lines.
    When a line is completed, it disappears, and you earn points.
    The game ends when the blocks reach the top of the grid.

Troubleshooting

    Game Not Running:
        Ensure you have the necessary libraries installed (ncurses).
        Verify that the game is compiled without errors.

    Controls Not Working:
        Make sure your terminal supports the arrow keys and other input.

    Graphics Issues:
        The game uses the ncurses library for console graphics. Ensure your terminal is compatible.
