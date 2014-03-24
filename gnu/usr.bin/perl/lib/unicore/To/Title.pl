# !!!!!!!   DO NOT EDIT THIS FILE   !!!!!!!
# This file is machine-generated by lib/unicore/mktables from the Unicode
# database, Version 6.2.0.  Any changes made here will be lost!

# !!!!!!!   IT IS DEPRECATED TO USE THIS FILE   !!!!!!!

# This file is for internal use by core Perl only.  It is retained for
# backwards compatibility with applications that may have come to rely on it,
# but its format and even its name or existence are subject to change without
# notice in a future Perl version.  Don't use it directly.  Instead, its
# contents are now retrievable through a stable API in the Unicode::UCD
# module: Unicode::UCD::prop_invmap('Titlecase_Mapping').



# The name this swash is to be known by, with the format of the mappings in
# the main body of the table, and what all code points missing from this file
# map to.
$utf8::SwashInfo{'ToTitle'}{'format'} = 'x'; # non-negative hex whole number; a code point
$utf8::SwashInfo{'ToTitle'}{'specials_name'} = 'utf8::ToSpecTitle'; # Name of hash of special mappings
$utf8::SwashInfo{'ToTitle'}{'missing'} = '<code point>'; # code point maps to itself

# Some code points require special handling because their mappings are each to
# multiple code points.  These do not appear in the main body, but are defined
# in the hash below.

