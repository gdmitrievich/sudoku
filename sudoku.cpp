// sudoku.cpp - simple console game
#include <iostream>
#include <random>
#include <vector>

enum Color {
      BlackFg = 90,
      BlackBg = 40,
      RedFg = 31,
      RedBg = 41,
      GreenFg = 32,
      GreenBg = 42,
      YellowFg = 33,
      YellowBg = 43,
      MagentaFg = 35,
      MagentaBg = 45,
      CyanFg = 36,
      CyanBg = 46,
      Default = 0
};

void game();
void initDigits(int ***defaultDigitsPtr, int ***solvedDigitsPtr, int ***currentDigitsPtr,
                const int dimension);
void doActionWithPlayerInput(char *playerInput, int *playerPosX, int *playerPosY, int ***currentDigits,
                             int ***defaultDigits, int ***solvedMtrx, int dimension,
                             int *countOfCorrectDigitsShown, bool *isMenuShown);
void drawField(int ***digits, int ***defaultDigits, const int dimension, const int playerPosX,
               const int playerPosY);
void setCountOfCorrectDigitsShown(int &countOfCorrectDigitsShown, int ***defaultDigits, int ***digits,
                                  const int dimension);

void fillMtrxOfRandomDigitsFully(int ***mtrxPtr, const int dimension);
void hideAnyRandomDigits(int ***digitsPtr);
void hideDigitsInThisSection(int ***digitPtr, int amountOfHiddenDigits, const int startRow,
                             const int startCol);

void copyDigits(int ***from, int ***to, const int dimension);
void clearLine(int ***digitsPtr, const int dimension, const int indexOfLine);

void refreshField(int ***currentDigits, int ***defaultDigits, const int dimension, int *playerPosY,
                  int *playerPosX, int *initialDigitsShownCount);
void refreshCurrentData(int ***defaultDigits, const int dimension, int *playerPosY, int *playerPosX,
                        int *initialDigitsShownCount);
void recreateDigits(int ***currentDigits, int ***defaultDigits, int ***solvedMtrx, const int dimension,
                    int *playerPosY, int *playerPosX, int *initialDigitsShownCount);

void clearSell(int ***currentDigits, int ***defaultDigits, const int row, const int col, const int dimension);
void changeSell(int ***currentDigits, int ***defaultDigits, const int row, const int col, const int dimension,
                const char playerInput);

template <typename T>
void setColor(const int fgColor, const int bgColor, const T text);
void clear();

void showRules();
void showMenu(const int countOfCorrectDigitsShown);
void showHint(int ***solvedMtrx, int ***defaultDigits, const int dimension);
void showEndOfGame(const int filledSellsCount);

void allocateMtrxs(const int dimension, std::vector<int ***> mtrxPtrs);
void freeMtrxs(const int dimension, std::vector<int ***> mtrxPtrs);

Color getFgColor(int ***digits, int ***defaultDigits, const int dimension, const int row, const int col,
                 const int playerPosX, const int playerPosY);

int getRandomUnusedDigit(int ***mtrxPtr, const int dimension, const int row, const int col);
int getCountOfShownDigits(int ***defaultDigits, const int dimension);

bool hasDigitInHorizontalLine(int ***mtrx, const int dimension, const int row, const int col,
                              const int digit);
bool hasDigitInVerticalLine(int ***mtrx, const int dimension, const int row, const int col, const int digit);
bool hasDigitInCurrentSection(int ***mtrx, const int digit, const int row, const int col);

bool isDefaultDigit(int ***currentDigits, int ***defaultDigits, const int row, const int col);
bool isEnteredDigitCorrect(int ***digits, const int dimension, const int row, const int col);

int main() {
      srand(time(NULL));

      game();

      return 0;
}

