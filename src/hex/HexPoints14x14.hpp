
#ifndef HEXPOINT14x14_HPP
#define HEXPOINT14x14_HPP

/** A location on a Hex board.  A HexPoint's neighbours can be
    calculated only when considering what size board the point is in.
    See ConstBoard for an example board layout.

    @note The order of these points if very important. There are
    several pieces of code that rely on this ordering
    (StoneBoard::GetBoardID() is one of them). Change this only if you
    know what you are doing!
*/
typedef enum
{
    /** Dummy point. */
    INVALID_POINT = 0,

    /** Point used to denote a resign move by a player. 

        @todo since this is not really a point on a board, should we
        create a HexMove class and put this there? */
    RESIGN = 1,

    /** Point used to denote a swap move by a player. 

        @todo since this is not really a point on a board, should 
        we create a HexMove class and put this there?  */
    SWAP_PIECES = 2,

    /** The top edge. */
    NORTH = 3,

    /** The right edge. */
    EAST  = 4,

    /** The bottom edge. */
    SOUTH = 5,

    /** The left edge. */
    WEST  = 6,

    /** @name Interior cells. */
    // @{

    HEX_CELL_A1, HEX_CELL_B1, HEX_CELL_C1, HEX_CELL_D1, HEX_CELL_E1,
    HEX_CELL_F1, HEX_CELL_G1, HEX_CELL_H1, HEX_CELL_I1, HEX_CELL_J1,
    HEX_CELL_K1, HEX_CELL_L1, HEX_CELL_M1, HEX_CELL_N1,

    HEX_CELL_A2, HEX_CELL_B2, HEX_CELL_C2, HEX_CELL_D2, HEX_CELL_E2,
    HEX_CELL_F2, HEX_CELL_G2, HEX_CELL_H2, HEX_CELL_I2, HEX_CELL_J2,
    HEX_CELL_K2, HEX_CELL_L2, HEX_CELL_M2, HEX_CELL_N2,

    HEX_CELL_A3, HEX_CELL_B3, HEX_CELL_C3, HEX_CELL_D3, HEX_CELL_E3,
    HEX_CELL_F3, HEX_CELL_G3, HEX_CELL_H3, HEX_CELL_I3, HEX_CELL_J3,
    HEX_CELL_K3, HEX_CELL_L3, HEX_CELL_M3, HEX_CELL_N3,

    HEX_CELL_A4, HEX_CELL_B4, HEX_CELL_C4, HEX_CELL_D4, HEX_CELL_E4,
    HEX_CELL_F4, HEX_CELL_G4, HEX_CELL_H4, HEX_CELL_I4, HEX_CELL_J4,
    HEX_CELL_K4, HEX_CELL_L4, HEX_CELL_M4, HEX_CELL_N4,

    HEX_CELL_A5, HEX_CELL_B5, HEX_CELL_C5, HEX_CELL_D5, HEX_CELL_E5,
    HEX_CELL_F5, HEX_CELL_G5, HEX_CELL_H5, HEX_CELL_I5, HEX_CELL_J5,
    HEX_CELL_K5, HEX_CELL_L5, HEX_CELL_M5, HEX_CELL_N5,

    HEX_CELL_A6, HEX_CELL_B6, HEX_CELL_C6, HEX_CELL_D6, HEX_CELL_E6,
    HEX_CELL_F6, HEX_CELL_G6, HEX_CELL_H6, HEX_CELL_I6, HEX_CELL_J6,
    HEX_CELL_K6, HEX_CELL_L6, HEX_CELL_M6, HEX_CELL_N6,

    HEX_CELL_A7, HEX_CELL_B7, HEX_CELL_C7, HEX_CELL_D7, HEX_CELL_E7,
    HEX_CELL_F7, HEX_CELL_G7, HEX_CELL_H7, HEX_CELL_I7, HEX_CELL_J7,
    HEX_CELL_K7, HEX_CELL_L7, HEX_CELL_M7, HEX_CELL_N7,

    HEX_CELL_A8, HEX_CELL_B8, HEX_CELL_C8, HEX_CELL_D8, HEX_CELL_E8,
    HEX_CELL_F8, HEX_CELL_G8, HEX_CELL_H8, HEX_CELL_I8, HEX_CELL_J8,
    HEX_CELL_K8, HEX_CELL_L8, HEX_CELL_M8, HEX_CELL_N8,

    HEX_CELL_A9, HEX_CELL_B9, HEX_CELL_C9, HEX_CELL_D9, HEX_CELL_E9,
    HEX_CELL_F9, HEX_CELL_G9, HEX_CELL_H9, HEX_CELL_I9, HEX_CELL_J9,
    HEX_CELL_K9, HEX_CELL_L9, HEX_CELL_M9, HEX_CELL_N9,

    HEX_CELL_A10, HEX_CELL_B10, HEX_CELL_C10, HEX_CELL_D10, HEX_CELL_E10,
    HEX_CELL_F10, HEX_CELL_G10, HEX_CELL_H10, HEX_CELL_I10, HEX_CELL_J10,
    HEX_CELL_K10, HEX_CELL_L10, HEX_CELL_M10, HEX_CELL_N10,

    HEX_CELL_A11, HEX_CELL_B11, HEX_CELL_C11, HEX_CELL_D11, HEX_CELL_E11,
    HEX_CELL_F11, HEX_CELL_G11, HEX_CELL_H11, HEX_CELL_I11, HEX_CELL_J11,
    HEX_CELL_K11, HEX_CELL_L11, HEX_CELL_M11, HEX_CELL_N11,

    HEX_CELL_A12, HEX_CELL_B12, HEX_CELL_C12, HEX_CELL_D12, HEX_CELL_E12,
    HEX_CELL_F12, HEX_CELL_G12, HEX_CELL_H12, HEX_CELL_I12, HEX_CELL_J12,
    HEX_CELL_K12, HEX_CELL_L12, HEX_CELL_M12, HEX_CELL_N12,

    HEX_CELL_A13, HEX_CELL_B13, HEX_CELL_C13, HEX_CELL_D13, HEX_CELL_E13,
    HEX_CELL_F13, HEX_CELL_G13, HEX_CELL_H13, HEX_CELL_I13, HEX_CELL_J13,
    HEX_CELL_K13, HEX_CELL_L13, HEX_CELL_M13, HEX_CELL_N13,

    HEX_CELL_A14, HEX_CELL_B14, HEX_CELL_C14, HEX_CELL_D14, HEX_CELL_E14,
    HEX_CELL_F14, HEX_CELL_G14, HEX_CELL_H14, HEX_CELL_I14, HEX_CELL_J14,
    HEX_CELL_K14, HEX_CELL_L14, HEX_CELL_M14, HEX_CELL_N14,
    
    // @}
    
    /** The invalid HexPoint. */
    FIRST_INVALID

} HexPoint;

/** The value of the last interior cell. */
static const HexPoint LAST_CELL = HEX_CELL_N14;

//----------------------------------------------------------------------------

#endif
