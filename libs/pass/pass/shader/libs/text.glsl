/*   ### How to use this shader ? ###

   = Setup =
   0. Copy the content of the Common Tab inside your shader
   1. Make sure the FONT_TEXTURE #define is set to the iChannel
      containing the alphabet texture

   = Declare String =
   2. Use makeStr to declare a new string (needs to be done outside any function)
   3. Write your text using _ before each char, and __ for spaces
   4. Finish your string with the _end keyword

       makeStr(printAnother) _A _n _o _t _h _e _r __ _E _x _a _m _p _l _e    _end

   = Print String =
   5. Call the new function by passing it your uvs. It returns a grayscale value.

       finalCol += printAnother(uv);

   Note that by default a character has a height of 1 (which is full height if
   the coordinates are normalized). You are responsible for scaling/offsetting
   the uvs to control the text placement before calling the function.


   ### Characters available ###

   uppercase: _A _B _C ...
   lowercase: _a _b _c ...
   digits   : _0 _1 _2 ...
   special  : _EXC _QUOT ... (see Common)


   ### Javascript string generator helper ###

   To make things even easier, I've made a tiny javascript helper function that you
   can use to convert strings to the right _F _o _r _m _a _t !
   (I've written it in the comments of this shader)


   ### Special functions ###

   _dig(i)       : write a specific digit [i ranges between 0-9]
   _dec(i, prec) : write a floating point number [prec: number of decimals to print]
   _ch(i)        : Write an uppercase character [i ranges between 0-25]

   To use these special functions, you need to pass additional parameters into makeStr:

   "makeStr1i" allows you to pass 1 int parameter named "i" :

   makeStr1i(test) _dig(i) _end
   test(uv, 5);

   "makeStr1f" allows you to pass 1 float parameter named "i":

   makeStr1f(test) _dec(i, 3) _end
   test(uv, 5.);

   You can also create your own makeStr with any parameter type! (see below)


   ### Help me optimize it! ###

   The _dec() function is only for debug purposes, I've never used it in a published shader.
   It's *very* intensive and can have rounding issues (ie 21.999 when the float is 22.),
   however it's done with a pretty naive approach so if you can come up with a better one
   I'd be glad! The function is defined at the very end of this tab.
*/

/// SETTINGS ///

// Set to the iChannel containing the alphabet texture
#define FONT_TEXTURE iChannel0

// Horizontal character spacing (default: 0.5)
#define CHAR_SPACING 0.44


/// STRING CREATION ///

// Create a basic string
#define makeStr(func_name) float func_name(vec2 u) { _print

// Create a string with an int parameter
#define makeStr1i(func_name) float func_name(vec2 u, int i) { _print

// Create a string with a float parameter
#define makeStr1f(func_name) float func_name(vec2 u, float i) { _print

// Create a string with two floats parameter
#define makeStr2f(func_name) float func_name(vec2 u, float i, float j) { _print

// ... Or create your own strings with any parameters
#define makeStrXX(func_name) float func_name(vec2 u, ...) { _print

// Terminate a string
#define _end    ); return d; }


/// SPECIAL FUNCTIONS ///

// Dynamic uppercase character
// i: [0-25]
#define _ch(i)  _ 65+int(i)

// Dynamic digit
// i: [0-9]
#define _dig(i) _ 48+int(i)

// Floating point debug
// x:   value to print
// dec: number of decimal places to print
#define _dec(x, dec) ); d += _decimal(FONT_TEXTURE, u, x, dec); (0


/// SPECIAL CHARACTERS ///

// Space
#define __    ); u.x -= CHAR_SPACING; (0

