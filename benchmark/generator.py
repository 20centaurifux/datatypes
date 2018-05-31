import random, string, sys

WORDS=1000000

def random_word():
    return ''.join(random.choice(string.ascii_uppercase + string.digits) for _ in range(random.randint(1, 32)))

sys.stdout.write(random_word())

for _ in xrange(WORDS - 1):
    sys.stdout.write(" %s" % random_word())
