#include "mbed.h"
#include "uLCD_4DGL.h"
#include <vector>
 
uLCD_4DGL uLCD(p28,p27,p30); // serial tx, serial rx, reset pin;
Serial Blue(p13, p14);
DigitalOut bad_move(p18);
DigitalIn reset_button(p19);
DigitalIn game_mode_switch(p20);
PwmOut speaker(p21);
 
enum Piece {e, wK, bK, wQ, bQ, wR, bR, wB, bB, wN, bN, w, b};
std::vector<Piece> whitePieces;
std::vector<Piece> blackPieces;
 
enum GameState {whiteSelecting, whitePickedUp, whiteAI, blackSelecting, blackPickedUp, blackAI};
 
struct pixelCoord {
    uint8_t x;
    uint8_t y;
};
 
struct boardPos {
    uint8_t row;
    uint8_t column;
 
    bool operator==(const boardPos &other) const
    {
        return row == other.row && column == other.column;
    }
};
 
class Nav_Switch
{
public:
    Nav_Switch(PinName up,PinName down,PinName left,PinName right,PinName fire);
    int read();
//boolean functions to test each switch
    bool up();
    bool down();
    bool left();
    bool right();
    bool fire();
//automatic read on RHS
    operator int ();
//index to any switch array style
    bool operator[](int index)
    {
        return _pins[index];
    };
private:
    BusIn _pins;
 
};
Nav_Switch::Nav_Switch (PinName up,PinName down,PinName left,PinName right,PinName fire):
    _pins(up, down, left, right, fire)
{
    _pins.mode(PullUp); //needed if pullups not on board or a bare nav switch is used - delete otherwise
    wait(0.001); //delays just a bit for pullups to pull inputs high
}
inline bool Nav_Switch::up()
{
    return !(_pins[0]);
}
inline bool Nav_Switch::down()
{
    return !(_pins[1]);
}
inline bool Nav_Switch::left()
{
    return !(_pins[2]);
}
inline bool Nav_Switch::right()
{
    return !(_pins[3]);
}
inline bool Nav_Switch::fire()
{
    return !(_pins[4]);
}
inline int Nav_Switch::read()
{
    return _pins.read();
}
inline Nav_Switch::operator int ()
{
    return _pins.read();
}

const int BOARD_DARK_COLOR = 0x769656;
const int BOARD_LIGHT_COLOR = 0xbaca44;
 
const float KING_POSITION_VALUES[] = {2.0, 3.0, 1.0, 0.0, 0.0, 1.0, 3.0, 2,0,
                                      2.0, 2.0, 0.0, 0.0, 0.0, 0.0, 2.0, 2.0,
                                      -1.0, -2.0, -2.0, -2.0, -2.0, -2.0, -2.0, -1.0,
                                      -2.0, -3.0, -3.0, -4.0, -4.0, -3.0, -3.0, -2,0,
                                      -3.0, -4.0, -4.0, -5.0, -5.0, -4.0, -4.0, -3.0,
                                      -3.0, -4.0, -4.0, -5.0, -5.0, -4.0, -4.0, -3.0,
                                      -3.0, -4.0, -4.0, -5.0, -5.0, -4.0, -4.0, -3.0,
                                      -3.0, -4.0, -4.0, -5.0, -5.0, -4.0, -4.0, -3.0
                                     };
 
const float QUEEN_POSITION_VALUES[] = {-2.0, -1.0, -1.0, -0.5, -0.5, -1.0, -1.0, -2.0,
                                       -1.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0, -1.0,
                                       -1.0, 0.5, 0.5, 0.5, 0.5, 0.5, 0.0, -1.0,
                                       0.0, 0.0, 0.5, 0.5, 0.5, 0.5, 0.0, -0.5,
                                       -0.5, 0.0, 0.5, 0.5, 0.5, 0.5, 0.0, -0.5,
                                       -1.0, 0.0, 0.5, 0.5, 0.5, 0.5, 0.0, -1.0,
                                       -1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, -1.0,
                                       -2.0, -1.0, -1.0, -0.5, -0.5, -1.0, -1.0, -2.0
                                      };
 
const float ROOK_POSITION_VALUES[] = {0.0, 0.0, 0.0, 0.5, 0.5, 0.0, 0.0, 0.0,
                                      -0.5, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, -0.5,
                                      -0.5, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, -0.5,
                                      -0.5, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, -0.5,
                                      -0.5, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, -0.5,
                                      -0.5, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, -0.5,
                                      0.5, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.5,
                                      0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
                                     };
 
const float BISHOP_POSITION_VALUES[] = {-2.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -2.0,
                                        -1.0, 0.5, 0.0, 0.0, 0.0, 0.0, 0.5, -1.0,
                                        -1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, -1.0,
                                        -1.0, 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, -1.0,
                                        -1.0, 0.5, 0.5, 1.0, 1.0, 0.5, 0.5, -1.0,
                                        -1.0, 0.0, 0.5, 1.0, 1.0, 0.5, 0.0, -1.0,
                                        -1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, -1.0,
                                        -2.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -2.0
                                       };
const float KNIGHT_POSITION_VALUES[] = {-5.0, -4.0, -3.0, -3.0, -3.0, -3.0, -4.0, -5.0,
                                        -4.0, -2.0, 0.0, 0.5, 0.5, 0.0, -2.0, -4.0,
                                        -3.0, 0.5, 1.0, 1.5, 1.5, 1.0, 0.5, -3.0,
                                        -3.0, 0.0, 1.5, 2.0, 2.0, 1.5, 0.0, -3.0,
                                        -3.0, 0.5, 1.5, 2.0, 2.0, 1.5, 0.5, -3.0,
                                        -3.0, 0.0, 1.0, 1.5, 1.5, 1.0, 0.0, -3.0,
                                        -4.0, -2.0, 0.0, 0.0, 0.0, 0.0, -2.0, -4.0,
                                        -5.0, -4.0, -3.0, -3.0, -3.0, -3.0, -4.0, -5.0
                                       };
const float PAWN_POSITION_VALUES[] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                                      0.5, 1.0, 1.0, -2.0, -2.0, 1.0, 1.0, 0.5,
                                      0.5, -0.5, -1.0, 0.0, 0.0, -1.0, -0.5, 0.5,
                                      0.0, 0.0, 0.0, 2.0, 2.0, 0.0, 0.0, 0.0,
                                      0.5, 0.5, 1.0, 2.5, 2.5, 1.0, 0.5, 0.5,
                                      1.0, 1.0, 2.0, 3.0, 3.0, 2.0, 1.0, 1.0,
                                      5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0,
                                      0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
                                     };
 
