Magnitude: dc.b #5
Half_period: dc.b #10

move.w #12800, A0 |address, start on line 100

jsr Draw_horizontal_line

Draw_horizontal_line: move.b #0, D0 | i
|store half period
|for i = 0; i < half_period; i++
|draw pixel at address + i
  |set D3 to zero, this is i - CALLED ON FIRST LINE
  |move.b 0, D3 | i

  |temporarily store A0 in A1
  draw_pixel_h: move.w A0, A1
  |add i to A1
  add.w D0, A1
  |write pixel
  move.b #255, D1
  move.b D1, (A1)
  |i++
  inc D0
  |i to D2 for test
  move.w D0, D2
  |i - 10
  sub.w #10, D2
  |i == 10, jump to end
  beq end_h
  |jump back to start
  jmp draw_pixel_h
  |end: add 10 to A0
  end_h: add.b Half_period, A0
RTS

|Draw_rising_edge:

|RTS

|Draw_falling_edge:

|RTS