void game() {
      const int DIMENSION = 9;
      const int END_COUNT = 81;
      const int MIDDLE = 4;
      const char ESC = '\x1B';

      int **defaultDigits = nullptr, **currentDigits = nullptr, **solvedMtrx = nullptr;
      allocateMtrxs(DIMENSION, {&defaultDigits, &currentDigits, &solvedMtrx});
      initDigits(&defaultDigits, &solvedMtrx, &currentDigits, DIMENSION);

      int playerPosX = MIDDLE, playerPosY = MIDDLE;
      int countOfCorrectDigitsShown = 0;
      int initialDigitsShownCount = getCountOfShownDigits(&defaultDigits, DIMENSION);

      char playerInput;

      bool isMenuShown = true;

      showRules();

      do {
            drawField(&currentDigits, &defaultDigits, DIMENSION, playerPosX, playerPosY);

            setCountOfCorrectDigitsShown(countOfCorrectDigitsShown, &defaultDigits, &currentDigits,
                                         DIMENSION);

            if (isMenuShown)
                  showMenu(countOfCorrectDigitsShown);
            else
                  setColor(Color::BlackFg, Color::Default, "\nm - show menu;\n");

            if (countOfCorrectDigitsShown != END_COUNT) {
                  doActionWithPlayerInput(&playerInput, &playerPosX, &playerPosY, &currentDigits,
                                          &defaultDigits, &solvedMtrx, DIMENSION, &initialDigitsShownCount,
                                          &isMenuShown);
            } else {
                  break;
            }

            clear();
      } while (playerInput != ESC);

      if (countOfCorrectDigitsShown == END_COUNT) showEndOfGame(END_COUNT - initialDigitsShownCount);

      freeMtrxs(DIMENSION, {&defaultDigits, &currentDigits, &solvedMtrx});
}

void setCountOfCorrectDigitsShown(int &countOfCorrectDigitsShown, int ***defaultDigits, int ***digits,
                                  const int dimension) {
      const int HIDDEN_DIGIT = 0;
      countOfCorrectDigitsShown = 0;

      for (int i = 0; i < dimension; i++) {
            for (int j = 0; j < dimension; j++) {
                  bool isDefaultDigit = *(*(*defaultDigits + i) + j) == *(*(*digits + i) + j);
                  bool isShownDigit = (*(*(*digits + i) + j)) != HIDDEN_DIGIT;

                  if (!isShownDigit)
                        continue;
                  else if (isEnteredDigitCorrect(digits, dimension, i, j) || isDefaultDigit)
                        countOfCorrectDigitsShown++;
            }
      }
}

void initDigits(int ***defaultDigitsPtr, int ***solvedDigitsPtr, int ***currentDigitsPtr,
                const int dimension) {
      fillMtrxOfRandomDigitsFully(solvedDigitsPtr, dimension);

      copyDigits(solvedDigitsPtr, defaultDigitsPtr, dimension);

      hideAnyRandomDigits(defaultDigitsPtr);

      copyDigits(defaultDigitsPtr, currentDigitsPtr, dimension);
}

void doActionWithPlayerInput(char *playerInput, int *playerPosX, int *playerPosY, int ***currentDigits,
                             int ***defaultDigits, int ***solvedMtrx, int dimension,
                             int *initialDigitsShownCount, bool *isMenuShown) {
      const int INDENT_TO_CAPITAL_LETTER = 32;

      std::cout << "\nInput: _\b";
      std::cin >> *playerInput;
      *playerInput =
          *playerInput >= 'A' && *playerInput <= 'Z' ? *playerInput + INDENT_TO_CAPITAL_LETTER : *playerInput;

      switch (*playerInput) {
            case 'w':
                  *playerPosY = (*playerPosY) - 1 != -1 ? (*playerPosY) - 1 : (*playerPosY);
                  break;
            case 's':
                  *playerPosY = (*playerPosY) + 1 != dimension ? (*playerPosY) + 1 : (*playerPosY);
                  break;
            case 'a':
                  *playerPosX = (*playerPosX) - 1 != -1 ? (*playerPosX) - 1 : (*playerPosX);
                  break;
            case 'd':
                  *playerPosX = (*playerPosX) + 1 != dimension ? (*playerPosX) + 1 : (*playerPosX);
                  break;
            case '0':
                  clearSell(currentDigits, defaultDigits, *playerPosY, *playerPosX, dimension);
                  break;
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                  changeSell(currentDigits, defaultDigits, *playerPosY, *playerPosX, dimension, *playerInput);
                  break;
            case 'm':
                  *isMenuShown = true;
                  break;
            case 'c':
                  *isMenuShown = false;
                  break;
            case 'r':
                  refreshField(currentDigits, defaultDigits, dimension, playerPosY, playerPosX,
                               initialDigitsShownCount);
                  break;
            case 'h':
                  showHint(solvedMtrx, defaultDigits, dimension);
                  break;
            case 'n':
                  recreateDigits(currentDigits, defaultDigits, solvedMtrx, dimension, playerPosY, playerPosX,
                                 initialDigitsShownCount);
                  break;
      }
}