class BoardState
{
private:
    Piece array[64];
    // These values are for white pieces, need to be inverted
    // when making calculations for black pieces.
    static const float PAWN_VALUE = 10.0;
    static const float KNIGHT_VALUE = 30.0;
    static const float BISHOP_VALUE = 30.0;
    static const float ROOK_VALUE = 50.0;
    static const float QUEEN_VALUE = 90.0;
    static const float KING_VALUE = 900.0;
public:
    BoardState() {}
 
    // calculates the advantage difference for the board state
    float calculateBoardState()
    {
        float sum = 0.0;
        for (int i = 0; i < 64; i++) {
            int row = i / 8;
            int column = i % 8;
            Piece curr = getPiece(row, column);
            switch(curr) {
                case wK:
                    sum += KING_VALUE + KING_POSITION_VALUES[row*8 + column];
                    break;
                case bK:
                    sum -= (KING_VALUE + KING_POSITION_VALUES[(7-row)*8 + (7-column)]);
                    break;
                case wQ:
                    sum += QUEEN_VALUE + QUEEN_POSITION_VALUES[row*8 + column];
                    break;
                case bQ:
                    sum -= (QUEEN_VALUE - QUEEN_POSITION_VALUES[(7-row)*8 + (7-column)]);
                    break;
                case wR:
                    sum += ROOK_VALUE + ROOK_POSITION_VALUES[row*8 + column];
                    break;
                case bR:
                    sum -= (ROOK_VALUE - ROOK_POSITION_VALUES[(7-row)*8 + (7-column)]);
                    break;
                case wB:
                    sum += BISHOP_VALUE + BISHOP_POSITION_VALUES[row*8 + column];
                    break;
                case bB:
                    sum -= (BISHOP_VALUE - BISHOP_POSITION_VALUES[(7-row)*8 + (7-column)]);
                    break;
                case wN:
                    sum += KNIGHT_VALUE + KNIGHT_POSITION_VALUES[row*8 + column];
                    break;
                case bN:
                    sum -= (KNIGHT_VALUE - KNIGHT_POSITION_VALUES[(7-row)*8 + (7-column)]);
                    break;
                case w:
                    sum += PAWN_VALUE + PAWN_POSITION_VALUES[row*8 + column];
                    break;
                case b:
                    sum -= (PAWN_VALUE - PAWN_POSITION_VALUES[(7-row)*8 + (7-column)]);
                    break;
                default:
                    break;
            }
        }
        return sum;
    }
 
    // returns the piece at a given location
    Piece getPiece(int row, int column)
    {
        return array[column + 8 * row];
    }
 
    // puts a piece at a given location
    void placePiece(Piece piece, int row, int column)
    {
        // pawn promotion handling
        if (piece == w && row == 7) {
            array[column + 8 * row] = wQ;
        } else if (piece == b && row == 0) {
            array[column + 8 * row] = bQ;
        } else {
            array[column + 8 * row] = piece;
        }
    }
 
    /*  removes a piece from the set position of the board
        returns the bit representation of the piece
    */
    Piece removePiece(int row, int column)
    {
        Piece removedPiece = array[column + 8 * row];
        array[column + 8 * row] = e;
        return removedPiece;
    }
 
    /*  moves a piece from one position to another
        returns the captured piece
    */
    Piece movePiece(int startRow, int startColumn, int endRow, int endColumn)
    {
        Piece movingPiece = removePiece(startRow, startColumn);
        Piece capturedPiece = removePiece(endRow, endColumn);
        placePiece(movingPiece, endRow, endColumn);
        return capturedPiece;
    }
 
