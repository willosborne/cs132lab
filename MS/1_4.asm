|put 1 into D0
move.b #1, D0


|start shifting left

loopleft: asl D0

|put D0 in D1
move.b D0, D1

| store 128 in D2
| move.b #%10000000, D2
| sub D2 from D1
| sub.w D1, D2
sub.w #%10000000, D1


| jiz loopleft
beq loopright
jmp loopleft

|start shifting left
loopright: asr D0
move.b D0, D1
| move.b #%10000000, D2
| sub.w D1, D2
sub.w #%00000001, D1
beq loopleft
jmp loopright