q: dc.b #40
max: dc.b #255

|D0 stores running total
|D1 used for doing the subtractions
|A0 holds address to store into

|loop: store D0 in (A0)
|increment A0
|D0 = D0 + q
|D1 = D0
|D1 = D1 - max
|bmi loop
|D0 = 0
|jmp loop

loop: move.b D0, (A0)
inc A0
add.w q, D0
move.w D0, D1
sub.w max, D1
bmi loop
move.w #0, D0
jmp loop