    // generates a list of possible moves for a piece
    // returns moves
    std::vector<boardPos> getMoves(boardPos pos)
    {
        return getMoves(pos.row, pos.column);
    }
    std::vector<boardPos> getMoves(int row, int column)
    {
        std::vector<boardPos> moves;
        std::vector<boardPos> lineMoves;
        Piece movingPiece = getPiece(row, column);
        uint8_t rowIndex;
        uint8_t columnIndex;
        bool isWhite;
        switch(movingPiece) {
            case wK:
            case bK:
                isWhite = movingPiece == wK;
                if (validMove(isWhite, row + 1, column)) {
                    moves.push_back((boardPos) {
                        row + 1, column
                    });
                }
                if (validMove(isWhite, row, column + 1)) {
                    moves.push_back((boardPos) {
                        row, column + 1
                    });
                }
                if (validMove(isWhite, row - 1, column)) {
                    moves.push_back((boardPos) {
                        row - 1, column
                    });
                }
                if (validMove(isWhite, row, column - 1)) {
                    moves.push_back((boardPos) {
                        row, column - 1
                    });
                }
                if (validMove(isWhite, row + 1, column + 1)) {
                    moves.push_back((boardPos) {
                        row + 1, column + 1
                    });
                }
                if (validMove(isWhite, row - 1, column + 1)) {
                    moves.push_back((boardPos) {
                        row - 1, column + 1
                    });
                }
                if (validMove(isWhite, row - 1, column - 1)) {
                    moves.push_back((boardPos) {
                        row - 1, column - 1
                    });
                }
                if (validMove(isWhite, row + 1, column - 1)) {
                    moves.push_back((boardPos) {
                        row + 1, column - 1
                    });
                }
                break;
            case wQ:
            case bQ:
                isWhite = movingPiece == wQ;
                rowIndex = row + 1;
                columnIndex = column + 1;
                while (validMove(isWhite, rowIndex, columnIndex)) {
                    moves.push_back((boardPos) {
                        rowIndex, columnIndex
                    });
                    if (getPiece(rowIndex, columnIndex) != e) {
                        break;
                    }
                    rowIndex++;
                    columnIndex++;
                }
                rowIndex = row - 1;
                columnIndex = column + 1;
                while (validMove(isWhite, rowIndex, columnIndex)) {
                    moves.push_back((boardPos) {
                        rowIndex, columnIndex
                    });
                    if (getPiece(rowIndex, columnIndex) != e) {
                        break;
                    }
                    rowIndex--;
                    columnIndex++;
                }
                rowIndex = row + 1;
                columnIndex = column - 1;
                while (validMove(isWhite, rowIndex, columnIndex)) {
                    moves.push_back((boardPos) {
                        rowIndex, columnIndex
                    });
                    if (getPiece(rowIndex, columnIndex) != e) {
                        break;
                    }
                    rowIndex++;
                    columnIndex--;
                }
                rowIndex = row - 1;
                columnIndex = column - 1;
                while (validMove(isWhite, rowIndex, columnIndex)) {
                    moves.push_back((boardPos) {
                        rowIndex, columnIndex
                    });
                    if (getPiece(rowIndex, columnIndex) != e) {
                        break;
                    }
                    rowIndex--;
                    columnIndex--;
                }
                rowIndex = row + 1;
                columnIndex = column;
                while (validMove(isWhite, rowIndex, columnIndex)) {
                    moves.push_back((boardPos) {
                        rowIndex, columnIndex
                    });
                    if (getPiece(rowIndex, columnIndex) != e) {
                        break;
                    }
                    rowIndex++;
                }
                rowIndex = row - 1;
                columnIndex = column;
                while (validMove(isWhite, rowIndex, columnIndex)) {
                    moves.push_back((boardPos) {
                        rowIndex, columnIndex
                    });
                    if (getPiece(rowIndex, columnIndex) != e) {
                        break;
                    }
                    rowIndex--;
                }
                rowIndex = row;
                columnIndex = column + 1;
                while (validMove(isWhite, rowIndex, columnIndex)) {
                    moves.push_back((boardPos) {
                        rowIndex, columnIndex
                    });
                    if (getPiece(rowIndex, columnIndex) != e) {
                        break;
                    }
                    columnIndex++;
                }
                rowIndex = row;
                columnIndex = column - 1;
                while (validMove(isWhite, rowIndex, columnIndex)) {
                    moves.push_back((boardPos) {
                        rowIndex, columnIndex
                    });
                    if (getPiece(rowIndex, columnIndex) != e) {
                        break;
                    }
                    columnIndex--;
                }
                break;
            case wB:
            case bB:
                isWhite = movingPiece == wB;
                rowIndex = row + 1;
                columnIndex = column + 1;
                while (validMove(isWhite, rowIndex, columnIndex)) {
                    moves.push_back((boardPos) {
                        rowIndex, columnIndex
                    });
                    if (getPiece(rowIndex, columnIndex) != e) {
                        break;
                    }
                    rowIndex++;
                    columnIndex++;
                }
                rowIndex = row - 1;
                columnIndex = column + 1;
                while (validMove(isWhite, rowIndex, columnIndex)) {
                    moves.push_back((boardPos) {
                        rowIndex, columnIndex
                    });
                    if (getPiece(rowIndex, columnIndex) != e) {
                        break;
                    }
                    rowIndex--;
                    columnIndex++;
                }
                rowIndex = row + 1;
                columnIndex = column - 1;
                while (validMove(isWhite, rowIndex, columnIndex)) {
                    moves.push_back((boardPos) {
                        rowIndex, columnIndex
                    });
                    if (getPiece(rowIndex, columnIndex) != e) {
                        break;
                    }
                    rowIndex++;
                    columnIndex--;
                }
                rowIndex = row - 1;
                columnIndex = column - 1;
                while (validMove(isWhite, rowIndex, columnIndex)) {
                    moves.push_back((boardPos) {
                        rowIndex, columnIndex
                    });
                    if (getPiece(rowIndex, columnIndex) != e) {
                        break;
                    }
                    rowIndex--;
                    columnIndex--;
                }
                break;
            case wN:
            case bN:
                isWhite = movingPiece == wN;
                if (validMove(isWhite, row + 2, column + 1)) {
                    moves.push_back((boardPos) {
                        row + 2, column + 1
                    });
                }
                if (validMove(isWhite, row + 2, column - 1)) {
                    moves.push_back((boardPos) {
                        row + 2, column - 1
                    });
                }
                if (validMove(isWhite, row - 2, column - 1)) {
                    moves.push_back((boardPos) {
                        row - 2, column - 1
                    });
                }
                if (validMove(isWhite, row - 2, column + 1)) {
                    moves.push_back((boardPos) {
                        row - 2, column + 1
                    });
                }
                if (validMove(isWhite, row + 1, column + 2)) {
                    moves.push_back((boardPos) {
                        row + 1, column + 2
                    });
                }
                if (validMove(isWhite, row - 1, column + 2)) {
                    moves.push_back((boardPos) {
                        row - 1, column + 2
                    });
                }
                if (validMove(isWhite, row - 1, column - 2)) {
                    moves.push_back((boardPos) {
                        row - 1, column - 2
                    });
                }
                if (validMove(isWhite, row + 1, column - 2)) {
                    moves.push_back((boardPos) {
                        row + 1, column - 2
                    });
                }
                break;
            case wR:
            case bR:
                isWhite = movingPiece == wR;
                rowIndex = row + 1;
                columnIndex = column;
                while (validMove(isWhite, rowIndex, columnIndex)) {
                    moves.push_back((boardPos) {
                        rowIndex, columnIndex
                    });
                    if (getPiece(rowIndex, columnIndex) != e) {
                        break;
                    }
                    rowIndex++;
                }
                rowIndex = row - 1;
                columnIndex = column;
                while (validMove(isWhite, rowIndex, columnIndex)) {
                    moves.push_back((boardPos) {
                        rowIndex, columnIndex
                    });
                    if (getPiece(rowIndex, columnIndex) != e) {
                        break;
                    }
                    rowIndex--;
                }
                rowIndex = row;
                columnIndex = column + 1;
                while (validMove(isWhite, rowIndex, columnIndex)) {
                    moves.push_back((boardPos) {
                        rowIndex, columnIndex
                    });
                    if (getPiece(rowIndex, columnIndex) != e) {
                        break;
                    }
                    columnIndex++;
                }
                rowIndex = row;
                columnIndex = column - 1;
                while (validMove(isWhite, rowIndex, columnIndex)) {
                    moves.push_back((boardPos) {
                        rowIndex, columnIndex
                    });
                    if (getPiece(rowIndex, columnIndex) != e) {
                        break;
                    }
                    columnIndex--;
                }
                break;
            case w:
            case b:
                isWhite = movingPiece == w;
                int rowChange = isWhite ? 1 : -1;
                if (validMove(isWhite, row + rowChange, column) && getPiece(row + rowChange, column) == e) {
                    moves.push_back((boardPos) {
                        row + rowChange, column
                    });
                    // The case for pawns moving two squares at the start
                    if (((isWhite && row == 1) || (!isWhite && row == 6)) && validMove(isWhite, row + rowChange + rowChange, column) && getPiece(row + rowChange + rowChange, column) == e) {
                        moves.push_back((boardPos) {
                            row + rowChange + rowChange, column
                        });
                    }
                }
                if (validMove(isWhite, row + rowChange, column + 1) && getPiece(row + rowChange, column + 1) != e) {
                    moves.push_back((boardPos) {
                        row + rowChange, column + 1
                    });
                }
                if (validMove(isWhite, row + rowChange, column - 1) && getPiece(row + rowChange, column - 1) != e) {
                    moves.push_back((boardPos) {
                        row + rowChange, column - 1
                    });
                }
                break;
            default:
                break;
        }
        return moves;
    }
 
