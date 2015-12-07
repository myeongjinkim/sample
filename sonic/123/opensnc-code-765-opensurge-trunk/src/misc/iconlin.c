#include <allegro.h>
/* XPM */
static const char *allegico_xpm[] = {
/* columns rows colors chars-per-pixel */
"16 16 118 2",
"   c #5B640D",
".  c #654D28",
"X  c #813D00",
"o  c #964900",
"O  c #CD4D00",
"+  c #CF4C03",
"@  c #CF4D09",
"#  c #DA4A0E",
"$  c #CA5200",
"%  c #CC5202",
"&  c #DE5900",
"*  c #DB4C13",
"=  c #DD5314",
"-  c #DD6004",
";  c #E16100",
":  c #E46C00",
">  c #E26806",
",  c #E0600F",
"<  c #EE7505",
"1  c #F17800",
"2  c #FB7B00",
"3  c #FA7F00",
"4  c #E57E1E",
"5  c #F57119",
"6  c #F15F2A",
"7  c #DF613A",
"8  c #F76C28",
"9  c #E87732",
"0  c #E97732",
"q  c #EB743E",
"w  c #B07779",
"e  c #E2624B",
"r  c #E26653",
"t  c #11842A",
"y  c #EF8000",
"u  c #F48200",
"i  c #F48300",
"p  c #F48400",
"a  c #FA8200",
"s  c #F98400",
"d  c #FA8603",
"f  c #FC8700",
"g  c #FA8007",
"h  c #FB8404",
"j  c #F88406",
"k  c #F98A00",
"l  c #FC8805",
"z  c #FA8008",
"x  c #FA800B",
"c  c #FA810B",
"v  c #FA820B",
"b  c #FC8208",
"n  c #FD800A",
"m  c #FF800B",
"M  c #FC830B",
"N  c #FB830C",
"B  c #FF8809",
"V  c #FF8B0B",
"C  c #FD8815",
"Z  c #F6851B",
"A  c #FA841A",
"S  c #F18919",
"D  c #EF8328",
"F  c #FC882B",
"G  c #FA9534",
"H  c #C39254",
"J  c #F79A52",
"K  c #EEA945",
"L  c #EFA45F",
"P  c #FBAD56",
"I  c #FEAB58",
"U  c #FD9561",
"Y  c #FD976B",
"T  c #FC986C",
"R  c #FD9670",
"E  c #FD9770",
"W  c #F89F76",
"Q  c #FAA663",
"!  c #FAA37C",
"~  c #FDC76D",
"^  c #FFC27C",
"/  c #AD9D8E",
"(  c #94A6B6",
")  c #F9A483",
"_  c #F6A982",
"`  c #FBAC81",
"'  c #FEA2A0",
"]  c #FEA1A2",
"[  c #FEA2A3",
"{  c #FFA5A4",
"}  c #FDADA3",
"|  c #FEA5A8",
" . c #FFA6A9",
".. c #FFA6AC",
"X. c #FEABAC",
"o. c #FEB2A9",
"O. c #FFB3AF",
"+. c #FFA8B0",
"@. c #FFAAB5",
"#. c #FFA9BA",
"$. c #FFADB9",
"%. c #BECE83",
"&. c #DCC78E",
"*. c #CFDB8B",
"=. c #FED894",
"-. c #FED69B",
";. c #FDE19C",
":. c #E5CBA5",
">. c #FCD5A2",
",. c #FFD8A0",
"<. c #FFD9AD",
"1. c #FFE0B5",
"2. c #FFE7B9",
"3. c #FFE7BA",
"4. c #FFEBC7",
"5. c #FBEAD7",
"6. c #FFFFFF",
"7. c None",
/* pixels */
"7.7 Q 7.7.% 7.7.7.7.7.7.7.7.7.7.",
"7.r o.Y e O % 7.7.7.7.7.7.7.7.7.",
"7.e O.' T 9 O @ 7.7.7.7.7.7.7.7.",
"7.* W $.' R 9 $ @ 7.7.7.7.7.7.7.",
"7.# D X...' R 4 @ 7.7.7.7.7.7.7.",
"7.7.& ! @. .{ S @ 7.8 0 0 7.7.7.",
"7.7.7., ) +. .Z 9 8 k 1 : > 7.7.",
"7.7.7.7.* } #.F f l < = 7.7.7.7.",
"7.7.7.7.* J U A 3 N c j 9 7.7.7.",
"7.7.7.7.* i 3 V C 3 c m p 9 7.7.",
"7.7.7.7.9 h z o X > m c h 9 7.7.",
"7.7.7.0 p z 2 / ( . ; C P 9 7.7.",
"7.7.0 5 h z G 5.6.t   ^ <.~ ` 7.",
"7.* * y h I >.:.4.%.*.3.&.w ` 7.",
"7.7.7.* * K ,.H -.3.1.;._ 7.7.7.",
"7.7.7.7.7.* * L H -.=.` 7.7.7.7."
};
#if defined ALLEGRO_WITH_XWINDOWS && defined ALLEGRO_USE_CONSTRUCTOR
extern void *allegro_icon;
CONSTRUCTOR_FUNCTION(static void _set_allegro_icon(void));
static void _set_allegro_icon(void)
{
    allegro_icon = allegico_xpm;
}
#endif