# Each key is the string of N bytes that together make up the UTF-8 encoding
# for the code point.  (i.e. the same as looking at the code point's UTF-8
# under "use bytes").  Each value is the UTF-8 of the translation, for speed.
%utf8::ToSpecTitle = (
"\xC3\x9F" => "\x{0053}\x{0073}",             # U+00DF => 0053 0073
"\xC5\x89" => "\x{02BC}\x{004E}",             # U+0149 => 02BC 004E
"\xC7\xB0" => "\x{004A}\x{030C}",             # U+01F0 => 004A 030C
"\xCE\x90" => "\x{0399}\x{0308}\x{0301}",     # U+0390 => 0399 0308 0301
"\xCE\xB0" => "\x{03A5}\x{0308}\x{0301}",     # U+03B0 => 03A5 0308 0301
"\xD6\x87" => "\x{0535}\x{0582}",             # U+0587 => 0535 0582
"\xE1\xBA\x96" => "\x{0048}\x{0331}",         # U+1E96 => 0048 0331
"\xE1\xBA\x97" => "\x{0054}\x{0308}",         # U+1E97 => 0054 0308
"\xE1\xBA\x98" => "\x{0057}\x{030A}",         # U+1E98 => 0057 030A
"\xE1\xBA\x99" => "\x{0059}\x{030A}",         # U+1E99 => 0059 030A
"\xE1\xBA\x9A" => "\x{0041}\x{02BE}",         # U+1E9A => 0041 02BE
"\xE1\xBD\x90" => "\x{03A5}\x{0313}",         # U+1F50 => 03A5 0313
"\xE1\xBD\x92" => "\x{03A5}\x{0313}\x{0300}", # U+1F52 => 03A5 0313 0300
"\xE1\xBD\x94" => "\x{03A5}\x{0313}\x{0301}", # U+1F54 => 03A5 0313 0301
"\xE1\xBD\x96" => "\x{03A5}\x{0313}\x{0342}", # U+1F56 => 03A5 0313 0342
"\xE1\xBE\xB2" => "\x{1FBA}\x{0345}",         # U+1FB2 => 1FBA 0345
"\xE1\xBE\xB4" => "\x{0386}\x{0345}",         # U+1FB4 => 0386 0345
"\xE1\xBE\xB6" => "\x{0391}\x{0342}",         # U+1FB6 => 0391 0342
"\xE1\xBE\xB7" => "\x{0391}\x{0342}\x{0345}", # U+1FB7 => 0391 0342 0345
"\xE1\xBF\x82" => "\x{1FCA}\x{0345}",         # U+1FC2 => 1FCA 0345
"\xE1\xBF\x84" => "\x{0389}\x{0345}",         # U+1FC4 => 0389 0345
"\xE1\xBF\x86" => "\x{0397}\x{0342}",         # U+1FC6 => 0397 0342
"\xE1\xBF\x87" => "\x{0397}\x{0342}\x{0345}", # U+1FC7 => 0397 0342 0345
"\xE1\xBF\x92" => "\x{0399}\x{0308}\x{0300}", # U+1FD2 => 0399 0308 0300
"\xE1\xBF\x93" => "\x{0399}\x{0308}\x{0301}", # U+1FD3 => 0399 0308 0301
"\xE1\xBF\x96" => "\x{0399}\x{0342}",         # U+1FD6 => 0399 0342
"\xE1\xBF\x97" => "\x{0399}\x{0308}\x{0342}", # U+1FD7 => 0399 0308 0342
"\xE1\xBF\xA2" => "\x{03A5}\x{0308}\x{0300}", # U+1FE2 => 03A5 0308 0300
"\xE1\xBF\xA3" => "\x{03A5}\x{0308}\x{0301}", # U+1FE3 => 03A5 0308 0301
"\xE1\xBF\xA4" => "\x{03A1}\x{0313}",         # U+1FE4 => 03A1 0313
"\xE1\xBF\xA6" => "\x{03A5}\x{0342}",         # U+1FE6 => 03A5 0342
"\xE1\xBF\xA7" => "\x{03A5}\x{0308}\x{0342}", # U+1FE7 => 03A5 0308 0342
"\xE1\xBF\xB2" => "\x{1FFA}\x{0345}",         # U+1FF2 => 1FFA 0345
"\xE1\xBF\xB4" => "\x{038F}\x{0345}",         # U+1FF4 => 038F 0345
"\xE1\xBF\xB6" => "\x{03A9}\x{0342}",         # U+1FF6 => 03A9 0342
"\xE1\xBF\xB7" => "\x{03A9}\x{0342}\x{0345}", # U+1FF7 => 03A9 0342 0345
"\xEF\xAC\x80" => "\x{0046}\x{0066}",         # U+FB00 => 0046 0066
"\xEF\xAC\x81" => "\x{0046}\x{0069}",         # U+FB01 => 0046 0069
"\xEF\xAC\x82" => "\x{0046}\x{006C}",         # U+FB02 => 0046 006C
"\xEF\xAC\x83" => "\x{0046}\x{0066}\x{0069}", # U+FB03 => 0046 0066 0069
"\xEF\xAC\x84" => "\x{0046}\x{0066}\x{006C}", # U+FB04 => 0046 0066 006C
"\xEF\xAC\x85" => "\x{0053}\x{0074}",         # U+FB05 => 0053 0074
"\xEF\xAC\x86" => "\x{0053}\x{0074}",         # U+FB06 => 0053 0074
"\xEF\xAC\x93" => "\x{0544}\x{0576}",         # U+FB13 => 0544 0576
"\xEF\xAC\x94" => "\x{0544}\x{0565}",         # U+FB14 => 0544 0565
"\xEF\xAC\x95" => "\x{0544}\x{056B}",         # U+FB15 => 0544 056B
"\xEF\xAC\x96" => "\x{054E}\x{0576}",         # U+FB16 => 054E 0576
"\xEF\xAC\x97" => "\x{0544}\x{056D}",         # U+FB17 => 0544 056D
);