void drawField(int ***digits, int ***defaultDigits, const int dimension, const int playerPosX,
               const int playerPosY) {
      using std::cout, std::endl;

      const int SIDE_BORDERS_COUNT = 4;
      const char VERTICAL_LINE = '|', HORIZONTAL_LINE = '-';
      const char SPACE_BAR = ' ';
      const char HIDDEN_CELL = '*';

      Color fgColor = Color::Default, bgColor = Color::Default;

      for (int i = 0, row = 0; i < dimension + SIDE_BORDERS_COUNT; i++) {
            for (int j = 0, col = 0; j < dimension + SIDE_BORDERS_COUNT; j++) {
                  cout << SPACE_BAR;

                  fgColor = getFgColor(digits, defaultDigits, dimension, row, col, playerPosX, playerPosY);

                  if (i % 4 == 0)
                        setColor(Color::Default, Color::Default, HORIZONTAL_LINE);
                  else if (j % 4 == 0)
                        setColor(Color::Default, Color::Default, VERTICAL_LINE);
                  else if (*(*(*digits + row) + col) == 0)
                        setColor(fgColor, bgColor, HIDDEN_CELL);
                  else
                        setColor(fgColor, bgColor, *(*(*digits + row) + col));

                  col = j % 4 != 0 ? col + 1 : col;
            }

            row = i % 4 != 0 && row + 1 != dimension ? row + 1 : row;

            cout << endl;
      }
}

void fillMtrxOfRandomDigitsFully(int ***mtrxPtr, const int dimension) {
      int clearedLinesCount = 0;
      int clastClearedLine = 0;

      for (int i = 0; i < dimension; i++) {
            for (int j = 0; j < dimension; j++) {
                  int result = getRandomUnusedDigit(mtrxPtr, dimension, i, j);

                  if (result != -1) {
                        *(*(*mtrxPtr + i) + j) = result;
                  } else {
                        clearLine(mtrxPtr, dimension, i);

                        if (++clearedLinesCount > 1 && clastClearedLine == i) {
                              for (int k = 0; k <= i; k++) clearLine(mtrxPtr, dimension, k);

                              i = 0;

                              clearedLinesCount = 0;
                        }

                        j = -1;

                        clastClearedLine = i;
                  }
            }
      }
}

void hideAnyRandomDigits(int ***digitsPtr) {
      const int LEFT_GROUND = 5, RIGHT_GROUND = 7;
      const int SECTIONS_COUNT = 9;
      const int SECTION_LEN = 3;

      for (int i = 0; i < SECTIONS_COUNT; i++) {
            int amountOfHiddenDigits = LEFT_GROUND + rand() % (RIGHT_GROUND - LEFT_GROUND + 1);

            int startIndexOfSectionRow = SECTION_LEN * (i / SECTION_LEN);
            int startIndexOfSectionCol = SECTION_LEN * (i - SECTION_LEN * (i / SECTION_LEN));

            hideDigitsInThisSection(digitsPtr, amountOfHiddenDigits, startIndexOfSectionRow,
                                    startIndexOfSectionCol);
      }
}

void hideDigitsInThisSection(int ***digitPtr, int amountOfHiddenDigits, const int startRow,
                             const int startCol) {
      const int SECTION_LEN = 3;

      while (amountOfHiddenDigits != 0) {
            int randomDigit = 1 + rand() % 9;

            bool isFound = false;
            while (!isFound) {
                  randomDigit = randomDigit < 9 ? randomDigit + 1 : 1;

                  for (int i = startRow; !isFound && i < startRow + SECTION_LEN; i++) {
                        for (int j = startCol; !isFound && j < startCol + SECTION_LEN; j++) {
                              if (*(*(*digitPtr + i) + j) == randomDigit) {
                                    *(*(*digitPtr + i) + j) = 0;

                                    isFound = true;

                                    amountOfHiddenDigits--;
                              }
                        }
                  }
            }
      }
}

void copyDigits(int ***from, int ***to, const int dimension) {
      for (int i = 0; i < dimension; i++) {
            for (int j = 0; j < dimension; j++) {
                  *(*(*to + i) + j) = *(*(*from + i) + j);
            }
      }
}

void clearLine(int ***digitsPtr, const int dimension, const int indexOfLine) {
      const int DEFAULT_VALUE = 0;

      for (int i = 0; i < dimension; i++) *(*(*digitsPtr + indexOfLine) + i) = DEFAULT_VALUE;
}

void refreshField(int ***currentDigits, int ***defaultDigits, const int dimension, int *playerPosY,
                  int *playerPosX, int *initialDigitsShownCount) {
      copyDigits(defaultDigits, currentDigits, dimension);

      refreshCurrentData(defaultDigits, dimension, playerPosY, playerPosX, initialDigitsShownCount);
}

void refreshCurrentData(int ***defaultDigits, const int dimension, int *playerPosY, int *playerPosX,
                        int *initialDigitsShownCount) {
      const int MIDDLE = 4;

      *playerPosX = *playerPosY = MIDDLE;

      *initialDigitsShownCount = getCountOfShownDigits(defaultDigits, dimension);
}