    // returns a vector of board positions that a piece can move in a line
    std::vector<boardPos> movesInLine(bool isMovingPieceWhite, int row, int column, int rowChange, int columnChange)
    {
        std::vector<boardPos> moves;
        for (int i = 1; ; i++) {
            // check if piece can move to location
            if (validMove(isMovingPieceWhite, row + i * rowChange, column + i * columnChange)) {
                moves.push_back((boardPos) {
                    row + i * rowChange, column + i * columnChange
                });
                // if piece is capturable, stop moving beyond it
                if (getPiece(row + i * rowChange, column + i * columnChange) != e) {
                    break;
                }
                // if unable to move, break out
            } else {
                break;
            }
        }
        return moves;
    }
 
    // returns if a piece can move to a given location
    bool validMove(bool isMovingPieceWhite, int row, int column)
    {
        if (row < 0 || row > 7 || column < 0 || column > 7) {
            return false;
        }
        Piece capturedPiece = getPiece(row, column);
        switch(capturedPiece) {
            case wK:
            case wQ:
            case wR:
            case wB:
            case wN:
            case w:
                return !isMovingPieceWhite;
            case bK:
            case bQ:
            case bR:
            case bB:
            case bN:
            case b:
                return isMovingPieceWhite;
            case e:
                return true;
        }
        return false;
    }
};
 
class GameBoard
{
private:
    BoardState boardState;
    static const int HOVER_COLOR = 0x0000ff;
    static const int SELECTED_COLOR = 0xff8800;
    static const int MOVE_COLOR = 0xff00ff;
 
    // piece sprites (12 x 12)
    static void drawPawn(int row, int column, bool white, bool light)
    {
        int X = white ? 0xffffff : 0x000000;
        int _ = light ? BOARD_LIGHT_COLOR : BOARD_DARK_COLOR;
        int sprite[144] = {_, _, _, _, _, _, _, _, _, _, _, _,
                           _, _, _, _, _, _, _, _, _, _, _, _,
                           _, _, _, _, _, _, _, _, _, _, _, _,
                           _, _, _, _, _, _, _, _, _, _, _, _,
                           _, _, _, _, _, X, X, _, _, _, _, _,
                           _, _, _, _, X, X, X, X, _, _, _, _,
                           _, _, _, _, X, X, X, X, _, _, _, _,
                           _, _, _, _, _, X, X, _, _, _, _, _,
                           _, _, _, _, _, X, X, _, _, _, _, _,
                           _, _, _, _, X, X, X, X, _, _, _, _,
                           _, _, X, X, X, X, X, X, X, X, _, _,
                           _, _, X, X, X, X, X, X, X, X, _, _
                          };
        pixelCoord tl = getTopLeftOfSquare(row, column);
        uLCD.BLIT(tl.x + 2, tl.y + 2, 12, 12, sprite);
    }
 
    static void drawRook(int row, int column, bool white, bool light)
    {
        int X = white ? 0xffffff : 0x000000;
        int _ = light ? BOARD_LIGHT_COLOR : BOARD_DARK_COLOR;
        int sprite[144] = {X, X, _, X, X, _, _, X, X, _, X, X,
                           X, X, _, X, X, _, _, X, X, _, X, X,
                           X, X, X, X, X, X, X, X, X, X, X, X,
                           X, X, X, X, X, X, X, X, X, X, X, X,
                           _, X, X, X, _, X, X, _, X, X, X, _,
                           _, X, X, X, _, X, X, _, X, X, X, _,
                           _, _, X, X, _, X, X, _, X, X, _, _,
                           _, _, X, X, _, X, X, _, X, X, _, _,
                           _, _, X, X, _, X, X, _, X, X, _, _,
                           _, X, X, X, X, X, X, X, X, X, X, _,
                           X, X, X, X, X, X, X, X, X, X, X, X,
                           X, X, X, X, X, X, X, X, X, X, X, X
                          };
        pixelCoord tl = getTopLeftOfSquare(row, column);
        uLCD.BLIT(tl.x + 2, tl.y + 2, 12, 12, sprite);
    }
 
    static void drawKnight(int row, int column, bool white, bool light)
    {
        int X = white ? 0xffffff : 0x000000;
        int _ = light ? BOARD_LIGHT_COLOR : BOARD_DARK_COLOR;
        int sprite[144] = {_, _, _, _, _, _, _, _, _, _, _, _,
                           _, _, _, _, _, X, X, _, X, X, _, _,
                           _, _, _, _, _, X, X, _, X, X, _, _,
                           _, _, _, X, X, X, X, X, X, _, _, _,
                           _, _, X, X, X, X, X, _, X, _, _, _,
                           _, _, X, X, X, X, X, X, X, _, _, _,
                           _, _, _, _, _, X, X, X, X, _, _, _,
                           _, _, _, _, X, X, X, X, X, _, _, _,
                           _, _, _, X, X, X, X, X, X, X, _, _,
                           _, _, X, X, X, X, X, X, X, X, _, _,
                           _, X, X, X, X, X, X, X, X, X, X, _,
                           _, X, X, X, X, X, X, X, X, X, X, _
                          };
        pixelCoord tl = getTopLeftOfSquare(row, column);
        uLCD.BLIT(tl.x + 2, tl.y + 2, 12, 12, sprite);
    }
 
