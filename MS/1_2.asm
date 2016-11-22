sum: ds.w 1

|declare n as 5
n: dc.w #50

| D0 is our running total
| D1 is i, should start as n and count down to zero

move.w n, D1

loop: add.w D1, D0
dec D1
beq end

jmp loop

end: move.w D0, sum