void recreateDigits(int ***currentDigits, int ***defaultDigits, int ***solvedMtrx, const int dimension,
                    int *playerPosY, int *playerPosX, int *initialDigitsShownCount) {
      for (int i = 0; i < dimension; i++) clearLine(solvedMtrx, dimension, i);

      initDigits(defaultDigits, solvedMtrx, currentDigits, dimension);

      refreshCurrentData(defaultDigits, dimension, playerPosY, playerPosX, initialDigitsShownCount);
}

void clearSell(int ***currentDigits, int ***defaultDigits, const int row, const int col,
               const int dimension) {
      if (!isDefaultDigit(currentDigits, defaultDigits, row, col)) {
            *(*(*currentDigits + row) + col) = 0;
      }
}

void changeSell(int ***currentDigits, int ***defaultDigits, const int row, const int col, const int dimension,
                const char playerInput) {
      if (!isDefaultDigit(currentDigits, defaultDigits, row, col)) {
            *(*(*currentDigits + row) + col) = playerInput - '0';
      }
}

template <typename T>
void setColor(const int fgColor, const int bgColor, const T text) {
      std::cout << "\033[" << bgColor << ";" << fgColor << "m" << text << "\033[" << Color::Default << "m";
}

void clear() { std::cout << "\x1B[2J\x1B[H"; }

void showRules() {
      using std::cout, std::endl;

      cout << "Rules of the game \"Sudoku\":" << endl;

      cout << "------------------------------" << endl;

      cout << "- Sudoku is played over a 9x9 grid, divided \nto 3x3 sub grids called \"regions\";" << endl;
      cout << "- Sudoku begins with some of the grid cells \nalready filled with numbers;" << endl;
      cout << "- The object of Sudoku is to fill the other \nempty cells with numbers between 1 and 9 \n"
              "according the following guidelines:"
           << endl;

      cout << "1. Number can appear only once on each row;" << endl;
      cout << "2. Number can appear only once on each column;" << endl;
      cout << "3. Number can appear only once on each region;\n" << endl;

      cout << "Note: you can't change the ";
      setColor(Color::YellowFg, Color::Default, "generated");
      cout << " digit;" << endl;

      cout << "------------------------------\n" << endl;
}

void showMenu(const int countOfCorrectDigitsShown) {
      using std::cout, std::endl;
      const int END_COUNT = 81;

      cout << "\nm - show menu;" << endl;
      cout << "c - close menu;" << endl;
      cout << "Esc - close the game;\n" << endl;

      cout << "n - create new field;" << endl;
      cout << "r - reset all changes;" << endl;
      cout << "h - show hint;\n" << endl;

      cout << "w,a,s,d - moving;" << endl;
      cout << "1,2...9 - press if you want to change the cell;" << endl;
      cout << "0 - press if you want to clear inputed digit;\n" << endl;

      cout << "Remains to be filled out: ";
      setColor(Color::YellowFg, Color::Default, END_COUNT - countOfCorrectDigitsShown);
      cout << ";" << endl;
      cout << "Filled in: ";
      setColor(Color::YellowFg, Color::Default, countOfCorrectDigitsShown);
      cout << ";\n" << endl;

      cout << "Field info: " << endl;
      setColor(Color::CyanFg, Color::Default, '*');
      cout << " or ";
      setColor(Color::CyanFg, Color::Default, '9');
      cout << " - current player position;" << endl;
      setColor(Color::BlackFg, Color::Default, '*');
      cout << " - empty cell;" << endl;
      setColor(Color::YellowFg, Color::Default, '9');
      cout << " - generated digit;" << endl;
      setColor(Color::RedFg, Color::Default, '9');
      cout << " - incorrect digit;" << endl;
      setColor(Color::GreenFg, Color::Default, '9');
      cout << " - correct digit;" << endl;
}

void showHint(int ***solvedMtrx, int ***defaultDigits, const int dimension) {
      clear();
      drawField(solvedMtrx, defaultDigits, dimension, -1, -1);
      setColor(Color::BlackFg, Color::Default, "Press Enter to hide the hint.");
      getchar();
      getchar();
}

void showEndOfGame(const int filledSellsCount) {
      clear();

      setColor(Color::GreenFg, Color::Default, "Congratulations!");
      setColor(Color::Default, Color::Default, " You have completed ");
      setColor(Color::CyanFg, Color::Default, "Sudoku");
      setColor(Color::Default, Color::Default, " and filled ");
      setColor(Color::MagentaFg, Color::Default, filledSellsCount);
      setColor(Color::Default, Color::Default, " empty cells!");
}