return <<'END';
0061		0041
0062		0042
0063		0043
0064		0044
0065		0045
0066		0046
0067		0047
0068		0048
0069		0049
006A		004A
006B		004B
006C		004C
006D		004D
006E		004E
006F		004F
0070		0050
0071		0051
0072		0052
0073		0053
0074		0054
0075		0055
0076		0056
0077		0057
0078		0058
0079		0059
007A		005A
00B5		039C
00E0		00C0
00E1		00C1
00E2		00C2
00E3		00C3
00E4		00C4
00E5		00C5
00E6		00C6
00E7		00C7
00E8		00C8
00E9		00C9
00EA		00CA
00EB		00CB
00EC		00CC
00ED		00CD
00EE		00CE
00EF		00CF
00F0		00D0
00F1		00D1
00F2		00D2
00F3		00D3
00F4		00D4
00F5		00D5
00F6		00D6
00F8		00D8
00F9		00D9
00FA		00DA
00FB		00DB
00FC		00DC
00FD		00DD
00FE		00DE
00FF		0178
0101		0100
0103		0102
0105		0104
0107		0106
0109		0108
010B		010A
010D		010C
010F		010E
0111		0110
0113		0112
0115		0114
0117		0116
0119		0118
011B		011A
011D		011C
011F		011E
0121		0120
0123		0122
0125		0124
0127		0126
0129		0128
012B		012A
012D		012C
012F		012E
0131		0049
0133		0132
0135		0134
0137		0136
013A		0139
013C		013B
013E		013D
0140		013F
0142		0141
0144		0143
0146		0145
0148		0147
014B		014A
014D		014C
014F		014E
0151		0150
0153		0152
0155		0154
0157		0156
0159		0158
015B		015A
015D		015C
015F		015E
0161		0160
0163		0162
0165		0164
0167		0166
0169		0168
016B		016A
016D		016C
016F		016E
0171		0170
0173		0172
0175		0174
0177		0176
017A		0179
017C		017B
017E		017D
017F		0053
0180		0243
0183		0182
0185		0184
0188		0187
018C		018B
0192		0191
0195		01F6
0199		0198
019A		023D
019E		0220
01A1		01A0
01A3		01A2
01A5		01A4
01A8		01A7
01AD		01AC
01B0		01AF
01B4		01B3
01B6		01B5
01B9		01B8
01BD		01BC
01BF		01F7
01C4		01C5
01C6		01C5
01C7		01C8
01C9		01C8
01CA		01CB
01CC		01CB
01CE		01CD
01D0		01CF
01D2		01D1
01D4		01D3
01D6		01D5
01D8		01D7
01DA		01D9
01DC		01DB
01DD		018E
01DF		01DE
01E1		01E0
01E3		01E2
01E5		01E4
01E7		01E6
01E9		01E8
01EB		01EA
01ED		01EC
01EF		01EE
01F1		01F2
01F3		01F2
01F5		01F4
01F9		01F8
01FB		01FA
01FD		01FC
01FF		01FE
0201		0200
0203		0202
0205		0204
0207		0206
0209		0208
020B		020A
020D		020C
020F		020E
0211		0210
0213		0212
0215		0214
0217		0216
0219		0218
021B		021A
021D		021C
021F		021E
0223		0222
0225		0224
0227		0226
0229		0228
022B		022A
022D		022C
022F		022E
0231		0230
0233		0232
023C		023B
023F		2C7E
0240		2C7F
0242		0241
0247		0246
0249		0248
024B		024A
024D		024C
024F		024E
0250		2C6F
0251		2C6D
0252		2C70
0253		0181
0254		0186
0256		0189
0257		018A
0259		018F
025B		0190
0260		0193
0263		0194
0265		A78D
0266		A7AA
0268		0197
0269		0196
026B		2C62
026F		019C
0271		2C6E
0272		019D
0275		019F
027D		2C64
0280		01A6
0283		01A9
0288		01AE
0289		0244
028A		01B1
028B		01B2
028C		0245
0292		01B7
0345		0399
0371		0370
0373		0372
0377		0376
037B		03FD
037C		03FE
037D		03FF
03AC		0386
03AD		0388
03AE		0389
03AF		038A
03B1		0391
03B2		0392
03B3		0393
03B4		0394
03B5		0395
03B6		0396
03B7		0397
03B8		0398
03B9		0399
03BA		039A
03BB		039B
03BC		039C
03BD		039D
03BE		039E
03BF		039F
03C0		03A0
03C1		03A1
03C2		03A3
03C3		03A3
03C4		03A4
03C5		03A5
03C6		03A6
03C7		03A7
03C8		03A8
03C9		03A9
03CA		03AA
03CB		03AB
03CC		038C
03CD		038E
03CE		038F
03D0		0392
03D1		0398
03D5		03A6
03D6		03A0
03D7		03CF
03D9		03D8
03DB		03DA
03DD		03DC
03DF		03DE
03E1		03E0
03E3		03E2
03E5		03E4
03E7		03E6
03E9		03E8
03EB		03EA
03ED		03EC
03EF		03EE
03F0		039A
03F1		03A1
03F2		03F9
03F5		0395
03F8		03F7
03FB		03FA
0430		0410
0431		0411
0432		0412
0433		0413
0434		0414
0435		0415
0436		0416
0437		0417
0438		0418
0439		0419
043A		041A
043B		041B
043C		041C
043D		041D
043E		041E
043F		041F
0440		0420
0441		0421
0442		0422
0443		0423
0444		0424
0445		0425
0446		0426
0447		0427
0448		0428
0449		0429
044A		042A
044B		042B
044C		042C
044D		042D
044E		042E
044F		042F
0450		0400
0451		0401
0452		0402
0453		0403
0454		0404
0455		0405
0456		0406
0457		0407
0458		0408
0459		0409
045A		040A
045B		040B
045C		040C
045D		040D
045E		040E
045F		040F
0461		0460
0463		0462
0465		0464
0467		0466
0469		0468
046B		046A
046D		046C
046F		046E
0471		0470
0473		0472
0475		0474
0477		0476
0479		0478
047B		047A
047D		047C
047F		047E
0481		0480
048B		048A
048D		048C
048F		048E
0491		0490
0493		0492
0495		0494
0497		0496
0499		0498
049B		049A
049D		049C
049F		049E
04A1		04A0
04A3		04A2
04A5		04A4
04A7		04A6
04A9		04A8
04AB		04AA
04AD		04AC
04AF		04AE
04B1		04B0
04B3		04B2
04B5		04B4
04B7		04B6
04B9		04B8
04BB		04BA
04BD		04BC
04BF		04BE
04C2		04C1
04C4		04C3
04C6		04C5
04C8		04C7
04CA		04C9
04CC		04CB
04CE		04CD
04CF		04C0
04D1		04D0
04D3		04D2
04D5		04D4
04D7		04D6
04D9		04D8
04DB		04DA
04DD		04DC
04DF		04DE
04E1		04E0
04E3		04E2
04E5		04E4
04E7		04E6
04E9		04E8
04EB		04EA
04ED		04EC
04EF		04EE
04F1		04F0
04F3		04F2
04F5		04F4
04F7		04F6
04F9		04F8
04FB		04FA
04FD		04FC
04FF		04FE
0501		0500
0503		0502
0505		0504
0507		0506
0509		0508
050B		050A
050D		050C
050F		050E
0511		0510
0513		0512
0515		0514
0517		0516
0519		0518
051B		051A
051D		051C
051F		051E
0521		0520
0523		0522
0525		0524
0527		0526
0561		0531
0562		0532
0563		0533
0564		0534
0565		0535
0566		0536
0567		0537
0568		0538
0569		0539
056A		053A
056B		053B
056C		053C
056D		053D
056E		053E
056F		053F
0570		0540
0571		0541
0572		0542
0573		0543
0574		0544
0575		0545
0576		0546
0577		0547
0578		0548
0579		0549
057A		054A
057B		054B
057C		054C
057D		054D
057E		054E
057F		054F
0580		0550
0581		0551
0582		0552
0583		0553
0584		0554
0585		0555
0586		0556
1D79		A77D
1D7D		2C63
1E01		1E00
1E03		1E02
1E05		1E04
1E07		1E06
1E09		1E08
1E0B		1E0A
1E0D		1E0C
1E0F		1E0E
1E11		1E10
1E13		1E12
1E15		1E14
1E17		1E16
1E19		1E18
1E1B		1E1A
1E1D		1E1C
1E1F		1E1E
1E21		1E20
1E23		1E22
1E25		1E24
1E27		1E26
1E29		1E28
1E2B		1E2A
1E2D		1E2C
1E2F		1E2E
1E31		1E30
1E33		1E32
1E35		1E34
1E37		1E36
1E39		1E38
1E3B		1E3A
1E3D		1E3C
1E3F		1E3E
1E41		1E40
1E43		1E42
1E45		1E44
1E47		1E46
1E49		1E48
1E4B		1E4A
1E4D		1E4C
1E4F		1E4E
1E51		1E50
1E53		1E52
1E55		1E54
1E57		1E56
1E59		1E58
1E5B		1E5A
1E5D		1E5C
1E5F		1E5E
1E61		1E60
1E63		1E62
1E65		1E64
1E67		1E66
1E69		1E68
1E6B		1E6A
1E6D		1E6C
1E6F		1E6E
1E71		1E70
1E73		1E72
1E75		1E74
1E77		1E76
1E79		1E78
1E7B		1E7A
1E7D		1E7C
1E7F		1E7E
1E81		1E80
1E83		1E82
1E85		1E84
1E87		1E86
1E89		1E88
1E8B		1E8A
1E8D		1E8C
1E8F		1E8E
1E91		1E90
1E93		1E92
1E95		1E94
1E9B		1E60
1EA1		1EA0
1EA3		1EA2
1EA5		1EA4
1EA7		1EA6
1EA9		1EA8
1EAB		1EAA
1EAD		1EAC
1EAF		1EAE
1EB1		1EB0
1EB3		1EB2
1EB5		1EB4
1EB7		1EB6
1EB9		1EB8
1EBB		1EBA
1EBD		1EBC
1EBF		1EBE
1EC1		1EC0
1EC3		1EC2
1EC5		1EC4
1EC7		1EC6
1EC9		1EC8
1ECB		1ECA
1ECD		1ECC
1ECF		1ECE
1ED1		1ED0
1ED3		1ED2
1ED5		1ED4
1ED7		1ED6
1ED9		1ED8
1EDB		1EDA
1EDD		1EDC
1EDF		1EDE
1EE1		1EE0
1EE3		1EE2
1EE5		1EE4
1EE7		1EE6
1EE9		1EE8
1EEB		1EEA
1EED		1EEC
1EEF		1EEE
1EF1		1EF0
1EF3		1EF2
1EF5		1EF4
1EF7		1EF6
1EF9		1EF8
1EFB		1EFA
1EFD		1EFC
1EFF		1EFE
1F00		1F08
1F01		1F09
1F02		1F0A
1F03		1F0B
1F04		1F0C
1F05		1F0D
1F06		1F0E
1F07		1F0F
1F10		1F18
1F11		1F19
1F12		1F1A
1F13		1F1B
1F14		1F1C
1F15		1F1D
1F20		1F28
1F21		1F29
1F22		1F2A
1F23		1F2B
1F24		1F2C
1F25		1F2D
1F26		1F2E
1F27		1F2F
1F30		1F38
1F31		1F39
1F32		1F3A
1F33		1F3B
1F34		1F3C
1F35		1F3D
1F36		1F3E
1F37		1F3F
1F40		1F48
1F41		1F49
1F42		1F4A
1F43		1F4B
1F44		1F4C
1F45		1F4D
1F51		1F59
1F53		1F5B
1F55		1F5D
1F57		1F5F
1F60		1F68
1F61		1F69
1F62		1F6A
1F63		1F6B
1F64		1F6C
1F65		1F6D
1F66		1F6E
1F67		1F6F
1F70		1FBA
1F71		1FBB
1F72		1FC8
1F73		1FC9
1F74		1FCA
1F75		1FCB
1F76		1FDA
1F77		1FDB
1F78		1FF8
1F79		1FF9
1F7A		1FEA
1F7B		1FEB
1F7C		1FFA
1F7D		1FFB
1F80		1F88
1F81		1F89
1F82		1F8A
1F83		1F8B
1F84		1F8C
1F85		1F8D
1F86		1F8E
1F87		1F8F
1F90		1F98
1F91		1F99
1F92		1F9A
1F93		1F9B
1F94		1F9C
1F95		1F9D
1F96		1F9E
1F97		1F9F
1FA0		1FA8
1FA1		1FA9
1FA2		1FAA
1FA3		1FAB
1FA4		1FAC
1FA5		1FAD
1FA6		1FAE
1FA7		1FAF
1FB0		1FB8
1FB1		1FB9
1FB3		1FBC
1FBE		0399
1FC3		1FCC
1FD0		1FD8
1FD1		1FD9
1FE0		1FE8
1FE1		1FE9
1FE5		1FEC
1FF3		1FFC
214E		2132
2170		2160
2171		2161
2172		2162
2173		2163
2174		2164
2175		2165
2176		2166
2177		2167
2178		2168
2179		2169
217A		216A
217B		216B
217C		216C
217D		216D
217E		216E
217F		216F
2184		2183
24D0		24B6
24D1		24B7
24D2		24B8
24D3		24B9
24D4		24BA
24D5		24BB
24D6		24BC
24D7		24BD
24D8		24BE
24D9		24BF
24DA		24C0
24DB		24C1
24DC		24C2
24DD		24C3
24DE		24C4
24DF		24C5
24E0		24C6
24E1		24C7
24E2		24C8
24E3		24C9
24E4		24CA
24E5		24CB
24E6		24CC
24E7		24CD
24E8		24CE
24E9		24CF
2C30		2C00
2C31		2C01
2C32		2C02
2C33		2C03
2C34		2C04
2C35		2C05
2C36		2C06
2C37		2C07
2C38		2C08
2C39		2C09
2C3A		2C0A
2C3B		2C0B
2C3C		2C0C
2C3D		2C0D
2C3E		2C0E
2C3F		2C0F
2C40		2C10
2C41		2C11
2C42		2C12
2C43		2C13
2C44		2C14
2C45		2C15
2C46		2C16
2C47		2C17
2C48		2C18
2C49		2C19
2C4A		2C1A
2C4B		2C1B
2C4C		2C1C
2C4D		2C1D
2C4E		2C1E
2C4F		2C1F
2C50		2C20
2C51		2C21
2C52		2C22
2C53		2C23
2C54		2C24
2C55		2C25
2C56		2C26
2C57		2C27
2C58		2C28
2C59		2C29
2C5A		2C2A
2C5B		2C2B
2C5C		2C2C
2C5D		2C2D
2C5E		2C2E
2C61		2C60
2C65		023A
2C66		023E
2C68		2C67
2C6A		2C69
2C6C		2C6B
2C73		2C72
2C76		2C75
2C81		2C80
2C83		2C82
2C85		2C84
2C87		2C86
2C89		2C88
2C8B		2C8A
2C8D		2C8C
2C8F		2C8E
2C91		2C90
2C93		2C92
2C95		2C94
2C97		2C96
2C99		2C98
2C9B		2C9A
2C9D		2C9C
2C9F		2C9E
2CA1		2CA0
2CA3		2CA2
2CA5		2CA4
2CA7		2CA6
2CA9		2CA8
2CAB		2CAA
2CAD		2CAC
2CAF		2CAE
2CB1		2CB0
2CB3		2CB2
2CB5		2CB4
2CB7		2CB6
2CB9		2CB8
2CBB		2CBA
2CBD		2CBC
2CBF		2CBE
2CC1		2CC0
2CC3		2CC2
2CC5		2CC4
2CC7		2CC6
2CC9		2CC8
2CCB		2CCA
2CCD		2CCC
2CCF		2CCE
2CD1		2CD0
2CD3		2CD2
2CD5		2CD4
2CD7		2CD6
2CD9		2CD8
2CDB		2CDA
2CDD		2CDC
2CDF		2CDE
2CE1		2CE0
2CE3		2CE2
2CEC		2CEB
2CEE		2CED
2CF3		2CF2
2D00		10A0
2D01		10A1
2D02		10A2
2D03		10A3
2D04		10A4
2D05		10A5
2D06		10A6
2D07		10A7
2D08		10A8
2D09		10A9
2D0A		10AA
2D0B		10AB
2D0C		10AC
2D0D		10AD
2D0E		10AE
2D0F		10AF
2D10		10B0
2D11		10B1
2D12		10B2
2D13		10B3
2D14		10B4
2D15		10B5
2D16		10B6
2D17		10B7
2D18		10B8
2D19		10B9
2D1A		10BA
2D1B		10BB
2D1C		10BC
2D1D		10BD
2D1E		10BE
2D1F		10BF
2D20		10C0
2D21		10C1
2D22		10C2
2D23		10C3
2D24		10C4
2D25		10C5
2D27		10C7
2D2D		10CD
A641		A640
A643		A642
A645		A644
A647		A646
A649		A648
A64B		A64A
A64D		A64C
A64F		A64E
A651		A650
A653		A652
A655		A654
A657		A656
A659		A658
A65B		A65A
A65D		A65C
A65F		A65E
A661		A660
A663		A662
A665		A664
A667		A666
A669		A668
A66B		A66A
A66D		A66C
A681		A680
A683		A682
A685		A684
A687		A686
A689		A688
A68B		A68A
A68D		A68C
A68F		A68E
A691		A690
A693		A692
A695		A694
A697		A696
A723		A722
A725		A724
A727		A726
A729		A728
A72B		A72A
A72D		A72C
A72F		A72E
A733		A732
A735		A734
A737		A736
A739		A738
A73B		A73A
A73D		A73C
A73F		A73E
A741		A740
A743		A742
A745		A744
A747		A746
A749		A748
A74B		A74A
A74D		A74C
A74F		A74E
A751		A750
A753		A752
A755		A754
A757		A756
A759		A758
A75B		A75A
A75D		A75C
A75F		A75E
A761		A760
A763		A762
A765		A764
A767		A766
A769		A768
A76B		A76A
A76D		A76C
A76F		A76E
A77A		A779
A77C		A77B
A77F		A77E
A781		A780
A783		A782
A785		A784
A787		A786
A78C		A78B
A791		A790
A793		A792
A7A1		A7A0
A7A3		A7A2
A7A5		A7A4
A7A7		A7A6
A7A9		A7A8
FF41		FF21
FF42		FF22
FF43		FF23
FF44		FF24
FF45		FF25
FF46		FF26
FF47		FF27
FF48		FF28
FF49		FF29
FF4A		FF2A
FF4B		FF2B
FF4C		FF2C
FF4D		FF2D
FF4E		FF2E
FF4F		FF2F
FF50		FF30
FF51		FF31
FF52		FF32
FF53		FF33
FF54		FF34
FF55		FF35
FF56		FF36
FF57		FF37
FF58		FF38
FF59		FF39
FF5A		FF3A
10428		10400
10429		10401
1042A		10402
1042B		10403
1042C		10404
1042D		10405
1042E		10406
1042F		10407
10430		10408
10431		10409
10432		1040A
10433		1040B
10434		1040C
10435		1040D
10436		1040E
10437		1040F
10438		10410
10439		10411
1043A		10412
1043B		10413
1043C		10414
1043D		10415
1043E		10416
1043F		10417
10440		10418
10441		10419
10442		1041A
10443		1041B
10444		1041C
10445		1041D
10446		1041E
10447		1041F
10448		10420
10449		10421
1044A		10422
1044B		10423
1044C		10424
1044D		10425
1044E		10426
1044F		10427
END
