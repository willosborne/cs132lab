| declare input value
  a: dc.b #20

| declare 1 byte variable b
  b: ds.b 1

| a to D0
  move.b a, D0

| 20 to D1
  move.b #20, D1

| D1 = D1 - D0
| or, D1 = 20 - a
  sub.b D1, D0

| if it's zero (they're equal), jump to yes
  BEQ yes

| else, put 10 in D2
  move.b #10, D2

| jump to end to skip the yes case
  JMP end

| yes case, store 100 in D2
  yes: move.b #100, D2

| put D2 into B
  end: move.b D2, b