void allocateMtrxs(const int dimension, std::vector<int ***> mtrxPtrs) {
      for (auto mtrxPtr : mtrxPtrs) {
            (*mtrxPtr) = new int *[dimension];

            for (int i = 0; i < dimension; i++) *(*mtrxPtr + i) = new int[dimension];
      }
}

void freeMtrxs(const int dimension, std::vector<int ***> mtrxPtrs) {
      for (auto mtrxPtr : mtrxPtrs) {
            for (int i = 0; i < dimension; i++) delete[] * (*mtrxPtr + i);

            delete[](*mtrxPtr);
      }
}

Color getFgColor(int ***digits, int ***defaultDigits, const int dimension, const int row, const int col,
                 const int playerPosX, const int playerPosY) {
      const int HIDDEN_DIGIT = 0;

      Color result;

      bool isDefaultDigit = *(*(*defaultDigits + row) + col) == *(*(*digits + row) + col);
      bool isShownDigit = (*(*(*digits + row) + col)) != HIDDEN_DIGIT;

      if (row == playerPosY && col == playerPosX)
            result = Color::CyanFg;
      else if (!isEnteredDigitCorrect(digits, dimension, row, col) && !isDefaultDigit && isShownDigit)
            result = Color::RedFg;
      else if (isDefaultDigit && isShownDigit)
            result = Color::YellowFg;
      else if (!isDefaultDigit)
            result = Color::GreenFg;
      else
            result = Color::BlackFg;

      return result;
}

int getRandomUnusedDigit(int ***mtrxPtr, const int dimension, const int row, const int col) {
      int randomDigit = 1 + rand() % 9;
      int initialDigit = randomDigit;

      while (hasDigitInVerticalLine(mtrxPtr, dimension, row, col, randomDigit) ||
             hasDigitInHorizontalLine(mtrxPtr, dimension, row, col, randomDigit) ||
             hasDigitInCurrentSection(mtrxPtr, randomDigit, row, col)) {
            randomDigit = randomDigit < 9 ? randomDigit + 1 : 1;

            if (initialDigit == randomDigit) {
                  randomDigit = -1;

                  break;
            }
      }

      return randomDigit;
}

int getCountOfShownDigits(int ***defaultDigits, const int dimension) {
      int result = 0;
      const int HIDDEN_DIGIT = 0;

      for (int i = 0; i < dimension; i++)
            for (int j = 0; j < dimension; j++)
                  result = *(*(*defaultDigits + i) + j) != HIDDEN_DIGIT ? result + 1 : result;

      return result;
}

bool hasDigitInHorizontalLine(int ***mtrx, const int dimension, const int row, const int col,
                              const int digit) {
      bool has = false;

      for (int i = 0; !has && i < dimension; i++) {
            if (i == col) continue;

            if (*(*(*mtrx + row) + i) == digit) has = true;
      }

      return has;
}

bool hasDigitInVerticalLine(int ***mtrx, const int dimension, const int row, const int col, const int digit) {
      bool has = false;

      for (int i = 0; !has && i < dimension; i++) {
            if (i == row) continue;

            if (*(*(*mtrx + i) + col) == digit) has = true;
      }

      return has;
}

bool hasDigitInCurrentSection(int ***mtrx, const int digit, const int row, const int col) {
      const int SECTION_LEN = 3;

      bool has = false;

      int startIndexOfSectionRow = SECTION_LEN * (row / SECTION_LEN);
      int startIndexOfSectionCol = SECTION_LEN * (col / SECTION_LEN);

      for (int i = startIndexOfSectionRow; !has && i < SECTION_LEN + startIndexOfSectionRow; i++) {
            for (int j = startIndexOfSectionCol; !has && j < SECTION_LEN + startIndexOfSectionCol; j++) {
                  if (i == row && j == col) continue;

                  if (*(*(*mtrx + i) + j) == digit) has = true;
            }
      }

      return has;
}

bool isDefaultDigit(int ***currentDigits, int ***defaultDigits, const int row, const int col) {
      return (*(*(*currentDigits + row) + col) == *(*(*defaultDigits + row) + col)) &&
             (*(*(*defaultDigits + row) + col) != 0);
}

bool isEnteredDigitCorrect(int ***digits, const int dimension, const int row, const int col) {
      return !((hasDigitInHorizontalLine(digits, dimension, row, col, *(*(*digits + row) + col)) ||
                hasDigitInVerticalLine(digits, dimension, row, col, *(*(*digits + row) + col))) ||
               hasDigitInCurrentSection(digits, *(*(*digits + row) + col), row, col));
}