    static void drawBishop(int row, int column, bool white, bool light)
    {
        int X = white ? 0xffffff : 0x000000;
        int _ = light ? BOARD_LIGHT_COLOR : BOARD_DARK_COLOR;
        int sprite[144] = {_, _, _, _, _, X, X, _, _, _, _, _,
                           _, _, _, _, X, X, X, _, _, _, _, _,
                           _, _, _, X, X, X, _, _, X, _, _, _,
                           _, _, _, X, X, _, _, X, X, _, _, _,
                           _, _, _, X, X, X, X, X, X, _, _, _,
                           _, _, _, _, X, X, X, X, _, _, _, _,
                           _, _, _, _, _, X, X, _, _, _, _, _,
                           _, _, _, _, X, X, X, X, _, _, _, _,
                           _, _, _, X, X, X, X, X, X, _, _, _,
                           _, _, _, X, X, X, X, X, X, _, _, _,
                           _, _, X, X, X, X, X, X, X, X, _, _,
                           _, _, X, X, X, X, X, X, X, X, _, _
                          };
        pixelCoord tl = getTopLeftOfSquare(row, column);
        uLCD.BLIT(tl.x + 2, tl.y + 2, 12, 12, sprite);
    }
 
    static void drawQueen(int row, int column, bool white, bool light)
    {
        int X = white ? 0xffffff : 0x000000;
        int _ = light ? BOARD_LIGHT_COLOR : BOARD_DARK_COLOR;
        int sprite[144] = {_, _, _, _, _, X, X, _, _, _, _, _,
                           _, _, X, _, _, X, X, _, _, X, _, _,
                           X, _, X, X, _, X, X, _, X, X, _, X,
                           X, _, X, X, _, X, X, _, X, X, _, X,
                           X, _, X, X, _, X, X, _, X, X, _, X,
                           X, X, X, X, X, X, X, X, X, X, X, X,
                           X, X, X, X, X, X, X, X, X, X, X, X,
                           X, X, _, X, X, X, X, X, X, _, X, X,
                           X, X, X, X, _, X, X, _, X, X, X, X,
                           _, X, X, X, X, X, X, X, X, X, X, _,
                           _, _, X, X, X, X, X, X, X, X, _, _,
                           _, X, X, X, X, X, X, X, X, X, X, _
                          };
        pixelCoord tl = getTopLeftOfSquare(row, column);
        uLCD.BLIT(tl.x + 2, tl.y + 2, 12, 12, sprite);
    }
 
    static void drawKing(int row, int column, bool white, bool light)
    {
        int X = white ? 0xffffff : 0x000000;
        int _ = light ? BOARD_LIGHT_COLOR : BOARD_DARK_COLOR;
        int sprite[144] = {_, _, _, _, _, X, X, _, _, _, _, _,
                           _, _, _, _, _, X, X, _, _, _, _, _,
                           _, _, _, X, X, X, X, X, X, _, _, _,
                           _, _, _, X, X, X, X, X, X, _, _, _,
                           X, X, _, _, _, X, X, _, _, _, X, X,
                           X, X, X, X, _, X, X, _, X, X, X, X,
                           X, _, X, X, X, X, X, X, X, X, _, X,
                           X, X, X, X, X, X, X, X, X, X, X, X,
                           X, X, X, _, X, X, X, X, _, X, X, X,
                           _, X, X, X, X, X, X, X, X, X, X, _,
                           _, _, X, X, X, _, _, X, X, X, _, _,
                           _, X, X, X, X, X, X, X, X, X, X, _
                          };
        pixelCoord tl = getTopLeftOfSquare(row, column);
        uLCD.BLIT(tl.x + 2, tl.y + 2, 12, 12, sprite);
    }
 
public:
    BoardState getBoardState()
    {
        return boardState;
    }
 
    void setBoardState(BoardState newBoardState)
    {
        boardState = newBoardState;
    }
 
    // initializes the starting board state
    GameBoard()
    {
        // draw board
        for (int row = 0; row < 8; row++) {
            for (int column = 0; column < 8; column++) {
                uint64_t color;
                if ((row + column) % 2 == 0) {
                    color = BOARD_DARK_COLOR;
                } else {
                    color = BOARD_LIGHT_COLOR;
                }
                pixelCoord tl = getTopLeftOfSquare(row, column);
                uLCD.filled_rectangle(tl.x, tl.y, tl.x + 15, tl.y + 15, color);
            }
        }
        // draw pieces
        placePieceAndDraw(wR, 0, 0);
        placePieceAndDraw(wN, 0, 1);
        placePieceAndDraw(wB, 0, 2);
        placePieceAndDraw(wQ, 0, 3);
        placePieceAndDraw(wK, 0, 4);
        placePieceAndDraw(wB, 0, 5);
        placePieceAndDraw(wN, 0, 6);
        placePieceAndDraw(wR, 0, 7);
        placePieceAndDraw(bR, 7, 0);
        placePieceAndDraw(bN, 7, 1);
        placePieceAndDraw(bB, 7, 2);
        placePieceAndDraw(bQ, 7, 3);
        placePieceAndDraw(bK, 7, 4);
        placePieceAndDraw(bB, 7, 5);
        placePieceAndDraw(bN, 7, 6);
        placePieceAndDraw(bR, 7, 7);
        for (int i = 0; i < 8; i++) {
            placePieceAndDraw(w, 1, i);
            placePieceAndDraw(b, 6, i);
        }
    }
    
    // gets the pixel coordinates of the top left of the square
    static pixelCoord getTopLeftOfSquare(boardPos pos)
    {
        return getTopLeftOfSquare(pos.row, pos.column);
    }
    static pixelCoord getTopLeftOfSquare(int row, int column)
    {
        pixelCoord topLeft;
        topLeft.x = 16 * column;
        topLeft.y = 112 - 16 * row;
        return topLeft;
    }
 
    // PIECE MOVEMENT AND GRAPHICS FUNCTIONS
 
    // returns the piece at a given location
    Piece getPiece(boardPos pos)
    {
        return getPiece(pos.row, pos.column);
    }
    Piece getPiece(int row, int column)
    {
        return boardState.getPiece(row, column);
    }
 
    /*  puts the bit representation of a piece at the set position of the board
        assumes that the position of the board is emptied beforehand
    */
    void placePieceAndDraw(Piece piece, boardPos pos)
    {
        placePieceAndDraw(piece, pos.row, pos.column);
    }
    void placePieceAndDraw(Piece piece, int row, int column)
    {
        boardState.placePiece(piece, row, column);
        pixelCoord tl = getTopLeftOfSquare(row, column);
        switch(piece) {
            case wK:
            case bK:
                drawKing(row, column, piece==wK, (row+column)%2);
                break;
            case wQ:
            case bQ:
                drawQueen(row, column, piece==wQ, (row+column)%2);
                break;
            case wB:
            case bB:
                drawBishop(row, column, piece==wB, (row+column)%2);
                break;
            case wN:
            case bN:
                drawKnight(row, column, piece==wN, (row+column)%2);
                break;
            case wR:
            case bR:
                drawRook(row, column, piece==wR, (row+column)%2);
                break;
            case w:
            case b:
                if (piece == w && row == 7) {
                    drawQueen(row, column, true, (row+column)%2);
                } else if (piece == b && row == 0) {
                    drawQueen(row, column, false, (row+column)%2);
                } else {
                    drawPawn(row, column, piece==w, (row+column)%2);
                }
                break;
 
        }
    }
 
