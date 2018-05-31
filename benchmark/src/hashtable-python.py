import time

def benchmark():
    m = {}

    with open("words.txt") as f:
        for line in f:
            for w in line.split():
                if w in m:
                    m[w] += 1
                else:
                    m[w] = 1

    print(len(m))

    s = 0

    for k, v in m.items():
        s += v

    print(s)

start = time.time()

benchmark()

print(time.time() - start)
