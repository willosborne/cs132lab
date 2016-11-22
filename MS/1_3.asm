array: ds.b 10
move.b #array, A0

sum: ds.b 1

|declare n as 5
n: dc.b #10

| D0 is our running total
| D1 is i, should start as n and count down to zero

move.b n, D1

| start storing values
loopstore: move.b D1, (A0)
| increment A0, next location
inc A0
| decrement D1
dec D1
| if zero, end storing
beq endstore

| loop
jmp loopstore

|reset A0 and D1
endstore: move.b #array, A0
move.b n, D1

| take each item in the array in turn and add it to the total, D0
loopsum: add.b (A0), D0
inc A0
dec D1
beq endsum

jmp loopsum

endsum: move.b D0, sum