    /*  removes a piece from the set position of the board
        returns the bit representation of the piece
    */
    Piece removePieceAndDraw(boardPos pos)
    {
        return removePieceAndDraw(pos.row, pos.column);
    }
    Piece removePieceAndDraw(int row, int column)
    {
        Piece removedPiece = boardState.removePiece(row, column);
        pixelCoord tl = getTopLeftOfSquare(row, column);
        uint64_t color;
        if ((row + column) % 2 == 0) {
            color = BOARD_DARK_COLOR;
        } else {
            color = BOARD_LIGHT_COLOR;
        }
        uLCD.filled_rectangle(tl.x+2, tl.y+2, tl.x + 13, tl.y + 13, color);
        return removedPiece;
    }
 
    /*  moves a piece from one position to another
        returns the captured piece
    */
    Piece movePieceAndDraw(boardPos startPos, boardPos endPos)
    {
        return movePieceAndDraw(startPos.row, startPos.column, endPos.row, endPos.column);
    }
    Piece movePieceAndDraw(int startRow, int startColumn, int endRow, int endColumn)
    {
        Piece movingPiece = removePieceAndDraw(startRow, startColumn);
        Piece capturedPiece = boardState.removePiece(endRow, endColumn);
        placePieceAndDraw(movingPiece, endRow, endColumn);
        return capturedPiece;
    }
 
    // SQUARE BORDER GRAPHICS FUNCTIONS
 
    // removes selection border around square
    void unselectSquare(boardPos pos)
    {
        unselectSquare(pos.row, pos.column);
    }
    void unselectSquare(int row, int column)
    {
        if (row < 0 || row > 7 || column < 0 || column > 7) {
            return;
        }
        pixelCoord tl = getTopLeftOfSquare(row, column);
        uint64_t color;
        if ((row + column) % 2 == 0) {
            color = BOARD_DARK_COLOR;
        } else {
            color = BOARD_LIGHT_COLOR;
        }
        uLCD.rectangle(tl.x, tl.y, tl.x + 15, tl.y + 15, color);
    }
 
    // draws the hover border around square
    void hoverSquare(boardPos pos)
    {
        hoverSquare(pos.row, pos.column);
    }
    void hoverSquare(int row, int column)
    {
        pixelCoord tl = getTopLeftOfSquare(row, column);
        uLCD.rectangle(tl.x, tl.y, tl.x + 15, tl.y + 15, HOVER_COLOR);
    }
 
    // draws selection border around square
    void selectSquare(boardPos pos)
    {
        selectSquare(pos.row, pos.column);
    }
    void selectSquare(int row, int column)
    {
        if (row < 0 || row > 7 || column < 0 || column > 7) {
            return;
        }
        pixelCoord tl = getTopLeftOfSquare(row, column);
        uLCD.rectangle(tl.x, tl.y, tl.x + 15, tl.y + 15, SELECTED_COLOR);
    }
 
    // draws the movement border around square
    void movementSquare(boardPos pos)
    {
        movementSquare(pos.row, pos.column);
    }
    void movementSquare(int row, int column)
    {
        pixelCoord tl = getTopLeftOfSquare(row, column);
        uLCD.rectangle(tl.x, tl.y, tl.x + 15, tl.y + 15, MOVE_COLOR);
    }
};
 
// game variables
GameBoard gameBoard;
GameState state = whiteSelecting;
boardPos cursorPos = (boardPos)
{
    3, 4
};
bool OnePlayer = true;
boardPos selectedPos;
Piece selectedPiece;
std::vector<boardPos> possibleMoves;

void playGameOverTune()
{
    speaker.period(1.0/261.63);
    speaker = 0.5;
    wait(0.5);
    speaker = 0.0;
    wait(0.01);
    speaker.period(1.0/293.66);
    speaker = 0.5;
    wait(0.5);
    speaker = 0.0;
    wait(0.01);
    speaker.period(1.0/329.63);
    speaker = 0.5;
    wait(0.5);
    speaker = 0.0;
}
 
// callbacks
void moveCursor(int rowChange, int columnChange)
{
    // calculate new positoin that is within board bounds
    int newRow = cursorPos.row + rowChange;
    int newColumn = cursorPos.column + columnChange;
    if (newRow > 7 || newRow < 0 || newColumn > 7 || newColumn < 0) {
        bad_move = 1;
        wait(0.2);
        bad_move = 0;
        wait(0.02);
    }
    newRow = newRow <= 7 ? newRow : 7;
    newRow = newRow >= 0 ? newRow : 0;
    newColumn = newColumn <= 7 ? newColumn : 7;
    newColumn = newColumn >= 0 ? newColumn : 0;
    boardPos newPos = (boardPos) {
        newRow, newColumn
    };
 
    // draw border around square that should be there after moving cursor off
    if (cursorPos == selectedPos) {
        gameBoard.selectSquare(cursorPos);
    } else if (std::find(possibleMoves.begin(), possibleMoves.end(), cursorPos) != possibleMoves.end()) {
        gameBoard.movementSquare(cursorPos);
    } else {
        gameBoard.unselectSquare(cursorPos);
    }
 
    // draw hover rectangle over new square
    cursorPos = newPos;
    gameBoard.hoverSquare(cursorPos);
}

