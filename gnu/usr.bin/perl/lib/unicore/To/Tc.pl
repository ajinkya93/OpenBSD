# !!!!!!!   DO NOT EDIT THIS FILE   !!!!!!!
# This file is machine-generated by lib/unicore/mktables from the Unicode
# database, Version 6.3.0.  Any changes made here will be lost!

# !!!!!!!   INTERNAL PERL USE ONLY   !!!!!!!
# This file is for internal use by core Perl only.  The format and even the
# name or existence of this file are subject to change without notice.  Don't
# use it directly.  Use Unicode::UCD to access the Unicode character data
# base.


# The mappings in the non-hash portion of this file must be modified to get the
# correct values by adding the code point ordinal number to each one that is
# numeric.

# The name this swash is to be known by, with the format of the mappings in
# the main body of the table, and what all code points missing from this file
# map to.
$utf8::SwashInfo{'ToTc'}{'format'} = 'ax'; # mapped value in hex; some entries need adjustment
$utf8::SwashInfo{'ToTc'}{'specials_name'} = 'utf8::ToSpecTc'; # Name of hash of special mappings
$utf8::SwashInfo{'ToTc'}{'missing'} = '0'; # code point maps to itself

# Some code points require special handling because their mappings are each to
# multiple code points.  These do not appear in the main body, but are defined
# in the hash below.

# Each key is the string of N bytes that together make up the UTF-8 encoding
# for the code point.  (i.e. the same as looking at the code point's UTF-8
# under "use bytes").  Each value is the UTF-8 of the translation, for speed.
%utf8::ToSpecTc = (
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
61	7A	41
B5		39C
E0	F6	C0
F8	FE	D8
FF		178
101		100
103		102
105		104
107		106
109		108
10B		10A
10D		10C
10F		10E
111		110
113		112
115		114
117		116
119		118
11B		11A
11D		11C
11F		11E
121		120
123		122
125		124
127		126
129		128
12B		12A
12D		12C
12F		12E
131		49
133		132
135		134
137		136
13A		139
13C		13B
13E		13D
140		13F
142		141
144		143
146		145
148		147
14B		14A
14D		14C
14F		14E
151		150
153		152
155		154
157		156
159		158
15B		15A
15D		15C
15F		15E
161		160
163		162
165		164
167		166
169		168
16B		16A
16D		16C
16F		16E
171		170
173		172
175		174
177		176
17A		179
17C		17B
17E		17D
17F		53
180		243
183		182
185		184
188		187
18C		18B
192		191
195		1F6
199		198
19A		23D
19E		220
1A1		1A0
1A3		1A2
1A5		1A4
1A8		1A7
1AD		1AC
1B0		1AF
1B4		1B3
1B6		1B5
1B9		1B8
1BD		1BC
1BF		1F7
1C4		1C5
1C6		1C5
1C7		1C8
1C9		1C8
1CA		1CB
1CC		1CB
1CE		1CD
1D0		1CF
1D2		1D1
1D4		1D3
1D6		1D5
1D8		1D7
1DA		1D9
1DC		1DB
1DD		18E
1DF		1DE
1E1		1E0
1E3		1E2
1E5		1E4
1E7		1E6
1E9		1E8
1EB		1EA
1ED		1EC
1EF		1EE
1F1		1F2
1F3		1F2
1F5		1F4
1F9		1F8
1FB		1FA
1FD		1FC
1FF		1FE
201		200
203		202
205		204
207		206
209		208
20B		20A
20D		20C
20F		20E
211		210
213		212
215		214
217		216
219		218
21B		21A
21D		21C
21F		21E
223		222
225		224
227		226
229		228
22B		22A
22D		22C
22F		22E
231		230
233		232
23C		23B
23F	240	2C7E
242		241
247		246
249		248
24B		24A
24D		24C
24F		24E
250		2C6F
251		2C6D
252		2C70
253		181
254		186
256	257	189
259		18F
25B		190
260		193
263		194
265		A78D
266		A7AA
268		197
269		196
26B		2C62
26F		19C
271		2C6E
272		19D
275		19F
27D		2C64
280		1A6
283		1A9
288		1AE
289		244
28A	28B	1B1
28C		245
292		1B7
345		399
371		370
373		372
377		376
37B	37D	3FD
3AC		386
3AD	3AF	388
3B1	3C1	391
3C2		3A3
3C3	3CB	3A3
3CC		38C
3CD	3CE	38E
3D0		392
3D1		398
3D5		3A6
3D6		3A0
3D7		3CF
3D9		3D8
3DB		3DA
3DD		3DC
3DF		3DE
3E1		3E0
3E3		3E2
3E5		3E4
3E7		3E6
3E9		3E8
3EB		3EA
3ED		3EC
3EF		3EE
3F0		39A
3F1		3A1
3F2		3F9
3F5		395
3F8		3F7
3FB		3FA
430	44F	410
450	45F	400
461		460
463		462
465		464
467		466
469		468
46B		46A
46D		46C
46F		46E
471		470
473		472
475		474
477		476
479		478
47B		47A
47D		47C
47F		47E
481		480
48B		48A
48D		48C
48F		48E
491		490
493		492
495		494
497		496
499		498
49B		49A
49D		49C
49F		49E
4A1		4A0
4A3		4A2
4A5		4A4
4A7		4A6
4A9		4A8
4AB		4AA
4AD		4AC
4AF		4AE
4B1		4B0
4B3		4B2
4B5		4B4
4B7		4B6
4B9		4B8
4BB		4BA
4BD		4BC
4BF		4BE
4C2		4C1
4C4		4C3
4C6		4C5
4C8		4C7
4CA		4C9
4CC		4CB
4CE		4CD
4CF		4C0
4D1		4D0
4D3		4D2
4D5		4D4
4D7		4D6
4D9		4D8
4DB		4DA
4DD		4DC
4DF		4DE
4E1		4E0
4E3		4E2
4E5		4E4
4E7		4E6
4E9		4E8
4EB		4EA
4ED		4EC
4EF		4EE
4F1		4F0
4F3		4F2
4F5		4F4
4F7		4F6
4F9		4F8
4FB		4FA
4FD		4FC
4FF		4FE
501		500
503		502
505		504
507		506
509		508
50B		50A
50D		50C
50F		50E
511		510
513		512
515		514
517		516
519		518
51B		51A
51D		51C
51F		51E
521		520
523		522
525		524
527		526
561	586	531
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
1F00	1F07	1F08
1F10	1F15	1F18
1F20	1F27	1F28
1F30	1F37	1F38
1F40	1F45	1F48
1F51		1F59
1F53		1F5B
1F55		1F5D
1F57		1F5F
1F60	1F67	1F68
1F70	1F71	1FBA
1F72	1F75	1FC8
1F76	1F77	1FDA
1F78	1F79	1FF8
1F7A	1F7B	1FEA
1F7C	1F7D	1FFA
1F80	1F87	1F88
1F90	1F97	1F98
1FA0	1FA7	1FA8
1FB0	1FB1	1FB8
1FB3		1FBC
1FBE		399
1FC3		1FCC
1FD0	1FD1	1FD8
1FE0	1FE1	1FE8
1FE5		1FEC
1FF3		1FFC
214E		2132
2170	217F	2160
2184		2183
24D0	24E9	24B6
2C30	2C5E	2C00
2C61		2C60
2C65		23A
2C66		23E
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
2D00	2D25	10A0
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
FF41	FF5A	FF21
10428	1044F	10400
END
