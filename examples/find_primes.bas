10  let lo = 2
20  let hi = 10001
30  let i = lo
35  let c = 0
40  if i >= hi
43    goto 155
47  end 
50  let j = 2
76  if j * 2 > i
77    goto 120
78  end
80  if i % j = 0
90    goto 130
100 end
105 let j = j + 1
110 goto 76
120 let c = c + 1
130 let i = i + 1
140 goto 40
155 print c
160 stop
run
KILL