void reset_game(void) {
    BoardState newBS;
    for (int i = 0; i < 64; i++) {
        newBS.placePiece(e, i/8, i%8);
    }
    gameBoard.setBoardState(newBS);
    // draw board
    for (int row = 0; row < 8; row++) {
        for (int column = 0; column < 8; column++) {
            uint64_t color;
            if ((row + column) % 2 == 0) {
                color = BOARD_DARK_COLOR;
            } else {
                color = BOARD_LIGHT_COLOR;
            }
            pixelCoord tl = gameBoard.getTopLeftOfSquare(row, column);
            uLCD.filled_rectangle(tl.x, tl.y, tl.x + 15, tl.y + 15, color);
        }
    }
    // draw pieces
    gameBoard.placePieceAndDraw(wR, 0, 0);
    gameBoard.placePieceAndDraw(wN, 0, 1);
    gameBoard.placePieceAndDraw(wB, 0, 2);
    gameBoard.placePieceAndDraw(wQ, 0, 3);
    gameBoard.placePieceAndDraw(wK, 0, 4);
    gameBoard.placePieceAndDraw(wB, 0, 5);
    gameBoard.placePieceAndDraw(wN, 0, 6);
    gameBoard.placePieceAndDraw(wR, 0, 7);
    gameBoard.placePieceAndDraw(bR, 7, 0);
    gameBoard.placePieceAndDraw(bN, 7, 1);
    gameBoard.placePieceAndDraw(bB, 7, 2);
    gameBoard.placePieceAndDraw(bQ, 7, 3);
    gameBoard.placePieceAndDraw(bK, 7, 4);
    gameBoard.placePieceAndDraw(bB, 7, 5);
    gameBoard.placePieceAndDraw(bN, 7, 6);
    gameBoard.placePieceAndDraw(bR, 7, 7);
    for (int i = 0; i < 8; i++) {
        gameBoard.placePieceAndDraw(w, 1, i);
        gameBoard.placePieceAndDraw(b, 6, i);
    }
    
    state = whiteSelecting;
    cursorPos = (boardPos)
    {
        3, 4
    };
    if (game_mode_switch) {
        OnePlayer = true;
    } else {
        OnePlayer = false;
    }
    selectedPos = (boardPos)
    {
        10, 10
    };
    possibleMoves.clear();
    moveCursor(0, 0);
}
 
void joyStickUp()
{
    moveCursor(1, 0);
}
 
void joyStickDown()
{
    moveCursor(-1, 0);
}
 
void joyStickLeft()
{
    moveCursor(0, -1);
}
 
void joyStickRight()
{
    moveCursor(0, 1);
}
 
void joyStickPressed()
{
    switch(state) {
        case whiteSelecting:
        case blackSelecting: {
            possibleMoves.clear();
            selectedPos = cursorPos;
            Piece tempPiece = gameBoard.getPiece(cursorPos);
            std::vector<Piece> pickablePieces = state == whiteSelecting ? whitePieces : blackPieces;
            // check that piece is white and able to be picked up
            if (std::find(pickablePieces.begin(), pickablePieces.end(), tempPiece) != pickablePieces.end()) {
                selectedPiece = tempPiece;
                possibleMoves = gameBoard.getBoardState().getMoves(cursorPos);
                // draw movement squares
                for (std::vector<boardPos>::iterator it = possibleMoves.begin(); it != possibleMoves.end(); ++it) {
                    gameBoard.movementSquare(*it);
                }
                gameBoard.selectSquare(selectedPos);
                // transition state
                if (OnePlayer) {
                    state = whitePickedUp;
                } else {
                    state = state == whiteSelecting ? whitePickedUp : blackPickedUp;
                }
            } else {
                selectedPos = (boardPos) {
                    10, 10
                };
                bad_move = 1;
                wait(0.1);
                bad_move = 0;
                wait(0.02);
            }
            break;
        }
        case whitePickedUp:
        case blackPickedUp: {
            // check if move is valid
            if (std::find(possibleMoves.begin(), possibleMoves.end(), cursorPos) != possibleMoves.end()) {
                // move the piece
                Piece capturedPiece = gameBoard.movePieceAndDraw(selectedPos, cursorPos);
                // check for king capture
                if (state == whitePickedUp && capturedPiece == bK) {
                    uLCD.cls();
                    uLCD.text_height(2);
                    uLCD.text_width(2);
                    uLCD.color(WHITE);
                    uLCD.locate(2,2);
                    uLCD.printf("WHITE");
                    uLCD.locate(2,4);
                    uLCD.printf("WINS");
                    playGameOverTune();
                    while(1) {
                        if (!reset_button) {
                            reset_game();
                            break;
                        }
                    }
                    break;
                } else if (state == blackPickedUp && capturedPiece == wK) {
                    uLCD.cls();
                    uLCD.text_height(2);
                    uLCD.text_width(2);
                    uLCD.color(WHITE);
                    uLCD.locate(2,2);
                    uLCD.printf("BLACK");
                    uLCD.locate(2,4);
                    uLCD.printf("WINS");
                    playGameOverTune();
                    while(1) {
                        if (!reset_button) {
                            reset_game();
                            break;
                        }
                    }
                    break;
                }
                // transition state
                if (OnePlayer) {
                    state = blackAI;
                } else {
                    state = state == whitePickedUp ? blackSelecting : whiteSelecting;
                }
                // check if placing piece back down
            } else {
                // transition state
                state = state == whitePickedUp ? whiteSelecting : blackSelecting;
            }
            // unselect movement squares
            for (std::vector<boardPos>::iterator it = possibleMoves.begin(); it != possibleMoves.end(); ++it) {
                gameBoard.unselectSquare(*it);
            }
            gameBoard.unselectSquare(selectedPos);
            gameBoard.hoverSquare(cursorPos);
            possibleMoves.clear();
            selectedPos = (boardPos) {
                10, 10
            };
            break;
        }
        case whiteAI:
            break;
        case blackAI: {
            boardPos bestMoveSourceDepth0 = (boardPos) {
                0, 0
            };
            boardPos bestMoveDestDepth0 =  (boardPos) {
                0, 0
            };
            float bestMoveValueDepth0 = 100000.0;
            for (int i = 0; i < 64; i++) {
                boardPos currSourceDepth0 =  (boardPos) {
                    i/8, i%8
                };
                Piece currPieceDepth0 = gameBoard.getBoardState().getPiece(i/8, i%8);
                if (currPieceDepth0 == bK || currPieceDepth0 == bQ || currPieceDepth0 == bR || currPieceDepth0 == bB || currPieceDepth0 == bN || currPieceDepth0 == b) {
                    std::vector<boardPos> possibleMovesDepth0 = gameBoard.getBoardState().getMoves(currSourceDepth0);
                    for (std::vector<boardPos>::iterator it = possibleMovesDepth0.begin(); it != possibleMovesDepth0.end(); ++it) {
                        BoardState bsDepth0;
                        for (int j = 0; j < 64; j++) {
                            bsDepth0.placePiece(gameBoard.getBoardState().getPiece(j/8, j%8), j/8, j%8);
                        }
                        bsDepth0.movePiece(currSourceDepth0.row, currSourceDepth0.column, (*it).row, (*it).column);
                        float bestMoveValueDepth1 = -100000.0;
                        for (int i2 = 0; i2 < 64; i2++) {
                            boardPos currSourceDepth1 =  (boardPos) {
                                i2/8, i2%8
                            };
                            Piece currPieceDepth1 = bsDepth0.getPiece(i2/8, i2%8);
                            if (currPieceDepth1 == wK || currPieceDepth1 == wQ || currPieceDepth1 == wR || currPieceDepth1 == wB || currPieceDepth1 == wN || currPieceDepth1 == w) {
                                std::vector<boardPos> possibleMovesDepth1 = bsDepth0.getMoves(currSourceDepth1);
                                for (std::vector<boardPos>::iterator it2 = possibleMovesDepth1.begin(); it2 != possibleMovesDepth1.end(); ++it2) {
                                    BoardState bsDepth1;
                                    for (int j2 = 0; j2 < 64; j2++) {
                                        bsDepth1.placePiece(bsDepth0.getPiece(j2/8, j2%8), j2/8, j2%8);
                                    }
                                    bsDepth1.movePiece(currSourceDepth1.row, currSourceDepth1.column, (*it2).row, (*it2).column);
                                    float bestMoveValueDepth2 = 100000.0;
                                    for (int i3 = 0; i3 < 64; i3++) {
                                        boardPos currSourceDepth2 =  (boardPos) {
                                            i3/8, i3%8
                                        };
                                        Piece currPieceDepth2 = bsDepth1.getPiece(i3/8, i3%8);
                                        if (currPieceDepth2 == bK || currPieceDepth2 == bQ || currPieceDepth2 == bR || currPieceDepth2 == bB || currPieceDepth2 == bN || currPieceDepth2 == b) {
                                            std::vector<boardPos> possibleMovesDepth2 = bsDepth1.getMoves(currSourceDepth2);
                                            for (std::vector<boardPos>::iterator it3 = possibleMovesDepth2.begin(); it3 != possibleMovesDepth2.end(); ++it3) {
                                                BoardState bsDepth2;
                                                for (int j3 = 0; j3 < 64; j3++) {
                                                    bsDepth2.placePiece(bsDepth1.getPiece(j3/8, j3%8), j3/8, j3%8);
                                                }
                                                bsDepth2.movePiece(currSourceDepth2.row, currSourceDepth2.column, (*it3).row, (*it3).column);
                                                float thisMoveValueDepth2 = bsDepth2.calculateBoardState();
                                                if (thisMoveValueDepth2 < bestMoveValueDepth2) {
                                                    bestMoveValueDepth2 = thisMoveValueDepth2;
                                                }
                                            }
                                        }
                                    }
                                    if (bestMoveValueDepth2 > bestMoveValueDepth1) {
                                        bestMoveValueDepth1 = bestMoveValueDepth2;
                                    }
                                }
                            }
                        }
 
                        if (bestMoveValueDepth1 < bestMoveValueDepth0) {
                            bestMoveSourceDepth0 = currSourceDepth0;
                            bestMoveDestDepth0 = *it;
                            bestMoveValueDepth0 = bestMoveValueDepth1;
                        }
                    }
                }
            }
            Piece capturedPiece = gameBoard.movePieceAndDraw(bestMoveSourceDepth0, bestMoveDestDepth0);
            if (capturedPiece == wK) {
                uLCD.cls();
                uLCD.text_height(2);
                uLCD.text_width(2);
                uLCD.color(WHITE);
                uLCD.locate(2,2);
                uLCD.printf("BLACK");
                uLCD.locate(2,4);
                uLCD.printf("WINS");
                playGameOverTune();
                while(1) {
                    if (!reset_button) {
                        reset_game();
                        break;
                    }
                }
                break;
            }
            state = whiteSelecting;
            break;
        }
        default: {
            break;
        }
    }
}
 