#define _EXC  _ 33 // " ! "
#define _DBQ  _ 34 // " " "
#define _NUM  _ 35 // " # "
#define _DOL  _ 36 // " $ "
#define _PER  _ 37 // " % "
#define _AMP  _ 38 // " & "
#define _QUOT _ 39 // " ' "
#define _LPR  _ 40 // " ( "
#define _RPR  _ 41 // " ) "
#define _MUL  _ 42 // " * "
#define _ADD  _ 43 // " + "
#define _COM  _ 44 // " , "
#define _SUB  _ 45 // " - "
#define _DOT  _ 46 // " . "
#define _DIV  _ 47 // " / "
#define _COL  _ 58 // " : "
#define _SEM  _ 59 // " ; "
#define _LES  _ 60 // " < "
#define _EQU  _ 61 // " = "
#define _GRE  _ 62 // " > "
#define _QUE  _ 63 // " ? "
#define _AT   _ 64 // " @ "
#define _LBR  _ 91 // " [ "
#define _ANTI _ 92 // " \ "
#define _RBR  _ 93 // " ] "
#define _UND  _ 95 // " _ "


/// CHARACTER DEFINITIONS ///

// Uppercase letters (65-90)
#define _A _ 65
#define _B _ 66
#define _C _ 67
#define _D _ 68
#define _E _ 69
#define _F _ 70
#define _G _ 71
#define _H _ 72
#define _I _ 73
#define _J _ 74
#define _K _ 75
#define _L _ 76
#define _M _ 77
#define _N _ 78
#define _O _ 79
#define _P _ 80
#define _Q _ 81
#define _R _ 82
#define _S _ 83
#define _T _ 84
#define _U _ 85
#define _V _ 86
#define _W _ 87
#define _X _ 88
#define _Y _ 89
#define _Z _ 90

// Lowercase letters (97-122)
#define _a _ 97
#define _b _ 98
#define _c _ 99
#define _d _ 100
#define _e _ 101
#define _f _ 102
#define _g _ 103
#define _h _ 104
#define _i _ 105
#define _j _ 106
#define _k _ 107
#define _l _ 108
#define _m _ 109
#define _n _ 110
#define _o _ 111
#define _p _ 112
#define _q _ 113
#define _r _ 114
#define _s _ 115
#define _t _ 116
#define _u _ 117
#define _v _ 118
#define _w _ 119
#define _x _ 120
#define _y _ 121
#define _z _ 122

// Digits (48-57)
#define _0 _ 48
#define _1 _ 49
#define _2 _ 50
#define _3 _ 51
#define _4 _ 52
#define _5 _ 53
#define _6 _ 54
#define _7 _ 55
#define _8 _ 56
#define _9 _ 57

/// Internal functions ///

// Start
#define _print  float d = 0.; (u.x += CHAR_SPACING

// Update
#define _       ); u.x -= CHAR_SPACING; d += _char(FONT_TEXTURE, u,

// Print character
float _char(sampler2D s, vec2 u, int id) {
    vec2 p = vec2(id%16, 15. - floor(float(id)/16.));
         p = (u + p) / 16.;
         u = step(abs(u-.5), vec2(.5));
    return texture(s, p).r * u.x * u.y;
}

// Floating point debug
float _decimal(sampler2D FONT_TEXTURE, inout vec2 u, float n, int decimals) {
    float d = 0., N = 1.; // d is the final color, N the number of digits before the decimal

    if (n < 0.) {  // If the number is negative
        n *= -1.;  // Make it positive
        (0 _SUB ); // Print a minus sign
    }

    // Calculate the number of digits before the decimal point
    for (float x = n; x >= 10.; x /= 10.) N++;

    // Print the digits before the decimal point
    for (float i = 0.; i < N; i++) {
        float magnitude = pow(10., N-i-1.);
        float leftDigit = floor(n / magnitude);
        n -= leftDigit * magnitude;

        (0 _dig(leftDigit) );
    }

    (0 _DOT ); // Print a dot

    // Print the digits after the decimal point
    for (int i = 0; i < decimals; i++) {
        float firstDecimal = floor((n - floor(n)) * 10.);
        n *= 10.;

        (0 _dig(firstDecimal) );
    }

    return d;
}