// bluetooth
volatile bool button_ready = 0;
volatile int bnum = 0;
volatile int bhit = 0;
enum statetype {start = 0, got_exclm, got_B, got_num, got_hit};
statetype bluetooth_state = start;
 
void parse_message()
{
    switch (bluetooth_state) {
        case start:
            if (Blue.getc() == '!') bluetooth_state = got_exclm;
            else bluetooth_state = start;
            break;
        case got_exclm:
            if (Blue.getc() == 'B') bluetooth_state = got_B;
            else bluetooth_state = start;
            break;
        case got_B:
            bnum = Blue.getc();
            bluetooth_state = got_num;
            break;
        case got_num:
            bhit = Blue.getc();
            bluetooth_state = got_hit;
            break;
        case got_hit:
            if (Blue.getc() == char(~('!' + ' B' + bnum + bhit))) button_ready = 1;
            bluetooth_state = start;
            break;
        default:
            Blue.getc();
            bluetooth_state = start;
    }
}
 
Nav_Switch myNav(p9, p6, p7, p5, p8); //pin order on Sparkfun breakout
 
int main()
{
    reset_button.mode(PullUp);
    game_mode_switch.mode(PullUp);
    wait(0.01);
    whitePieces.push_back(wK);
    whitePieces.push_back(wQ);
    whitePieces.push_back(wB);
    whitePieces.push_back(wN);
    whitePieces.push_back(wR);
    whitePieces.push_back(w);
    blackPieces.push_back(bK);
    blackPieces.push_back(bQ);
    blackPieces.push_back(bB);
    blackPieces.push_back(bN);
    blackPieces.push_back(bR);
    blackPieces.push_back(b);
 
    if (game_mode_switch) {
        OnePlayer = true;
    } else {
        OnePlayer = false;
    }
    
    moveCursor(0, 0);
    Blue.attach(&parse_message,Serial::RxIrq);
 
    while (1) {
        if (!reset_button) {
            reset_game();
        } else if (state == blackAI) {
            joyStickPressed();
        } else if (myNav.up()) {
            joyStickUp();
        } else if (myNav.down()) {
            joyStickDown();
        } else if (myNav.left()) {
            joyStickLeft();
        } else if (myNav.right()) {
            joyStickRight();
        } else if (myNav.fire()) {
            joyStickPressed();
        } else if (button_ready && bhit == '1') {
            switch(bnum) {
                case '1':
                    joyStickPressed();
                    break;
                case '2':
                    reset_game();
                    break;
                case '5':
                    joyStickUp();
                    break;
                case '6':
                    joyStickDown();
                    break;
                case '7':
                    joyStickLeft();
                    break;
                case '8':
                    joyStickRight();
                    break;
            }
            button_ready = false;
        }
        wait(0.2);
    }
}